// ==============================
// File: include/maxcore/derived.h
// ==============================
#ifndef MAXCORE_DERIVED_H
#define MAXCORE_DERIVED_H

#include <optional>
#include "maxcore/types.h"

namespace maxcore {

// Read-only derived frame computed from (Current, Previous, Lifecycle, Params, dt).
// This layer MUST:
// - be deterministic
// - be side-effect free
// - not mutate core state
// - not store hidden persistent data
struct DerivedFrame {
    // Deltas between committed steps (current - previous)
    double d_phi;
    double d_memory;
    double d_kappa;

    // Rates per dt (finite only if dt > 0)
    double phi_rate;
    double memory_rate;
    double kappa_rate;

    // Normalized stability (0..1)
    double kappa_ratio; // kappa / kappa_max

    // Distance to collapse boundary (>=0)
    double kappa_distance; // == kappa

    // Instantaneous load & regeneration terms (from canonical model, using CURRENT snapshot)
    double load_term; // (lambda_phi*phi + lambda_m*memory)
    double regen_term; // rho*(kappa_max - kappa)

    // Lifecycle mirrors (for convenience; read-only)
    uint64_t step_counter;
    bool terminal;
    bool collapse_emitted;
};

// Returns std::nullopt if inputs are invalid or non-finite.
// This function is a pure deterministic projection.
std::optional<DerivedFrame> ComputeDerived(
    const StructuralState& current,
    const StructuralState& previous,
    const LifecycleContext& lifecycle,
    const ParameterSet& params,
    double dt
);

} // namespace maxcore

#endif // MAXCORE_DERIVED_H
