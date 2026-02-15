// ==============================
// File: types.h
// ==============================
#ifndef MAXCORE_TYPES_H
#define MAXCORE_TYPES_H

#include <cstdint>

namespace maxcore {

enum class EventFlag : uint8_t {
    NORMAL = 0,
    COLLAPSE = 1,
    ERROR = 2
};

struct StructuralState {
    double phi;
    double memory;
    double kappa;
};

struct LifecycleContext {
    uint64_t step_counter;
    bool terminal;
    bool collapse_emitted;
};

struct ParameterSet {
    // Canonical coefficients (all MUST be finite and > 0)
    double alpha;       // energy injection from norm2
    double eta;         // energy decay rate
    double beta;        // memory gain from phi
    double gamma;       // memory decay rate
    double rho;         // kappa regeneration rate
    double lambda_phi;  // kappa load from phi
    double lambda_m;    // kappa load from memory
    double kappa_max;   // upper bound for kappa (MUST be finite and > 0)
};

} // namespace maxcore

#endif // MAXCORE_TYPES_H
