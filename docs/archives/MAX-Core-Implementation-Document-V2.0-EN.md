# MAX Core Implementation Document V2.0
## Deterministic Structural Runtime with Fundamental Operator Engine

---

# 1. Implementation Scope

## 1.1 Objective

This document defines the reference implementation
of MAX Core V2.0 as specified in:

- MAX-Core-Specification-V2.0-EN
- MAX-Core-Design-Document-V2.0-EN

The implementation provides:

- Deterministic structural evolution (Layer 1)
- Deterministic fundamental operator computation (Layer 2)
- Atomic commit model
- Collapse irreversibility
- Replay compatibility

---

# 2. Core Runtime Structure

## 2.1 Core Containers

The runtime maintains:

1) StructuralState X_current
2) StructuralState X_previous
3) LifecycleContext lifecycle
4) Immutable Config cfg

Only X_current is mutable during Step.

---

## 2.2 Initialization

InitResult MAX_Init(
    const Config* cfg,
    const StructuralState* X_initial,
    LifecycleContext* lifecycle
);

Initialization MUST:

- Validate invariants:
  Φ ≥ 0
  M ≥ 0
  κ ≥ 0
- Require κ_initial > 0 for active cycle
- Copy X_initial into X_current
- Set X_previous = X_initial
- Initialize lifecycle fields deterministically

---

# 3. StepWithPack Implementation

## 3.1 Primary Execution Function

StepResult MAX_StepWithPack(
    StructuralState* X,
    const double* Δ_input,
    double dt,
    LifecycleContext* lifecycle,
    FundamentalPack* out_pack
);

---

## 3.2 Execution Order (Strict)

1) Validate:
   - X finite
   - Δ_input finite
   - dt > 0
   - lifecycle valid

2) If lifecycle terminal and mode blocks mutation:
   - Skip mutation
   - Produce static pack
   - Return appropriate StepResult

3) Create local copy:
   StructuralState X_next = *X

4) Apply Δ update

5) Compute Φ_new
   Clamp to ≥ 0

6) Compute M_new
   Enforce M ≥ previous M

7) Compute κ_new
   Clamp to ≥ 0

8) Validate finiteness

9) Detect collapse:
   if κ_prev > 0 AND κ_new == 0:
       collapse_triggered = true

10) Commit:
    *X = X_next
    Update X_previous
    lifecycle.step_counter++

11) Compute FundamentalPack
    using committed X and X_previous snapshot

12) Return StepResult

---

## 3.3 Atomicity Rule

Mutation occurs only at commit stage.

If any failure before commit:

- Discard X_next
- Do not modify X
- Do not modify X_previous
- Return ERROR
- Do not compute operator packs

---

# 4. Operator Engine Implementation

## 4.1 Context Construction

Before operator execution:

OperatorContext ctx:
    X_current
    X_previous
    dt
    lifecycle fields
    cfg

Context is immutable during pack computation.

---

## 4.2 Pack Execution Order

Implementation MUST compute packs in order:

1) BasePack
2) FlowPack
3) SpacePack
4) TimePack
5) FieldPack
6) ObserverPack
7) CollapsePack
8) EntanglementPack (if enabled)
9) IntelligencePack (if enabled)

Each pack writes only to its own memory region.

---

## 4.3 Failure Isolation

If any pack computation produces non-finite values:

- Set pack_status = PACK_ERROR
- Continue computing remaining packs
- Do not modify structural state
- Do not modify lifecycle

---

# 5. Snapshot Management

## 5.1 Snapshot Update Rule

After successful commit:

X_previous ← old committed X
X_current  ← new committed X

Snapshot update occurs before operator execution.

---

## 5.2 First Step Handling

On first step:

- X_previous == X_current
- FlowPack derivatives MUST be zero
- has_prev flag = false

---

# 6. Deterministic Math Policy

All math MUST:

- Use IEEE-754 double
- Avoid fast-math
- Avoid platform-dependent intrinsics
- Avoid parallel reductions

If transcendental functions are required:

- Use deterministic implementation
- Or version-lock library

---

# 7. Error Handling Model

StepResult MUST include:

- event_flag
- error_code
- lifecycle_state

Operator pack errors MUST NOT modify StepResult.

---

# 8. Collapse Handling

If collapse detected:

- lifecycle.terminal = true
- event_flag = COLLAPSE
- κ stored as 0

Subsequent calls follow lifecycle mode.

Collapse MUST NOT be re-emitted.

---

# 9. Memory and Allocation Policy

Implementation MUST:

- Avoid dynamic allocation during Step
- Avoid dynamic allocation during operator computation
- Use caller-provided buffers

Heap allocation during initialization only.

---

# 10. Replay Validation

Replay MUST validate:

- StructuralState equality
- EventFlag sequence
- FundamentalPack deterministic equality

Mismatch indicates implementation non-compliance.

---

---

# 4.X TimePack Implementation (FTT-Ref-1)

## 4.X.1 Goal

TimePack MUST compute, deterministically and finite-only:

- T_field = T(X)
- gradT = ∇_X T
- HessianT (optional; config-enabled)
- C_time = 1 - T_field / T_max (optional; requires T_max)

This implementation uses the reference temporal operator **FTT-Ref-1**.
Other temporal operators MAY be added later, but the runtime MUST always
declare which operator ID is active in BasePack.

---

## 4.X.2 Reference Temporal Operator (FTT-Ref-1)

Let:

- Δ ∈ ℝ^N
- Φ ≥ 0
- M ≥ 0
- κ ≥ 0
- ||Δ|| = sqrt( Σ Δ_i^2 )

Configuration parameters (all fixed at Init):

- Δ_max > 0
- Φ_max > 0
- M_max > 0
- a ≥ 0, b ≥ 0, c ≥ 0            // sensitivity weights
- T_max > 0                      // only if C_time enabled
- eps_norm > 0                   // e.g. 1e-12

Define normalized coordinates:

- uΔ = min(||Δ|| / Δ_max, U_cap)
- uΦ = min(Φ / Φ_max, U_cap)
- uM = min(M / M_max, U_cap)

Where U_cap is a deterministic finite cap (e.g. 64) to prevent exp overflow.

Define exponent:

E = a*uΦ + b*uM + c*uΔ

Define structural time:

T_field = κ * exp(E)

Domain handling:
- If κ == 0 → T_field = 0
- If any Viability bound is violated (||Δ||>Δ_max or Φ>Φ_max or M>M_max):
  TimePack returns PACK_ERROR (the state is already non-viable for FTT).
  (NOTE: Layer 1 should prevent this if the Viability Domain is enforced there.)

---

## 4.X.3 Gradient Computation (∇_X T)

We compute partial derivatives with respect to:

X = (Δ_1..Δ_N, Φ, M, κ)

Let expE = exp(E).
Then:

T_field = κ * expE

### 4.X.3.1 ∂T/∂Φ
Since uΦ = Φ/Φ_max (clamped before E):
dE/dΦ = a / Φ_max   if Φ/Φ_max < U_cap else 0
dT/dΦ = κ * expE * dE/dΦ

### 4.X.3.2 ∂T/∂M
dE/dM = b / M_max   if M/M_max < U_cap else 0
dT/dM = κ * expE * dE/dM

### 4.X.3.3 ∂T/∂κ
dT/dκ = expE

Special case:
- If κ==0, expE still finite; dT/dκ = expE is still defined.
  (We keep it as expE; it remains telemetry-only.)

### 4.X.3.4 ∂T/∂Δ_i
Let r = ||Δ||.
uΔ = r/Δ_max (clamped before E).

For r > eps_norm and uΔ < U_cap:
dE/dΔ_i = c * (1/Δ_max) * (Δ_i / r)
dT/dΔ_i = κ * expE * dE/dΔ_i

For r <= eps_norm OR uΔ == U_cap:
dE/dΔ_i = 0
dT/dΔ_i = 0

This avoids division by zero and removes non-deterministic branching.

---

## 4.X.4 Hessian Computation (Optional)

If TimePack Hessian is enabled, we provide:

- H_ΔΔ block (NxN)
- H_ΦΦ, H_MM, H_κκ (scalars)
- Mixed terms may be omitted unless explicitly enabled
  (pack_status = PACK_UNSUPPORTED for mixed tensor blocks if not enabled)

### 4.X.4.1 Second derivatives (scalar coordinates)

If uΦ < U_cap:
H_ΦΦ = d²T/dΦ² = T_field * (a/Φ_max)²
Else:
H_ΦΦ = 0

If uM < U_cap:
H_MM = T_field * (b/M_max)²
Else:
H_MM = 0

H_κκ = 0  (since T is linear in κ for fixed E)

Mixed:
H_ΦM = T_field * (a/Φ_max)*(b/M_max)  (if enabled)
H_Φκ = expE * (a/Φ_max)               (if enabled)
H_Mκ = expE * (b/M_max)               (if enabled)

### 4.X.4.2 Δ block (H_ΔΔ)

For r = ||Δ|| > eps_norm and uΔ < U_cap:

Let q = c/Δ_max.

We have:
dT/dΔ_i = T_field * q * (Δ_i / r)

Then:
H_ij = ∂²T/(∂Δ_i∂Δ_j)
     = T_field * q² * (Δ_iΔ_j / r²)
       + T_field * q * ( (δ_ij / r) - (Δ_iΔ_j / r³) )

Where δ_ij is Kronecker delta.

For r <= eps_norm OR uΔ == U_cap:
H_ij = 0

This tensor is symmetric by construction.

---

## 4.X.5 C_time Computation (Optional)

If enabled and T_max configured:

C_time = 1 - (T_field / T_max)

Clamp policy:
- C_time is clamped to [0, 1] for reporting only.

If T_max is not configured:
- TimePack sets C_time as PACK_UNSUPPORTED.

---

## 4.X.6 Finite-Only and Determinism Rules

TimePack MUST:

- reject any NaN/Inf in inputs (X_current)
- reject any NaN/Inf in outputs
- cap exponent inputs deterministically (U_cap)
- avoid division by zero with eps_norm rule
- compute exp() only on bounded E

If any non-finite occurs:
- pack_status = PACK_ERROR
- all TimePack numeric fields set to 0
- validity flag set false

TimePack failure MUST NOT affect Step() or other packs.

---

## 4.X.7 Traceability Fields

TimePack MUST include:

- time_model_id = "FTT-Ref-1"
- flags: { hessian_enabled, mixed_terms_enabled, c_time_enabled }

This guarantees replay verification and audit clarity.

---

# 4.Y FieldPack Implementation (FFT-Ref-1)

## 4.Y.1 Goal

FieldPack MUST compute, deterministically and finite-only, the Flexion Field:

𝔽(X) = (FΔ, FΦ, FM, Fκ)

as a derived operator over the committed structural state:

X = (Δ, Φ, M, κ)

This pack is telemetry-only and MUST NOT influence Step().

This implementation uses the reference field operator **FFT-Ref-1**.
The active field model ID MUST be reported in FieldPack for replay/audit.

---

## 4.Y.2 Reference Field Operator (FFT-Ref-1)

Flexion Field Theory defines (conceptually):

FΔ = -∇Φ(Δ, M) + G(Δ)·σ + μ·M + C(κ)

FΦ = (∂Φ/∂Δ) · FΔ + η·M

FM = h(Δ, σ)

Fκ = K(Δ, Φ, M) - λ·κ

In MAX-Core V2.0, κ MUST be monotonic non-increasing within a cycle.
Therefore FFT-Ref-1 MUST satisfy:

Fκ <= 0  (for all valid states)

This is achieved by choosing a strictly dissipative K(Δ,Φ,M).

---

## 4.Y.3 Configuration Parameters (Immutable)

All parameters below MUST be provided in Config and remain constant:

General:
- eps_norm > 0               (e.g., 1e-12)
- sigma_mode ∈ { FIXED, AUTO }
- sigma_fixed ∈ { +1, -1 }   (used if FIXED)

Potential model (for ∇Φ and ∂Φ/∂Δ):
- w_phi > 0                  (quadratic potential weight)

Regime / drift terms:
- g_gain >= 0                (G(Δ) amplitude)
- mu_gain >= 0               (memory→deviation drift)
- c_kappa >= 0               (κ geometry effect strength)
- eta_gain >= 0              (memory→energy term)

Memory generation:
- h0 >= 0                    (baseline memory flow)
- h1 >= 0                    (deviation-driven memory flow)
- h2 >= 0                    (regime-driven memory flow)

Stability (κ) dissipation (MUST enforce Fκ<=0):
- k_kappa >= 0               (linear κ damping, λ)
- k_delta >= 0               (deviation dissipation)
- k_phi   >= 0               (energy dissipation)
- k_mem   >= 0               (memory dissipation)

AUTO sigma (if enabled):
- sigma_threshold > 0        (deterministic switching threshold)
- sigma_source ∈ { DDELTA_NORM_DT, DPHI_DT, DKAPPA_DT }  (choose one)

---

## 4.Y.4 Supporting Definitions

Let:

- r = ||Δ|| = sqrt( Σ_i Δ_i² )

Define a quadratic structural potential consistent with a deterministic gradient:

Φ_pot(Δ) = 0.5 * w_phi * r²

Then:

∇Φ_pot(Δ) = w_phi * Δ   (vector in ℝ^N)

Also define:

∂Φ_pot/∂Δ = w_phi * Δ   (same vector)

Note:
- This uses Δ-only potential for the field operator.
- The runtime scalar Φ stored in X is still the canonical state coordinate;
  FieldPack does not overwrite Φ.

---

## 4.Y.5 Regime Selector σ (Reporting Only)

σ ∈ {+1, -1} is computed deterministically.

### FIXED mode
σ = sigma_fixed

### AUTO mode (deterministic)
Let S be the configured source:

- If sigma_source = DKAPPA_DT:
  S = dκ/dt
- If sigma_source = DPHI_DT:
  S = dΦ/dt
- If sigma_source = DDELTA_NORM_DT:
  S = d||Δ||/dt

AUTO rule:
- if S >= sigma_threshold -> σ = +1
- else                   -> σ = -1

σ is reported in FieldPack (optional) but MUST NOT gate collapse.
Collapse gate remains κ == 0 only.

---

## 4.Y.6 Component Field Definitions (FFT-Ref-1)

### 4.Y.6.1 Deviation Field FΔ

We implement each conceptual term deterministically:

1) Energetic tension:
Tension(Δ) = -∇Φ_pot(Δ) = -w_phi * Δ

2) Regime term:
G(Δ)·σ = (g_gain * Δ) * σ

3) Memory-induced drift (scalar memory projected along Δ direction):
If r > eps_norm:
    Drift_M = (mu_gain * M) * (Δ / r)
Else:
    Drift_M = 0 vector

4) Stability geometry effect (contractive pull proportional to κ):
C(κ) = -c_kappa * κ * Δ

Combine:
FΔ = Tension(Δ) + (g_gain * σ) * Δ + Drift_M + (-c_kappa * κ) * Δ

Properties:
- Deterministic
- Finite-only (guarded by eps_norm)
- Uses only committed X and config

---

### 4.Y.6.2 Energy Field FΦ

FFT definition:
FΦ = (∂Φ/∂Δ) · FΔ + η·M

Using Φ_pot:
∂Φ_pot/∂Δ = w_phi * Δ

So dot product:
Dot = (w_phi * Δ) · FΔ

Then:
FΦ = Dot + eta_gain * M

---

### 4.Y.6.3 Memory Field FM

FFT definition:
FM = h(Δ, σ)

Reference deterministic choice:

FM = h0 + h1 * r + h2 * (σ == +1 ? 1 : 0)

This ensures:
- FM ≥ 0 if parameters ≥ 0
- monotonic memory accumulation is supported by Step rules

FM is a rate; Step integrates it into M.

---

### 4.Y.6.4 Stability Field Fκ  (MUST be non-positive)

FFT form:
Fκ = K(Δ, Φ, M) - λκ

We define strictly dissipative:

K_diss(Δ, Φ, M) = -(k_delta * r + k_phi * Φ + k_mem * M)

λ = k_kappa

So:
Fκ = -(k_delta * r + k_phi * Φ + k_mem * M) - k_kappa * κ

This guarantees:
Fκ <= 0 for all r>=0, Φ>=0, M>=0, κ>=0.

---

## 4.Y.7 Output Fields

FieldPack MUST output:

- field_model_id = "FFT-Ref-1"
- sigma (optional)
- FΔ[N]
- FΦ
- FM
- Fκ
- pack_status ∈ { PACK_OK, PACK_ERROR, PACK_UNSUPPORTED }

---

## 4.Y.8 Finite-Only Rules

FieldPack MUST:

1) Validate all inputs (Δ, Φ, M, κ) are finite.
2) Validate all outputs are finite.
3) Guard division by r using eps_norm.
4) For any NaN/Inf:
   - pack_status = PACK_ERROR
   - all numeric outputs set to 0
   - validity flag = false

FieldPack failure MUST NOT affect Step().

---

## 4.Y.9 Consistency Notes (Important)

1) FieldPack is an operator report. It does NOT define the Step equations directly.
   Step may use its own internal deterministic update functions, but the reported
   field must remain deterministic and reproducible.

2) κ monotonic non-increasing is enforced by Layer 1.
   FFT-Ref-1 additionally guarantees Fκ <= 0 to match the V2.0 ontology.

3) σ is telemetry-only. It MUST NOT introduce additional collapse gates.
   Collapse remains κ == 0 only.

---

## 4.Y.10 Traceability Flags

FieldPack MUST include flags:

- sigma_mode (FIXED/AUTO)
- uses_quadratic_potential = true
- f_kappa_dissipative = true
- memory_drift_projection = true

---

# 4.Z SpacePack + ObserverPack Implementation (FST-Ref-1 / FOT-Ref-1)

This section defines the reference implementations:

- SpacePack: **FST-Ref-1** (structural geometry + metric + path-length)
- ObserverPack: **FOT-Ref-1** (projection load + future multiplicity proxy)

All outputs are telemetry-only:
- MUST be deterministic
- MUST be finite-only
- MUST NOT mutate X
- MUST NOT influence Step()

---

# 4.Z.1 SpacePack Implementation (FST-Ref-1)

## 4.Z.1.1 Goal
SpacePack MUST compute:

- delta_norm = ||Δ||
- kappa_margin = κ
- collapse_boundary_hit = (κ == 0)

Optional (if configured):
- delta_margin = Δ_max_norm - ||Δ||
- phi_margin   = Φ_max - Φ
- mem_margin   = M_max - M

Metric-enabled (if enabled):
- dS = sqrt( g_ij(X) * v^i * v^j ) * dt
  where v = dX/dt from FlowPack

SpacePack MUST expose `space_model_id = "FST-Ref-1"`.

---

## 4.Z.1.2 Configuration Parameters (Immutable)
Geometry / limits (optional but recommended):
- Δ_max_norm > 0 (optional)
- Φ_max > 0      (optional)
- M_max > 0      (optional)

Metric model parameters:
- metric_mode ∈ { DIAGONAL }   (FST-Ref-1 uses DIAGONAL only)
- g_delta > 0
- g_phi   > 0
- g_mem   > 0
- g_kappa > 0

Numerics:
- eps_norm > 0 (e.g., 1e-12)

---

## 4.Z.1.3 Core Geometry Outputs

### 4.Z.1.3.1 Deviation Norm
r = ||Δ|| = sqrt( Σ_i Δ_i^2 )

### 4.Z.1.3.2 Viability Margin
kappa_margin = κ

### 4.Z.1.3.3 Collapse Boundary Flag
collapse_boundary_hit = (κ == 0)

---

## 4.Z.1.4 Optional Domain Margins (If Limits Configured)

If Δ_max_norm configured:
- delta_margin = Δ_max_norm - r

If Φ_max configured:
- phi_margin = Φ_max - Φ

If M_max configured:
- mem_margin = M_max - M

NOTE:
- If a limit is not configured, the corresponding margin field MUST be omitted
  or marked PACK_UNSUPPORTED at the field level (implementation choice).
- Margins are diagnostics only and MUST NOT gate collapse.

---

## 4.Z.1.5 Metric Tensor g_ij (DIAGONAL)

FST-Ref-1 defines a diagonal metric in coordinates:

X = (Δ_1..Δ_N, Φ, M, κ)

with:

g_ΔiΔi = g_delta   for i=1..N
g_ΦΦ   = g_phi
g_MM   = g_mem
g_κκ   = g_kappa

All off-diagonal terms are zero.

This yields a deterministic path-length model.

---

## 4.Z.1.6 Path-Length Increment dS (If Metric Enabled)

Let v = dX/dt from FlowPack:

- vΔ_i = dΔ_i/dt
- vΦ   = dΦ/dt
- vM   = dM/dt
- vκ   = dκ/dt

Compute squared speed under metric:

speed2 =
  g_delta * Σ_i (vΔ_i^2)
+ g_phi   * (vΦ^2)
+ g_mem   * (vM^2)
+ g_kappa * (vκ^2)

Then:

dS = sqrt(speed2) * dt

Rules:
- If FlowPack.has_prev == false → dS MUST be 0
- If terminal == true → dS MUST be 0
- If dt <= 0 → PACK_ERROR (dt should already be validated by Layer 1)
- Any non-finite intermediate → PACK_ERROR

---

## 4.Z.1.7 Finite-Only Policy
SpacePack MUST:

- validate all inputs are finite (Δ, Φ, M, κ)
- validate all outputs are finite
- reject any NaN/Inf → PACK_ERROR and zero outputs

SpacePack failure MUST NOT affect Step().

---

# 4.Z.2 ObserverPack Implementation (FOT-Ref-1)

## 4.Z.2.1 Goal
ObserverPack MUST compute:

- obs_load (projection intensity / cost proxy)
- future_multiplicity (deterministic proxy for |𝔽(X)|)
Optional:
- obs_frequency (if configured)

ObserverPack MUST expose `observer_model_id = "FOT-Ref-1"`.

Observer metrics MUST NOT:
- mutate X
- advance time
- create evolution
- reduce κ directly
(telemetry-only)

---

## 4.Z.2.2 Configuration Parameters (Immutable)

Observation base:
- obs_frequency >= 0           (optional; if not set, report UNSUPPORTED)
- obs_gain >= 0                (load gain)
- obs_kappa_weight >= 0        (viability sensitivity)
- obs_time_weight  >= 0        (time sensitivity)
- obs_space_weight >= 0        (space/flow sensitivity)

Future multiplicity proxy:
- mult_base >= 0
- mult_field_weight >= 0
- mult_kappa_weight >= 0
- mult_cap > 0                 (finite cap, e.g., 1e6)

Numerics:
- eps_norm > 0

Dependencies:
- requires TimePack.T_field if obs_time_weight > 0
- requires SpacePack.dS or FlowPack magnitude if obs_space_weight > 0
- requires FieldPack magnitude if mult_field_weight > 0

If a required dependency is not enabled, corresponding contribution MUST be 0
and a dependency flag MUST be set.

---

## 4.Z.2.3 obs_load Definition (FOT-Ref-1)

We define observation load as a deterministic scalar:

obs_load =
  obs_gain
* ( 1
  + obs_kappa_weight * (1 / max(κ, eps_norm))
  + obs_time_weight  * T_field
  + obs_space_weight * flow_mag
  )

Where:

- T_field is from TimePack (if enabled, else 0)
- flow_mag is a deterministic motion proxy:

Option A (preferred if SpacePack metric enabled):
- flow_mag = dS / dt   (if dt>0 and dS available)

Option B (fallback):
- flow_mag = sqrt( Σ_i (vΔ_i^2) )   (from FlowPack)

Terminal rule:
- if terminal==true → obs_load MUST be computed as if flow_mag=0
  (static observation cost only), i.e. do not imply ongoing time.

Finite-only:
- All divisions use max(κ, eps_norm)
- Any non-finite → PACK_ERROR

---

## 4.Z.2.4 future_multiplicity Proxy (FOT-Ref-1)

We define a deterministic proxy for admissible futures size:

future_multiplicity =
  mult_base
+ mult_field_weight * ||FΔ||
+ mult_kappa_weight * κ

Where:

||FΔ|| = sqrt( Σ_i (FΔ_i^2) ) from FieldPack.

Clamp:
- future_multiplicity = min(future_multiplicity, mult_cap)

If FieldPack is unavailable or PACK_ERROR:
- treat ||FΔ|| = 0 and set dependency flag.

This is a telemetry proxy only; it MUST NOT gate evolution.

---

## 4.Z.2.5 Output Fields

ObserverPack MUST output:

- observer_model_id = "FOT-Ref-1"
- obs_load
- future_multiplicity
Optional:
- obs_frequency (if configured)
- dependency_flags:
  - used_time_pack (bool)
  - used_space_pack (bool)
  - used_flow_fallback (bool)
  - used_field_pack (bool)

---

## 4.Z.2.6 Finite-Only and Failure Policy

ObserverPack MUST:

- validate inputs are finite
- validate outputs are finite

If any non-finite occurs:
- pack_status = PACK_ERROR
- outputs set to 0
- validity flag false

ObserverPack failure MUST NOT affect Step().

---

# 4.Z.3 Traceability Requirements

SpacePack MUST include:
- space_model_id = "FST-Ref-1"
- metric_mode = DIAGONAL
- flags: { margins_enabled, metric_enabled }

ObserverPack MUST include:
- observer_model_id = "FOT-Ref-1"
- flags: { frequency_enabled, uses_time, uses_space, uses_field }

These IDs and flags are REQUIRED for replay audit and comparison.

---

# 4.W CollapsePack + EntanglementPack + IntelligencePack Implementation
## (FCT-Ref-1 / FET-Ref-1 / FIT-Ref-1)

This section completes Implementation V2.0 operator coverage by defining:

- CollapsePack: **FCT-Ref-1** (collapse geometry around κ-boundary)
- EntanglementPack: **FET-Ref-1** (pairwise structural coupling metrics, optional)
- IntelligencePack: **FIT-Ref-1** (research-only FIT diagnostics, optional)

All packs are telemetry-only:
- MUST be deterministic
- MUST be finite-only
- MUST NOT mutate X
- MUST NOT influence Step()
- MUST NOT introduce new collapse gates (collapse gate remains κ == 0 only)

---

# 4.W.1 CollapsePack Implementation (FCT-Ref-1)

## 4.W.1.1 Goal
CollapsePack MUST report:

- collapse = (κ == 0)
- v_boundary = dκ/dt (boundary approach velocity)
- near_collapse (optional)
- proximity (optional) in [0,1]

CollapsePack MUST expose `collapse_model_id = "FCT-Ref-1"`.

---

## 4.W.1.2 Configuration Parameters (Immutable)

- kappa_warn_threshold >= 0         (optional; if not set -> UNSUPPORTED for near_collapse)
- kappa_eps > 0                     (e.g., 1e-12)
- proximity_mode ∈ { NONE, LINEAR } (optional)
- kappa_ref > 0                     (required if proximity_mode=LINEAR)

---

## 4.W.1.3 Definitions

A) Collapse gate (normative):
collapse = (κ == 0)

B) Boundary velocity (from FlowPack):
If FlowPack.has_prev == true AND terminal==false:
    v_boundary = dκ/dt
Else:
    v_boundary = 0

C) Near-collapse flag (optional):
If kappa_warn_threshold configured:
    near_collapse = (κ <= kappa_warn_threshold)
Else:
    near_collapse = UNSUPPORTED

D) Proximity index (optional, LINEAR):
If proximity_mode = LINEAR:
    proximity = 1 - min(κ / kappa_ref, 1)
Else:
    proximity = UNSUPPORTED

Notes:
- proximity is diagnostic only.
- Collapse gate remains κ==0 only.

---

## 4.W.1.4 Post-Collapse Rules
If terminal==true OR κ==0:
- collapse = true
- v_boundary MUST be 0
- near_collapse MAY be true (if threshold >= 0)
- proximity MAY be 1 (if enabled)

No metric in CollapsePack may imply continued evolution after collapse.

---

## 4.W.1.5 Finite-Only Policy
CollapsePack MUST:
- validate κ finite
- validate v_boundary finite
- validate proximity finite if enabled
Any NaN/Inf -> PACK_ERROR with numeric outputs set to 0.

---

# 4.W.2 EntanglementPack Implementation (FET-Ref-1) — OPTIONAL

## 4.W.2.1 Enable Condition
EntanglementPack is enabled only if multi-structure mode is configured AND
the caller provides a deterministic set of partner structures for evaluation.

Single-structure runtimes MUST return PACK_UNSUPPORTED.

EntanglementPack MUST expose `entanglement_model_id = "FET-Ref-1"`.

---

## 4.W.2.2 Inputs

EntanglementPack evaluates a pair:

A = (Δ_A, Φ_A, M_A, κ_A)
B = (Δ_B, Φ_B, M_B, κ_B)

and outputs pairwise metrics.

The evaluation set and ordering MUST be deterministic.

---

## 4.W.2.3 Configuration Parameters (Immutable)

Weights (all >= 0):
- w_C   (curvature / geometry compatibility weight)
- w_mu  (drift alignment weight)
- w_M   (memory overlap weight)
- w_k   (stability coherence weight)

Normalization:
- eps_norm > 0           (e.g., 1e-12)
- ES_cap in [0,1]        (typically 1)

Optional caps:
- M_ref > 0              (memory normalization reference)
- k_ref > 0              (viability normalization reference)

---

## 4.W.2.4 Component Metrics (All in [0,1])

All components are deterministic similarity measures.

### A) S_C — Structural (Δ) Compatibility
Let:
rA = ||Δ_A||, rB = ||Δ_B||
cos = (Δ_A · Δ_B) / max(rA*rB, eps_norm)

Clamp cos to [-1,1].

Define:
S_C = (cos + 1) / 2

If rA<=eps_norm OR rB<=eps_norm:
S_C = 1  (both near-zero deviation treated as compatible)

### B) S_μ — Drift Alignment
Using FieldPack (recommended) if available for each structure:

If FΔ_A and FΔ_B available:
  uA = FΔ_A / max(||FΔ_A||, eps_norm)
  uB = FΔ_B / max(||FΔ_B||, eps_norm)
  cos_mu = (uA · uB) / max(||uA||*||uB||, eps_norm)  (effectively dot(uA,uB))
  clamp to [-1,1]
  S_μ = (cos_mu + 1) / 2
Else:
  S_μ = PACK_UNSUPPORTED (component)

### C) S_M — Memory Overlap
Deterministic overlap proxy:
S_M = 1 - |M_A - M_B| / max(max(M_A, M_B), eps_norm)

Clamp to [0,1].

If both M are ~0:
S_M = 1

### D) S_κ — Stability Coherence
Deterministic coherence proxy:
S_κ = 1 - |κ_A - κ_B| / max(k_ref, max(κ_A, κ_B), eps_norm)

Clamp to [0,1].

If both κ are ~0:
S_κ = 1

---

## 4.W.2.5 Entanglement Strength ES

Define weighted mean (weights may be zero; handle deterministically):

W = w_C + w_mu + w_M + w_k

If W > 0:
ES_raw = (w_C*S_C + w_mu*S_μ + w_M*S_M + w_k*S_κ) / W
Else:
ES_raw = 0

Clamp:
ES = min(max(ES_raw, 0), ES_cap)

---

## 4.W.2.6 Output Fields
EntanglementPack outputs:

- entanglement_model_id = "FET-Ref-1"
- ES
- S_C, S_μ, S_M, S_κ  (components)
- pack_status
- component_status flags (PACK_OK / PACK_UNSUPPORTED / PACK_ERROR)

---

## 4.W.2.7 Identity Preservation Rule (Normative)
EntanglementPack MUST NOT:
- merge A and B state vectors
- write into either structure’s state
- introduce new collapse gates
It only reports coupling metrics.

---

## 4.W.2.8 Finite-Only Policy
Any NaN/Inf in inputs or outputs -> PACK_ERROR,
numeric outputs set to 0, component flags set false.

---

# 4.W.3 IntelligencePack Implementation (FIT-Ref-1) — OPTIONAL (Research Mode)

## 4.W.3.1 Enable Condition
Enabled only if `cfg->enable_intelligence_pack = true`.

IntelligencePack MUST expose `intelligence_model_id = "FIT-Ref-1"`.

---

## 4.W.3.2 Goal
IntelligencePack reports FIT-aligned diagnostics:

- delta_phi_wave (ΔΦ_wave)
- overload_flag = (|ΔΦ_wave| > κ)
Optional:
- identity_fixed_point_error
- T_int (internal time proxy)

These are telemetry-only and MUST NOT influence evolution.

---

## 4.W.3.3 Configuration Parameters (Immutable)

- wave_mode ∈ { ABS_DPHI_DT, FLOW_DOT_GRADPHI }
- eps_norm > 0
Optional:
- enable_identity_error (bool)
- enable_T_int (bool)
- T_int_mode ∈ { PROXY_FROM_M, PROXY_FROM_PACKS }
- M_ref > 0 (if T_int uses M normalization)

---

## 4.W.3.4 ΔΦ_wave Definition (FIT-Ref-1)

### Mode 1: ABS_DPHI_DT (default, simplest, deterministic)
If FlowPack.has_prev == true AND terminal==false:
  delta_phi_wave = abs(dΦ/dt)
Else:
  delta_phi_wave = 0

### Mode 2: FLOW_DOT_GRADPHI (requires Time/Field potential gradient)
We use the potential gradient from FieldPack model:
∂Φ_pot/∂Δ = w_phi * Δ

Compute:
flow_dot_grad = (dΔ/dt) · (w_phi * Δ)

delta_phi_wave = abs(flow_dot_grad)

If required inputs unavailable:
- delta_phi_wave = 0
- set component dependency flag

---

## 4.W.3.5 overload_flag (Normative)
overload_flag = (delta_phi_wave > κ)

Notes:
- This is a diagnostic overload indicator only.
- It MUST NOT trigger collapse.
- Collapse remains κ == 0 only.

If κ == 0:
- overload_flag MAY be true or false; recommended: true if delta_phi_wave>0 else false.
- In terminal state, delta_phi_wave should be 0 by policy, so overload_flag false.

---

## 4.W.3.6 identity_fixed_point_error (Optional)
If enable_identity_error:

Define a deterministic self-consistency proxy:

identity_fixed_point_error =
  | dM/dt | + | dκ/dt | + || dΔ/dt ||

(using FlowPack; if has_prev=false -> 0)

Interpretation:
- near-zero indicates locally stationary identity under flow.
- purely diagnostic.

If FlowPack unavailable:
- set to 0 and flag dependency.

---

## 4.W.3.7 T_int (Optional)
If enable_T_int:

### T_int_mode = PROXY_FROM_M
T_int = min(M / M_ref, 1)   (requires M_ref)

### T_int_mode = PROXY_FROM_PACKS
T_int = clamp01( 0.5*C_time + 0.5*proximity )
(requires TimePack.C_time and CollapsePack.proximity)

If dependencies missing:
- T_int = 0
- flag dependency

T_int is diagnostic only.

---

## 4.W.3.8 Output Fields
IntelligencePack outputs:

- intelligence_model_id = "FIT-Ref-1"
- delta_phi_wave
- overload_flag
Optional:
- identity_fixed_point_error
- T_int
- dependency_flags
- pack_status

---

## 4.W.3.9 Finite-Only Policy
Any NaN/Inf in inputs or outputs -> PACK_ERROR,
numeric outputs set to 0, flags set false.

IntelligencePack failure MUST NOT affect Step().
