# MAX Core Specification V2.0
## Deterministic Structural Runtime with Fundamental Operator Packs

---

# 1. Scope and Ontological Model

## 1.1 Objective

MAX Core V2.0 defines a deterministic structural runtime engine that:

1) Evolves the canonical structural state:
   X = (Δ, Φ, M, κ)

2) Computes and exposes all fundamental operator quantities
   defined in Flexion foundational theories:

   - Time Theory
   - Space Theory
   - Field Theory
   - Observer Theory
   - Entanglement Theory
   - Collapse Theory
   - Intelligence Theory

The Core does NOT expand the ontological state.
All fundamental quantities are computed as operators over X
and its trajectory.

---

## 1.2 Canonical Ontological State

The only structural state coordinates are:

X = (Δ, Φ, M, κ)

Where:

- Δ ∈ ℝ^N   — deviation vector
- Φ ≥ 0     — structural energy
- M ≥ 0     — structural memory
- κ ≥ 0     — structural viability

No additional coordinates are permitted.

All other quantities are derived.

---

## 1.3 Deterministic Evolution Principle

Structural evolution is defined exclusively by:

Step(X, Δ_input, dt) → X_new, EventFlag

The Core MUST guarantee:

- Determinism
- Atomic mutation
- Invariant preservation
- Collapse irreversibility (κ == 0)

No operator pack may influence Step().

---

## 1.4 Collapse Definition

Collapse occurs if and only if:

κ == 0

after invariant enforcement.

Collapse is terminal within the structural cycle.

No resurrection of κ is permitted within the same cycle.

---

## 1.5 Separation of Layers

Layer 1 — Ontological Evolution
    X = (Δ, Φ, M, κ)
    Modified only by Step()

Layer 2 — Fundamental Operator Packs
    Time / Space / Field / Observer / Entanglement / Collapse / Intelligence
    Computed from committed X snapshots
    Read-only
    Deterministic

Layer 2 MUST NOT:

- modify X
- influence invariants
- create new collapse gates
- advance time independently

---

## 1.6 Deterministic Replay Requirement

For identical:

- binary build
- configuration
- initial state
- Δ_input sequence
- dt sequence

The Core MUST produce:

- identical structural trajectories
- identical EventFlag sequence
- identical operator pack outputs

Replay determinism applies within identical floating-point implementation.

---

## 1.7 Non-Scope

MAX Core V2.0 does NOT:

- interpret structure
- optimize decisions
- perform prediction
- perform control
- expand admissible futures
- override collapse gate

It is a structural runtime and operator engine only.

---

# 2. Structural Evolution (Layer 1)

## 2.1 Step Interface

Structural evolution is defined by:

Step(X, Δ_input, dt) → (X_new, EventFlag)

Where:

- X = (Δ, Φ, M, κ)
- Δ_input ∈ ℝ^N
- dt > 0
- EventFlag ∈ { NORMAL, COLLAPSE, ERROR }

Only Step() may mutate X.

---

## 2.2 Deterministic Update Order

The update pipeline MUST execute strictly in this order:

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

Invariant enforcement occurs after each stage.

No reordering is permitted.

---

## 2.3 Invariants

The Core MUST enforce:

- Φ ≥ 0
- M ≥ previous M
- κ ≥ 0

κ MUST NOT increase during a cycle.

κ == 0 is the only collapse gate.

---

## 2.4 Atomic Mutation

Step() MUST:

1) Compute X_next in local buffer.
2) Validate finiteness of all values.
3) Enforce invariants.
4) Commit X_next only if valid.

Partial mutation is prohibited.

If validation fails:

- X remains unchanged.
- EventFlag = ERROR.

---

## 2.5 Collapse Semantics

If:

κ_prev > 0 AND κ_new == 0

Then:

- EventFlag = COLLAPSE
- Collapse triggers exactly once per cycle.

Subsequent Step calls within the same cycle:

- MUST NOT emit additional COLLAPSE events.
- MUST follow lifecycle policy (Frozen / Strict / Regenesis).

---

## 2.6 Time Advancement Rule

Structural time advances only if Step commits X_new.

If no commit occurs:

- No structural time advancement.
- No derivative-based operators may advance.

---

## 2.7 Numerical Discipline

All computations MUST:

- Use IEEE-754 double precision.
- Reject NaN or Inf.
- Disable non-deterministic optimizations.
- Avoid hidden state.

Any non-finite intermediate result:

- Causes ERROR.
- Prevents mutation.

---

## 2.8 Structural Cycle

A structural cycle begins with κ > 0.

It ends when κ == 0.

Reinitialization creates a new independent cycle.

Cycle identity MUST be tracked deterministically.

---

# 3. Fundamental Operator Layer (Layer 2)

## 3.1 Purpose

Layer 2 defines the deterministic computation of all
fundamental operator quantities specified in Flexion theories.

These operators:

- Are derived exclusively from committed X(t)
- May use X(t-1) for discrete derivatives
- May use dt
- May use configuration parameters
- MUST NOT mutate X
- MUST NOT influence Step()

Layer 2 exists purely for structural analysis and diagnostics.

---

## 3.2 Fundamental Operator Output

After each committed Step(),
the Core MUST be capable of producing:

FundamentalPack(t)

This pack contains all operator values for the current state.

Operator computation MUST be deterministic.

---

## 3.3 Flow Operators (Dynamics)

The Core MUST compute discrete structural flow:

dX/dt = (X(t) - X(t-1)) / dt

Including:

- dΔ/dt ∈ ℝ^N
- dΦ/dt
- dM/dt
- dκ/dt

If no previous committed state exists:

- Derivatives MUST be zero
  or explicitly flagged as undefined.

Flow operators represent structural velocity.

---

## 3.4 Space Operators (Geometry)

The Core MUST compute geometric quantities:

1) Norm of deviation:
   ||Δ|| = sqrt(sum_i Δ_i²)

2) Distance to viability boundary:
   κ_margin = κ

3) Optional configurable margins:
   Δ_margin = Δ_max_norm - ||Δ||
   Φ_margin = Φ_max - Φ
   M_margin = M_max - M

4) Path-length increment:
   dS = sqrt( g_ij(X) * (dX/dt)^i * (dX/dt)^j ) * dt

Metric tensor g_ij MUST be deterministic
and configuration-defined.

These operators describe the structural manifold geometry.

---

## 3.5 Time Operators (Temporal Field)

Time in Flexion is defined as a structural functional.

The Core MUST compute:

1) Structural time field:
   T = T(X)

2) Temporal gradient:
   ∇T = (∂T/∂Δ, ∂T/∂Φ, ∂T/∂M, ∂T/∂κ)

3) Temporal curvature (if enabled):
   H_T = Hessian of T

4) Collapse proximity index:
   C_time = 1 - T / T_max

Time operators MUST NOT define collapse.
Collapse remains κ == 0 only.

---

## 3.6 Field Operators (Flexion Field)

The Core MUST compute the structural field:

𝔽(X) = (FΔ, FΦ, FM, Fκ)

Where each component is a deterministic function of X.

The Field Pack MUST expose:

- FΔ ∈ ℝ^N
- FΦ
- FM
- Fκ

Field operators describe structural drift tendencies.

They MUST NOT override invariants.

---

## 3.7 Observer Operators

Observation is modeled as projection load.

The Core MUST compute:

- Observation load metric (Obs_load)
- Future multiplicity proxy |𝔽(X)|
- Observation frequency (if configured)

Observer operators MUST:

- Not mutate X
- Not advance time
- Not reduce κ directly
- Not create collapse

They are purely diagnostic.

---

## 3.8 Collapse Operators

In addition to κ gate,
the Core MUST compute:

- Boundary velocity:
  v_boundary = dκ/dt

- Near-collapse flag:
  near_collapse = (κ <= κ_warn_threshold)

Collapse is triggered only by κ == 0.

No operator may introduce additional collapse criteria.

---

## 3.9 Entanglement Operators (Optional)

If multi-structure mode is enabled:

The Core MAY compute entanglement strength:

ES ∈ [0,1]

And supporting components:

- Structural compatibility
- Memory overlap
- Stability coherence

Entanglement operators MUST:

- Preserve identity of individual structures
- Not merge states
- Not alter invariants

---

## 3.10 Intelligence Operators (Optional Research Mode)

If enabled, the Core MAY compute:

- ΔΦ_wave magnitude
- Overload indicator (|ΔΦ_wave| > κ)
- Identity fixed-point error
- Internal time proxy

These operators MUST:

- Remain diagnostic
- Not influence Step()
- Not modify collapse gate

---

## 3.11 Determinism Requirement

For identical:

- X(t)
- X(t-1)
- dt
- configuration

All operator packs MUST produce identical outputs.

Operator computation MUST NOT allocate memory dynamically
or use non-deterministic routines.

---

# 4. Lifecycle and Structural Cycles

## 4.1 Structural Cycle Definition

A structural cycle is defined as:

- Initialization with κ > 0
- Deterministic evolution under Step()
- Termination when κ == 0

Within a cycle:

- κ MUST NOT increase
- Collapse MUST occur at most once
- No resurrection is permitted

A new cycle may begin only through explicit reinitialization.

---

## 4.2 Lifecycle States

The Core MUST support the following lifecycle states:

ACTIVE
COLLAPSED
TERMINAL (policy-defined)

Lifecycle state is meta-state and MUST NOT alter ontological X.

---

## 4.3 Collapse Transition

Collapse occurs if and only if:

κ_prev > 0 AND κ_new == 0

On first detection:

- EventFlag = COLLAPSE
- Lifecycle state transitions to COLLAPSED
- Collapse MUST be recorded deterministically

Subsequent Step calls MUST NOT re-emit COLLAPSE.

---

## 4.4 Post-Collapse Policies

The Core MUST support deterministic post-collapse policies:

### Frozen Mode

- Step() returns NORMAL
- No mutation of X
- No time advancement
- Operator packs remain static

### Strict Mode

- Step() returns ERROR
- No mutation permitted

### Deterministic Regenesis Mode

- A new structural state may be initialized
- κ_init > 0 required
- New cycle identity assigned
- No restoration of prior state allowed

All policies MUST be deterministic.

---

## 4.5 Time Advancement Rule

Structural time advances only when:

- X is successfully mutated and committed.

If:

- ERROR occurs
- Frozen Mode active
- Strict Mode active

Then:

- No time advancement
- No derivative progression

---

## 4.6 Cycle Identity

Each structural cycle MUST have:

- Deterministic cycle_id
- Deterministic step counter

These values are meta-state and MUST NOT influence evolution.

---

## 4.7 Replay Integrity Across Cycles

Replay MUST reproduce:

- Identical cycle boundaries
- Identical collapse points
- Identical operator pack outputs

Cycle transitions MUST be deterministic
given identical inputs.

---

# 5. Determinism, Serialization and Replay

## 5.1 Deterministic Execution Requirement

The Core MUST guarantee deterministic execution under identical:

- Binary build
- Configuration parameters
- Initial structural state
- Δ_input sequence
- dt sequence
- Floating-point environment

Determinism applies to:

- Structural trajectory X(t)
- EventFlag sequence
- All Fundamental Operator Packs

---

## 5.2 Numerical Discipline

All arithmetic MUST:

- Use IEEE-754 double precision
- Reject NaN and Inf
- Disable fast-math optimizations
- Avoid undefined behavior
- Avoid hidden mutable state

If any non-finite value occurs:

- Step() MUST return ERROR
- No state mutation occurs

Operator packs MUST also reject non-finite results.

---

## 5.3 Serialization Requirements

The Core MUST support deterministic serialization of:

- StructuralState X
- Previous committed state (if required for derivatives)
- Lifecycle meta-state
- Configuration parameters

Serialization MUST:

- Preserve raw IEEE-754 representation
- Use fixed endianness
- Be fully reversible

Deserialization MUST validate invariants before accepting state.

---

## 5.4 Replay Procedure

Replay consists of:

1) Load initial serialized state
2) Load configuration
3) Apply identical Δ_input and dt sequence
4) Execute Step deterministically

Replay MUST produce:

- Identical X(t)
- Identical EventFlags
- Identical FundamentalPack outputs

Mismatch at any step indicates non-compliance.

---

## 5.5 Logging Requirements

If logging is enabled, each committed step MUST record:

- cycle_id
- step_counter
- Δ_input
- dt
- X_before (or deterministic hash)
- X_after (or deterministic hash)
- EventFlag
- Optional FundamentalPack hash

Logging MUST occur only after atomic commit.

Partial steps MUST NOT be logged.

---

## 5.6 Versioning

Serialized state MUST include:

- Specification version (V2.0)
- Configuration version
- Operator pack configuration flags

Incompatible versions MUST be rejected
unless deterministic migration logic is provided.

---

## 5.7 Compliance Statement

A V2.0 compliant implementation MUST:

- Preserve structural invariants
- Enforce collapse irreversibility
- Separate evolution from operator computation
- Guarantee deterministic replay
- Prevent hidden state mutation
- Compute all enabled Fundamental Operator Packs deterministically

This completes MAX-Core-Specification-V2.0-EN.
