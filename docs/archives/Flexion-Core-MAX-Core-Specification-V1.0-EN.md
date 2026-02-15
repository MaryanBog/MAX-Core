# Flexion Core — MAX Core Specification v1.0
## Section 1. Scope, Definitions, and Normative Requirements

Status: Draft  
Applies to: MAX Core runtime implementing the immutable ontology X = (Δ, Φ, M, κ)  
Normative keywords: MUST, MUST NOT, SHOULD, MAY

---

## 1.1 Scope

This Specification defines the normative behavior of the Flexion MAX Core.

It specifies:

- the structural state space X = (Δ, Φ, M, κ)
- the step evolution interface Step(X, Δ_in, dt) → (X', event_flags)
- mandatory invariants and failure conditions
- deterministic execution requirements
- the required separation between state evolution and telemetry computation

This Specification does not define:

- UI, dashboards, or external adapters
- domain-specific semantics of observations
- data acquisition protocols
- implementation language, build system, or packaging
- performance targets (these belong to Design)

---

## 1.2 Definitions

Structural state:

- Δ (Delta): deviation vector, Δ ∈ ℝ^N, fixed N = DELTA_DIM
- Φ (Phi): structural energy, Φ ≥ 0, strictly derived from Δ
- M (Memory): irreversibility accumulator, M ≥ 0, monotonic non-decreasing
- κ (Kappa): viability resource, κ ≥ 0, monotonic non-increasing

State:

X := (Δ, Φ, M, κ)

Input:

- Δ_in: external stimulus (deviation increment or deviation replacement; defined by mode)
- dt: step duration, dt > 0

Events:

- NORMAL: regular step without lifecycle transitions
- COLLAPSE: collapse boundary reached (κ = 0)
- GENESIS: state was re-initialized by genesis procedure
- ERROR: step rejected due to invalid inputs or invariant violation

Derived objects (operator layer):

- F(X): structural flow, approximated as dX/dt or discrete equivalent
- Telemetry: any computed outputs that do not mutate X

---

## 1.3 Normative Requirements (Global)

### 1.3.1 Determinism

MAX Core MUST be deterministic.

Given identical:

- configuration
- initial state X(0)
- sequence of inputs {Δ_in(t)}
- sequence of time steps {dt}

the Core MUST produce identical:

- state trajectory {X(t)}
- event trajectory
- telemetry outputs (if enabled)

No stochastic components are allowed.

---

### 1.3.2 Finite Arithmetic Only

All numerical values entering the Core and produced by the Core MUST be finite IEEE-754 doubles.

The Core MUST reject any input containing:

- NaN
- +Infinity
- -Infinity

Upon rejection, the Core MUST:

- return ERROR event
- leave the state unmodified

---

### 1.3.3 No Hidden State

The Core MUST NOT maintain hidden mutable state affecting evolution besides X and explicit configuration.

Any cached values used for performance MUST NOT change results.

---

### 1.3.4 No State Expansion

The Core MUST NOT introduce additional fundamental state coordinates.

All additional values beyond X MUST be:

- derived operators, or
- telemetry-only diagnostics

and MUST NOT influence evolution unless explicitly defined in this Specification as part of Step.

---

### 1.3.5 Invariants

The Core MUST enforce these invariants at all times for any valid state:

- Φ = Φ(Δ) and Φ ≥ 0
- M ≥ 0 and M(t+1) ≥ M(t)
- κ ≥ 0 and κ(t+1) ≤ κ(t)
- Collapse condition: κ = 0 is the only collapse gate

If an invariant would be violated by a step, the Core MUST:

- reject the step (ERROR)
- leave state unmodified

---

### 1.3.6 Telemetry Non-Mutation

Telemetry computation MUST NOT mutate X.

Telemetry MAY be computed:

- during Step, or
- after Step

but results MUST be identical for the same final state and configuration.

---

## 1.4 Configuration Surface (Normative, Minimal)

The Core MUST expose configuration parameters required to guarantee reproducibility.

At minimum:

- DELTA_DIM (compile-time or runtime constant; if runtime, MUST remain fixed after initialization)
- limits for structural margins (e.g., Δ_max_norm, Φ_max, M_max) if used by collapse proximity operators
- regime selection (contractive / divergent / critical) if regimes are supported
- input application mode for Δ_in (see Section 2)

Configuration MUST be part of replay logs.

---

## 1.5 Conformance

An implementation conforms to this Specification if and only if:

- it satisfies all MUST and MUST NOT statements in this section
- it passes the conformance tests defined in the Auto-Test Suite for these requirements

---

## 2.1 Structural State Vector

The structural state of the Core is defined as:

X(t) = (Δ(t), Φ(t), M(t), κ(t))

Where:

- Δ(t) ∈ ℝ^N
- Φ(t) ∈ ℝ
- M(t) ∈ ℝ
- κ(t) ∈ ℝ

The dimension N = DELTA_DIM MUST be fixed for the lifetime of the Core instance.

The state X is the only mutable structural state.

No additional fundamental coordinates are permitted.

---

## 2.2 Structural Deviation (Δ)

Δ is a real-valued vector:

Δ = (Δ₁, Δ₂, ..., Δ_N)

Requirements:

- Each Δᵢ MUST be finite
- Δ MUST be stored as double precision IEEE-754
- Norm ||Δ|| MUST be well-defined

The Core MUST support at least one canonical norm:

||Δ|| = sqrt( Σ Δᵢ² )

Alternative norms MAY be supported but MUST be deterministic and documented.

---

## 2.3 Structural Energy (Φ)

Φ is strictly derived from Δ.

Normative definition:

Φ = ½ ||Δ||²

This definition is mandatory unless a future major revision redefines energy consistently.

Requirements:

- Φ MUST be non-negative
- Φ MUST NOT be independently mutated
- Φ MUST be recomputed from Δ after each state update

---

## 2.4 Structural Memory (M)

M represents accumulated irreversibility.

Requirements:

- M ≥ 0 at all times
- M(0) ≥ 0 at initialization
- M(t+1) ≥ M(t)

Normative update rule (baseline definition):

M(t+1) = M(t) + μ * ||Δ(t)||² * dt

Where:

- μ ≥ 0 is a configuration constant
- dt > 0

Alternative monotonic accumulation formulas MAY be defined in future revisions,
but MUST preserve:

- non-negativity
- monotonic non-decreasing behavior
- deterministic dependence on current state and inputs

Memory MUST NEVER decrease.

---

## 2.5 Structural Viability (κ)

κ represents remaining structural viability.

Requirements:

- κ ≥ 0 at all times
- κ(0) > 0 at genesis
- κ(t+1) ≤ κ(t)

Normative baseline update rule:

κ(t+1) = max( 0, κ(t) − ν * ||Δ(t)||² * dt )

Where:

- ν > 0 is a configuration constant
- dt > 0

κ MUST decrease or remain constant.

κ MUST NEVER increase.

Collapse condition:

Collapse occurs if and only if:

κ = 0

No secondary collapse condition is permitted.

---

## 2.6 Input Deviation (Δ_in)

At each step, the Core receives:

Δ_in(t)

The Core MUST define one of two input application modes:

### Mode A — Additive

Δ(t+1) = Δ(t) + Δ_in(t)

### Mode B — Replacement

Δ(t+1) = Δ_in(t)

The selected mode MUST be fixed at initialization and logged in configuration.

Mixed modes are NOT allowed within a single Core instance.

All Δ_in values MUST be finite.

---

## 2.7 Time Step (dt)

dt MUST satisfy:

- dt > 0
- dt is finite
- dt is provided explicitly for each step

The Core MUST NOT infer dt implicitly.

dt MUST be part of replay logs.

---

## 2.8 State Validity Conditions

A state X is valid if and only if:

- All components of Δ are finite
- Φ = ½ ||Δ||²
- M ≥ 0
- κ ≥ 0
- Monotonic constraints are satisfied relative to previous state

If a state becomes invalid:

- Step MUST abort
- ERROR event MUST be returned
- Previous valid state MUST be preserved

---

## 2.9 Genesis State

Genesis initializes X₀:

Δ(0) = Δ_genesis (finite, may be zero or non-zero)
Φ(0) = ½ ||Δ(0)||²
M(0) = M₀ ≥ 0
κ(0) = κ₀ > 0

Genesis parameters MUST be explicitly defined and logged.

Genesis MUST produce a valid state.

---

## 3.1 Overview

This section defines the formal evolution operator:

Step(X(t), Δ_in(t), dt) → (X(t+1), event_flags)

The Step function is the only mechanism allowed to mutate X.

All state evolution MUST occur inside Step.

---

## 3.2 Step Execution Order (Normative Sequence)

Each Step MUST execute the following phases in strict order:

1. Input validation
2. Δ update
3. Φ recomputation
4. Memory update
5. Viability update
6. Collapse evaluation
7. Final invariant validation
8. Event determination

The order MUST NOT be changed.

---

## 3.3 Phase 1 — Input Validation

The Core MUST validate:

- Δ_in components are finite
- dt > 0 and finite
- current state X is valid

If validation fails:

- Step MUST return ERROR
- X MUST remain unchanged

---

## 3.4 Phase 2 — Δ Update

Let Δ_prev = Δ(t)

Depending on input mode:

Mode A (Additive):
    Δ_new = Δ_prev + Δ_in

Mode B (Replacement):
    Δ_new = Δ_in

Δ_new MUST be finite.

---

## 3.5 Phase 3 — Φ Recalculation

Φ MUST be recomputed as:

Φ_new = ½ ||Δ_new||²

Φ MUST NOT be independently mutated.

---

## 3.6 Phase 4 — Memory Update

Normative baseline:

M_new = M_prev + μ * ||Δ_new||² * dt

Where μ ≥ 0 is a configuration constant.

Constraints:

- M_new ≥ M_prev
- M_new MUST be finite

If M_new < M_prev due to numeric error:

- Step MUST return ERROR
- State MUST remain unchanged

---

## 3.7 Phase 5 — Viability Update

Normative baseline:

κ_new = κ_prev − ν * ||Δ_new||² * dt

If κ_new < 0:

    κ_new = 0

Constraints:

- κ_new ≤ κ_prev
- κ_new ≥ 0

κ MUST NEVER increase.

---

## 3.8 Phase 6 — Collapse Evaluation

Collapse condition:

κ_new = 0

If κ_new = 0 and κ_prev > 0:

    event_flag MUST include COLLAPSE

No other collapse condition is permitted.

---

## 3.9 Phase 7 — Final Invariant Validation

After updates, the Core MUST verify:

- Φ_new = ½ ||Δ_new||²
- M_new ≥ 0
- κ_new ≥ 0
- M monotonicity preserved
- κ monotonicity preserved

If any invariant fails:

- Step MUST return ERROR
- State MUST revert to X_prev

---

## 3.10 Phase 8 — Event Determination

Event flags MUST be determined as follows:

If ERROR occurred:
    return ERROR

Else if collapse triggered:
    return COLLAPSE

Else:
    return NORMAL

GENESIS event is not produced by Step.
GENESIS is produced only by explicit initialization.

---

## 3.11 Structural Flow Definition

For research and telemetry purposes, the structural flow F(X) is defined as:

F(X) ≈ (X(t+1) − X(t)) / dt

This discrete flow MUST be consistent with the Step operator.

If dt is constant, flow MUST be reproducible.

Flow computation MUST NOT mutate X.

---

## 3.12 Deterministic Replay Guarantee

Given identical:

- X(0)
- configuration
- Δ_in sequence
- dt sequence

Repeated execution MUST produce identical:

- X(t)
- flow values
- collapse events

Any deviation constitutes non-conformance.

---

## 3.13 Forbidden Behaviors

The Step function MUST NOT:

- modify Δ before input phase
- alter M outside update rule
- alter κ outside update rule
- introduce random noise
- depend on wall-clock time
- read external data

All evolution MUST depend only on:

- previous X
- Δ_in
- dt
- configuration constants

---

## 4.1 Purpose

This section defines structural regimes governing the evolution law
applied during Step execution.

Regimes do not introduce new state variables.
Regimes modify how Δ influences structural flow.

Regime selection MUST be explicit and fixed at initialization
unless deterministic switching rules are defined.

---

## 4.2 Regime Model

The Core MUST support at least one regime.

If multiple regimes are supported, the active regime
MUST be part of configuration and replay logs.

Regime affects:

- interpretation of Δ dynamics
- internal flow operator
- stability classification
- telemetry outputs

Regime MUST NOT violate:

- Φ definition
- M monotonicity
- κ monotonicity
- collapse rule κ = 0

---

## 4.3 Contractive Regime

Contractive regime models structural correction toward equilibrium.

In this regime:

- Δ tends toward reduction
- Flow is inward-directed
- Stability classification favors convergence

Normative behavioral constraints:

- ||Δ(t+1)|| SHOULD NOT systematically diverge without Δ_in stimulus
- Regime MUST preserve deterministic behavior

Optional contractive modifier:

Δ_eff = (1 − λ * dt) * Δ_new

Where:

- λ ≥ 0
- configuration constant

If λ is used:

- Δ_eff MUST remain finite
- Φ MUST be recomputed from Δ_eff
- Memory and viability updates MUST use Δ_eff

Contractive modifier MUST NOT violate invariants.

---

## 4.4 Divergent Regime

Divergent regime models structural expansion away from equilibrium.

In this regime:

- Δ growth is amplified
- Flow is outward-directed
- Instability propagation is allowed

Optional divergent modifier:

Δ_eff = (1 + α * dt) * Δ_new

Where:

- α ≥ 0
- configuration constant

If α is used:

- Δ_eff MUST remain finite
- Φ MUST be recomputed from Δ_eff
- Memory and viability updates MUST use Δ_eff

Divergent amplification MUST NOT allow κ increase.

---

## 4.5 Critical Regime (Optional)

Critical regime models near-boundary structural behavior.

Characteristics:

- small Δ oscillations
- high sensitivity to Δ_in
- low κ margin

Critical regime MUST NOT introduce stochasticity.

Critical regime MUST preserve:

- monotonic memory
- monotonic viability
- collapse condition

---

## 4.6 Regime Switching

If regime switching is supported:

Switching MUST be:

- deterministic
- fully defined by X and configuration
- logged for replay

Switching MUST NOT:

- reset state silently
- modify M or κ directly
- bypass invariant checks

---

## 4.7 Regime and Flow Consistency

For any regime:

The discrete flow F(X) MUST remain:

F(X) = (X(t+1) − X(t)) / dt

Regime MUST influence evolution only through Δ transformation,
not through direct manipulation of Φ, M, or κ outside normative rules.

---

## 4.8 Regime Validity Constraints

The Core MUST ensure:

- Regime parameters are finite
- Regime parameters are non-negative where required
- Parameter changes are not allowed mid-step

Invalid regime configuration MUST produce ERROR during initialization.

---

## 5.1 Purpose

This section defines the mandatory and optional derived quantities
that the MAX Core MUST be able to compute for research and real-world
operation.

Derived operators MUST:

- be deterministic
- depend only on X, dt, configuration
- NOT mutate X
- be reproducible under replay

Telemetry is observational only.

---

## 5.2 Structural Flow Operator

The Core MUST expose structural flow:

F(X) = (X(t+1) − X(t)) / dt

Components:

- dΔ/dt
- dΦ/dt
- dM/dt
- dκ/dt

Flow MUST be consistent with Step execution.

Flow computation MUST NOT alter state.

---

## 5.3 Structural Velocity and Acceleration

The Core MUST support computation of:

Structural velocity:

vΔ = ||dΔ/dt||

Structural acceleration (discrete):

aΔ = (vΔ(t) − vΔ(t−1)) / dt

Acceleration MAY require internal caching of previous vΔ,
but caching MUST NOT influence evolution.

---

## 5.4 Structural Norm and Energy Metrics

The Core MUST expose:

- ||Δ||
- Φ
- Energy density (optional normalization)
- Margin values relative to configured limits

Margin examples:

- Δ_margin = Δ_max_norm − ||Δ||
- Φ_margin = Φ_max − Φ
- M_margin = M_max − M
- κ_margin = κ

If limits are not configured,
margin computation MAY be omitted.

---

## 5.5 Collapse Proximity Metrics

The Core MUST compute collapse proximity as a function of κ.

Baseline proximity definition:

CollapseProximity = 1 − (κ / κ_initial)

If κ_initial is defined.

Alternative deterministic definitions MAY be used,
but MUST:

- be monotonic in κ
- equal 1 when κ = 0
- equal 0 at initialization

Collapse proximity MUST NOT alter κ.

---

## 5.6 Structural Reversibility Metrics

The Core MAY compute Structural Reversibility Index (SRI).

Baseline definition:

SRI = exp( −β * M )

Where:

- β ≥ 0
- configuration constant

SRI MUST:

- decrease as M increases
- remain finite
- not affect evolution

Derived density metrics (SRD) MAY be included.

---

## 5.7 Structural Time Operator

The Core MAY define structural time functional T(X).

Baseline definition:

T = M / (M + κ + ε)

Where:

- ε > 0 small constant for stability

Requirements:

- T ∈ [0, 1)
- T increases as M increases
- T increases as κ decreases
- T MUST be deterministic

Time derivatives MAY be computed:

- dT/dt
- discrete acceleration of T

Structural time MUST NOT become an independent state coordinate.

---

## 5.8 Stability Classification

The Core MAY classify stability regimes based on:

- sign of d||Δ||/dt
- magnitude of vΔ
- κ_margin
- collapse proximity

Example classification:

- Stable: ||Δ|| decreasing and κ large
- Critical: κ small but positive
- Divergent: ||Δ|| increasing and κ decreasing rapidly
- Dead: κ = 0

Classification MUST NOT affect state.

---

## 5.9 Invariant Diagnostics

The Core MUST expose invariant checks:

- Φ consistency with Δ
- M monotonicity
- κ monotonicity
- finite-only validation
- regime consistency

Diagnostics MUST be:

- deterministic
- replay-consistent
- non-mutating

---

## 5.10 Telemetry Integrity

Telemetry MUST:

- be computed from final X(t)
- not depend on previous external context
- not depend on wall-clock time
- be reproducible under replay

If telemetry computation fails (e.g., overflow),
Core MUST return ERROR without mutating X.

---

## Section 6. Lifecycle Management and Collapse Handling

---

## 6.1 Purpose

This section defines the structural lifecycle of the Core:

- Pre-structural state
- Genesis
- Evolution
- Collapse
- Optional re-initialization

Lifecycle transitions MUST be explicit and deterministic.

---

## 6.2 Pre-Structural State

Before Genesis, the Core is in a non-evolving state.

In this state:

- X is undefined or marked invalid
- Step MUST NOT execute
- Only Genesis operation is permitted

Any attempt to Step before Genesis MUST return ERROR.

---

## 6.3 Genesis Operation

Genesis initializes a valid structural state X₀.

Genesis MUST:

- define Δ₀ (finite vector)
- compute Φ₀ = ½ ||Δ₀||²
- define M₀ ≥ 0
- define κ₀ > 0
- validate invariants

Genesis MUST:

- produce a valid state
- emit GENESIS event
- log initialization parameters

Genesis MUST NOT:

- depend on external hidden randomness
- modify configuration implicitly

---

## 6.4 Evolution Phase

During evolution:

- Step may be called repeatedly
- State evolves according to Section 3
- Telemetry may be computed at each step

Evolution continues while:

κ > 0

---

## 6.5 Collapse Condition

Collapse occurs if and only if:

κ = 0

Upon collapse:

- COLLAPSE event MUST be emitted
- State MUST remain valid
- κ MUST remain exactly 0
- M MUST remain unchanged except via Step rules

No secondary collapse gates are permitted.

---

## 6.6 Post-Collapse Behavior

After collapse:

The Core MUST support one of the following modes:

Mode A — Frozen:
- Step calls return COLLAPSE
- X remains unchanged

Mode B — Explicit Re-Genesis Required:
- Step calls return ERROR
- Only Genesis operation allowed

Mode C — Auto-Reinitialization (Optional):
- Genesis is automatically invoked with predefined parameters
- GENESIS event MUST be logged
- New cycle_id MUST increment

The selected post-collapse mode MUST be defined at initialization.

---

## 6.7 Multi-Cycle Tracking

If re-genesis is supported:

The Core MUST maintain:

- cycle_id
- step counter per cycle

cycle_id MUST increment on each Genesis after collapse.

cycle transitions MUST be deterministic.

---

## 6.8 Lifecycle Invariants

At all times:

- Collapse MUST NOT reset memory implicitly
- Collapse MUST NOT increase κ
- Genesis MUST NOT inherit hidden state
- All lifecycle transitions MUST be replay-consistent

---

## 6.9 Error Handling

If a lifecycle violation occurs:

- The Core MUST emit ERROR
- The state MUST remain unchanged
- The failure MUST be reproducible under replay

Lifecycle errors include:

- Step before Genesis
- Genesis during active evolution (if not allowed)
- Invalid collapse mode

---

## Section 7. Determinism, Numerical Stability, and Error Model

## 7.1 Purpose

This section defines the strict numerical and determinism guarantees
required for production-grade deployment of the MAX Core.

The Core is a deterministic structural runtime engine.
Numerical instability is considered a conformance failure.

---

## 7.2 Deterministic Execution Model

The Core MUST produce identical results under identical:

- configuration
- initial state
- Δ_in sequence
- dt sequence

Determinism requirements:

- No random number generation
- No use of system time
- No thread-race dependence
- No non-deterministic container iteration
- No undefined evaluation order dependencies

All arithmetic MUST be explicitly defined.

---

## 7.3 IEEE-754 Compliance

The Core MUST:

- Use IEEE-754 double precision arithmetic
- Reject NaN and Infinity inputs
- Avoid implicit float-to-double precision loss
- Avoid extended precision drift (where applicable)

All intermediate values SHOULD be stored in double precision.

---

## 7.4 Finite Arithmetic Enforcement

At each Step, the Core MUST validate:

- All Δ components finite
- Φ finite
- M finite
- κ finite
- dt finite and positive

If any value becomes non-finite:

- Step MUST abort
- ERROR event MUST be emitted
- State MUST revert to previous valid state

---

## 7.5 Overflow and Underflow Handling

The Core MUST guard against:

- overflow in norm calculation
- overflow in squared norm
- catastrophic cancellation in subtraction
- negative zero propagation (optional normalization)

Recommended mitigation:

- safe norm calculation
- pre-check for magnitude bounds
- configuration-based maximum Δ norm

If overflow is detected:

- ERROR MUST be returned
- State MUST remain unchanged

---

## 7.6 Monotonicity Enforcement

Due to floating-point rounding,
the Core MUST explicitly enforce:

M_new = max(M_new, M_prev)

κ_new = min(κ_new, κ_prev)
κ_new = max(κ_new, 0)

This enforcement MUST NOT introduce artificial increases or decreases
beyond floating-point tolerance.

A small epsilon MAY be used for monotonic comparisons,
but MUST be fixed and deterministic.

---

## 7.7 Tolerance Policy

If tolerance ε is used for comparisons:

- ε MUST be constant
- ε MUST be logged in configuration
- ε MUST be identical under replay

Tolerance MUST NOT depend on magnitude heuristics that change across runs.

---

## 7.8 Error Model

The Core MUST define a clear error model.

Error categories:

1. Input Validation Error
2. Numerical Instability Error
3. Invariant Violation Error
4. Lifecycle Error
5. Configuration Error

On ERROR:

- No mutation to X is allowed
- Event flag MUST include ERROR
- Diagnostic information MAY be provided
- Execution MAY continue or halt depending on integration mode

---

## 7.9 Strict Mode vs Production Mode

The Core MAY define two operational modes:

Strict Mode:
- Full invariant checking at every step
- Full telemetry validation
- Intended for testing and certification

Production Mode:
- Invariants checked but optimized
- Telemetry optional
- Determinism fully preserved

Both modes MUST produce identical structural evolution.

---

## 7.10 Replay Consistency Guarantee

The Core MUST support deterministic replay.

Replay requirements:

- Log configuration
- Log Genesis parameters
- Log Δ_in sequence
- Log dt sequence
- Log regime selection

Replay MUST reproduce:

- identical state trajectory
- identical collapse timing
- identical telemetry values

Replay is mandatory for certification.

---

## Section 8. Multi-Structure Mode and Coupling

---

### 8.1 Purpose

This section defines how the Core supports multiple structural instances
without expanding the ontology of a single structure.

Multi-structure mode allows the Core to process:

X₁, X₂, …, X_k

Each structure remains:

Xᵢ = (Δᵢ, Φᵢ, Mᵢ, κᵢ)

No additional coordinates are introduced.

---

### 8.2 Structural Independence Principle

By default:

Each structure evolves independently:

Step(Xᵢ, Δ_inᵢ, dt)

There is no implicit coupling.

Independent mode MUST be supported.

---

### 8.3 Deterministic Coupling Model

If coupling is enabled, it MUST be explicit and deterministic.

Coupling MUST operate only through Δ.

Allowed coupling form:

Δ_effᵢ = Δᵢ + Σ ( Cᵢⱼ · Δⱼ )

Where:

- Cᵢⱼ are fixed deterministic coefficients
- Coupling matrix C is part of configuration
- C MUST be finite and fixed for a cycle

Coupling MUST NOT:

- modify M directly
- modify κ directly
- introduce stochastic interactions
- depend on external hidden state

All effects must propagate through Δ only.

---

### 8.4 Coupling Stability Constraints

Coupling MUST preserve:

- M monotonic increase
- κ monotonic decrease
- collapse gate κ = 0

If coupling produces numerical instability:

- Step MUST abort
- ERROR returned
- No mutation applied

---

### 8.5 Multi-Structure Collapse

Collapse remains local:

Structure Xᵢ collapses if:

κᵢ = 0

Global collapse MAY be defined as:

All κᵢ = 0

But global collapse MUST NOT override local collapse logic.

---

### 8.6 Cross-Structure Telemetry

The Core MAY compute:

- aggregate energy ΣΦᵢ
- aggregate memory ΣMᵢ
- minimum κ across structures
- coupling intensity metrics

These metrics are observational only.

They MUST NOT influence evolution unless explicitly configured.

---

### 8.7 Multi-Cycle Integrity

Each structure MUST maintain:

- its own cycle_id
- its own lifecycle state

Coupled systems MUST preserve deterministic replay across all structures.

---

### 8.8 Scalability Constraints

The number of structures k MUST be fixed at initialization.

Dynamic allocation or deletion during evolution
MUST NOT occur unless explicitly defined in a future revision.

---

## Section 9. Integration Interface and API Surface

---

### 9.1 Purpose

This section defines the minimal integration boundary
between MAX Core and external systems.

The Core must remain domain-neutral.

All domain logic must remain outside the Core.

---

### 9.2 Initialization Interface

The Core MUST provide an explicit initialization procedure.

Initialization MUST define:

- DELTA_DIM
- regime selection
- μ and ν constants
- post-collapse mode
- optional coupling configuration
- tolerance ε (if used)
- Genesis parameters (Δ₀, M₀, κ₀)

Initialization MUST occur exactly once per Core instance.

Re-initialization MUST require explicit reset.

---

### 9.3 Genesis Interface

The Core MUST provide:

Genesis(config) → event_flag

Genesis MUST:

- create valid structural state
- increment cycle_id
- reset step counter
- emit GENESIS event

Genesis MUST NOT:

- depend on external hidden state
- perform implicit randomization

---

### 9.4 Step Interface

The Core MUST expose:

Step(Δ_in, dt) → (event_flag)

After Step execution, the current state MUST be queryable.

Step MUST:

- validate inputs
- update state deterministically
- enforce invariants
- return event status

Step MUST NOT:

- perform I/O
- fetch data
- allocate dynamic domain resources
- log externally unless explicitly configured

---

### 9.5 State Access Interface

The Core MUST allow read-only access to:

- Δ
- Φ
- M
- κ
- cycle_id
- step counter

State access MUST NOT permit mutation.

---

### 9.6 Telemetry Interface

The Core MUST expose a telemetry query mechanism.

Telemetry MAY include:

- flow values
- collapse proximity
- stability classification
- structural time
- diagnostic flags

Telemetry MUST:

- be computed from current state
- be reproducible
- not alter state

Telemetry computation MAY be lazy or cached,
but caching MUST NOT alter results.

---

### 9.7 Replay Interface

The Core MUST support deterministic replay.

Replay requirements:

- configuration log
- Genesis parameters
- ordered Δ_in sequence
- ordered dt sequence

Replay MUST reproduce identical:

- state evolution
- collapse timing
- telemetry

Replay MUST NOT require external system time.

---

### 9.8 Error Reporting Interface

Errors MUST be:

- explicit
- categorized
- reproducible

Error interface MUST allow:

- retrieval of error code
- retrieval of last valid state
- reset after error

Errors MUST NOT corrupt state.

---

### 9.9 Threading Model

The Core MUST define one of:

- Single-threaded deterministic execution
- Explicitly synchronized deterministic execution

Implicit parallel mutation is forbidden.

If parallel telemetry is used,
it MUST NOT affect evolution.

---

### 9.10 Integration Constraints

The Core MUST NOT:

- interpret semantic meaning of inputs
- embed business rules
- contain domain heuristics
- modify inputs implicitly
- modify configuration mid-cycle

The Core MUST act strictly as:

A deterministic structural runtime engine.

---

## Section 10. Compliance and Conformance Requirements

---

### 10.1 Conformance Definition

An implementation of MAX Core is considered conformant
if and only if it satisfies all mandatory behavioral requirements
defined in this Specification.

Conformance is determined exclusively by observable behavior,
not by internal implementation details.

---

### 10.2 Mandatory Conformance Criteria

The implementation must demonstrate:

1. Deterministic replay consistency
2. Strict invariant preservation
3. Collapse triggered only by κ = 0
4. Memory monotonic non-decreasing
5. Viability monotonic non-increasing
6. Finite arithmetic enforcement
7. No hidden state mutation
8. Telemetry non-mutation guarantee

Failure of any criterion results in non-conformance.

---

### 10.3 Deterministic Replay Test

Given:

- identical configuration
- identical Genesis parameters
- identical Δ_in sequence
- identical dt sequence

Two independent runs MUST produce identical:

- X(t) for all t
- collapse events
- telemetry outputs

Any divergence constitutes failure.

---

### 10.4 Invariant Verification

For every step:

- Φ MUST equal ½ ||Δ||²
- M(t+1) ≥ M(t)
- κ(t+1) ≤ κ(t)
- κ ≥ 0 at all times

Invariant verification MAY be runtime or test-time,
but MUST be provable.

---

### 10.5 Numerical Stability Verification

The implementation MUST demonstrate:

- safe norm computation
- overflow detection
- rejection of NaN/Infinity
- monotonic enforcement under floating-point rounding

Numerical instability under valid finite input constitutes failure.

---

### 10.6 Regime Conformance

If multiple regimes are supported:

- Regime behavior MUST be deterministic
- Regime switching MUST be reproducible
- Regime MUST NOT alter invariant logic

---

### 10.7 Multi-Structure Conformance

If multi-structure mode is enabled:

- Each structure MUST preserve independent lifecycle logic
- Coupling MUST operate only via Δ
- Local collapse MUST NOT corrupt other structures

---

### 10.8 Error Handling Conformance

Errors MUST:

- Leave state unchanged
- Be reproducible
- Not mask invariant violations

Silent correction of invalid states is forbidden.

---

## Section 11. Auto-Test Requirements

---

### 11.1 Required Test Categories

The Core MUST be validated by automated tests covering:

1. Initialization and Genesis
2. Basic Step execution
3. Collapse boundary
4. Deterministic replay
5. Numerical overflow handling
6. Invariant enforcement
7. Regime behavior
8. Multi-structure coupling (if enabled)

---

### 11.2 Minimal Deterministic Test

A fixed deterministic input sequence MUST produce
a known reference trajectory.

Reference output MUST include:

- full state trace
- event trace
- telemetry trace

---

### 11.3 Collapse Test

The test suite MUST include a scenario where:

κ reaches 0 exactly

The implementation MUST:

- trigger COLLAPSE once
- never trigger collapse prematurely
- never skip collapse

---

### 11.4 Error Injection Test

The test suite MUST include:

- NaN input
- Infinite input
- Negative dt
- Overflow-triggering Δ

All must produce ERROR without mutation.

---

### 11.5 Multi-Cycle Test

If re-genesis is supported:

- cycle_id MUST increment
- state MUST reset deterministically
- replay MUST reproduce identical cycles

---

## Section 12. Development and Certification Binding

---

### 12.1 Development Order

Development MUST follow:

1. Specification
2. Design Document
3. Implementation
4. Static Analysis
5. Integration Testing
6. Theoretical Validation
7. Compliance Matrix
8. Final Code Review
9. Certification Report

Skipping steps invalidates certification.

---

### 12.2 Certification Criteria

Certification requires:

- All mandatory tests passing
- Deterministic replay validation
- Invariant audit confirmation
- Static analysis free of undefined behavior
- Compliance matrix fully satisfied

---

### 12.3 Versioning Rules

Major version increment required if:

- State definition changes
- Collapse definition changes
- Monotonic rules change
- Determinism guarantee changes

Minor version increment allowed if:

- Telemetry expands
- Additional operators added
- Diagnostic coverage improves