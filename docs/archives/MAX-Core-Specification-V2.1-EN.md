# MAX Core Specification V2.1
## Deterministic Single-Structure Structural Runtime

---

# 1. Scope and Ontological Boundary

## 1.1 Objective

MAX Core V2.1 defines a deterministic runtime engine
for the evolution of a single structural state:

X = (Δ, Φ, M, κ)

The Core:

1) Evolves X deterministically.
2) Enforces structural invariants.
3) Detects collapse exclusively via κ.
4) Computes fundamental operator packs
   strictly as read-only diagnostics.

This specification introduces strict separation
between ontological evolution (Layer 1)
and operator diagnostics (Layer 2).

---

## 1.2 Canonical Structural State

The only ontological coordinates are:

X = (Δ, Φ, M, κ)

Where:

- Δ ∈ ℝ^N   — deviation vector
- Φ ≥ 0     — structural energy
- M ≥ 0     — structural memory
- κ ≥ 0     — structural viability

No additional state coordinates are permitted.

All other quantities are derived operators.

---

## 1.3 Single-Structure Constraint

V2.1 is explicitly a single-structure runtime.

The Core:

- Maintains exactly one StructuralState per cycle.
- Does not support multi-structure evolution.
- Does not implement structural coupling.
- Does not implement entanglement.

True structural entanglement requires
a multi-structure runtime and is out of scope.

---

## 1.4 Deterministic Evolution Principle

Structural evolution is defined exclusively by:

Step(X, Δ_input, dt) → (X_new, EventFlag)

Only Step() may mutate X.

No operator pack may:

- Modify X
- Modify lifecycle
- Influence invariants
- Create collapse gates

Evolution and observation are strictly separated.

---

## 1.5 Normative Collapse Rule

Collapse occurs if and only if:

κ == 0

after invariant enforcement.

This condition is:

- Necessary
- Sufficient
- Exclusive
- Irreversible within a cycle

No other metric may trigger collapse.

---

## 1.6 Deterministic Replay Requirement

For identical:

- binary build
- configuration
- initial state
- Δ_input sequence
- dt sequence

The Core MUST produce identical:

- structural trajectories
- collapse timing
- EventFlag sequence
- operator outputs

Replay determinism applies under identical IEEE-754 environment.

---

## 1.7 Non-Scope

MAX Core V2.1 does NOT:

- Perform control
- Perform optimization
- Select futures
- Expand admissible futures
- Override collapse
- Interpret structure
- Implement multi-structure coupling

It is a deterministic structural runtime only.

---

# 2. Structural Evolution (Layer 1 — Normative)

## 2.1 Step Interface

Structural evolution is defined exclusively by:

Step(X, Δ_input, dt) → (X_new, EventFlag)

Where:

- X = (Δ, Φ, M, κ)
- Δ_input ∈ ℝ^N
- dt > 0
- EventFlag ∈ { NORMAL, COLLAPSE, ERROR }

Only Step() may mutate X.

---

## 2.2 Terminal Condition

If κ == 0 at Step entry:

- Step() MUST NOT mutate X.
- EventFlag MUST be NORMAL.
- No further structural evolution is permitted.

Terminal state is final.

No resurrection of κ is allowed.

The only way to begin a new structural cycle
is explicit reinitialization via MAX_Init.

---

## 2.3 Deterministic Update Order

For κ > 0, Step MUST execute strictly in this order:

1) Δ update:
   Δ_new = Δ_prev + Δ_input

2) Φ computation:
   Φ_new = Φ(Δ_new)
   Φ_new = max(Φ_new, 0)

3) M update:
   M_candidate = M_prev + MemoryIncrement(Δ_new, Φ_new, dt)
   M_new = max(M_candidate, M_prev)

4) κ update:
   κ_candidate = κ_prev - KappaDecay(Δ_new, Φ_new, M_new, dt)
   κ_new = max(κ_candidate, 0)

No reordering is permitted.

---

## 2.4 Invariants

The Core MUST enforce:

- Φ ≥ 0
- M ≥ M_prev
- κ ≥ 0
- κ ≤ κ_prev

κ MUST be monotonically non-increasing.

κ == 0 is the only collapse gate.

---

## 2.5 Atomic Mutation

Step MUST:

1) Compute X_next in a local buffer.
2) Validate all values are finite.
3) Enforce invariants.
4) Detect collapse condition.
5) Commit X_next atomically.

If any validation fails:

- X remains unchanged.
- EventFlag = ERROR.
- No operator packs execute.

Partial mutation is prohibited.

---

## 2.6 Collapse Semantics

Collapse occurs if and only if:

κ_prev > 0 AND κ_new == 0

On first occurrence:

- EventFlag = COLLAPSE.

Subsequent Step calls:

- MUST NOT emit additional COLLAPSE events.
- MUST NOT mutate X.

Collapse is terminal and irreversible.

---

## 2.7 Structural Cycle Definition

A structural cycle is defined as:

- Initialization with κ > 0
- Deterministic evolution under Step()
- Termination when κ == 0

Within a cycle:

- κ MUST NOT increase
- Collapse occurs at most once
- No resurrection is permitted

New cycle requires new MAX_Init.

---

## 2.8 Numerical Discipline

All computations MUST:

- Use IEEE-754 double precision
- Reject NaN and Inf
- Disable fast-math optimizations
- Avoid undefined behavior

Any non-finite intermediate result:

- Causes ERROR
- Prevents mutation
- Preserves X unchanged

---

## 2.9 Structural Time Rule

Structural time advances only if:

- Step commits a new X.

If κ == 0:

- No time advancement
- No derivative progression

---

# 3. Fundamental Operator Layer (Layer 2 — Telemetry Only)

## 3.1 Ontological Separation

Layer 2 computes deterministic diagnostic operators
over committed structural state X.

Layer 2:

- Observes committed X only.
- May use X(t-1) for finite differences.
- May use dt.
- May use configuration parameters.
- MUST NOT mutate X.
- MUST NOT modify κ.
- MUST NOT alter lifecycle.
- MUST NOT trigger collapse.
- MUST NOT influence Step().

Layer 2 is strictly observational.

---

## 3.2 Operator Execution Rule

Operator packs execute only after a successful commit in Step().

If Step() returns ERROR:

- No operator pack execution is permitted.

If κ == 0:

- Operator packs compute static diagnostics only.
- No derivative-based progression is allowed.

---

## 3.3 Flow Operators (Structural Dynamics)

The Core MUST compute discrete structural flow:

dX/dt = (X(t) - X(t-1)) / dt

Including:

- dΔ/dt ∈ ℝ^N
- dΦ/dt
- dM/dt
- dκ/dt

If no previous committed state exists:

- All derivatives MUST be zero.

If κ == 0:

- All derivatives MUST be zero.

Flow operators are diagnostic only.

---

## 3.4 Space Operators (Structural Geometry)

The Core MUST compute:

1) Deviation norm:
   ||Δ|| = sqrt(sum_i Δ_i²)

2) Viability margin:
   κ_margin = κ

3) Optional configurable margins:
   Δ_margin = Δ_max_norm - ||Δ||
   Φ_margin = Φ_max - Φ
   M_margin = M_max - M

4) Path-length increment:
   dS = sqrt( g_ij(X) * (dX/dt)^i * (dX/dt)^j ) * dt

Metric tensor g_ij MUST:

- Be configuration-defined.
- Be positive-definite.
- Be deterministic.
- Not depend on runtime history.

These operators describe structural manifold geometry.

---

## 3.5 Time Operators (Structural Temporal Field)

Time is defined as a deterministic functional:

T = T(X)

The Core MUST compute:

1) Structural time field T.
2) Temporal gradient ∇T.
3) Optional Hessian H_T.
4) Optional collapse proximity index C_time.

Time operators:

- MUST be bounded.
- MUST avoid κ singularities.
- MUST NOT define collapse.
- MUST NOT influence κ.
- MUST NOT alter Step().

Collapse remains κ == 0 only.

---

## 3.6 Field Operators (Structural Drift Diagnostics)

The Core MUST compute:

𝔽(X) = (FΔ, FΦ, FM, Fκ)

Each component is a deterministic function of X.

Field operators:

- Describe structural drift tendencies.
- MUST NOT define evolution equations.
- MUST NOT override invariants.
- MUST NOT influence κ update.

They are diagnostic representations only.

---

## 3.7 Observer Operators (Bounded Form)

Observation is modeled as bounded structural load.

The Core MUST compute:

- Observation load metric Obs_load.
- Future contraction proxy (diagnostic only).
- Optional cumulative observation metric.

Observer operators MUST:

- Use bounded κ-response (no 1/κ singularity).
- Remain finite for all valid X.
- NOT reduce κ.
- NOT create collapse.
- NOT alter admissible futures.

Observation is diagnostic projection only.

---

## 3.8 Collapse Diagnostics

In addition to κ gate, the Core MUST compute:

- Boundary velocity:
  v_boundary = dκ/dt

- κ_margin = κ

Optional predictive metrics MAY be computed,
but they MUST:

- Be finite.
- Be diagnostic only.
- NOT influence lifecycle.
- NOT redefine collapse condition.

Collapse is triggered only by κ == 0 in Layer 1.

---

## 3.9 Intelligence Diagnostics (Optional Research Mode)

If enabled, the Core MAY compute:

- Interpretive activity proxy.
- Overload indicator (diagnostic only).
- Structural strain metric.
- Stability ratio.

Intelligence diagnostics MUST:

- Remain bounded.
- NOT trigger collapse.
- NOT alter κ.
- NOT modify X.
- NOT influence Step().

They are structural telemetry only.

---

## 3.10 Deterministic Output Requirement

For identical:

- X(t)
- X(t-1)
- dt
- configuration

All operator packs MUST produce identical outputs.

Operator computation MUST:

- Avoid dynamic memory allocation during Step.
- Avoid non-deterministic routines.
- Reject non-finite results.
- Return zeroed outputs on failure.

Layer 2 failure MUST NOT alter structural evolution.

---

# 4. Determinism, Serialization and Replay

## 4.1 Deterministic Execution Requirement

The Core MUST guarantee deterministic execution under identical:

- Binary build
- Compiler flags
- IEEE-754 floating-point environment
- Configuration parameters
- Initial structural state
- Δ_input sequence
- dt sequence

Determinism applies to:

- Structural trajectory X(t)
- Collapse timing
- EventFlag sequence
- All operator pack outputs

Any deviation invalidates compliance.

---

## 4.2 Floating-Point Environment

The implementation MUST:

- Use IEEE-754 double precision (binary64)
- Disable fast-math optimizations
- Disable unsafe floating-point reordering
- Avoid non-deterministic reductions
- Avoid hardware-dependent intrinsics unless identical across builds

The floating-point environment MUST be fixed.

Denormal handling MUST be consistent.

---

## 4.3 Finite-Only Enforcement

At no time may committed StructuralState contain:

- NaN
- +Inf
- -Inf

If any intermediate non-finite value occurs during Step():

- Step() MUST return ERROR.
- No mutation may occur.

Operator packs MUST also:

- Reject non-finite results.
- Zero outputs if instability occurs.
- Not affect structural state.

---

## 4.4 Snapshot Model

The Core MUST maintain:

- X_current — committed state
- X_previous — last committed state

Update rule:

1) X_previous ← old X_current
2) X_current  ← X_next

This update MUST be atomic.

X_previous MUST always represent the immediately preceding committed state.

---

## 4.5 Serialization Requirements

The Core MUST support deterministic serialization of:

- X_current
- X_previous
- Structural cycle status (κ state)
- Step counter
- Configuration parameters

Serialization MUST:

- Preserve raw IEEE-754 bit representation
- Use fixed endianness
- Be fully reversible
- Validate invariants on load

Deserialization MUST reject invalid state.

---

## 4.6 Replay Procedure

Replay consists of:

1) Load serialized initial state.
2) Load configuration.
3) Apply identical Δ_input and dt sequence.
4) Execute Step deterministically.

Replay MUST reproduce:

- Identical X(t)
- Identical collapse step
- Identical EventFlag sequence
- Identical operator outputs

Mismatch indicates non-compliance.

---

## 4.7 Logging Requirements (Optional but Deterministic)

If logging is enabled, each committed step MUST record:

- Step counter
- Δ_input
- dt
- X_before (or deterministic hash)
- X_after (or deterministic hash)
- EventFlag
- Optional operator pack hash

Logging MUST occur only after atomic commit.

Failed steps MUST NOT be logged as committed steps.

Logging MUST NOT alter runtime behavior.

---

## 4.8 Versioning

Serialized state MUST include:

- Specification version = "MAX-Core-Spec-V2.1"
- Configuration version
- Operator enable flags

Incompatible versions MUST be rejected.

No implicit migration is allowed unless deterministic and explicitly defined.

---

## 4.9 Compliance Statement

A V2.1 compliant implementation MUST:

- Preserve structural invariants
- Enforce κ monotonic non-increase
- Enforce single-source collapse rule (κ == 0)
- Separate evolution from operator diagnostics
- Guarantee deterministic replay
- Prevent hidden state mutation
- Maintain atomic commit semantics
- Reject non-finite structural states

This completes MAX Core Specification V2.1.