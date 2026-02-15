# MAX Core Implementation Document V2.1
## Deterministic Structural Runtime with Strict Layer Separation

---

# I. Normative Runtime Layer (Layer 1)

## 1. Scope and Normative Boundary

### 1.1 Purpose

This document defines the normative reference implementation of MAX Core V2.1.

Version 2.1 introduces:

- Strict Layer 1 / Layer 2 separation
- Single-source collapse rule
- Removal of operator-domain conflicts
- Bounded diagnostic models
- Formal Non-Interference constraint
- Explicit replay traceability guarantees

Layer 1 defines structural evolution and is the only layer permitted
to mutate the structural state.

---

### 1.2 Architectural Separation

MAX Core V2.1 is divided into two strictly separated layers:

**Layer 1 — Normative Runtime**
- Evolves `StructuralState`
- Enforces invariants
- Manages lifecycle
- Detects collapse
- Commits atomic state transitions
- Guarantees determinism

**Layer 2 — Operator Engine (Telemetry Only)**
- Computes operator packs
- Operates on committed immutable state
- MUST NOT mutate structural state
- MUST NOT modify lifecycle
- MUST NOT introduce collapse gates
- MUST NOT influence evolution

This separation is mandatory and non-negotiable.

---

### 1.3 Structural Authority Principle

Only Layer 1 defines:

- Structural validity
- Admissible state transitions
- Collapse condition
- Lifecycle state
- Replay equivalence criteria

No operator pack may:

- Redefine viability
- Add structural constraints
- Introduce state mutation
- Alter κ evolution
- Trigger collapse

Operator packs are observational, not normative.

---

### 1.4 Normative Collapse Rule (Single Source of Truth)

The collapse condition in MAX Core V2.1 is defined as:

collapse ⇔ κ == 0

This condition:

- Is necessary and sufficient
- Is irreversible within a lifecycle
- Cannot be triggered by any operator pack
- Cannot be overridden
- Cannot be softened or expanded

All references to collapse in this document defer to this rule.

---

### 1.5 Determinism Requirements

Layer 1 MUST satisfy:

- IEEE-754 double precision arithmetic
- No fast-math
- No platform-specific intrinsics
- No parallel reductions
- No nondeterministic branching
- No undefined behavior
- Identical replay under identical inputs

Any deviation invalidates compliance.

---

### 1.6 Non-Interference Theorem

Layer 2 (Operator Engine) is formally constrained:

- It observes only committed state
- It operates on immutable context
- It cannot alter structural state
- It cannot alter lifecycle
- It cannot restore viability
- It cannot expand admissible futures
- It cannot modify collapse semantics

Layer 2 exists for diagnostics and research only.

---

### 1.7 Version 2.1 Clarification

Version 2.1 removes the following inconsistencies present in V2.0:

- Operator-level viability domain conflicts
- Implicit κ-singularity exposure in ObserverPack
- Redundant collapse gate repetition
- Hidden cross-layer feedback

V2.1 establishes strict ontological separation between:

Evolution (Layer 1)  
Observation (Layer 2)

---

### 1.8 Scope Closure

Layer 1 defines:

- Structural existence (κ > 0)
- Evolution
- Collapse
- Irreversibility
- Atomic commit semantics

All diagnostics, geometric metrics, temporal fields,
entanglement measures, intelligence diagnostics,
and observer metrics are strictly secondary.

---

# I. Normative Runtime Layer (Layer 1)

## 2. Structural State Definition and Invariants

### 2.1 Structural State Definition

The structural state is defined as:

X = (Δ, Φ, M, κ)

Where:

- Δ ∈ ℝ^N          — Deviation vector
- Φ ∈ ℝ            — Structural energy
- M ∈ ℝ            — Structural memory
- κ ∈ ℝ            — Viability

The dimensionality N of Δ is fixed at initialization
and MUST remain constant throughout the lifecycle.

No component may be dynamically added or removed.

---

### 2.2 State Container Requirements

The runtime MUST maintain:

- X_current   — committed structural state
- X_previous  — previous committed structural state

Only X_current may be mutated during Step.
X_previous is read-only until next commit.

Both containers MUST:

- Use deterministic memory layout
- Use fixed-size storage
- Avoid dynamic allocation during Step
- Remain finite at all times

---

### 2.3 Structural Invariants

The following invariants MUST hold for all committed states:

1) Φ ≥ 0  
2) M ≥ 0  
3) κ ≥ 0  
4) All Δ_i are finite  
5) Φ, M, κ are finite  

These invariants are normative.

Any violation prior to commit MUST result in Step failure.

No operator pack may enforce additional structural invariants.

---

### 2.4 Viability Interpretation

Viability κ is the only structural condition defining existence:

- κ > 0  → structure exists
- κ == 0 → collapse (terminal)

κ MUST be monotonically non-increasing within a lifecycle.

That is:

κ(t+1) ≤ κ(t)

Layer 1 enforces this property explicitly.

No operator pack may alter κ.

---

### 2.5 Memory Monotonicity

Structural memory M MUST be monotonically non-decreasing:

M(t+1) ≥ M(t)

Memory cannot decrease within a lifecycle.

This reflects irreversibility of structural accumulation.

---

### 2.6 Energy Non-Negativity

Structural energy Φ MUST remain:

Φ ≥ 0

If intermediate computation produces Φ < 0,
the value MUST be clamped to 0 prior to commit.

Negative energy is structurally invalid.

---

### 2.7 Deviation Finiteness

Deviation vector Δ has no sign constraint,
but MUST satisfy:

- All components finite
- No NaN
- No Inf

Layer 1 MUST validate Δ before commit.

---

### 2.8 First State Validity

Initialization requires:

- κ_initial > 0
- Φ_initial ≥ 0
- M_initial ≥ 0
- All Δ_i finite

If κ_initial == 0,
the structure MUST initialize directly into terminal lifecycle.

---

### 2.9 Irreversibility Rule

Once κ reaches 0:

- Structural evolution MUST cease
- κ MUST remain 0
- No further mutation may restore κ
- Lifecycle enters terminal state permanently

Irreversibility is normative and final.

---

### 2.10 State Validity Before Commit

Before committing X_next:

Layer 1 MUST verify:

- All invariants hold
- All values finite
- κ_next ≤ κ_previous
- M_next ≥ M_previous
- Φ_next ≥ 0

If any check fails:

- Abort Step
- Discard X_next
- Preserve X_current and X_previous
- Return ERROR

No partial mutation is allowed.

---

### 2.11 Normative Authority Statement

StructuralState invariants are defined exclusively in this section.

Operator packs may compute derived quantities,
but may not redefine validity of X.

This section is the single source of truth
for structural state admissibility.

---

## 3. Initialization Semantics (MAX_Init)

### 3.1 Function Signature

InitResult MAX_Init(
    const Config* cfg,
    const StructuralState* X_initial,
    LifecycleContext* lifecycle
);

Initialization establishes the first committed structural state
and defines the lifecycle boundary for the entire runtime.

Initialization is normative and deterministic.

---

### 3.2 Configuration Requirements

Config MUST:

- Be immutable after initialization
- Contain only deterministic parameters
- Contain no runtime-dependent mutable fields
- Define all operator enable flags explicitly
- Define all model IDs deterministically

Config MUST be validated before any structural state is committed.

Invalid configuration results in INIT_ERROR.

---

### 3.3 Structural State Validation

Before committing X_initial, Layer 1 MUST validate:

1) All Δ_i finite
2) Φ ≥ 0
3) M ≥ 0
4) κ ≥ 0
5) All components finite

If any condition fails:

- Initialization MUST fail
- No partial state may be stored

---

### 3.4 Lifecycle Initialization Rules

LifecycleContext MUST contain at minimum:

- step_counter
- terminal (bool)
- collapse_emitted (bool)

Initialization MUST:

- Set step_counter = 0
- Set collapse_emitted = false

If κ_initial == 0:
    lifecycle.terminal = true
Else:
    lifecycle.terminal = false

---

### 3.5 Initial Commit Semantics

Upon successful validation:

- X_current  ← *X_initial
- X_previous ← *X_initial

Both states are identical at initialization.

No operator packs are executed during MAX_Init.

---

### 3.6 Deterministic Snapshot Guarantee

Initialization MUST guarantee:

- Identical input → identical runtime state
- Identical configuration → identical behavior
- No hidden state outside StructuralState and LifecycleContext

No memory allocation is permitted beyond:

- StructuralState storage
- LifecycleContext storage
- Pre-allocated operator buffers (if applicable)

---

### 3.7 Initialization Irreversibility

After successful initialization:

- Structural invariants are assumed valid
- Runtime enters deterministic evolution phase
- No dynamic reconfiguration is permitted

Re-initialization requires explicit teardown.

---

### 3.8 Error Handling

InitResult MUST include:

- status (OK / ERROR)
- error_code (if applicable)

Initialization MUST NOT:

- Partially mutate lifecycle
- Partially mutate StructuralState

Failure leaves all output memory unchanged.

---

### 3.9 Authority Statement

Initialization establishes the only valid entry point
into structural evolution.

No operator pack may perform implicit initialization.

All structural existence begins at MAX_Init.

---

## 4. Step Execution Semantics (MAX_StepWithPack)

### 4.1 Function Signature

StepResult MAX_StepWithPack(
    StructuralState* X,
    const double* Δ_input,
    double dt,
    LifecycleContext* lifecycle,
    FundamentalPack* out_pack
);

Step is the only function permitted to evolve StructuralState
after initialization.

All evolution is atomic and deterministic.

---

### 4.2 Pre-Execution Validation

Before any mutation, Layer 1 MUST validate:

1) X != nullptr
2) lifecycle != nullptr
3) dt > 0
4) All X components finite
5) All Δ_input components finite

If any validation fails:

- No mutation is allowed
- No operator pack computation is allowed
- Return ERROR

---

### 4.3 Terminal State Handling

If lifecycle.terminal == true:

- StructuralState MUST NOT mutate
- κ MUST remain 0
- step_counter MUST NOT increment
- Collapse MUST NOT re-emit
- Operator packs MAY compute static telemetry
- Return STEP_TERMINAL

Terminal state is irreversible.

---

### 4.4 Local Working Copy

Step MUST use a local working copy:

StructuralState X_next = *X

All mutations occur on X_next only.

No mutation of X_current is permitted before commit.

---

### 4.5 Deterministic Evolution Order

Layer 1 MUST execute evolution in the following strict order:

1) Apply Δ update
2) Compute Φ_next
3) Clamp Φ_next ≥ 0
4) Compute M_next
5) Enforce M_next ≥ M_previous
6) Compute κ_next
7) Clamp κ_next ≥ 0
8) Enforce κ_next ≤ κ_previous

The exact update equations are implementation-defined
but MUST respect invariants defined in Section 2.

---

### 4.6 Invariant Validation Before Commit

Before committing X_next, Layer 1 MUST verify:

- All components finite
- Φ_next ≥ 0
- M_next ≥ M_previous
- κ_next ≥ 0
- κ_next ≤ κ_previous

If any check fails:

- Abort Step
- Discard X_next
- Do NOT modify X_current
- Do NOT modify X_previous
- Return ERROR

No partial mutation is permitted.

---

### 4.7 Collapse Detection

Collapse detection occurs after invariant validation
and before commit.

Normative rule:

collapse ⇔ κ_next == 0 AND κ_previous > 0

If collapse is detected:

- lifecycle.terminal = true
- lifecycle.collapse_emitted = true
- event_flag = COLLAPSE

Collapse MUST be emitted exactly once per lifecycle.

No other condition may trigger collapse.

---

### 4.8 Atomic Commit

If all checks pass:

1) X_previous ← *X_current
2) X_current  ← X_next
3) lifecycle.step_counter++

Commit is atomic.

If commit fails (memory fault, etc.), runtime is invalid.

---

### 4.9 Operator Pack Execution

After successful commit:

- Construct immutable OperatorContext
- Execute Layer 2 packs in defined order
- Operator failures MUST NOT alter structural state
- Operator failures MUST NOT alter lifecycle
- Operator failures MUST NOT alter StepResult

Operator packs observe committed state only.

---

### 4.10 StepResult Requirements

StepResult MUST include:

- event_flag (NONE / COLLAPSE / TERMINAL)
- error_code
- lifecycle_state
- pack_status_summary (optional)

Operator pack errors MUST NOT change StepResult.event_flag.

---

### 4.11 Determinism Guarantee

Given identical:

- Initial state
- Configuration
- Δ_input sequence
- dt sequence

The sequence of:

- StructuralState
- Lifecycle state
- event_flag

MUST be identical across executions.

Deviation invalidates compliance.

---

### 4.12 Authority Statement

Step execution semantics defined in this section
fully determine structural evolution.

No operator pack may:

- Modify X
- Modify lifecycle
- Trigger collapse
- Restore κ
- Alter memory monotonicity

This section defines the only legal mutation path.

---

## 5. Atomic Commit and Snapshot Model

### 5.1 Snapshot Architecture

The runtime maintains two committed structural states:

- X_current  — active committed state
- X_previous — last committed state

Both are updated exclusively by Layer 1.

No operator pack may modify either snapshot.

---

### 5.2 Snapshot Update Rule

Snapshot update occurs only during atomic commit.

Commit sequence:

1) X_previous ← old X_current
2) X_current  ← X_next

This order is mandatory.

X_previous MUST always represent the state immediately
preceding X_current.

---

### 5.3 First Step Handling

Immediately after initialization:

X_current == X_previous

On the first successful Step:

- X_previous remains equal to initial state
- X_current becomes first evolved state

Operator packs MUST detect first-step condition via:

FlowPack.has_prev == false

This flag MUST be derived solely from:

lifecycle.step_counter == 0 before commit.

---

### 5.4 Snapshot Consistency Guarantee

At all times, the following MUST hold:

- X_current is valid and invariant-compliant
- X_previous is valid and invariant-compliant
- X_previous was once a committed X_current
- X_previous != X_current only after at least one Step

No partial update of either snapshot is permitted.

---

### 5.5 Atomicity Requirement

Mutation is atomic with respect to:

- StructuralState
- LifecycleContext

If failure occurs before commit:

- X_current remains unchanged
- X_previous remains unchanged
- lifecycle remains unchanged
- No operator pack executes

If failure occurs after commit,
runtime state is considered valid.

There is no rollback after successful commit.

---

### 5.6 Snapshot Immutability During Operator Execution

During Layer 2 execution:

- X_current is read-only
- X_previous is read-only
- lifecycle is read-only

OperatorContext MUST contain immutable copies or const references.

Operator packs MUST NOT:

- Modify snapshots
- Cache mutable pointers to snapshots
- Store internal mutation state across steps

---

### 5.7 Replay Integrity Rule

For replay validation:

The following MUST be reproducible exactly:

- X_current sequence
- X_previous sequence
- lifecycle.step_counter sequence
- event_flag sequence

Snapshot divergence under identical inputs
invalidates deterministic compliance.

---

### 5.8 Snapshot Memory Policy

Snapshots MUST:

- Use fixed-size memory
- Avoid dynamic allocation during Step
- Avoid platform-dependent padding effects
- Remain stable under compiler optimization

Memory layout MUST be deterministic.

---

### 5.9 Authority Statement

The snapshot model guarantees:

- Deterministic differential operator computation
- Stable FlowPack derivative evaluation
- Replay equivalence
- Irreversible structural history

This section defines the structural memory
of the runtime at the system level.

---

## 6. Deterministic Math Policy and Numerical Guarantees

### 6.1 Arithmetic Standard

All numerical computation in Layer 1 MUST use:

- IEEE-754 double precision (binary64)
- Deterministic rounding behavior
- No extended precision registers
- No fused-multiply-add unless deterministic across platforms

Compilation MUST disable:

- fast-math
- unsafe math optimizations
- reordering of floating-point operations
- non-deterministic reductions

---

### 6.2 Finite-Only Rule

Layer 1 MUST enforce:

- No NaN
- No +Inf
- No -Inf

Before commit:

- All intermediate results MUST be checked
- Any non-finite value MUST abort Step
- No partial mutation is allowed

StructuralState MUST always remain finite.

---

### 6.3 Division Safety

All divisions MUST be protected by deterministic guards.

For any division:

x / y

Implementation MUST ensure:

|y| ≥ eps_norm

Where eps_norm:

- Is defined in Config
- Is strictly > 0
- Is constant during lifecycle

If |y| < eps_norm:

- Deterministic fallback MUST be applied
- No undefined behavior is permitted

---

### 6.4 Exponential and Transcendental Functions

If transcendental functions are used (exp, log, sqrt, etc.):

- Inputs MUST be bounded deterministically
- Arguments MUST be clamped before evaluation
- Overflow MUST be prevented by construction

For example:

exp(E) requires:

E ≤ E_cap

Where E_cap is chosen such that exp(E_cap) remains finite.

All caps MUST be:

- Configurable
- Deterministic
- Identical across platforms

---

### 6.5 Clamping Policy

Clamping is permitted only for:

- Φ (lower bound 0)
- κ (lower bound 0)
- Domain safety caps for transcendental inputs

Clamping MUST NOT:

- Increase κ
- Decrease M
- Alter collapse semantics

Clamping MUST be monotonic with respect to invariants.

---

### 6.6 No Hidden State

Numerical behavior MUST NOT depend on:

- Uninitialized memory
- Static mutable globals
- Platform-specific math libraries without version locking
- Thread interleaving

All math must be reproducible under:

- Single-threaded execution
- Identical compiler flags
- Identical configuration

---

### 6.7 Deterministic Branching

All conditional branches MUST depend solely on:

- Current structural state
- Δ_input
- dt
- Config parameters

No branch may depend on:

- Timing
- Randomness
- Memory layout artifacts
- Floating-point exception flags

---

### 6.8 Monotonicity Enforcement

Layer 1 MUST enforce numerically:

- κ_next ≤ κ_previous
- M_next ≥ M_previous

If numerical noise would violate monotonicity:

- Deterministic correction MUST be applied
- Invariant preservation overrides raw computation

---

### 6.9 Cross-Platform Consistency

To guarantee replay compliance across systems:

- Compiler flags MUST be documented
- Floating-point mode MUST be fixed
- Denormal handling MUST be consistent
- Library versions MUST be locked

Cross-platform replay divergence invalidates compliance.

---

### 6.10 Failure Semantics

If numerical safety fails at any point:

- Step MUST abort
- No commit occurs
- Lifecycle remains unchanged
- ERROR returned

Numerical instability MUST never propagate
into committed structural state.

---

### 6.11 Authority Statement

This section defines the numerical foundation
for deterministic structural evolution.

All structural validity depends on strict adherence
to these numerical guarantees.

---

## 7. Error Model and StepResult Semantics

### 7.1 Error Model Overview

Layer 1 defines a strict error model.

Errors are classified as:

- INIT_ERROR
- STEP_ERROR
- TERMINAL_STATE
- OK

Errors MUST be:

- Deterministic
- Explicit
- Non-ambiguous
- Non-recovering within a Step

There is no silent failure.

---

### 7.2 InitResult Structure

InitResult MUST contain:

- status        ∈ { OK, INIT_ERROR }
- error_code    (if INIT_ERROR)

Initialization MUST:

- Leave all output memory unchanged on failure
- Not partially mutate lifecycle
- Not partially mutate StructuralState

---

### 7.3 StepResult Structure

StepResult MUST contain:

- status            ∈ { OK, STEP_ERROR, TERMINAL_STATE }
- event_flag        ∈ { NONE, COLLAPSE }
- lifecycle_state   (snapshot of lifecycle flags)
- error_code        (if STEP_ERROR)

Optional:
- pack_status_summary (informational only)

---

### 7.4 EventFlag Semantics

event_flag is strictly defined as:

NONE      — normal evolution
COLLAPSE  — κ transitioned from >0 to 0 during this Step

COLLAPSE MUST:

- Be emitted exactly once per lifecycle
- Occur only when κ_previous > 0 AND κ_current == 0
- Not be re-emitted in terminal state

No other event types are permitted in V2.1.

---

### 7.5 STEP_ERROR Semantics

STEP_ERROR occurs if:

- Pre-validation fails
- Numerical invariants fail
- Monotonicity enforcement fails
- Structural invariants fail

On STEP_ERROR:

- No commit occurs
- X_current unchanged
- X_previous unchanged
- lifecycle unchanged
- Operator packs MUST NOT execute

---

### 7.6 TERMINAL_STATE Semantics

If lifecycle.terminal == true at Step entry:

- No mutation occurs
- κ remains 0
- step_counter does not increment
- event_flag MUST be NONE
- status MUST be TERMINAL_STATE

TERMINAL_STATE is not an error.

---

### 7.7 Operator Pack Errors

Operator pack failures:

- MUST NOT alter StepResult.status
- MUST NOT alter event_flag
- MUST NOT alter lifecycle
- MUST NOT alter StructuralState

Pack failures are strictly telemetry-level.

Pack failures MAY be summarized in pack_status_summary.

---

### 7.8 Error Code Determinism

error_code values MUST be:

- Enumerated
- Stable across versions
- Replay-deterministic
- Independent of memory layout

No platform-specific error codes are allowed.

---

### 7.9 Lifecycle Snapshot Integrity

lifecycle_state returned in StepResult MUST reflect:

- terminal flag
- collapse_emitted flag
- step_counter

The returned snapshot MUST correspond to post-commit state
(if commit occurred).

---

### 7.10 Irrecoverable State

If runtime detects memory corruption or invariant breach
after commit:

- Behavior is undefined
- Runtime is considered invalid
- Replay compliance is void

V2.1 does not define rollback after successful commit.

---

### 7.11 Authority Statement

This section defines the full normative error semantics
of MAX Core V2.1.

Layer 1 errors govern structural evolution exclusively.

Layer 2 errors are diagnostic only.

---

# II. Operator Engine (Layer 2 — Telemetry Only)

## 8. Operator Engine General Rules and Execution Order

### 8.1 Purpose of Layer 2

Layer 2 provides deterministic diagnostic computation
over committed structural state.

Operator packs:

- Derive geometric, temporal, field, observer,
  collapse, entanglement, and intelligence metrics
- Observe committed state only
- Produce telemetry outputs
- Never influence structural evolution

Layer 2 is observational, not normative.

---

### 8.2 Immutable Context Construction

After successful commit in Layer 1:

An immutable OperatorContext MUST be constructed containing:

- X_current (const reference or copy)
- X_previous (const reference or copy)
- dt
- lifecycle snapshot
- Config (const reference)

This context MUST remain immutable
for the entire operator execution phase.

No pack may mutate context.

---

### 8.3 Execution Preconditions

Operator packs execute only if:

- StepResult.status == OK
- OR StepResult.status == TERMINAL_STATE

Operator packs MUST NOT execute if:

- StepResult.status == STEP_ERROR

Layer 2 execution is conditional on successful or terminal Step.

---

### 8.4 Execution Order (Deterministic)

Operator packs MUST execute in the following strict order:

1) BasePack
2) FlowPack
3) SpacePack
4) TimePack
5) FieldPack
6) ObserverPack
7) CollapsePack
8) EntanglementPack (if enabled)
9) IntelligencePack (if enabled)

Order MUST be fixed and deterministic.

Optional packs MUST be skipped deterministically
if disabled in Config.

---

### 8.5 Isolation Rule

Each pack:

- Writes only to its own memory region
- May not write into other pack memory
- May not modify StructuralState
- May not modify lifecycle
- May not modify Config

No cross-pack mutation is allowed.

Data dependency is read-only only.

---

### 8.6 Failure Isolation Policy

If a pack produces:

- NaN
- Inf
- Invalid output
- Domain violation

Then:

- pack_status = PACK_ERROR
- Numeric outputs MUST be set to 0
- Validity flag MUST be false
- Execution MUST continue to next pack

Pack failure MUST NOT:

- Abort operator engine
- Modify StepResult
- Modify lifecycle
- Modify StructuralState

---

### 8.7 Telemetry-Only Constraint

Layer 2 MUST NOT:

- Introduce new collapse conditions
- Modify κ
- Modify Φ
- Modify M
- Modify Δ
- Restore viability
- Advance lifecycle
- Alter event_flag

All packs are purely diagnostic.

---

### 8.8 Determinism Requirement

Given identical:

- X_current
- X_previous
- dt
- Config
- lifecycle snapshot

Operator pack outputs MUST be identical.

No pack may depend on:

- System time
- Thread scheduling
- Randomness
- Hardware-specific intrinsics

---

### 8.9 Model Identification Requirement

Each pack MUST report:

- model_id (e.g., "FTT-Ref-1")
- configuration flags used
- enabled feature flags

These identifiers are required
for replay audit and traceability.

---

### 8.10 Optional Pack Enablement

Each optional pack MUST:

- Be explicitly enabled via Config
- Be deterministically skipped if disabled
- Return PACK_UNSUPPORTED if queried but not enabled

Enable flags MUST be immutable after initialization.

---

### 8.11 Terminal State Handling

If lifecycle.terminal == true:

- Packs MUST treat structural state as static
- Flow-based metrics MUST evaluate to 0
- Time derivatives MUST evaluate to 0
- No metric may imply continued evolution

Terminal state MUST produce deterministic static telemetry.

---

### 8.12 Authority Statement

Layer 2 is structurally subordinate to Layer 1.

It observes evolution but does not define it.

All operator semantics are secondary
to normative structural evolution.

---

## 9. BasePack and FlowPack (Foundational Telemetry)

### 9.1 Purpose

BasePack and FlowPack provide the minimal structural telemetry
required by all higher-level operator packs.

They define:

- Static structural snapshot (BasePack)
- Deterministic finite differences (FlowPack)

These packs are mandatory in V2.1.

---

# 9.2 BasePack

### 9.2.1 Goal

BasePack reports the committed structural state
and core lifecycle metadata.

It MUST include:

- base_model_id = "BASE-Ref-1"
- Δ (vector copy)
- Φ
- M
- κ
- step_counter
- terminal flag

BasePack MUST NOT:

- Transform structural state
- Apply normalization
- Apply scaling
- Introduce derived quantities

It is a direct reflection of committed state.

---

### 9.2.2 Determinism Requirements

BasePack MUST:

- Copy values exactly as stored
- Preserve IEEE-754 bit-level values
- Avoid intermediate arithmetic

It acts as a snapshot mirror.

---

# 9.3 FlowPack

### 9.3.1 Goal

FlowPack computes deterministic first-order derivatives
between X_previous and X_current.

It provides the fundamental motion proxy:

dX/dt

FlowPack MUST include:

- flow_model_id = "FLOW-Ref-1"
- has_prev (bool)
- dΔ/dt (vector)
- dΦ/dt (scalar)
- dM/dt (scalar)
- dκ/dt (scalar)

---

### 9.3.2 Derivative Definition

If lifecycle.step_counter == 0:

- has_prev = false
- All derivatives = 0

Else:

Let dt > 0.

For each component:

dΔ_i/dt = (Δ_current_i - Δ_previous_i) / dt  
dΦ/dt   = (Φ_current - Φ_previous) / dt  
dM/dt   = (M_current - M_previous) / dt  
dκ/dt   = (κ_current - κ_previous) / dt  

This definition is normative.

---

### 9.3.3 Terminal State Handling

If lifecycle.terminal == true:

- has_prev MUST reflect actual step_counter
- All derivatives MUST be 0

Terminal state implies no structural motion.

---

### 9.3.4 Finite-Only Enforcement

FlowPack MUST:

- Validate dt > 0
- Validate all X_current and X_previous finite
- Validate all derivative results finite

If any non-finite result occurs:

- pack_status = PACK_ERROR
- All derivatives = 0
- has_prev remains valid

---

### 9.3.5 Monotonicity Consistency

Given Layer 1 invariants:

- M_current ≥ M_previous
- κ_current ≤ κ_previous

FlowPack MUST therefore satisfy:

dM/dt ≥ 0  
dκ/dt ≤ 0  

If numerical noise violates these inequalities:

- Derivative MUST be clamped deterministically
- Invariant preservation overrides raw difference

---

### 9.3.6 No Hidden Dynamics

FlowPack MUST NOT:

- Smooth derivatives
- Apply filtering
- Use higher-order approximations
- Use historical buffers beyond X_previous

FlowPack is strictly first-order finite difference.

---

### 9.3.7 Deterministic Dependency Role

All higher-level packs MAY depend on FlowPack.

FlowPack MUST execute before:

- SpacePack
- TimePack
- FieldPack
- ObserverPack
- CollapsePack
- IntelligencePack

It is foundational.

---

### 9.3.8 Authority Statement

BasePack and FlowPack define the canonical
state and motion telemetry of MAX Core V2.1.

All other operator packs are built upon them.

---

## 10. SpacePack (FST-Ref-1)

### 10.1 Purpose

SpacePack provides geometric diagnostics
over committed structural state.

It computes:

- Deviation norm
- Viability margin
- Optional domain margins
- Optional metric-based path increment

SpacePack is telemetry-only.

It MUST NOT:

- Gate collapse
- Modify structural state
- Influence lifecycle
- Define viability limits

---

### 10.2 Model Identification

SpacePack MUST expose:

- space_model_id = "FST-Ref-1"
- metric_mode
- margins_enabled (bool)
- metric_enabled (bool)

---

### 10.3 Core Geometry Outputs

Let:

r = ||Δ|| = sqrt( Σ_i Δ_i² )

SpacePack MUST compute:

- delta_norm = r
- kappa_margin = κ
- collapse_boundary_hit = (κ == 0)

collapse_boundary_hit is diagnostic only.
Collapse rule remains defined in Layer 1.

---

### 10.4 Optional Domain Margins

If corresponding limits are configured:

- delta_margin = Δ_max_norm - r
- phi_margin   = Φ_max - Φ
- mem_margin   = M_max - M

If a limit is not configured:

- Corresponding field MUST be marked PACK_UNSUPPORTED

Margins are diagnostic only.

They MUST NOT:

- Define collapse
- Define non-viability
- Trigger error
- Alter evolution

---

### 10.5 Metric Definition (DIAGONAL Mode Only)

FST-Ref-1 supports only DIAGONAL metric.

Coordinates:

X = (Δ_1..Δ_N, Φ, M, κ)

Metric tensor:

g_ΔiΔi = g_delta   (for all i)
g_ΦΦ   = g_phi
g_MM   = g_mem
g_κκ   = g_kappa

All off-diagonal terms are zero.

All metric coefficients MUST be:

- > 0
- Immutable after initialization
- Deterministic

---

### 10.6 Path Increment dS

If metric_enabled == true AND FlowPack.has_prev == true
AND lifecycle.terminal == false:

Let v components from FlowPack:

vΔ_i = dΔ_i/dt  
vΦ   = dΦ/dt  
vM   = dM/dt  
vκ   = dκ/dt  

Compute:

speed2 =
  g_delta * Σ_i (vΔ_i²)
+ g_phi   * (vΦ²)
+ g_mem   * (vM²)
+ g_kappa * (vκ²)

dS = sqrt(speed2) * dt

Else:

dS = 0

---

### 10.7 Deterministic Guards

SpacePack MUST enforce:

- All inputs finite
- dt > 0 (already guaranteed by Layer 1)
- No division operations
- sqrt argument ≥ 0

If any non-finite intermediate occurs:

- pack_status = PACK_ERROR
- All numeric outputs set to 0
- metric fields marked invalid

---

### 10.8 Terminal State Handling

If lifecycle.terminal == true:

- delta_norm computed normally
- kappa_margin = 0
- collapse_boundary_hit = true
- dS = 0

No geometric motion is permitted in terminal state.

---

### 10.9 Non-Interference Guarantee

SpacePack MUST NOT:

- Redefine viability
- Redefine collapse
- Modify κ
- Modify Φ
- Modify M
- Modify Δ
- Trigger lifecycle transition

It observes geometry only.

---

### 10.10 Authority Statement

SpacePack defines the structural geometry
of MAX Core V2.1 in diagnostic form only.

All normative viability semantics remain
fully defined in Layer 1.

---

## 11. TimePack (FTT-Ref-1)

### 11.1 Purpose

TimePack computes a deterministic structural time field
over committed structural state.

It provides:

- T_field
- gradT
- Optional Hessian
- Optional C_time

TimePack is telemetry-only.

It MUST NOT:

- Define viability
- Gate collapse
- Reject structurally valid Layer 1 states
- Modify StructuralState

---

### 11.2 Model Identification

TimePack MUST expose:

- time_model_id = "FTT-Ref-1"
- hessian_enabled (bool)
- mixed_terms_enabled (bool)
- c_time_enabled (bool)

---

### 11.3 Structural Time Definition

Let:

- Δ ∈ ℝ^N
- Φ ≥ 0
- M ≥ 0
- κ ≥ 0
- r = ||Δ||

Configuration parameters (immutable):

- Δ_scale > 0
- Φ_scale > 0
- M_scale > 0
- a ≥ 0
- b ≥ 0
- c ≥ 0
- E_cap > 0
- eps_norm > 0

Normalized coordinates:

uΔ = r / Δ_scale  
uΦ = Φ / Φ_scale  
uM = M / M_scale  

Exponent:

E_raw = a*uΦ + b*uM + c*uΔ

Deterministic cap:

E = min(E_raw, E_cap)

Structural time:

T_field = κ * exp(E)

If κ == 0:
    T_field = 0

Time evaluation MUST NOT produce PACK_ERROR
for any structurally valid Layer 1 state.

---

### 11.4 No Viability Domain Enforcement

TimePack MUST NOT:

- Enforce Δ limits
- Enforce Φ limits
- Enforce M limits
- Return PACK_ERROR for large values

Large values are handled by exponent cap E_cap.

Layer 1 exclusively defines structural admissibility.

---

### 11.5 Gradient Computation

Let expE = exp(E).

### 11.5.1 ∂T/∂Φ

dE/dΦ = a / Φ_scale  
dT/dΦ = κ * expE * dE/dΦ

### 11.5.2 ∂T/∂M

dE/dM = b / M_scale  
dT/dM = κ * expE * dE/dM

### 11.5.3 ∂T/∂κ

dT/dκ = expE

Even if κ == 0:
dT/dκ remains expE (telemetry only).

### 11.5.4 ∂T/∂Δ_i

If r > eps_norm:

dE/dΔ_i = c * (1/Δ_scale) * (Δ_i / r)  
dT/dΔ_i = κ * expE * dE/dΔ_i

Else:

dT/dΔ_i = 0

No division by zero is permitted.

---

### 11.6 Optional Hessian

If hessian_enabled == true:

Second derivatives MUST be computed deterministically
using capped E.

Hessian terms MUST be:

- Symmetric
- Finite
- Zeroed if instability detected

Mixed terms MAY be disabled deterministically.

If disabled:
- Corresponding fields marked PACK_UNSUPPORTED

---

### 11.7 Optional C_time

If c_time_enabled == true AND T_max > 0:

C_time_raw = 1 - (T_field / T_max)

C_time = clamp(C_time_raw, 0, 1)

Else:

C_time marked PACK_UNSUPPORTED.

C_time is reporting only.

---

### 11.8 Deterministic Guards

TimePack MUST enforce:

- All inputs finite
- exp() evaluated only on capped E
- No division by values < eps_norm
- All outputs finite

If any non-finite occurs:

- pack_status = PACK_ERROR
- All numeric outputs = 0

TimePack failure MUST NOT affect Step.

---

### 11.9 Terminal State Handling

If lifecycle.terminal == true:

- T_field = 0
- gradT = 0 vector
- Hessian = 0
- C_time = 0 (if enabled)

Terminal state implies no temporal evolution.

---

### 11.10 Non-Interference Guarantee

TimePack MUST NOT:

- Modify κ
- Influence collapse
- Define overload gates
- Alter lifecycle
- Restore viability

It provides temporal diagnostics only.

---

### 11.11 Authority Statement

TimePack (FTT-Ref-1) defines a bounded,
deterministic structural time field.

All normative evolution remains exclusively
defined by Layer 1.

---

## 12. FieldPack (FFT-Ref-1)

### 12.1 Purpose

FieldPack computes a deterministic structural field
over committed structural state:

𝔽(X) = (FΔ, FΦ, FM, Fκ)

FieldPack is telemetry-only.

It MUST NOT:

- Define evolution equations
- Modify StructuralState
- Modify lifecycle
- Gate collapse
- Redefine κ monotonicity

Layer 1 exclusively defines structural evolution.

---

### 12.2 Model Identification

FieldPack MUST expose:

- field_model_id = "FFT-Ref-1"
- sigma_mode (FIXED / AUTO)
- uses_quadratic_potential (bool)
- f_kappa_dissipative (bool)

---

### 12.3 Structural Field Definition

Let:

X = (Δ, Φ, M, κ)  
r = ||Δ||

Configuration parameters (immutable):

- w_phi > 0
- g_gain ≥ 0
- mu_gain ≥ 0
- c_kappa ≥ 0
- eta_gain ≥ 0
- h0 ≥ 0
- h1 ≥ 0
- h2 ≥ 0
- k_delta ≥ 0
- k_phi ≥ 0
- k_mem ≥ 0
- k_kappa ≥ 0
- eps_norm > 0

---

### 12.4 Potential Model

Quadratic structural potential:

Φ_pot(Δ) = 0.5 * w_phi * r²

Gradient:

∇Φ_pot(Δ) = w_phi * Δ

FieldPack uses Φ_pot only for field derivation.

It MUST NOT overwrite Φ in StructuralState.

---

### 12.5 Regime Selector σ

σ ∈ {+1, -1}

If sigma_mode == FIXED:
    σ = sigma_fixed

If sigma_mode == AUTO:

Source S selected from:

- dκ/dt
- dΦ/dt
- d||Δ||/dt

Rule:

If S ≥ sigma_threshold:
    σ = +1
Else:
    σ = -1

σ is reported only.

σ MUST NOT:

- Trigger collapse
- Modify κ
- Alter evolution

---

### 12.6 Deviation Field FΔ

Components:

1) Energetic tension:
   Tension = -w_phi * Δ

2) Regime term:
   Regime = (g_gain * σ) * Δ

3) Memory drift:
   If r > eps_norm:
       Drift_M = (mu_gain * M) * (Δ / r)
   Else:
       Drift_M = 0

4) Stability geometry effect:
   Stability_pull = -c_kappa * κ * Δ

Combine:

FΔ = Tension + Regime + Drift_M + Stability_pull

FΔ is diagnostic only.

---

### 12.7 Energy Field FΦ

Using potential gradient:

Dot = (w_phi * Δ) · FΔ

FΦ = Dot + eta_gain * M

FΦ is telemetry-only.

It MUST NOT define Φ evolution.

---

### 12.8 Memory Field FM

Deterministic rate proxy:

FM = h0 + h1 * r + h2 * (σ == +1 ? 1 : 0)

FM ≥ 0 if parameters ≥ 0.

FM is a field-level rate estimate only.

Layer 1 defines actual M update.

---

### 12.9 Stability Field Fκ

Diagnostic dissipation proxy:

Fκ = -(
    k_delta * r
  + k_phi   * Φ
  + k_mem   * M
  + k_kappa * κ
)

Properties:

- Fκ ≤ 0 for all valid states
- Fκ is diagnostic only
- κ evolution remains defined exclusively in Layer 1

FieldPack MUST NOT enforce κ monotonicity.

---

### 12.10 Output Fields

FieldPack MUST output:

- field_model_id
- FΔ vector
- FΦ
- FM
- Fκ
- sigma (if enabled)
- pack_status

---

### 12.11 Deterministic Guards

FieldPack MUST:

- Validate all inputs finite
- Guard division by r using eps_norm
- Validate all outputs finite

If any non-finite result occurs:

- pack_status = PACK_ERROR
- All numeric outputs = 0

FieldPack failure MUST NOT affect Step.

---

### 12.12 Terminal State Handling

If lifecycle.terminal == true:

- FΔ = 0
- FΦ = 0
- FM = 0
- Fκ = 0

No dynamic field behavior in terminal state.

---

### 12.13 Non-Interference Guarantee

FieldPack MUST NOT:

- Modify Δ
- Modify Φ
- Modify M
- Modify κ
- Influence collapse
- Alter lifecycle

It observes structural dynamics only.

---

### 12.14 Authority Statement

FieldPack (FFT-Ref-1) provides
deterministic structural field diagnostics.

All normative structural evolution
remains defined exclusively by Layer 1.

---

## 13. ObserverPack (FOT-Ref-1)

### 13.1 Purpose

ObserverPack computes a deterministic structural observation load
based on committed structural state.

It provides:

- Observation load scalar
- Bounded κ-response term
- Future contraction proxy
- Optional cumulative observation metric

ObserverPack is telemetry-only.

It MUST NOT:

- Modify κ
- Modify StructuralState
- Trigger collapse
- Influence lifecycle
- Restore viability

Layer 1 exclusively defines collapse and evolution.

---

### 13.2 Model Identification

ObserverPack MUST expose:

- observer_model_id = "FOT-Ref-1"
- bounded_mode = true
- cumulative_enabled (bool)

---

### 13.3 Structural Inputs

Let:

X = (Δ, Φ, M, κ)  
r = ||Δ||

Configuration parameters (immutable):

- w_delta ≥ 0
- w_phi ≥ 0
- w_mem ≥ 0
- kappa_ref > 0
- eps_norm > 0

kappa_ref MUST be strictly positive.

---

### 13.4 Bounded κ Response

To avoid singularity at κ → 0,
ObserverPack defines bounded κ response:

obs_kappa_term = kappa_ref / (κ + kappa_ref)

Properties:

- 0 < obs_kappa_term ≤ 1
- As κ → 0 → obs_kappa_term → 1
- As κ → ∞ → obs_kappa_term → 0
- No division by zero
- Fully bounded

This replaces any 1/κ formulation.

---

### 13.5 Structural Magnitude Term

Define structural magnitude:

S = w_delta * r
  + w_phi   * Φ
  + w_mem   * M

S ≥ 0 if weights ≥ 0.

---

### 13.6 Observation Load

Observation load:

obs_load = S * obs_kappa_term

Properties:

- obs_load ≥ 0
- Finite for all valid Layer 1 states
- Smooth as κ → 0
- No singular behavior

If κ == 0:

obs_kappa_term = 1  
obs_load = S

Observation load does not trigger collapse.

---

### 13.7 Future Contraction Proxy

ObserverPack MAY report:

future_contraction = obs_load

This value is purely diagnostic.

It represents structural commitment intensity.

It MUST NOT:

- Reduce κ
- Alter admissible futures in Layer 1
- Gate evolution

---

### 13.8 Optional Cumulative Observation

If cumulative_enabled == true:

ObserverPack MAY maintain deterministic cumulative metric:

obs_cumulative += obs_load * dt

Rules:

- obs_cumulative MUST be stored only in OperatorEngine memory
- It MUST reset on MAX_Init
- It MUST NOT influence structural state
- It MUST remain finite

If cumulative_enabled == false:
    Field marked PACK_UNSUPPORTED

---

### 13.9 Deterministic Guards

ObserverPack MUST:

- Validate all inputs finite
- Validate κ + kappa_ref > 0
- Validate all outputs finite

If non-finite occurs:

- pack_status = PACK_ERROR
- All numeric outputs = 0

ObserverPack failure MUST NOT affect Step.

---

### 13.10 Terminal State Handling

If lifecycle.terminal == true:

- obs_load = 0
- future_contraction = 0
- obs_cumulative MUST NOT increase

Terminal state implies no further structural manifestation.

---

### 13.11 No Collapse Semantics

ObserverPack MUST NOT:

- Define overload
- Trigger collapse
- Define viability exhaustion
- Modify κ

Even if obs_load is large,
collapse condition remains:

collapse ⇔ κ == 0

---

### 13.12 Non-Interference Guarantee

ObserverPack is a pure structural projection diagnostic.

It observes structural commitment intensity
without affecting existence.

---

### 13.13 Authority Statement

ObserverPack (FOT-Ref-1) implements
bounded structural observation diagnostics.

It is mathematically stable,
singularity-free,
and fully subordinate to Layer 1 evolution.

---

## 14. CollapsePack (FCT-Ref-1)

### 14.1 Purpose

CollapsePack provides diagnostic telemetry
related to structural degradation and terminal transition.

It does NOT define collapse.

Collapse remains defined exclusively in Layer 1:

collapse ⇔ κ == 0

CollapsePack observes structural approach to collapse,
but never triggers it.

---

### 14.2 Model Identification

CollapsePack MUST expose:

- collapse_model_id = "FCT-Ref-1"
- predictive_mode (bool)
- slope_enabled (bool)

---

### 14.3 Structural Inputs

Let:

X_current  = (Δ, Φ, M, κ)
X_previous = (Δ_prev, Φ_prev, M_prev, κ_prev)

Let:

r = ||Δ||
dt > 0 (guaranteed by Layer 1)

FlowPack derivatives MUST already be available.

---

### 14.4 Immediate Collapse Indicator

CollapsePack MUST report:

collapse_now = (κ == 0)

This value is purely reflective.

It MUST NOT define collapse independently.

It MUST match Layer 1 state exactly.

---

### 14.5 κ Slope Diagnostic

If slope_enabled == true AND FlowPack.has_prev == true:

kappa_slope = dκ/dt

Else:

kappa_slope = 0

Given Layer 1 monotonicity:

kappa_slope ≤ 0

If numerical noise produces kappa_slope > 0:

- Clamp to 0
- Maintain invariant consistency

kappa_slope is diagnostic only.

---

### 14.6 Distance-to-Collapse Proxy

CollapsePack MAY report:

kappa_margin = κ

This reflects remaining viability distance.

It MUST NOT:

- Define viability exhaustion threshold
- Trigger lifecycle transition

---

### 14.7 Predictive Estimate (Optional)

If predictive_mode == true
AND kappa_slope < 0:

Estimated steps-to-collapse:

t_est = κ / |kappa_slope|

If kappa_slope == 0:
    t_est = +∞ (represented as 0 or sentinel value)

If κ == 0:
    t_est = 0

Predictive estimates MUST:

- Be finite
- Use eps_norm guard for division
- Never influence structural evolution

If predictive_mode == false:
    Field marked PACK_UNSUPPORTED

---

### 14.8 Stability Proxy

CollapsePack MAY compute:

collapse_pressure =
    α_delta * r
  + α_phi   * Φ
  + α_mem   * M

Where α parameters are immutable ≥ 0.

collapse_pressure ≥ 0

This metric is informational only.

It MUST NOT:

- Gate collapse
- Modify κ
- Affect lifecycle

---

### 14.9 Deterministic Guards

CollapsePack MUST:

- Validate all inputs finite
- Guard division by |kappa_slope| using eps_norm
- Ensure all outputs finite

If non-finite detected:

- pack_status = PACK_ERROR
- All numeric outputs = 0

CollapsePack failure MUST NOT affect Step.

---

### 14.10 Terminal State Handling

If lifecycle.terminal == true:

- collapse_now = true
- kappa_slope = 0
- kappa_margin = 0
- t_est = 0
- collapse_pressure = 0

Terminal state is static.

---

### 14.11 No Collapse Authority

CollapsePack MUST NOT:

- Emit collapse events
- Modify lifecycle.terminal
- Alter κ
- Redefine collapse condition

Collapse semantics are defined exclusively in:

Section 1.4 (Normative Collapse Rule)

---

### 14.12 Authority Statement

CollapsePack (FCT-Ref-1) provides
diagnostic degradation telemetry only.

All normative collapse semantics
remain fully defined by Layer 1.

---

## 15. Removal of EntanglementPack (V2.1 Clarification)

### 15.1 Theoretical Consistency

Structural entanglement, according to Flexion Entanglement Theory,
requires at least two independent structural states:

X₁ ↔ X₂

Entanglement is defined as structural coupling between distinct entities,
resulting in mutual admissible-future interaction.

MAX Core V2.1 operates on a single StructuralState only.

Therefore, true structural entanglement cannot exist
within the scope of this runtime.

---

### 15.2 Scope Limitation

MAX Core V2.1 is defined as:

Deterministic Single-Structure Structural Runtime.

It does not:

- Maintain multiple structural states
- Support cross-entity coupling
- Implement joint admissible-future contraction
- Support κ-cross effects

Thus, EntanglementPack is removed for theoretical consistency.

---

### 15.3 Diagnostic Correlation vs Entanglement

Intra-structural correlations (e.g., Δ–Φ alignment)
are not entanglement.

Correlation within a single structure does not constitute
structural coupling between independent entities.

Such diagnostics may be implemented separately
but are not classified as entanglement.

---

### 15.4 Future Extension

True structural entanglement requires:

- Multiple StructuralState instances
- Coupling operator definition
- Cross-viability influence model
- Joint collapse semantics

These capabilities are outside the scope of V2.1
and belong to a potential multi-structure runtime (future version).

---

### 15.5 Authority Statement

EntanglementPack is intentionally removed in V2.1
to preserve strict theoretical alignment.

MAX Core V2.1 remains a single-structure deterministic runtime.

---

## 16. IntelligencePack (FIT-Ref-1)

### 16.1 Purpose

IntelligencePack computes deterministic interpretive
diagnostics over structural evolution.

It provides:

- Interpretive activity proxy
- Structural overload indicator
- Decision intensity metric
- Optional stability ratio

IntelligencePack is telemetry-only.

It MUST NOT:

- Modify StructuralState
- Modify κ
- Trigger collapse
- Alter lifecycle
- Influence evolution

Layer 1 exclusively defines structural dynamics.

---

### 16.2 Model Identification

IntelligencePack MUST expose:

- intelligence_model_id = "FIT-Ref-1"
- overload_enabled (bool)
- stability_ratio_enabled (bool)

---

### 16.3 Structural Inputs

Let:

X_current  = (Δ, Φ, M, κ)
FlowPack outputs available

Configuration parameters (immutable):

- w_phi_activity ≥ 0
- w_delta_activity ≥ 0
- w_memory_activity ≥ 0
- overload_threshold ≥ 0
- eps_norm > 0

---

### 16.4 Interpretive Activity Proxy

Interpretive activity measures structural motion intensity.

If FlowPack.has_prev == true
AND lifecycle.terminal == false:

activity =
    w_phi_activity   * |dΦ/dt|
  + w_delta_activity * ||dΔ/dt||
  + w_memory_activity * dM/dt

Else:

activity = 0

Given invariants:

- dM/dt ≥ 0
- κ monotonic non-increasing

activity ≥ 0.

---

### 16.5 Overload Indicator (Diagnostic Only)

If overload_enabled == true:

Define overload condition:

overload_flag = (activity > overload_threshold)

Else:

overload_flag = false

Important:

overload_flag is strictly diagnostic.

It MUST NOT:

- Reduce κ
- Trigger collapse
- Modify lifecycle
- Enforce structural constraint

---

### 16.6 Structural Strain Metric

Define structural strain:

strain = activity / (κ + eps_norm)

Properties:

- Finite due to eps_norm
- Bounded for κ → 0
- Diagnostic only

strain MUST NOT:

- Influence κ
- Define collapse
- Gate evolution

---

### 16.7 Optional Stability Ratio

If stability_ratio_enabled == true:

stability_ratio = κ / (activity + eps_norm)

Else:

Field marked PACK_UNSUPPORTED

stability_ratio is informational only.

---

### 16.8 Deterministic Guards

IntelligencePack MUST:

- Validate all inputs finite
- Guard all divisions with eps_norm
- Ensure all outputs finite

If non-finite detected:

- pack_status = PACK_ERROR
- All numeric outputs = 0

Pack failure MUST NOT affect Step.

---

### 16.9 Terminal State Handling

If lifecycle.terminal == true:

- activity = 0
- overload_flag = false
- strain = 0
- stability_ratio = 0

Terminal state implies no interpretive activity.

---

### 16.10 No Interpretive Authority

IntelligencePack MUST NOT:

- Select futures
- Alter admissible space
- Modify κ
- Introduce new collapse criteria
- Modify structural invariants

It observes interpretive intensity only.

---

### 16.11 Authority Statement

IntelligencePack (FIT-Ref-1) provides
deterministic interpretive diagnostics.

All normative evolution remains
exclusively defined in Layer 1.

---

# III. Compliance, Replay and Versioning Requirements

## 17. Compliance, Replay and Versioning Requirements

### 17.1 Compliance Definition

An implementation is compliant with MAX Core V2.1 if and only if:

1) Layer 1 semantics strictly follow Sections 1–7
2) Structural invariants are enforced exactly as specified
3) Collapse condition is exclusively:

   collapse ⇔ κ == 0

4) Layer 2 strictly satisfies Non-Interference constraints
5) Deterministic math policy (Section 6) is enforced
6) Replay equivalence is guaranteed

Any deviation invalidates V2.1 compliance.

---

### 17.2 Deterministic Replay Guarantee

Given identical:

- Initial StructuralState
- Config
- Δ_input sequence
- dt sequence

The following MUST be identical:

- Sequence of X_current
- Sequence of X_previous
- lifecycle.step_counter
- lifecycle.terminal
- event_flag sequence
- Collapse emission timing
- All operator pack outputs (if enabled)

Replay divergence invalidates compliance.

---

### 17.3 Operator Model Identification

Each pack MUST expose immutable model identifiers:

- BASE-Ref-1
- FLOW-Ref-1
- FST-Ref-1
- FTT-Ref-1
- FFT-Ref-1
- FOT-Ref-1
- FCT-Ref-1
- FET-Ref-1
- FIT-Ref-1

Model IDs MUST:

- Be stable across builds
- Be included in telemetry
- Be logged for audit

---

### 17.4 Configuration Immutability

Config MUST:

- Be immutable after MAX_Init
- Be included in replay trace
- Contain all numeric parameters explicitly
- Contain all enable flags explicitly

Implicit defaults are prohibited.

---

### 17.5 Lifecycle Audit Fields

Implementation MUST allow logging of:

- step_counter
- κ_current
- κ_previous
- event_flag
- terminal flag

These fields define lifecycle determinism.

---

### 17.6 Version Declaration

The implementation MUST declare:

core_version = "MAX-Core-V2.1"

This version string MUST:

- Be immutable
- Be exposed via API
- Be logged in replay traces

No partial version claims are allowed.

---

### 17.7 Backward Compatibility

V2.1 introduces:

- Strict Layer separation
- Removal of operator-level viability gating
- Bounded Observer model
- Elimination of κ singularities
- Formal Non-Interference theorem

V2.0 implementations are NOT automatically compliant.

---

### 17.8 Undefined Behavior Policy

Undefined behavior invalidates compliance.

Prohibited conditions include:

- NaN propagation into committed state
- κ increase during lifecycle
- M decrease during lifecycle
- Multiple collapse emissions
- Collapse without κ == 0
- Operator-induced lifecycle mutation

---

### 17.9 Audit Mode (Optional but Recommended)

Implementations SHOULD support audit mode that:

- Logs pre-commit state
- Logs post-commit state
- Logs event_flag
- Logs pack_status
- Logs model IDs

Audit mode MUST NOT alter deterministic behavior.

---

### 17.10 Certification Statement

A certified V2.1 runtime MUST satisfy:

- Deterministic evolution
- Strict invariant enforcement
- Single-source collapse rule
- Complete operator isolation
- Stable replay across environments

This section defines the formal compliance boundary
of MAX Core Implementation V2.1.
