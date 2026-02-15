// ==============================
// File: src/maxcore/derived.cpp
// ==============================
#include "maxcore/derived.h"

#include <algorithm>
#include <cmath>

namespace maxcore {

static inline bool is_finite(double x) noexcept {
    return std::isfinite(x) != 0;
}

std::optional<DerivedFrame> ComputeDerived(
    const StructuralState& current,
    const StructuralState& previous,
    const LifecycleContext& lifecycle,
    const ParameterSet& params,
    double dt
) {
    // Basic validation (projection MUST reject non-finite)
    if (!is_finite(current.phi) || !is_finite(current.memory) || !is_finite(current.kappa)) return std::nullopt;
    if (!is_finite(previous.phi) || !is_finite(previous.memory) || !is_finite(previous.kappa)) return std::nullopt;

    if (!is_finite(params.lambda_phi) || !is_finite(params.lambda_m) || !is_finite(params.rho) || !is_finite(params.kappa_max)) return std::nullopt;
    if (!(params.kappa_max > 0.0)) return std::nullopt;

    if (!is_finite(dt) || !(dt > 0.0)) return std::nullopt;

    DerivedFrame out{};

    // Step deltas
    out.d_phi    = current.phi    - previous.phi;
    out.d_memory = current.memory - previous.memory;
    out.d_kappa  = current.kappa  - previous.kappa;

    if (!is_finite(out.d_phi) || !is_finite(out.d_memory) || !is_finite(out.d_kappa)) return std::nullopt;

    // Rates
    out.phi_rate    = out.d_phi / dt;
    out.memory_rate = out.d_memory / dt;
    out.kappa_rate  = out.d_kappa / dt;

    if (!is_finite(out.phi_rate) || !is_finite(out.memory_rate) || !is_finite(out.kappa_rate)) return std::nullopt;

    // Normalized stability
    out.kappa_ratio = current.kappa / params.kappa_max;
    if (!is_finite(out.kappa_ratio)) return std::nullopt;
    out.kappa_ratio = std::max(0.0, std::min(1.0, out.kappa_ratio));

    // Distance to collapse boundary
    out.kappa_distance = current.kappa;
    if (!is_finite(out.kappa_distance) || out.kappa_distance < 0.0) return std::nullopt;

    // Canonical instantaneous terms (using CURRENT snapshot)
    out.load_term = (params.lambda_phi * current.phi) + (params.lambda_m * current.memory);
    out.regen_term = params.rho * (params.kappa_max - current.kappa);

    if (!is_finite(out.load_term) || !is_finite(out.regen_term)) return std::nullopt;

    // Lifecycle mirrors
    out.step_counter = lifecycle.step_counter;
    out.terminal = lifecycle.terminal;
    out.collapse_emitted = lifecycle.collapse_emitted;

    return out;
}

} // namespace maxcore
