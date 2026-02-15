// ==============================
// File: maxcore.h
// ==============================
#ifndef MAXCORE_MAXCORE_H
#define MAXCORE_MAXCORE_H

#include <cstddef>
#include <optional>
#include "types.h"

namespace maxcore {

class MaxCore final {
public:
    // Create() is the only construction entry point.
    // Returns std::nullopt on any validation failure.
    static std::optional<MaxCore> Create(
        const ParameterSet& params,
        size_t delta_dim,
        const StructuralState& initial_state,
        std::optional<double> delta_max = std::nullopt
    );

    // Step() is the only mutation authority.
    // Returns EventFlag::ERROR on invalid input / numerical failure (no mutation).
    // Returns EventFlag::COLLAPSE exactly once when kappa reaches 0 (commit happens).
    // Returns EventFlag::NORMAL otherwise.
    EventFlag Step(
        const double* delta_input,
        size_t delta_len,
        double dt
    );

    const StructuralState& Current() const noexcept { return current_; }
    const StructuralState& Previous() const noexcept { return previous_; }
    const LifecycleContext& Lifecycle() const noexcept { return lifecycle_; }

private:
    MaxCore(
        const ParameterSet& params,
        size_t delta_dim,
        const StructuralState& initial_state,
        std::optional<double> delta_max
    ) noexcept;

    // Persistent immutable configuration
    ParameterSet params_;
    size_t delta_dim_;
    std::optional<double> delta_max_;

    // Persistent structural state
    StructuralState current_;
    StructuralState previous_;
    LifecycleContext lifecycle_;
};

} // namespace maxcore

#endif // MAXCORE_MAXCORE_H
