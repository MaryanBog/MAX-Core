# Derived Parameter Map (DPM) V1.1
## Deterministic Structural Parameter Layer

## 0. Status

Version: 1.1
Scope: Deterministic derived layer above MAX-Core V2.3
Normative keywords: MUST, MUST NOT, SHALL, MAY
This document supersedes DPM V1.0.

## 1. Purpose

Derived Parameter Map defines a deterministic, side-effect-free layer that computes higher-level structural parameters from the canonical MAX-Core snapshot.

The derived layer:
- MUST NOT modify core state.
- MUST be pure and deterministic.
- MUST use only Snapshot + Config.
- MUST produce identical output for identical input.
- MUST NOT introduce new dynamics.
- MUST NOT reconstruct removed state variables.

## 2. Canonical Input Snapshot

Derived layer operates on immutable Snapshot:

Snapshot {
    Phi
    Memory
    Kappa
    Kappa_prev
    Phi_prev
    Memory_prev
    delta_norm2
    dt
    EventFlag
}

### 2.1 DeltaDigest (Optional but Canonical Extension)

Snapshot MAY include:

DeltaDigest {
    delta_l2
    delta_l1w
    delta_maxabs
    delta_dim
}

If absent, only delta_norm2 is used.
Derived layer MUST NOT reconstruct Delta.

## 3. Global Meta Rules

### 3.1 Required Model Parameters

Config MUST contain:

alpha > 0
eta > 0
gamma > 0
rho > 0
lambda_phi >= 0
lambda_m >= 0
beta >= 0
kappa_max > 0
eps > 0
kappa_terminal_eps >= 0

Additionally:
(lambda_phi + lambda_m * (beta / gamma)) > 0
dt > 0

If any condition fails, all dependent parameters MUST return:
value = null
status = INVALID_MODEL_CONFIG

### 3.2 Terminal Definition

terminal := (Kappa <= kappa_terminal_eps)

### 3.3 Division Guard Rule

If denominator <= eps:
value = null
status = DIVISION_UNSTABLE

### 3.4 clamp01

clamp01(x) = min(1, max(0, x))

### 3.5 Compute Status

Each parameter MUST provide:
status in { OK, NULL, INVALID_INPUT, DIVISION_UNSTABLE, ERROR_EVENT }

If EventFlag == ERROR:
status = ERROR_EVENT

## 4. Derived Parameters

### A. Primary Rates

DPM.A.001 dPhi_dt
(Phi - Phi_prev) / dt
DependsOn: Phi, Phi_prev, dt

DPM.A.002 dMemory_dt
(Memory - Memory_prev) / dt
DependsOn: Memory, Memory_prev, dt

DPM.A.003 dKappa_dt
(Kappa - Kappa_prev) / dt
DependsOn: Kappa, Kappa_prev, dt

### B. Load and Injection

DPM.B.001 energy_injection_rate
alpha * delta_norm2
Valid only if Core Phi update matches canonical form.

DPM.B.002 energy_dissipation_rate
eta * Phi_prev

DPM.B.003 kappa_net_rate_model
- rho * (lambda_phi * Phi_prev + lambda_m * Memory_prev)

### C. Stability and Distance

DPM.C.001 kappa_margin
Kappa / kappa_max

DPM.C.002 collapse_imminence
clamp01(1 - Kappa / kappa_max)

DPM.C.003 phi_norm_by_kappa
Phi / max(Kappa, eps)

DPM.C.004 memory_norm_by_kappa
Memory / max(Kappa, eps)

### D. Critical Surfaces

DPM.D.001 u_critical
(eta * rho / alpha) * (lambda_phi + lambda_m * (beta / gamma))

DPM.D.002 kappa_star
kappa_max - (rho / gamma) * (lambda_phi + lambda_m * (beta / gamma))

DPM.D.003 regime_id
+1 if kappa_net_rate_model > 0
-1 if kappa_net_rate_model < 0
0 if abs(kappa_net_rate_model) <= eps
DependsOn: kappa_net_rate_model

DPM.D.004 regime_confidence
abs(kappa_net_rate_model) / (abs(kappa_net_rate_model) + eps)

### E. Space Proxies

DPM.E.001 contraction_ratio
clamp01(-dKappa_dt / max(Kappa_prev, eps))

DPM.E.002 expansion_indicator
+1 if dKappa_dt > 0
-1 if dKappa_dt < 0
0 otherwise

### F. Collapse Channel Proxies

DPM.F.001 collapse_channel_phi
Alias of DPM.C.003

DPM.F.002 collapse_channel_memory
Alias of DPM.C.004

DPM.F.003 collapse_velocity
- dKappa_dt

### G. Time Proxy

DPM.G.001 time_dilation_proxy
dMemory_dt / max(memory_star / dt, eps)
Requires memory_star defined in model.

### H. Curvature Proxy

DPM.H.001 curvature_proxy
Alias of DPM.C.002

### I. Observer Proxy

DPM.I.001 observation_cost_proxy
max(0, -dKappa_dt)
This is a viability-loss proxy only.

### J. Entanglement Readiness

DPM.J.001 entanglement_readiness
clamp01(Kappa / kappa_max)

### K. Intelligence Proxy

DPM.K.001 identity_stability_proxy
clamp01(Kappa / kappa_max)

## 5. Determinism Guarantee

All parameters:
- MUST be pure functions.
- MUST be IEEE-754 stable.
- MUST not allocate memory.
- MUST not depend on global state.
- MUST not depend on history beyond Snapshot.

## 6. Output Frame

DerivedFrame {
    values: map<ID, value or null>
    status: map<ID, status>
    Meta {
        eps
        kappa_terminal_eps
        valid_snapshot
    }
}

## 7. Non-Goals

DPM V1.1 does NOT:
- Restore Delta.
- Modify dynamics.
- Replace Core.
- Introduce new invariants.
- Expand admissible futures.

It is a deterministic structural projection layer only.
