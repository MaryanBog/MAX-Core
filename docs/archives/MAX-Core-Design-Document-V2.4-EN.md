# MAX-Core Design Document
## Version 2.4
### Architecture of the Canonical Deterministic Living Structural Runtime

---

# 1. Architectural Foundation

## 1.1 Purpose

MAX-Core V2.4 is the architectural realization of the
canonical deterministic living structural runtime
defined in MAX-Core Specification V2.4.

The Core implements exactly one persistent structural state:

    S = (Phi, Memory, Kappa)

Delta is transient input and is not part of the persistent state.

The Core is responsible exclusively for:

- Canonical structural evolution
- Enforcement of structural invariants
- Regenerative stability dynamics
- Collapse detection (Kappa == 0)
- Atomic state commitment
- Deterministic lifecycle control

The Core does NOT implement:

- Observation semantics
- Multi-structure interaction
- Entanglement
- External control logic
- Optimization policies
- Environment modeling

The Core defines canonical evolution only.

---

## 1.2 Persistent vs Transient Model

Architecturally, MAX-Core distinguishes:

Persistent State:

    S = (Phi, Memory, Kappa)

Transient Input:

    Delta_input (size = delta_dim)

Persistent state is stored across Step() calls.
Delta_input exists only during Step() execution.

Delta is never stored in StructuralState.

---

## 1.3 Mutation Authority

Persistent state may be mutated ONLY:

    Step() → Canonical Updates → Invariant Enforcement → AtomicCommit

No other component may mutate state.

There are no setters, background updates,
or hidden mutation paths.

---

## 1.4 Layer Separation

MAX-Core is divided into two layers:

### Layer 1 — Canonical Runtime (Mandatory)

Responsible for:

- State mutation
- Parameter enforcement
- AdmissibleDelta processing
- Canonical update equations
- Invariant enforcement
- Collapse detection
- Atomic commit
- Lifecycle control

Layer 1 is the only mutation authority.

---

### Layer 2 — Diagnostic Layer (Optional)

Responsible for:

- Observing committed snapshots
- Computing derived deterministic metrics
- Exporting telemetry

Layer 2:

- Has no mutation authority
- Cannot influence evolution
- Cannot trigger collapse
- Cannot modify admissibility

Layer separation is strict and enforced by design.

---

## 1.5 Deterministic Design Principle

Determinism is enforced structurally by:

- Single-threaded mutation authority
- Strict execution order
- Immutable configuration
- No hidden state
- Explicit atomic commit
- IEEE-754 double precision discipline

Determinism is an architectural property,
not a runtime validation artifact.

---

# 2. Core Components and State Ownership

This section defines the architectural components
of MAX-Core V2.4 and their state ownership rules.

All components described here are mandatory
unless explicitly marked optional.

---

## 2.1 StructuralState

StructuralState represents the only persistent structural state
of the runtime.

It contains exactly:

    double Phi
    double Memory
    double Kappa

No additional structural coordinates are permitted.

Delta is NOT stored persistently.

---

### 2.1.1 Structural Invariants

At every committed state, the following MUST hold:

    Phi ≥ 0
    Memory ≥ 0
    0 ≤ Kappa ≤ kappa_max

All values MUST be finite IEEE-754 double precision.

StructuralState MUST NOT contain:

- cached derived metrics
- auxiliary correction terms
- historical Delta values
- hidden accumulators

The state remains minimal by design.

---

### 2.1.2 Ownership Rules

StructuralState is owned exclusively by the Core.

It may be mutated ONLY:

    inside Step()
    through canonical updates
    during atomic commit

No external setter or direct mutation is permitted.

All reads outside Step() MUST observe committed state only.

---

## 2.2 LifecycleContext

LifecycleContext stores deterministic evolution metadata.

It contains exactly:

    uint64_t step_counter
    bool terminal
    bool collapse_emitted

LifecycleContext is updated only during atomic commit.

---

### 2.2.1 Lifecycle Invariants

The following MUST hold at all times:

    terminal == (Kappa == 0)

collapse_emitted:

    false before first collapse
    true permanently after collapse

step_counter:

    increments only after successful commit
    does NOT increment on ERROR
    does NOT increment in terminal state

---

## 2.3 ParameterSet

ParameterSet defines canonical dynamics.

It contains:

    alpha
    eta
    beta
    gamma
    rho
    lambda_phi
    lambda_m
    kappa_max

ParameterSet is:

    validated at initialization
    immutable during lifecycle
    finite
    strictly positive where required

---

## 2.4 delta_dim Configuration

delta_dim defines the dimensionality of Delta_input.

delta_dim:

    MUST be defined at initialization
    MUST be > 0
    MUST remain constant during lifecycle
    MUST be stored as immutable configuration

delta_dim is NOT part of StructuralState.

It is runtime configuration.

---

## 2.4.1 Optional DeltaGuardConfig

If norm guard is enabled, the Core MUST store:

    double Delta_max

Delta_max:

    MUST be provided during initialization
    MUST be finite
    MUST be strictly positive
    MUST remain immutable during lifecycle

Delta_max is runtime configuration.

Delta_max is NOT part of StructuralState.

If norm guard is disabled:

    No DeltaGuardConfig is stored.

---

## 2.5 Step Engine

Step Engine is the only mutation authority.

Responsibilities:

    validate input
    enforce numerical stability
    compute admissible Delta
    compute canonical updates
    enforce invariants
    detect collapse
    perform atomic commit
    emit EventFlag

The Step Engine MUST be single-threaded.

No other component may mutate state.

---

## 2.6 AtomicCommit

AtomicCommit is responsible for:

    copying S_current → S_previous
    copying S_next → S_current
    incrementing step_counter
    updating terminal
    updating collapse_emitted

AtomicCommit MUST:

    execute only after successful validation
    be logically indivisible
    never partially update state
    never execute on ERROR

---

# 3. Execution Pipeline

This section defines the exact execution flow of Step()
in MAX-Core V2.4.

The order defined here is mandatory.
No reordering of stages is permitted.

---

## 3.1 High-Level Flow

For non-terminal states, Step() MUST execute
the following stages in strict order:

1) Input Validation  
2) Numerical Stability Check  
3) Local Candidate State Creation  
4) Admissible Delta Processing  
5) Canonical Energy Update  
6) Canonical Memory Update  
7) Canonical Stability Update  
8) Invariant Enforcement  
9) Collapse Detection  
10) Atomic Commit  
11) EventFlag Return  

If any stage fails:

- Step() MUST return ERROR
- No mutation MUST occur
- LifecycleContext MUST remain unchanged

---

### 3.2 Stage 1 — Input Validation

The following conditions MUST be validated before any computation:

- Runtime is initialized.
- delta_input is not null.
- delta_len == delta_dim.
- dt is finite.
- dt > 0.
- All delta_input[i] for i in [0, delta_len) are finite.
- StructuralState contains only finite values.
- ParameterSet contains only finite values.

If any validation fails:

    Step() MUST return ERROR.

No mutation is permitted during validation.

---

## 3.3 Stage 2 — Numerical Stability Check

Compute:

    max_rate = max(
        eta,
        gamma,
        rho,
        lambda_phi,
        lambda_m
    )

Verify:

    dt * max_rate < 1

If violated:

    return ERROR

This prevents discrete-time instability.

---

## 3.4 Stage 3 — Local Candidate State Creation

Create working copy:

    S_next = S_current

S_current MUST NOT be modified
before AtomicCommit.

---

## 3.5 Stage 4 — Admissible Delta Processing

Compute:

    Delta_step = ComputeAdmissibleDelta(delta_input)

Requirements:

- deterministic
- finite
- no expansion beyond input magnitude
- no mutation of StructuralState
- no mutation of ParameterSet
- no mutation of LifecycleContext

Delta_step exists only within Step().

If non-finite occurs:

    return ERROR

---

## 3.6 Stage 5 — Canonical Energy Update

Compute:

    Phi_next =
        Phi_current
        + alpha * ||Delta_step||^2
        - eta   * Phi_current * dt

Then enforce:

    Phi_next = max(0, Phi_next)

If Phi_next is non-finite:

    return ERROR

---

## 3.7 Stage 6 — Canonical Memory Update

Compute:

    Memory_next =
        Memory_current
        + beta  * Phi_next * dt
        - gamma * Memory_current * dt

Then enforce:

    Memory_next = max(0, Memory_next)

If Memory_next is non-finite:

    return ERROR

---

## 3.8 Stage 7 — Canonical Stability Update

Compute:

    Kappa_next =
        Kappa_current
        + rho * (kappa_max - Kappa_current) * dt
        - lambda_phi * Phi_next    * dt
        - lambda_m   * Memory_next * dt

Then enforce:

    Kappa_next = max(0, Kappa_next)
    Kappa_next = min(Kappa_next, kappa_max)

If Kappa_next is non-finite:

    return ERROR

---

## 3.9 Stage 8 — Invariant Enforcement

Verify candidate state:

    Phi_next ≥ 0
    Memory_next ≥ 0
    0 ≤ Kappa_next ≤ kappa_max
    all values finite

If violation detected:

    return ERROR

No silent correction beyond defined clamping is permitted.

---

## 3.10 Stage 9 — Collapse Detection

If:

    Kappa_current > 0
    and
    Kappa_next == 0

Then:

    event_flag = COLLAPSE

Else:

    event_flag = NORMAL

Collapse detection MUST occur before AtomicCommit.

---

### 3.11 Stage 10 — Atomic Commit

Atomic commit MUST occur only after successful validation
and invariant enforcement.

The following operations MUST execute in this exact order:

1) Copy S_current → S_previous.
2) Copy S_next → S_current.
3) Increment step_counter.
4) Update terminal flag:
       terminal = (S_current.Kappa == 0)
5) If event_flag == COLLAPSE and collapse_emitted == false:
       collapse_emitted = true

Atomic commit MUST be logically indivisible.

No partial mutation is permitted.

If Step() returns ERROR, Atomic Commit MUST NOT execute.

---

## 3.12 Stage 11 — Return

Return:

    event_flag

Where:

- NORMAL   → successful evolution
- COLLAPSE → first transition where Kappa becomes zero
- ERROR    → validation or numerical failure

No further mutation occurs after return.

---

## 3.13 Terminal Short-Circuit

If:

    S_current.Kappa == 0

Then:

- Skip all stages
- Return NORMAL
- Do NOT increment step_counter
- Do NOT mutate state

Terminal state is irreversible within lifecycle.

---

# 4. Lifecycle and Structural Time Architecture

This section defines the deterministic lifecycle model
and the concept of structural time in MAX-Core V2.4.

Lifecycle behavior is fully state-driven.

---

## 4.1 Lifecycle Phases

A structure progresses through the following phases:

1) Initialization  
2) Active Evolution  
3) Collapse Event  
4) Terminal State  
5) Explicit Reinitialization (external action)

No implicit transitions are permitted.

---

## 4.2 Initialization Phase

Initialization MUST:

- Validate ParameterSet
- Validate delta_dim
- Validate initial StructuralState invariants
- Initialize LifecycleContext deterministically

Upon successful initialization:

    S_current   = initial_state
    S_previous  = initial_state
    step_counter = 0
    terminal     = (Kappa_initial == 0)
    collapse_emitted = false

If Kappa_initial == 0:

- The lifecycle begins in terminal state.
- Step() short-circuits immediately.

---

## 4.3 Active Evolution Phase

Active evolution occurs when:

    S_current.Kappa > 0

In this phase:

- Step() executes the canonical pipeline.
- step_counter increments only on successful commit.
- Kappa may increase or decrease.
- Collapse detection remains active.

Active evolution continues until collapse.

---

## 4.4 Collapse Event Phase

Collapse occurs when:

    Kappa_current > 0
    and
    Kappa_next == 0

Collapse detection occurs before Atomic Commit.

If collapse is detected:

- EventFlag = COLLAPSE.

During Atomic Commit of that step:

- collapse_emitted is set to true
  if it was previously false.
- terminal is set to true
  because S_current.Kappa == 0 after commit.

Collapse is emitted exactly once per lifecycle.

No recovery is permitted after collapse.

---

## 4.5 Terminal Phase

Terminal phase is defined by:

    S_current.Kappa == 0

During terminal phase:

- Step() performs no mutation.
- step_counter does not increment.
- EventFlag = NORMAL.
- No regeneration is allowed.
- No structural coordinate may change.

Terminal phase persists indefinitely
until explicit reinitialization.

---

## 4.6 Structural Time Definition

Structural time is defined as:

    t_structural = step_counter

Structural time advances only when:

- AtomicCommit completes successfully.
- EventFlag is NORMAL or COLLAPSE.

Structural time does NOT advance when:

- Step() returns ERROR.
- Step() is short-circuited due to terminal state.

Structural time is discrete and deterministic.

---

## 4.7 Deterministic Time Progression

Given identical:

- initial StructuralState
- ParameterSet
- delta_input sequence
- dt sequence

The sequence of structural times and states
MUST be identical under identical floating-point environment.

Structural time is independent of wall-clock time.

---

## 4.8 Lifecycle Isolation

Each lifecycle is independent.

After collapse:

- Reinitialization MUST construct a new StructuralState.
- step_counter MUST reset to zero.
- collapse_emitted MUST reset to false.
- terminal MUST be recalculated.

No structural state carries across lifecycles
unless explicitly transferred by external logic.

---

## 4.9 Lifecycle Guarantees

The lifecycle architecture guarantees:

- No hidden revival after collapse.
- No mutation in terminal state.
- No time advancement without commit.
- No partial collapse.
- No silent state transition.

Lifecycle semantics are fully determined
by StructuralState and AtomicCommit rules.

---

# 5. Error Handling and Runtime Safety Architecture

This section defines how MAX-Core V2.4 handles invalid input,
numerical instability, and runtime safety violations.

Error handling is deterministic and non-destructive.

---

## 5.1 Execution Outcomes

Step() may return exactly one of:

    NORMAL
    COLLAPSE
    ERROR

Definitions:

- NORMAL   → successful evolution without collapse
- COLLAPSE → first transition where Kappa reaches zero
- ERROR    → validation or numerical violation

ERROR is not structural collapse.

---

## 5.2 Non-Destructive Error Policy

If Step() returns ERROR:

- S_current MUST remain unchanged.
- S_previous MUST remain unchanged.
- step_counter MUST remain unchanged.
- terminal MUST remain unchanged.
- collapse_emitted MUST remain unchanged.

No partial mutation is permitted.

The runtime MUST remain valid and usable after ERROR.

---

## 5.3 Input Validation Errors

The following MUST produce ERROR:

- runtime not initialized
- dt is not finite
- dt ≤ 0
- delta_input is null
- delta_input length ≠ delta_dim
- any delta_input component is NaN or Inf
- StructuralState contains NaN or Inf
- ParameterSet is invalid

Input validation MUST occur before any mutation.

---

## 5.4 Numerical Stability Errors

The following MUST produce ERROR:

- dt * max_rate ≥ 1
- non-finite norm computation
- overflow during Phi update
- overflow during Memory update
- overflow during Kappa update
- any intermediate value becomes non-finite

Silent overflow is prohibited.

---

## 5.5 Invariant Violation Errors

After canonical computation and clamping,
if any invariant is violated:

    Phi < 0
    Memory < 0
    Kappa < 0
    Kappa > kappa_max
    any value non-finite

Then:

    Step() MUST return ERROR
    No mutation MUST occur

No silent correction beyond defined clamping is allowed.

---

## 5.6 Parameter Validation Errors

Initialization MUST return ERROR if:

- any parameter is non-finite
- any required-positive parameter ≤ 0
- kappa_max ≤ 0
- delta_dim == 0
- initial StructuralState violates invariants

No partially constructed runtime instance is permitted.

Parameter mutation during lifecycle is prohibited.

---

## 5.7 Separation of ERROR and COLLAPSE

ERROR and COLLAPSE are strictly distinct.

ERROR MUST NOT:

- trigger collapse
- alter Kappa
- modify terminal
- modify collapse_emitted

COLLAPSE MUST NOT be masked as ERROR.

Collapse results exclusively from canonical dynamics.

---

## 5.8 Deterministic Error Semantics

ERROR behavior MUST:

- be deterministic
- be reproducible under identical inputs
- not depend on wall-clock time
- not depend on external entropy
- not introduce hidden side effects

Error reporting MUST not alter runtime state.

---

## 5.9 Safety Guarantees

The architecture guarantees:

- No undefined structural state is committed.
- No silent instability.
- No partial state corruption.
- No inconsistent lifecycle flags.
- No invalid committed state.

Safety enforcement occurs before AtomicCommit.

---

# 6. Determinism and Reproducibility Architecture

Determinism is a fundamental architectural property
of MAX-Core V2.4.

This section defines how deterministic behavior
is enforced structurally.

---

## 6.1 Deterministic Execution Guarantee

Given identical:

- Initial StructuralState
- ParameterSet
- delta_dim
- Sequence of delta_input values
- Sequence of dt values
- Floating-point environment

The runtime MUST produce identical:

- S_current sequence
- S_previous sequence
- LifecycleContext sequence
- EventFlag sequence
- step_counter sequence

Determinism is mandatory.

---

## 6.2 Single-Threaded Mutation Model

MAX-Core V2.4 is single-threaded by design.

The runtime:

- MUST NOT spawn internal threads
- MUST NOT perform asynchronous mutation
- MUST NOT rely on parallel reductions
- MUST NOT depend on execution interleaving

If parallel execution is required,
it MUST be implemented externally.

---

## 6.3 Strict Execution Ordering

The canonical execution pipeline defined in Section 3
MUST be followed exactly.

No reordering of update stages is permitted.

No speculative execution is permitted.

Execution order is part of the contract.

---

## 6.4 Floating-Point Discipline

The runtime MUST:

- Use IEEE-754 double precision
- Avoid fast-math optimizations
- Avoid undefined floating-point behavior
- Avoid compiler transformations that alter evaluation order
- Avoid platform-dependent fused operations
  unless identical across builds

Recommended build discipline:

- Disable fast-math
- Enforce strict IEEE compliance
- Avoid extended precision registers

---

## 6.5 No Hidden Mutable State

The runtime MUST NOT depend on:

- Static mutable globals
- Thread-local hidden variables
- Implicit caches affecting evolution
- Non-deterministic containers
- System time

All evolution inputs MUST be explicit:

    StructuralState
    ParameterSet
    LifecycleContext
    delta_input
    dt

No implicit external dependency is permitted.

---

## 6.6 Bitwise Reproducibility

Under identical:

- Compiler
- Architecture
- Build flags
- Floating-point environment

Repeated executions MUST produce bit-identical results.

Cross-platform bitwise identity is recommended
but not mandatory.

---

## 6.7 Deterministic Delta Processing

AdmissibleDelta MUST:

- Compute norm in deterministic order
- Preserve input direction if scaling
- Not introduce randomness
- Not depend on historical state
- Not depend on environment

Delta processing MUST be purely functional.

---

## 6.8 Deterministic Lifecycle Transitions

The following transitions are deterministic:

- Initialization
- Active evolution
- Collapse detection
- Terminal short-circuit
- Reinitialization

Lifecycle state transitions MUST depend only
on committed StructuralState and explicit inputs.

---

## 6.9 Deterministic Error Behavior

If Step() returns ERROR under specific inputs,
the same ERROR MUST occur under identical conditions.

ERROR MUST:

- Not mutate state
- Not advance structural time
- Not alter lifecycle flags
- Not trigger collapse

Error handling is deterministic and isolated.

---

## 6.10 Architectural Determinism Guarantee

Determinism in MAX-Core V2.4 is guaranteed by:

- Minimal persistent ontology
- Single mutation authority
- Explicit state ownership
- Strict stage ordering
- Immutable configuration
- Atomic commit enforcement

Determinism is enforced structurally,
not validated after execution.

---

# 7. Architectural Compliance with Specification V2.4

This section defines how MAX-Core Design V2.4
realizes and enforces the normative requirements
defined in MAX-Core Specification V2.4.

The Specification defines mathematical and behavioral rules.
This Design defines their structural realization.

---

## 7.1 Canonical Equation Compliance

The architecture guarantees that:

- Energy update follows Specification Section 7.
- Memory update follows Specification Section 8.
- Stability update follows Specification Section 9.
- Collapse condition follows Specification Section 9.
- Numerical stability rule follows Specification Section 13.

The Step Engine enforces strict stage ordering
and prevents alternative update paths.

No alternative dynamic equations are permitted.

---

## 7.2 Invariant Enforcement Compliance

The architecture ensures:

- All invariants are validated before AtomicCommit.
- No state may be committed unless valid.
- Clamping behavior matches the Specification.
- No hidden correction beyond defined clamping occurs.
- No non-finite value is ever committed.

Invariant enforcement is mandatory.

---

## 7.3 Collapse Semantics Compliance

The architecture enforces:

- Collapse ⇔ Kappa == 0.
- Collapse is emitted exactly once.
- terminal == (Kappa == 0).
- Terminal state is irreversible.
- No regeneration after collapse.

Collapse detection occurs before AtomicCommit,
ensuring deterministic emission.

---

## 7.4 Lifecycle Compliance

The architecture guarantees:

- Structural time advances only after successful AtomicCommit.
- No mutation occurs in terminal state.
- No mutation occurs on ERROR.
- No partial state transition is possible.
- Lifecycle flags are updated only during commit.

Lifecycle semantics derive solely from StructuralState
and commit rules.

---

## 7.5 Determinism Compliance

The architecture enforces determinism by:

- Single-threaded mutation authority.
- Strict execution ordering.
- Immutable ParameterSet and delta_dim.
- Explicit AtomicCommit.
- No hidden state or external dependencies.

Specification-level determinism is realized
through structural constraints.

---

## 7.6 Numerical Stability Compliance

The architecture ensures:

- dt is finite and strictly positive.
- dt * max_rate < 1 is validated before updates.
- Overflow produces ERROR.
- Non-finite intermediate results produce ERROR.
- No invalid state is ever committed.

Numerical safety precedes mutation.

---

## 7.7 Parameter Compliance

The architecture enforces:

- Validation during initialization.
- Strict positivity constraints.
- Immutability during lifecycle.
- No runtime parameter mutation.
- No hidden coefficient adjustment.

ParameterSet is treated as read-only during evolution.

---

## 7.8 Minimal Ontology Compliance

The architecture maintains minimal persistent ontology:

    StructuralState = (Phi, Memory, Kappa)

Delta is transient input only.

No additional structural coordinates are introduced.

No hidden ontological entities exist at the architectural level.

---

## 7.9 Non-Compliance Conditions

An implementation violates Design V2.4 if it:

- Reorders update stages.
- Allows parameter mutation during lifecycle.
- Allows recovery after collapse.
- Introduces alternative dynamic equations.
- Commits non-finite values.
- Mutates state on ERROR.
- Introduces hidden structural state.
- Expands persistent ontology beyond defined state.

Any of the above constitutes architectural non-conformance.

---

## 7.10 Design Authority Statement

This Design Document V2.4 is fully aligned
with MAX-Core Specification V2.4.

The Specification defines normative mathematics.
This Design defines structural realization.

No contradictions are permitted between the two.

Design V2.4 supersedes all prior architectural versions.
