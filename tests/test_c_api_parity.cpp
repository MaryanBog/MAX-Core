// ==============================
// File: tests/test_c_api_parity.cpp
// ==============================
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>

#include "maxcore/maxcore.h"
#include "maxcore/derived.h"
#include "maxcore/c_api.h"

static int g_fail = 0;

static void expect_true(bool cond, const char* msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << "\n";
        g_fail += 1;
    }
}

static bool almost_equal(double a, double b, double rel_eps = 1e-12, double abs_eps = 1e-15) {
    const double diff = std::fabs(a - b);
    if (diff <= abs_eps) return true;
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return diff <= rel_eps * scale;
}

static void expect_near(double a, double b, const char* msg, double rel_eps = 1e-12, double abs_eps = 1e-15) {
    if (!almost_equal(a, b, rel_eps, abs_eps)) {
        std::cout << "[FAIL] " << msg << " (a=" << a << " b=" << b << ")\n";
        g_fail += 1;
    }
}

static maxcore_event to_c_event(maxcore::EventFlag ev) noexcept {
    using namespace maxcore;
    switch (ev) {
        case EventFlag::NORMAL:   return MAXCORE_EVENT_NORMAL;
        case EventFlag::COLLAPSE: return MAXCORE_EVENT_COLLAPSE;
        case EventFlag::ERROR:    return MAXCORE_EVENT_ERROR;
        default:                  return MAXCORE_EVENT_ERROR;
    }
}

static maxcore::ParameterSet make_params_cpp() {
    using namespace maxcore;
    return ParameterSet{
        1.0,    // alpha
        0.1,    // eta
        0.5,    // beta
        0.1,    // gamma
        0.05,   // rho
        0.25,   // lambda_phi
        0.25,   // lambda_m
        10.0    // kappa_max
    };
}

static maxcore_params to_params_c(const maxcore::ParameterSet& p) {
    maxcore_params cp{};
    cp.alpha = p.alpha;
    cp.eta = p.eta;
    cp.beta = p.beta;
    cp.gamma = p.gamma;
    cp.rho = p.rho;
    cp.lambda_phi = p.lambda_phi;
    cp.lambda_m = p.lambda_m;
    cp.kappa_max = p.kappa_max;
    return cp;
}

static maxcore_state to_state_c(const maxcore::StructuralState& s) {
    maxcore_state out{};
    out.phi = s.phi;
    out.memory = s.memory;
    out.kappa = s.kappa;
    return out;
}

int main() {
    using namespace maxcore;

    std::cout << "test_c_api_parity\n";

    const ParameterSet p = make_params_cpp();
    const maxcore_params cp = to_params_c(p);

    const size_t delta_dim = 2;
    const StructuralState init_cpp{0.0, 0.0, p.kappa_max};
    const maxcore_state init_c = to_state_c(init_cpp);

    // ---- Create C++ core
    auto cpp_opt = MaxCore::Create(p, delta_dim, init_cpp);
    expect_true(cpp_opt.has_value(), "C++ Create must succeed");
    if (!cpp_opt) return 1;
    MaxCore core_cpp = *cpp_opt;

    // ---- Create C API handle
    maxcore_handle* h = maxcore_create(&cp, delta_dim, &init_c, nullptr);
    expect_true(h != nullptr, "C API create must succeed");
    if (!h) return 2;

    const double dt = 0.01;
    const double delta[2] = {1.0, 2.0};

    // Run until collapse (or guard limit) and compare at each step
    for (int t = 0; t < 300; ++t) {
        const EventFlag ev_cpp = core_cpp.Step(delta, delta_dim, dt);
        const maxcore_event ev_c = maxcore_step(h, delta, delta_dim, dt);

        const maxcore_event ev_cpp_as_c = to_c_event(ev_cpp);
        expect_true(ev_c == ev_cpp_as_c, "C API event must match C++ event");
        if (ev_c != ev_cpp_as_c) {
            std::cout << "[DIFF] t=" << t << " ev_c=" << (int)ev_c << " ev_cpp=" << (int)ev_cpp_as_c << "\n";
        }

        // ---- State parity: current
        maxcore_state cur_c{};
        expect_true(maxcore_get_current(h, &cur_c) == 1, "maxcore_get_current must succeed");
        const StructuralState cur_cpp = core_cpp.Current();

        expect_near(cur_c.phi, cur_cpp.phi, "C current.phi matches");
        expect_near(cur_c.memory, cur_cpp.memory, "C current.memory matches");
        expect_near(cur_c.kappa, cur_cpp.kappa, "C current.kappa matches");

        // ---- State parity: previous
        maxcore_state prev_c{};
        expect_true(maxcore_get_previous(h, &prev_c) == 1, "maxcore_get_previous must succeed");
        const StructuralState prev_cpp = core_cpp.Previous();

        expect_near(prev_c.phi, prev_cpp.phi, "C previous.phi matches");
        expect_near(prev_c.memory, prev_cpp.memory, "C previous.memory matches");
        expect_near(prev_c.kappa, prev_cpp.kappa, "C previous.kappa matches");

        // ---- Lifecycle parity
        maxcore_lifecycle lc_c{};
        expect_true(maxcore_get_lifecycle(h, &lc_c) == 1, "maxcore_get_lifecycle must succeed");
        const LifecycleContext lc_cpp = core_cpp.Lifecycle();

        expect_true(lc_c.step_counter == lc_cpp.step_counter, "C step_counter matches");
        expect_true((lc_c.terminal != 0) == lc_cpp.terminal, "C terminal matches");
        expect_true((lc_c.collapse_emitted != 0) == lc_cpp.collapse_emitted, "C collapse_emitted matches");

        // ---- Derived parity: C API vs C++ projection
        maxcore_derived_frame df_c{};
        const int ok = maxcore_compute_derived(h, dt, &df_c);
        expect_true(ok == 1, "maxcore_compute_derived must succeed");

        auto df_cpp = ComputeDerived(cur_cpp, prev_cpp, lc_cpp, p, dt);
        expect_true(df_cpp.has_value(), "ComputeDerived(C++) must succeed");
        if (df_cpp) {
            expect_near(df_c.d_phi, df_cpp->d_phi, "derived d_phi matches");
            expect_near(df_c.d_memory, df_cpp->d_memory, "derived d_memory matches");
            expect_near(df_c.d_kappa, df_cpp->d_kappa, "derived d_kappa matches");

            expect_near(df_c.phi_rate, df_cpp->phi_rate, "derived phi_rate matches");
            expect_near(df_c.memory_rate, df_cpp->memory_rate, "derived memory_rate matches");
            expect_near(df_c.kappa_rate, df_cpp->kappa_rate, "derived kappa_rate matches");

            expect_near(df_c.kappa_ratio, df_cpp->kappa_ratio, "derived kappa_ratio matches");
            expect_near(df_c.kappa_distance, df_cpp->kappa_distance, "derived kappa_distance matches");

            expect_near(df_c.load_term, df_cpp->load_term, "derived load_term matches");
            expect_near(df_c.regen_term, df_cpp->regen_term, "derived regen_term matches");

            expect_true(df_c.step_counter == df_cpp->step_counter, "derived step_counter matches");
            expect_true((df_c.terminal != 0) == df_cpp->terminal, "derived terminal matches");
            expect_true((df_c.collapse_emitted != 0) == df_cpp->collapse_emitted, "derived collapse_emitted matches");
        }

        if (ev_cpp == EventFlag::ERROR || ev_c == MAXCORE_EVENT_ERROR) {
            std::cout << "STOP: ERROR (C last_error='" << maxcore_last_error(h) << "')\n";
            break;
        }

        if (ev_cpp == EventFlag::COLLAPSE) {
            break;
        }
    }

    maxcore_destroy(h);

    if (g_fail == 0) {
        std::cout << "[OK] test_c_api_parity\n";
        return 0;
    }

    std::cout << "[FAIL] test_c_api_parity: " << g_fail << " failures\n";
    return 2;
}
