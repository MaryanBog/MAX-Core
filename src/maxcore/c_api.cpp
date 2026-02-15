// ==============================
// File: src/maxcore/c_api.cpp
// ==============================
#include "maxcore/c_api.h"

#include "maxcore/maxcore.h"
#include "maxcore/derived.h"

#include <new>
#include <optional>
#include <string>

struct maxcore_handle {
    maxcore::ParameterSet params;
    size_t delta_dim;
    std::optional<double> delta_max;
    maxcore::MaxCore core;
    std::string last_error;

    maxcore_handle(
        const maxcore::ParameterSet& p,
        size_t dd,
        std::optional<double> dm,
        const maxcore::MaxCore& c
    )
        : params(p), delta_dim(dd), delta_max(dm), core(c), last_error() {}
};

static inline maxcore::ParameterSet to_cpp_params(const maxcore_params& p) noexcept {
    maxcore::ParameterSet out{};
    out.alpha = p.alpha;
    out.eta = p.eta;
    out.beta = p.beta;
    out.gamma = p.gamma;
    out.rho = p.rho;
    out.lambda_phi = p.lambda_phi;
    out.lambda_m = p.lambda_m;
    out.kappa_max = p.kappa_max;
    return out;
}

static inline maxcore::StructuralState to_cpp_state(const maxcore_state& s) noexcept {
    return maxcore::StructuralState{s.phi, s.memory, s.kappa};
}

static inline void from_cpp_state(const maxcore::StructuralState& s, maxcore_state& out) noexcept {
    out.phi = s.phi;
    out.memory = s.memory;
    out.kappa = s.kappa;
}

static inline void from_cpp_lifecycle(const maxcore::LifecycleContext& lc, maxcore_lifecycle& out) noexcept {
    out.step_counter = lc.step_counter;
    out.terminal = lc.terminal ? 1 : 0;
    out.collapse_emitted = lc.collapse_emitted ? 1 : 0;
}

const char* maxcore_version(void) {
    return "MAX-Core C API V2.5.0";
}

maxcore_handle* maxcore_create(
    const maxcore_params* params,
    size_t delta_dim,
    const maxcore_state* initial_state,
    const double* delta_max_opt
) {
    if (!params || !initial_state) return nullptr;

    const maxcore::ParameterSet p = to_cpp_params(*params);
    const maxcore::StructuralState init = to_cpp_state(*initial_state);

    std::optional<double> dm = std::nullopt;
    if (delta_max_opt) dm = *delta_max_opt;

    auto core_opt = maxcore::MaxCore::Create(p, delta_dim, init, dm);
    if (!core_opt) return nullptr;

    maxcore_handle* h = new (std::nothrow) maxcore_handle(p, delta_dim, dm, *core_opt);
    if (!h) return nullptr;

    h->last_error.clear();
    return h;
}

void maxcore_destroy(maxcore_handle* h) {
    delete h;
}

maxcore_event maxcore_step(
    maxcore_handle* h,
    const double* delta_input,
    size_t delta_len,
    double dt
) {
    if (!h) return MAXCORE_EVENT_ERROR;

    maxcore::EventFlag ev = h->core.Step(delta_input, delta_len, dt);

    if (ev == maxcore::EventFlag::ERROR) {
        h->last_error = "Step() returned ERROR";
        return MAXCORE_EVENT_ERROR;
    }

    h->last_error.clear();

    if (ev == maxcore::EventFlag::COLLAPSE) return MAXCORE_EVENT_COLLAPSE;
    return MAXCORE_EVENT_NORMAL;
}

int maxcore_get_current(const maxcore_handle* h, maxcore_state* out) {
    if (!h || !out) return 0;
    from_cpp_state(h->core.Current(), *out);
    return 1;
}

int maxcore_get_previous(const maxcore_handle* h, maxcore_state* out) {
    if (!h || !out) return 0;
    from_cpp_state(h->core.Previous(), *out);
    return 1;
}

int maxcore_get_lifecycle(const maxcore_handle* h, maxcore_lifecycle* out) {
    if (!h || !out) return 0;
    from_cpp_lifecycle(h->core.Lifecycle(), *out);
    return 1;
}

int maxcore_compute_derived(
    const maxcore_handle* h,
    double dt,
    maxcore_derived_frame* out
) {
    if (!h || !out) return 0;

    auto d = maxcore::ComputeDerived(
        h->core.Current(),
        h->core.Previous(),
        h->core.Lifecycle(),
        h->params,
        dt
    );

    if (!d) return 0;

    out->d_phi = d->d_phi;
    out->d_memory = d->d_memory;
    out->d_kappa = d->d_kappa;

    out->phi_rate = d->phi_rate;
    out->memory_rate = d->memory_rate;
    out->kappa_rate = d->kappa_rate;

    out->kappa_ratio = d->kappa_ratio;
    out->kappa_distance = d->kappa_distance;

    out->load_term = d->load_term;
    out->regen_term = d->regen_term;

    out->step_counter = d->step_counter;
    out->terminal = d->terminal ? 1 : 0;
    out->collapse_emitted = d->collapse_emitted ? 1 : 0;

    return 1;
}

const char* maxcore_last_error(const maxcore_handle* h) {
    if (!h) return "null handle";
    if (h->last_error.empty()) return "";
    return h->last_error.c_str();
}
