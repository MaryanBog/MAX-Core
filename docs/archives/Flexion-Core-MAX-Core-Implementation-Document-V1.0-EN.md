# Flexion Core — MAX Core Implementation Document v1.0

## Section 1. Implementation Strategy and Phasing

---

### 1.1 Implementation Philosophy

The implementation must strictly follow:

Specification → Design → Implementation

No deviation from mathematical definitions is allowed.

Implementation must prioritize:

- determinism
- invariant preservation
- numerical safety
- replay reproducibility
- auditability

---

### 1.2 Implementation Phases

Implementation must proceed in the following order:

Phase 1 — Core State Skeleton
Phase 2 — Deterministic Step Engine
Phase 3 — Numerical Guard Integration
Phase 4 — Regime Engine Integration
Phase 5 — Lifecycle Manager
Phase 6 — Telemetry Operators
Phase 7 — Multi-Structure Support
Phase 8 — Replay Integration
Phase 9 — Conformance Tests
Phase 10 — Optimization Pass (without semantic change)

No phase may be skipped.

---

### 1.3 Phase 1 — Core State Skeleton

Implement:

- State struct
- Configuration struct
- Initialization logic
- Genesis function

Requirements:

- fixed DELTA_DIM
- no dynamic resizing
- immutable configuration after Init
- Genesis validates invariants

No evolution logic implemented in this phase.

---

### 1.4 Phase 2 — Deterministic Step Engine

Implement Step pipeline exactly in this order:

1. Validate inputs
2. Compute Δ_new
3. Apply regime transform (stub initially)
4. Compute Φ_new
5. Update M
6. Update κ
7. Collapse check
8. Commit state atomically

Important:

State mutation must occur only after all checks pass.

Use temporary variables until commit.

---

### 1.5 Phase 3 — Numerical Guard Integration

Integrate:

- finite checks
- safe norm computation
- overflow detection
- monotonic enforcement
- tolerance comparisons

All numeric utilities must be pure functions.

No undefined behavior permitted.

---

### 1.6 Phase 4 — Regime Engine

Implement:

- contractive modifier
- divergent modifier
- regime selection
- regime validation at init

Regime logic must modify Δ only.

Regime must not directly alter M or κ.

---

### 1.7 Phase 5 — Lifecycle Manager

Implement:

- PRE_STRUCTURAL state
- ACTIVE state
- COLLAPSED state
- Genesis transitions
- Post-collapse behavior modes

Cycle ID must increment only on Genesis.

Step must reject execution in invalid lifecycle state.

---

### 1.8 Phase 6 — Telemetry Operators

Implement operators as pure functions:

- flow
- velocity
- acceleration
- collapse proximity
- structural time
- SRI
- invariant diagnostics

Telemetry must operate on snapshot.

No mutation allowed.

---

### 1.9 Phase 7 — Multi-Structure Mode

Implement:

- fixed-size array of Core instances
- deterministic coupling matrix
- per-structure Step
- local collapse logic
- aggregate telemetry

Coupling must modify Δ only.

---

### 1.10 Phase 8 — Replay Integration

Integrate replay support:

- config serialization
- genesis serialization
- Δ_in logging
- dt logging
- deterministic re-execution

Replay must pass bitwise comparison test.

---

### 1.11 Phase 9 — Conformance Test Implementation

Implement tests covering:

- deterministic replay
- collapse correctness
- invariant enforcement
- overflow handling
- error recovery
- regime correctness

Golden trace must be generated after passing tests.

---

### 1.12 Phase 10 — Optimization Pass

Optimization allowed only after:

- all tests pass
- replay equality verified
- invariants audited

Optimization must not:

- change arithmetic order
- remove checks
- introduce UB
- alter replay outputs

Every optimization must be validated against golden trace.

---

## Section 2. Detailed Step Execution Implementation Plan

---

### 2.1 Core Principle

Step must be implemented as a transactional operation.

No partial mutation of state is allowed.

All computations must occur in local temporary variables.
State is committed only after full validation.

---

### 2.2 Local Working Variables

Inside Step, allocate local variables:

- Δ_prev (read-only reference or const view)
- Δ_candidate (temporary buffer)
- Φ_candidate
- M_candidate
- κ_candidate
- event_flag (local)

No direct modification of state members is allowed
until final commit stage.

---

### 2.3 Step Execution Pipeline (Exact Order)

#### Stage 1 — Validate Lifecycle State

If lifecycle != ACTIVE:
    return LIFECYCLE_ERROR

#### Stage 2 — Validate Inputs

Check:

- dt > 0
- dt finite
- all Δ_in components finite

If invalid:
    return INVALID_INPUT

---

#### Stage 3 — Compute Δ_candidate

If input_mode == ADDITIVE:
    Δ_candidate[i] = Δ_prev[i] + Δ_in[i]

If input_mode == REPLACEMENT:
    Δ_candidate[i] = Δ_in[i]

Validate:

- each Δ_candidate[i] finite

If any non-finite:
    return NUMERIC_ERROR

---

#### Stage 4 — Apply Regime Transform

Call:

Δ_effective = RegimeTransform(Δ_candidate, dt)

Validate:

- all components finite

RegimeTransform must not modify state.

---

#### Stage 5 — Compute Φ_candidate

Compute squared norm safely:

norm2 = Σ (Δ_effective[i] * Δ_effective[i])

Φ_candidate = 0.5 * norm2

Validate:

- norm2 finite
- Φ_candidate finite
- Φ_candidate ≥ 0

If overflow detected:
    return NUMERIC_OVERFLOW

---

#### Stage 6 — Compute M_candidate

M_candidate = M_prev + μ * norm2 * dt

Enforce monotonicity:

If M_candidate < M_prev:
    M_candidate = M_prev

Validate finite.

If non-finite:
    return NUMERIC_OVERFLOW

---

#### Stage 7 — Compute κ_candidate

κ_candidate = κ_prev − ν * norm2 * dt

If κ_candidate < 0:
    κ_candidate = 0

Enforce monotonicity:

If κ_candidate > κ_prev:
    κ_candidate = κ_prev

Validate finite.

If non-finite:
    return NUMERIC_OVERFLOW

---

#### Stage 8 — Collapse Evaluation

If κ_candidate == 0 and κ_prev > 0:
    event_flag = COLLAPSE
Else:
    event_flag = NORMAL

---

#### Stage 9 — Final Invariant Verification

Verify:

- Φ_candidate == 0.5 * ||Δ_effective||² (recompute if necessary)
- M_candidate ≥ 0
- κ_candidate ≥ 0
- M_candidate ≥ M_prev
- κ_candidate ≤ κ_prev

If any violation:
    return INVARIANT_VIOLATION

---

#### Stage 10 — Atomic Commit

Only now update state:

Δ = Δ_effective
Φ = Φ_candidate
M = M_candidate
κ = κ_candidate
step_counter++

If event_flag == COLLAPSE:
    lifecycle = COLLAPSED

Return event_flag

---

### 2.4 Atomicity Guarantee

Under no circumstance may state fields be partially updated.

If an error occurs in any stage:

- state remains identical to pre-Step state
- step_counter does not increment
- lifecycle remains unchanged

---

### 2.5 Safe Norm Computation Strategy

To avoid overflow:

Option A — Scaling method:
- find max absolute Δ value
- scale before squaring

Option B — Kahan-style summation

Reduction order must remain fixed.

Parallel reduction forbidden unless order preserved.

---

### 2.6 Floating-Point Discipline

Compiler must be configured to:

- disable fast-math
- preserve strict FP model
- avoid reassociation

Evaluation order must remain deterministic.

---

### 2.7 Performance Constraint

The Step implementation must:

- avoid heap allocation
- avoid virtual dispatch inside loop
- avoid branch-heavy patterns in Δ loop
- maintain fixed iteration order

---

### 2.8 Test Hook

Implementation must allow test mode where:

- internal temporary values may be inspected
- invariant checks may be forced

Test mode must not alter evolution results.

---

## Section 3. RegimeTransform Detailed Implementation Plan

---

### 3.1 Purpose

RegimeTransform modifies only the effective deviation used by Step.

It must:

- be deterministic
- be pure (no mutation of state)
- produce finite output
- preserve fixed dimensionality
- not alter Φ, M, κ directly

RegimeTransform is applied after the raw Δ update and before Φ/M/κ updates.

---

### 3.2 Inputs and Outputs

Inputs:

- Δ_candidate: vector<double>[DELTA_DIM]
- dt: double
- regime configuration (immutable)

Output:

- Δ_effective: vector<double>[DELTA_DIM]

If RegimeTransform fails:

- it returns an error code
- Step aborts without mutation

---

### 3.3 Regime Enumeration

Supported regimes:

- REG_CONTRACTIVE
- REG_DIVERGENT
- REG_CRITICAL (optional)

The regime is fixed at initialization unless deterministic switching is enabled.

---

### 3.4 Contractive Regime Transform

Goal:

Reduce effective deviation magnitude over time.

Baseline contractive form:

Δ_effective[i] = s * Δ_candidate[i]

Where:

s = max(0, 1 − λ * dt)

Constraints:

- λ ≥ 0
- dt > 0
- s ∈ [0, 1]

Implementation steps:

1. Validate λ finite, dt finite
2. Compute s deterministically
3. Multiply each component in fixed order
4. Validate finiteness of each output component

Notes:

- s computation must avoid negative due to rounding
- clamp to [0, 1] deterministically

---

### 3.5 Divergent Regime Transform

Goal:

Amplify effective deviation magnitude over time.

Baseline divergent form:

Δ_effective[i] = s * Δ_candidate[i]

Where:

s = 1 + α * dt

Constraints:

- α ≥ 0
- dt > 0
- s ≥ 1

Implementation steps:

1. Validate α finite, dt finite
2. Compute s deterministically
3. Multiply each component in fixed order
4. Validate finiteness

Overflow risk:

- multiplication may overflow for large Δ_candidate or dt
- if overflow detected → error

---

### 3.6 Critical Regime Transform (Optional)

Goal:

Represent near-boundary behavior without randomness.

Critical transform must be deterministic.

Allowed patterns:

Option A — Reduced responsiveness:
s = clamp(1 − c * CollapseProximity, s_min, 1)

Option B — Bounded scaling:
Δ_effective[i] = clamp(Δ_candidate[i], -Δ_cap, +Δ_cap)

All parameters must be fixed constants from configuration.

Critical regime must never introduce stochasticity.

---

### 3.7 Deterministic Regime Switching (Optional)

If switching is enabled, implement:

Regime = SwitchRule(X_snapshot, config)

SwitchRule constraints:

- deterministic
- depends only on X and config
- produces a regime id
- is logged for replay

Recommended minimal switching rule:

If κ is below threshold → CRITICAL
Else → configured base regime

Threshold must be constant and logged.

Switching must not mutate state outside Step.

---

### 3.8 Validation and Error Policy

RegimeTransform must validate:

- all parameters finite
- dt finite and positive
- scaling factor finite

If any output component becomes non-finite:

- return NUMERIC_OVERFLOW
- do not modify caller buffers (or write into a separate output buffer)

RegimeTransform must not allocate memory.

---

### 3.9 Testing Strategy

Tests must cover:

- contractive scaling with dt variations
- divergent scaling with dt variations
- boundary cases: dt → small, λ = 0, α = 0
- finiteness validation
- overflow detection

All tests must confirm deterministic equality under replay.

---

## Section 4. Numerical Guard Utilities Implementation Plan

---

### 4.1 Purpose

Numerical Guard utilities enforce:

- finite-only arithmetic
- overflow safety
- monotonic constraints under rounding
- deterministic clamping and comparisons

All utilities must be:

- pure functions
- deterministic
- allocation-free
- portable

They must never mutate Core state.

---

### 4.2 Utility Set (Required)

The implementation must provide the following utilities:

1. IsFinite(x) -> bool
2. ValidateFiniteArray(arr, n) -> bool
3. SafeNorm2(arr, n) -> (ok, norm2)
4. Clamp(x, lo, hi) -> x_clamped
5. EnforceMonotonicNonDecreasing(prev, cand) -> value
6. EnforceMonotonicNonIncreasing(prev, cand) -> value
7. EnforceNonNegative(x) -> value
8. SafeMulAdd(a, b, c) -> (ok, result) (optional)

All outputs must be deterministic.

---

### 4.3 Finite Validation

IsFinite must:

- reject NaN
- reject ±Infinity

ValidateFiniteArray must:

- check every element in fixed order
- return false on first failure (fixed order ensures determinism)
- not rely on vectorized checks that may reorder evaluation unless order preserved

---

### 4.4 SafeNorm2 Implementation (Critical)

SafeNorm2 computes:

norm2 = Σ (x[i]²)

without overflow when possible.

Preferred method: scaling

Algorithm outline:

1. max_abs = max(|x[i]|) in fixed order
2. if max_abs == 0: return norm2 = 0
3. scaled = x[i] / max_abs
4. sum = Σ (scaled[i]²) in fixed order
5. norm2 = (max_abs²) * sum

Validation:

- max_abs finite
- sum finite
- norm2 finite

If any intermediate non-finite:

- return ok=false

Reduction order must be fixed.

No parallel reductions allowed unless order is explicitly preserved.

---

### 4.5 Deterministic Clamp

Clamp must:

- behave identically across platforms
- avoid branch ambiguity
- treat -0 consistently (optional normalization)

Clamp must be used for:

- regime scale factor bounds
- post-update κ flooring to 0
- any configured caps

---

### 4.6 Monotonic Enforcement

Monotonic enforcement must override rounding artifacts.

Functions:

EnforceMonotonicNonDecreasing(prev, cand):
- if cand < prev: return prev
- else return cand

EnforceMonotonicNonIncreasing(prev, cand):
- if cand > prev: return prev
- else return cand

These functions must use deterministic comparisons.

If tolerance ε is used:

- ε must be constant
- ε must be stored in config
- comparisons must be:
  cand + ε < prev   (for non-decreasing)
  cand - ε > prev   (for non-increasing)

No adaptive or magnitude-scaled tolerance allowed in v1.0.

---

### 4.7 Underflow and Denormal Policy

Denormals may create performance variability.

The implementation must define a policy:

Option A — Leave denormals as-is (portable, deterministic)
Option B — Flush-to-zero (only if deterministic and explicitly configured)

If flush-to-zero is enabled:

- it must be applied consistently for all builds and platforms
- it must be part of configuration and replay logs

---

### 4.8 Overflow Detection Policy

Overflow detection must be explicit.

Cases to detect:

- multiplication overflow in scaling factor application
- max_abs² overflow (max_abs too large)
- μ * norm2 * dt overflow
- ν * norm2 * dt overflow

On overflow detection:

- abort Step with NUMERIC_OVERFLOW
- do not mutate state

---

### 4.9 Deterministic Error Propagation

Utilities must not throw exceptions.

Return policy:

- bool ok
- error code enumeration (preferred)
- value output via out-parameter

Error codes must be stable.

---

### 4.10 Unit Test Requirements

Utilities must have dedicated unit tests:

- IsFinite correctness
- SafeNorm2 stability across magnitude scales
- overflow cases
- monotonic enforcement cases
- clamp edge cases

Unit tests must validate bitwise equality across repeated runs.

---

## Section 5. Lifecycle Manager Implementation Plan

---

### 5.1 Purpose

Lifecycle Manager governs:

- allowed transitions
- Genesis behavior
- collapse handling behavior
- cycle_id tracking

Lifecycle must be deterministic and explicit.

No implicit recovery or hidden resets are permitted.

---

### 5.2 Lifecycle States

The Core instance maintains a lifecycle state:

- PRE_STRUCTURAL
- ACTIVE
- COLLAPSED

Rules:

- PRE_STRUCTURAL: Step is forbidden
- ACTIVE: Step is allowed
- COLLAPSED: behavior depends on post-collapse mode

Lifecycle state must be stored as a small integer enum.

---

### 5.3 Genesis Implementation

Genesis() must:

1. Validate configuration is initialized
2. Validate genesis parameters:
   - Δ₀ finite
   - M₀ ≥ 0 and finite
   - κ₀ > 0 and finite
3. Compute Φ₀ = ½ ||Δ₀||² using SafeNorm2
4. Validate invariants
5. Commit state atomically:
   - set Δ = Δ₀
   - set Φ = Φ₀
   - set M = M₀
   - set κ = κ₀
6. Update counters:
   - cycle_id++
   - step_counter = 0
7. Set lifecycle = ACTIVE
8. Return GENESIS event

Genesis must not allocate memory.

Genesis must not depend on external time or randomness.

Genesis is the only allowed transition out of PRE_STRUCTURAL
unless a future revision defines alternative birth mechanisms.

---

### 5.4 Collapse Detection Integration

Collapse is detected inside Step when:

κ_candidate == 0 and κ_prev > 0

When collapse triggers:

- lifecycle must transition to COLLAPSED
- event COLLAPSE must be returned for that Step
- κ must remain exactly 0 after commit

No other collapse gate is permitted.

---

### 5.5 Post-Collapse Modes

The lifecycle manager must support one of:

Mode A — FROZEN
- Step calls are accepted but produce:
  - event COLLAPSE
  - no state change
  - step_counter does not increment (recommended)
- Telemetry may still be computed

Mode B — REQUIRE_REGENESIS
- Step calls return ERROR (LIFECYCLE_VIOLATION)
- Only Genesis is allowed

Mode C — AUTO_REGENESIS (optional)
- Upon collapse, the Core auto-invokes Genesis with:
  - predefined genesis parameters
  - new cycle_id
- The collapse step still returns COLLAPSE
- Next Step begins the new cycle

Selected mode is fixed at initialization.

Mode C must be explicit and logged.

---

### 5.6 Cycle and Counter Semantics

cycle_id semantics:

- cycle_id increments only on Genesis commit
- cycle_id never increments on Step unless auto-genesis commits

step_counter semantics:

- increments only on successful Step commit in ACTIVE lifecycle
- does not increment on ERROR
- behavior in COLLAPSED depends on selected mode
  (recommended: do not increment in FROZEN)

All semantics must be documented and consistent under replay.

---

### 5.7 Allowed Transition Table

Transitions:

PRE_STRUCTURAL -> ACTIVE
- allowed only via Genesis

ACTIVE -> COLLAPSED
- allowed only via collapse event (κ reaches 0)

COLLAPSED -> ACTIVE
- allowed only via Genesis (Mode B), or auto-genesis (Mode C)

COLLAPSED -> COLLAPSED
- allowed in frozen mode (Mode A)

Any other transition is invalid and must return ERROR.

---

### 5.8 Error Handling

Lifecycle errors include:

- Step called in PRE_STRUCTURAL
- Step called in COLLAPSED when mode forbids
- Genesis called while ACTIVE (if not allowed by policy)
- Genesis called without Init

On lifecycle error:

- state remains unchanged
- error code returned
- error reproducible

---

### 5.9 Tests

Lifecycle tests must cover:

- Step before Genesis -> ERROR
- Genesis produces valid X
- collapse triggers exactly once
- post-collapse mode behavior
- multi-cycle cycle_id increment correctness
- replay reproduces lifecycle transitions exactly

---

## Section 6. Telemetry Operator Implementation Plan

---

### 6.1 Purpose

This section defines how telemetry operators are implemented in code,
while preserving:

- non-mutation of state
- determinism
- snapshot consistency
- zero influence on evolution

Telemetry must operate strictly on immutable snapshots.

---

### 6.2 Snapshot Acquisition

Before telemetry computation:

- Obtain const reference to current X
- Obtain const reference to previous X (if flow required)
- Read configuration

If previous X is required (for flow or acceleration):

- Store previous X only after successful Step commit
- Do not update previous snapshot on ERROR

Snapshot must represent a fully committed state.

---

### 6.3 Flow Computation

Flow components:

dΔ/dt[i] = (Δ_current[i] − Δ_prev[i]) / dt

dΦ/dt = (Φ_current − Φ_prev) / dt  
dM/dt = (M_current − M_prev) / dt  
dκ/dt = (κ_current − κ_prev) / dt

Implementation requirements:

- Use fixed loop order
- Validate dt > 0
- Validate finite intermediate values
- No dynamic allocation

If no previous snapshot exists (first step after Genesis):

- Flow values must be defined as 0
- Or flagged as undefined via telemetry mask

---

### 6.4 Structural Velocity

vΔ = sqrt( Σ (dΔ/dt[i]²) )

Use SafeNorm2 for squared velocity.

Velocity must be finite.

If non-finite:

- telemetry error flag set
- state remains unchanged

---

### 6.5 Structural Acceleration

Acceleration requires previous velocity:

aΔ = (vΔ_current − vΔ_prev) / dt

Implementation:

- Store vΔ_prev only after telemetry calculation
- Do not allow telemetry failure to modify stored velocity
- Fixed computation order

If no previous velocity available:

- acceleration = 0
- or flagged undefined

---

### 6.6 Collapse Proximity

Baseline:

CollapseProximity = 1 − (κ_current / κ_initial)

κ_initial must be stored at Genesis.

If κ_initial == 0 (invalid case), set proximity = 1.

Clamp output deterministically to [0, 1].

---

### 6.7 Structural Time Operator

If enabled:

T = M_current / (M_current + κ_current + ε)

Requirements:

- ε constant from configuration
- Denominator validated > 0
- Clamp to [0, 1)

Optional derivative:

dT/dt = (T_current − T_prev) / dt

Store T_prev only after successful telemetry calculation.

---

### 6.8 Structural Reversibility Index (SRI)

Baseline:

SRI = exp( −β * M_current )

β must be finite constant from configuration.

Implementation:

- Validate exponent finite
- Use deterministic exp implementation
- Result must be finite

No mutation allowed.

---

### 6.9 Stability Classification

Classification logic must depend only on snapshot values.

Example rule set:

If κ == 0 → DEAD  
Else if κ < κ_threshold → CRITICAL  
Else if vΔ decreasing and κ stable → STABLE  
Else → DIVERGENT

Thresholds must be constants from configuration.

Classification must not influence evolution.

---

### 6.10 Diagnostic Flags

Telemetry must compute diagnostic booleans:

- invariant_valid
- finite_valid
- regime_active
- collapse_imminent (κ < threshold)

Diagnostics must reflect snapshot state.

Diagnostics must not mutate or correct state.

---

### 6.11 Feature Mask Implementation

Telemetry system should support feature flags:

- FLOW_ENABLED
- VELOCITY_ENABLED
- ACCELERATION_ENABLED
- TIME_ENABLED
- SRI_ENABLED
- DIAGNOSTICS_ENABLED

If feature disabled:

- value set to 0 or documented neutral
- computation skipped deterministically

Feature mask must be part of configuration.

---

### 6.12 Deterministic Storage of Previous Metrics

Previous metrics (X_prev, vΔ_prev, T_prev) must:

- update only after successful Step
- update in fixed order
- not update on ERROR

No rolling buffers beyond what is required.

---

### 6.13 Telemetry Tests

Tests must verify:

- flow equals discrete state difference
- velocity equals norm of flow
- acceleration equals discrete diff of velocity
- proximity monotonic as κ decreases
- structural time monotonic as M increases and κ decreases
- replay equality across runs

Telemetry must pass golden trace comparison.

---

## Section 7. Multi-Structure and Coupling Implementation Plan

---

### 7.1 Purpose

This section defines how multiple structural cores are coordinated
while preserving:

- strict determinism
- isolation of individual X states
- collapse independence
- replay reproducibility

Multi-structure mode must not alter single-core semantics.

---

### 7.2 Structural Topology

A Multi-Structure Coordinator manages:

- fixed number k of Core instances
- optional deterministic coupling matrix C[k][k]

Each structure maintains its own:

- X = (Δ, Φ, M, κ)
- lifecycle state
- cycle_id
- step_counter

No shared state between structures except via explicit coupling.

---

### 7.3 Initialization

At initialization:

1. k must be defined and fixed
2. Coupling matrix C must be validated:
   - dimensions k×k
   - all finite values
3. Coupling must be immutable after initialization

Each Core instance must be initialized independently.

---

### 7.4 StepAll Execution Model

Multi-step execution must follow strict ordering:

Stage 1 — Validate all inputs for all structures  
Stage 2 — Compute all Δ_candidate independently  
Stage 3 — Apply deterministic coupling  
Stage 4 — Execute individual Step pipelines  
Stage 5 — Commit all states atomically

No structure may commit state before all are validated.

---

### 7.5 Coupling Model

Coupling must operate only on Δ before Φ computation.

Baseline coupling equation:

For each structure i:

Δ_coupled_i = Δ_i + Σ ( C[i][j] * Δ_j )

Implementation rules:

- Loop over i in fixed order
- For each i, compute sum in fixed j order
- Use temporary buffers to avoid overwriting Δ_j prematurely
- Validate finiteness of intermediate sums

Coupling must be symmetric or asymmetric exactly as configured.

No dynamic modification of C allowed.

---

### 7.6 Isolation Guarantees

Even under coupling:

- M_i must be updated only by its own norm²
- κ_i must be updated only by its own norm²
- Collapse of structure i must not alter M_j or κ_j directly

Cross-effects must occur exclusively through Δ coupling.

---

### 7.7 Collapse Semantics in Multi-Structure

Local collapse:

κ_i == 0 → structure i transitions to COLLAPSED

Global collapse (optional diagnostic):

All κ_i == 0

Global collapse must not override local collapse logic.

Each structure must maintain independent lifecycle transitions.

---

### 7.8 Atomic Multi-Commit

To preserve determinism:

- Compute all candidate states first
- Validate all candidate states
- If any structure fails → abort entire multi-step
- If all valid → commit all states in fixed order

This prevents partial update inconsistencies.

---

### 7.9 Multi-Structure Replay

Replay must log:

- configuration
- coupling matrix
- genesis parameters for each structure
- ordered Δ_in lists per structure
- dt sequence

Replay must produce identical multi-trajectory evolution.

Order of execution must be identical under replay.

---

### 7.10 Performance Constraints

Multi-structure implementation must:

- avoid per-step allocations
- use contiguous storage for Δ arrays
- maintain fixed iteration order
- avoid parallel reductions unless deterministic ordering enforced

Parallel execution of independent structures is allowed only if:

- synchronization ensures fixed commit order
- no race conditions
- identical numeric ordering preserved

---

### 7.11 Multi-Structure Tests

Tests must verify:

- independent behavior when C = 0
- symmetric coupling behavior
- asymmetric coupling behavior
- local collapse independence
- deterministic replay with coupling
- atomic rollback on one-structure failure