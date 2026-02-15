// ==============================
// File: src/maxcore/maxcore.cpp
// ==============================
#include "maxcore/maxcore.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace maxcore {

static inline bool is_finite(double x) noexcept {
    return std::isfinite(x) != 0;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
#endif

static inline bool is_zero(double x) noexcept {
    return x == 0.0;
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

static inline double clamp_range(double x, double lo, double hi) noexcept {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static bool validate_params(const ParameterSet& p) noexcept {
    const double vals[] = {
        p.alpha, p.eta, p.beta, p.gamma, p.rho, p.lambda_phi, p.lambda_m, p.kappa_max
    };

    for (double v : vals) {
        if (!is_finite(v) || !(v > 0.0)) return false;
    }
    return true;
}

static bool validate_initial_state(const StructuralState& s, double kappa_max) noexcept {
    if (!is_finite(s.phi) || !is_finite(s.memory) || !is_finite(s.kappa)) return false;
    if (s.phi < 0.0) return false;
    if (s.memory < 0.0) return false;
    if (s.kappa < 0.0) return false;
    if (s.kappa > kappa_max) return false;
    return true;
}

static inline double max_rate(const ParameterSet& p) noexcept {
    double r = p.eta;
    r = std::max(r, p.gamma);
    r = std::max(r, p.rho);
    r = std::max(r, p.lambda_phi);
    r = std::max(r, p.lambda_m);
    return r;
}

MaxCore::MaxCore(
    const ParameterSet& params,
    size_t delta_dim,
    const StructuralState& initial_state,
    std::optional<double> delta_max
) noexcept
    : params_(params),
      delta_dim_(delta_dim),
      delta_max_(delta_max),
      current_(initial_state),
      previous_(initial_state),
      lifecycle_{0u, is_zero(initial_state.kappa), false} {}

std::optional<MaxCore> MaxCore::Create(
    const ParameterSet& params,
    size_t delta_dim,
    const StructuralState& initial_state,
    std::optional<double> delta_max
) {
    if (delta_dim == 0) return std::nullopt;
    if (!validate_params(params)) return std::nullopt;
    if (!validate_initial_state(initial_state, params.kappa_max)) return std::nullopt;

    if (delta_max.has_value()) {
        const double dm = *delta_max;
        if (!is_finite(dm) || !(dm > 0.0)) return std::nullopt;
    }

    return MaxCore(params, delta_dim, initial_state, delta_max);
}

EventFlag MaxCore::Step(
    const double* delta_input,
    size_t delta_len,
    double dt
) {
    // 1) Terminal short-circuit MUST execute before validation
    if (is_zero(current_.kappa)) {
        return EventFlag::NORMAL;
    }

    // 2) Input validation MUST precede computation
    if (delta_input == nullptr) return EventFlag::ERROR;
    if (delta_len != delta_dim_) return EventFlag::ERROR;
    if (!is_finite(dt) || !(dt > 0.0)) return EventFlag::ERROR;

    // 3) dt stability check MUST precede canonical updates
    const double mr = max_rate(params_);
    if (!is_finite(mr)) return EventFlag::ERROR;

    const double prod = dt * mr;
    if (!is_finite(prod)) return EventFlag::ERROR;
    if (!(prod < 1.0)) return EventFlag::ERROR;

    // 4) Candidate state MUST be created before mutation
    StructuralState next = current_;

    // 5) Delta processing (deterministic norm2)
    double norm2 = 0.0;
    for (size_t i = 0; i < delta_dim_; ++i) {
        const double v = delta_input[i];
        if (!is_finite(v)) return EventFlag::ERROR;
        const double term = v * v;
        norm2 += term;
    }
    if (!is_finite(norm2) || norm2 < 0.0) return EventFlag::ERROR;

// Optional norm guard (preserve direction by uniform scaling)
if (delta_max_.has_value()) {
    const double dm = *delta_max_;
    if (!is_finite(dm) || !(dm > 0.0)) return EventFlag::ERROR;

    const double dm2 = dm * dm;
    if (!is_finite(dm2)) return EventFlag::ERROR;

    if (norm2 > dm2) {
        // scale = dm / ||delta||, applied uniformly to all components (direction preserved)
        const double n = std::sqrt(norm2);
        if (!is_finite(n) || !(n > 0.0)) return EventFlag::ERROR;

        const double scale = dm / n;
        if (!is_finite(scale) || !(scale > 0.0)) return EventFlag::ERROR;

        // For this canonical model, only norm2 is used downstream.
        // Uniform scaling implies norm2_scaled = (scale^2) * norm2 == dm^2.
        norm2 = dm2;

        // (Optional sanity) Ensure computed equivalence holds:
        // const double norm2_scaled = (scale * scale) * norm2_original;
        // but we intentionally avoid extra ops to preserve determinism and performance.
    }
}

    // 6) Energy update (canonical)
    double phi_next = current_.phi + (params_.alpha * norm2) - (params_.eta * current_.phi * dt);
    if (!is_finite(phi_next)) return EventFlag::ERROR;
    if (phi_next < 0.0) phi_next = 0.0;

    // 7) Memory update (canonical, uses Phi_next)
    double memory_next =
        current_.memory
        + (params_.beta * phi_next * dt)
        - (params_.gamma * current_.memory * dt);
    if (!is_finite(memory_next)) return EventFlag::ERROR;
    if (memory_next < 0.0) memory_next = 0.0;

    // 8) Stability update (canonical)
    double kappa_next =
        current_.kappa
        + (params_.rho * (params_.kappa_max - current_.kappa) * dt)
        - (params_.lambda_phi * phi_next * dt)
        - (params_.lambda_m * memory_next * dt);
    if (!is_finite(kappa_next)) return EventFlag::ERROR;

    // 9) Invariants MUST be enforced before commit (clamps)
    kappa_next = clamp_range(kappa_next, 0.0, params_.kappa_max);

    next.phi = phi_next;
    next.memory = memory_next;
    next.kappa = kappa_next;

    if (!is_finite(next.phi) || !is_finite(next.memory) || !is_finite(next.kappa)) {
        return EventFlag::ERROR;
    }

    // 10) Collapse detection MUST occur before commit
    const bool collapse_now = (current_.kappa > 0.0) && is_zero(next.kappa);

    // 11) AtomicCommit (the only mutation boundary)
    previous_ = current_;
    current_ = next;

    lifecycle_.step_counter += 1u;
    lifecycle_.terminal = is_zero(current_.kappa);
    if (collapse_now) {
        lifecycle_.collapse_emitted = true;
    }

    // 12) Return EventFlag
    return collapse_now ? EventFlag::COLLAPSE : EventFlag::NORMAL;
}

} // namespace maxcore
