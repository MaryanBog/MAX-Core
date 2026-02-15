# MAX-Core Specification V2.5
## Canonical Deterministic Living Structural Runtime

---

## 1. Scope and Objective

MAX-Core V2.5 defines a strictly deterministic structural runtime implementing the canonical living structural system:

S = (Phi, Memory, Kappa)

This specification is normative and authoritative.

MAX-Core V2.5:

- Implements regenerative stability dynamics.
- Preserves strict structural invariants.
- Defines collapse exclusively as Kappa == 0.
- Is single-threaded by architectural contract.
- Guarantees deterministic evolution under identical inputs and floating-point environment.
- Enforces logically atomic state commits.
- Prohibits hidden mutation, implicit state transitions, or alternative dynamic paths.
- Defines exactly one canonical Step() operator.
- Defines exactly one deterministic initialization interface.

MAX-Core is NOT:

- A probabilistic engine.
- A stochastic simulator.
- A machine learning runtime.
- A multi-threaded system.
- An optimization framework.
- A control system.
- An observer layer.
- An entanglement engine.

MAX-Core implements canonical structural evolution only.

This specification strictly defines:

- Persistent structural state
- Transient structural input
- ParameterSet definition
- Canonical update equations
- Numerical stability constraints
- Structural invariants
- Collapse semantics
- Lifecycle rules
- Determinism guarantees
- Error isolation rules
- Atomic commit semantics

No deviation from this specification is permitted if compliance with MAX-Core V2.5 is claimed.

---

## 2. Structural State Model

MAX-Core V2.5 strictly distinguishes between:

1) Persistent Structural State  
2) Transient Step Input  

Only the persistent structural state exists between Step() calls.

---

### 2.1 Persistent Structural State

The persistent structural state is defined as:

S = (Phi, Memory, Kappa)

Where:

- Phi     ∈ ℝ, Phi ≥ 0
- Memory  ∈ ℝ, Memory ≥ 0
- Kappa   ∈ ℝ, 0 ≤ Kappa ≤ kappa_max

The runtime MUST maintain:

- S_current
- S_previous
- LifecycleContext

Only S_current and S_previous are committed structural states.

Delta is NOT part of the persistent structural state.

No additional structural coordinates are permitted.

---

### 2.2 Transient Structural Input (Delta)

Delta represents external structural excitation supplied to Step().

Properties:

- Delta MUST be finite.
- Delta MUST have fixed dimensionality within a lifecycle.
- Delta MUST NOT be stored persistently.
- Delta MUST exist only during execution of Step().

Delta influences the system only through canonical updates of:

- Phi
- Memory
- Kappa

---

### 2.3 Structural State Storage Requirements

The runtime MUST store:

- S_current
- S_previous
- LifecycleContext
- ParameterSet (immutable)
- delta_dim (immutable)

Where:

delta_dim > 0

delta_dim MUST be defined at initialization and MUST remain constant during the lifecycle.

delta_dim is configuration, not structural state.

---

### 2.4 Mutation Authority

Persistent structural state MAY be mutated ONLY:

- inside Step()
- after successful validation
- through atomic commit

No external setter or direct mutation of S_current or S_previous is permitted.

---

### 2.5 Finite Representation

All structural coordinates MUST be:

- IEEE-754 double precision
- finite
- not NaN
- not ±Inf

If any non-finite value is detected during computation:

- Step() MUST return ERROR
- no mutation MUST occur
- S_current and S_previous MUST remain unchanged

---

## 3. ParameterSet

MAX-Core V2.5 requires a deterministic ParameterSet P,
provided at initialization and fixed for the entire lifecycle.

The ParameterSet defines the canonical living structural dynamics.

---

### 3.1 Parameter Structure

A conforming implementation MUST define:

struct ParameterSet {
    double alpha;
    double eta;
    double beta;
    double gamma;
    double rho;
    double lambda_phi;
    double lambda_m;
    double kappa_max;
};

No additional hidden coefficients are permitted.

---

### 3.2 Parameter Semantics

Each parameter has the following meaning:

- alpha        : energy sensitivity coefficient
- eta          : energy dissipation coefficient
- beta         : memory formation coefficient
- gamma        : memory decay coefficient
- rho          : stability regeneration coefficient
- lambda_phi   : stability load from energy
- lambda_m     : stability load from memory
- kappa_max    : maximum structural stability

All coefficients influence canonical update equations only as defined in this specification.

---

### 3.3 Parameter Constraints

The following constraints MUST hold:

- alpha        > 0
- eta          > 0
- beta         > 0
- gamma        > 0
- rho          > 0
- lambda_phi   > 0
- lambda_m     > 0
- kappa_max    > 0

Additionally:

- All parameters MUST be finite.
- No NaN or Inf values are permitted.

ParameterSet MUST be validated during initialization.

If validation fails:

- Initialization MUST fail.
- No runtime instance MUST be constructed.

---

### 3.4 Parameter Immutability

ParameterSet MUST:

- remain constant throughout a lifecycle
- NOT be modified by Step()
- NOT be modified implicitly
- be replaced only via explicit reinitialization

Runtime mutation of parameters during execution is prohibited.

---

### 3.5 Numerical Stability Constraint

For each Step(), the following MUST hold:

dt is finite  
dt > 0  

Let:

max_rate = max(
    eta,
    gamma,
    rho,
    lambda_phi,
    lambda_m
)

The condition:

dt * max_rate < 1

MUST be verified before canonical updates.

If violated:

- Step() MUST return ERROR
- no mutation MUST occur

---

### 3.6 Deterministic Parameter Usage

Given identical:

- ParameterSet
- initial state
- Delta sequence
- dt sequence

the runtime MUST produce identical outputs under identical floating-point environment.

No parameter-dependent randomness is permitted.

---

## 4. Initialization Interface

MAX-Core V2.5 requires explicit deterministic initialization.

Initialization MUST be performed before any call to Step().

---

### 4.1 Initialization Contract

Initialization MUST be explicit and deterministic.

The runtime MUST expose an initialization interface
that either:

- returns a fully constructed runtime instance, or
- returns a deterministic failure indicator.

Initialization failure reporting MUST be deterministic.

No partially initialized runtime instance is permitted.

---

### 4.2 Initial Structural State Requirements

The provided initial_state MUST satisfy:

Phi ≥ 0  
Memory ≥ 0  
0 ≤ Kappa ≤ kappa_max  

All values MUST be finite.

If any invariant is violated:

- Initialization MUST fail.
- No runtime instance MUST be created.

---

### 4.3 delta_dim Requirements

delta_dim MUST satisfy:

delta_dim > 0  

delta_dim MUST:

- be defined at initialization
- remain immutable during lifecycle
- define the dimensionality of Delta_input for all Step() calls

If delta_dim == 0:

- Initialization MUST fail.

---

### 4.4 Optional Norm Guard Configuration

An implementation MAY support an optional norm guard:

Delta_max

If provided:

- Delta_max MUST be finite
- Delta_max MUST be strictly positive
- Delta_max MUST remain immutable during lifecycle

If norm guard is not provided:

- Delta processing operates without magnitude restriction.

Delta_max is configuration only.
It is NOT part of StructuralState.

---

### 4.5 Lifecycle Initialization Rules

Upon successful initialization:

S_current   = initial_state  
S_previous  = initial_state  

step_counter      = 0  
terminal          = (Kappa == 0)  
collapse_emitted  = false  

If Kappa == 0 at initialization:

- The lifecycle begins in terminal state.
- No collapse event is emitted.
- Step() will short-circuit.

---

### 4.6 Initialization Determinism

Given identical:

- ParameterSet
- delta_dim
- initial_state
- Delta_max (if provided)

Initialization MUST produce identical runtime state.

Initialization MUST NOT depend on:

- system time
- randomness
- global mutable state
- external environment

The runtime MUST be either fully valid or nonexistent after initialization.

---

## 5. Step Interface and Execution Contract

MAX-Core V2.5 exposes a single canonical evolution operator:

EventFlag Step(
    const double* delta_input,
    size_t delta_len,
    double dt
);

Step() is the only mutation authority of the persistent structural state.

---

### 5.1 Pre-Execution Requirements

Before execution, the following MUST hold:

- Runtime is successfully initialized.
- delta_input is not null.
- delta_len == delta_dim.
- dt is finite.
- dt > 0.
- All delta_input[i] for i in [0, delta_len) are finite.
- S_current contains only finite values.
- ParameterSet contains only finite values.

If any condition is violated:

- Step() MUST return ERROR.
- No mutation MUST occur.
- LifecycleContext MUST remain unchanged.

---

### 5.2 Terminal Short-Circuit Rule

If:

S_current.Kappa == 0

Then:

- Step() MUST perform no mutation.
- step_counter MUST NOT increment.
- LifecycleContext MUST remain unchanged.
- Step() MUST return NORMAL.

Terminal state is irreversible within a lifecycle.

---

### 5.3 Execution Order (Normative)

Terminal short-circuit defined in Section 5.2
MUST be evaluated before Stage 1.

If S_current.Kappa == 0:

- Step() MUST immediately return NORMAL.
- No input validation is performed.
- No numerical stability check is performed.
- No candidate state creation is performed.
- No Delta processing is performed.
- No canonical update is performed.
- No invariant enforcement is performed.
- No collapse detection is performed.
- No atomic commit occurs.
- step_counter MUST NOT increment.
- LifecycleContext MUST remain unchanged.

For non-terminal states, Step() MUST execute strictly in the following order:

1. Input validation.
2. Numerical stability check.
3. Local candidate state creation (S_next = S_current).
4. Admissible Delta processing.
5. Canonical Energy update.
6. Canonical Memory update.
7. Canonical Stability update.
8. Invariant enforcement.
9. Collapse detection.
10. Atomic commit.
11. Return EventFlag.

No reordering of these stages is permitted.

---

### 5.4 EventFlag Definition

The return type MUST be:

enum class EventFlag {
    NORMAL,
    COLLAPSE,
    ERROR
};

Semantics:

- NORMAL   → successful evolution without collapse.
- COLLAPSE → first transition where Kappa becomes zero.
- ERROR    → validation or numerical failure; no mutation performed.

EventFlag MUST be deterministic.

---

### 5.5 Structural Time Definition

Structural time is defined as:

t_structural = step_counter

step_counter MUST increment only upon successful atomic commit.

Structural time MUST NOT advance when:

- Step() returns ERROR.
- Step() is short-circuited in terminal state.

Structural time is discrete and deterministic.

---

### 5.6 Deterministic Execution Guarantee

Given identical:

- S_current
- ParameterSet
- delta_input sequence
- delta_len
- dt sequence
- floating-point environment

Step() MUST produce identical:

- S_current sequence
- S_previous sequence
- LifecycleContext sequence
- EventFlag sequence
- step_counter sequence.

---

## 6. Admissible Delta Processing

Delta represents external structural excitation applied at each step.

Before canonical updates, Delta_input MUST be transformed into:

Delta_step

Delta_step is the only excitation permitted to influence Phi.

Delta_step MUST exist only within the execution of Step().

---

### 6.1 Finite Validation

Before any computation:

- Every component of delta_input MUST be finite.
- No NaN or Inf values are permitted.

If any component is non-finite:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 6.2 Dimensional Consistency

delta_len MUST equal delta_dim.

If dimensionality mismatch occurs:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 6.3 Norm Definition

The squared Euclidean norm is defined as:

norm2 = Σ (delta_i * delta_i)

The accumulation order MUST be deterministic.

norm2 MUST be computed in IEEE-754 double precision.

If norm2 becomes non-finite:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 6.4 Optional Norm Guard

If norm guard is enabled:

||Delta_step|| ≤ Delta_max

The following MUST hold:

- Delta_max MUST be finite.
- Delta_max MUST be strictly positive.
- Delta_max MUST be immutable during lifecycle.

If norm2 > Delta_max^2:

Delta_step MUST be deterministically scaled:

scale = Delta_max / sqrt(norm2)

For all i:

Delta_step[i] = delta_input[i] * scale

Scaling MUST:

- Preserve direction.
- Never increase magnitude.
- Be deterministic.
- Not introduce hidden state.

If norm guard is disabled:

Delta_step = delta_input

---

### 6.5 No Persistence

Delta_step:

- MUST NOT be stored persistently.
- MUST NOT modify StructuralState directly.
- MUST influence state only through canonical equations.

Delta_step MUST NOT affect LifecycleContext directly.

---

### 6.6 Determinism Requirement

Delta processing MUST:

- be deterministic,
- not introduce randomness,
- not depend on system time,
- not depend on external environment,
- not mutate ParameterSet,
- not mutate LifecycleContext.

All influence of Delta MUST be explicit and local to Step().

---

## 7. Canonical Energy Update

Energy (Phi) represents instantaneous structural excitation.

Phi MUST evolve according to the canonical dissipative equation.

---

### 7.1 Normative Update Equation

For each non-terminal step:

Phi_next =
    Phi_current
    + alpha * norm2
    - eta   * Phi_current * dt

Where:

- norm2 is computed from Delta_step.
- alpha > 0
- eta   > 0
- dt    > 0

Phi_next MUST be computed in IEEE-754 double precision.

---

### 7.2 Non-Negativity Enforcement

After computation:

Phi_next = max(0, Phi_next)

Negative values are not permitted in committed state.

Clamping MUST be deterministic.

---

### 7.3 Finite Enforcement

If Phi_next becomes:

- NaN
- +Inf
- -Inf

Then:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 7.4 Deterministic Requirement

Phi update MUST:

- depend only on Phi_current, alpha, eta, norm2, and dt,
- not depend on hidden global state,
- not use stochastic behavior,
- not depend on system time.

Given identical inputs, Phi_next MUST be identical bit-for-bit
under identical floating-point environment.

---

### 7.5 Exclusive Mutation Rule

Phi MUST NOT be modified outside Step().

No setter, background adjustment, smoothing,
or implicit mutation is permitted.

All changes to Phi MUST occur exclusively
through the canonical update pipeline.

---

## 8. Canonical Memory Update

Memory represents accumulated structural load derived from energy.

Memory MUST evolve according to the canonical formation–decay equation.

---

### 8.1 Normative Update Equation

For each non-terminal step:

Memory_next =
    Memory_current
    + beta  * Phi_next   * dt
    - gamma * Memory_current * dt

Where:

- beta  > 0
- gamma > 0
- dt    > 0
- Phi_next is computed in Section 7

Memory_next MUST be computed in IEEE-754 double precision.

---

### 8.2 Non-Negativity Enforcement

After computation:

Memory_next = max(0, Memory_next)

Negative values are not permitted in committed state.

Clamping MUST be deterministic.

---

### 8.3 Finite Enforcement

If Memory_next becomes:

- NaN
- +Inf
- -Inf

Then:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 8.4 Deterministic Requirement

Memory update MUST:

- depend only on Memory_current, Phi_next, beta, gamma, and dt,
- not depend on hidden global state,
- not use stochastic behavior,
- not depend on system time.

Given identical inputs, Memory_next MUST be identical bit-for-bit
under identical floating-point environment.

---

### 8.5 Exclusive Mutation Rule

Memory MUST NOT be modified outside Step().

No implicit accumulation, smoothing,
or hidden integrator is permitted.

All changes to Memory MUST occur exclusively
through the canonical update pipeline.

---

## 9. Canonical Stability Update

Stability (Kappa) represents structural viability.

Kappa MUST evolve according to the canonical regenerative load-sensitive equation.

---

### 9.1 Normative Update Equation

For each non-terminal step:

Kappa_next =
    Kappa_current
    + rho * (kappa_max - Kappa_current) * dt
    - lambda_phi * Phi_next    * dt
    - lambda_m   * Memory_next * dt

Where:

- rho         > 0
- lambda_phi  > 0
- lambda_m    > 0
- kappa_max   > 0
- dt          > 0
- Phi_next and Memory_next are defined in Sections 7 and 8

Kappa_next MUST be computed in IEEE-754 double precision.

---

### 9.2 Bounds Enforcement

After computation:

Kappa_next = max(0, Kappa_next)
Kappa_next = min(Kappa_next, kappa_max)

The invariant:

0 ≤ Kappa ≤ kappa_max

MUST hold at every committed state.

Clamping MUST be deterministic.

---

### 9.3 Finite Enforcement

If Kappa_next becomes:

- NaN
- +Inf
- -Inf

Then:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 9.4 Collapse Detection Rule

Collapse occurs if and only if:

Kappa_next == 0

Collapse detection MUST occur after bounds enforcement
and before atomic commit.

If:

Kappa_current > 0
and
Kappa_next == 0

Then:

- EventFlag MUST be COLLAPSE.
- collapse_emitted MUST be set to true during atomic commit.
- terminal MUST be set to true during atomic commit.

Collapse MUST be emitted exactly once per lifecycle.

---

### 9.5 No Recovery After Collapse

If:

S_current.Kappa == 0

Then:

- Stability update MUST NOT execute.
- No regeneration is permitted.
- No structural coordinate may change.

Terminal state is irreversible within a lifecycle.

---

### 9.6 Deterministic Requirement

Kappa update MUST:

- depend only on Kappa_current, Phi_next, Memory_next, parameters, and dt,
- not depend on hidden global state,
- not use stochastic behavior,
- not depend on system time.

Given identical inputs, Kappa_next MUST be identical bit-for-bit
under identical floating-point environment.

---

## 10. Structural Invariants

The following invariants MUST hold at every committed structural state.

Invariant enforcement MUST occur before atomic commit.

---

### 10.1 Energy Invariant

Phi ≥ 0

Phi MUST be finite and non-negative at all committed states.

---

### 10.2 Memory Invariant

Memory ≥ 0

Memory MUST be finite and non-negative at all committed states.

---

### 10.3 Stability Bounds

0 ≤ Kappa ≤ kappa_max

Kappa MUST remain within the closed interval
at all committed states.

---

### 10.4 Finite Representation

All structural coordinates MUST be:

- finite
- IEEE-754 double precision
- not NaN
- not ±Inf

If any coordinate becomes non-finite during computation:

- Step() MUST return ERROR.
- No mutation MUST occur.
- S_current MUST remain unchanged.
- S_previous MUST remain unchanged.

---

### 10.5 Minimal Persistent Ontology

The runtime MUST NOT maintain hidden structural variables
that influence Phi, Memory, or Kappa.

All structural influence MUST be expressible through:

S_current  
ParameterSet  
delta_input  
dt  

No implicit accumulators, cached correction terms,
or alternative state channels are permitted.

---

### 10.6 Deterministic Invariant Enforcement

Invariant enforcement MUST:

- be deterministic,
- operate on candidate state only,
- occur before atomic commit,
- not introduce randomness.

No silent correction beyond explicitly defined clamping
(max(0, ...), min(..., kappa_max)) is permitted.

---

### 10.7 Post-Commit Guarantee

After atomic commit, the committed state MUST satisfy:

Phi ≥ 0  
Memory ≥ 0  
0 ≤ Kappa ≤ kappa_max  
all values finite  

Any violation constitutes non-compliance
with MAX-Core V2.5.

---

## 11. Atomic Commit and Lifecycle Semantics

Atomic commit defines the only valid mechanism
for mutating the persistent structural state S.

All structural transitions MUST occur through atomic commit.

---

### 11.1 Logical Atomicity

Atomic commit MUST be logically indivisible
within single-threaded execution.

Atomicity means:

- No partial mutation of S_current.
- No externally observable intermediate state.
- No mutation before invariant validation completes.

The runtime is single-threaded by contract.
Thread-safety, if required, is the responsibility of the caller.

---

### 11.2 Commit Procedure

For a successful non-terminal step:

1. Validate candidate state.
2. If validation fails → return ERROR (no mutation).
3. Copy S_current → S_previous.
4. Copy S_next → S_current.
5. Increment step_counter.
6. Update lifecycle flags deterministically.

All updates MUST occur in this exact order.

---

### 11.3 Lifecycle Flag Update Rules

Lifecycle flags MUST be updated only after successful atomic commit.

After commit:

terminal = (S_current.Kappa == 0)

If EventFlag == COLLAPSE and collapse_emitted == false:

collapse_emitted = true

Otherwise:

collapse_emitted remains unchanged.

collapse_emitted MUST remain true permanently
after the first COLLAPSE event.

Lifecycle flags MUST NOT depend on pre-commit candidate values.

---

### 11.4 No Partial Commit

If any of the following occurs before commit:

- validation failure,
- invariant violation,
- numerical instability,
- non-finite value,
- stability constraint violation,

Then:

- S_current MUST remain unchanged.
- S_previous MUST remain unchanged.
- step_counter MUST remain unchanged.
- terminal MUST remain unchanged.
- collapse_emitted MUST remain unchanged.
- Step() MUST return ERROR.

---

### 11.5 Previous State Requirement

The runtime MUST retain:

S_previous

S_previous MUST contain the fully committed structural state
from the immediately preceding successful Step().

S_previous MUST NOT be modified outside atomic commit.

---

### 11.6 Deterministic Commit Guarantee

Given identical:

- initial state,
- ParameterSet,
- Delta sequence,
- dt sequence,
- floating-point environment,

The sequence of committed states MUST be identical bit-for-bit.

Atomic commit MUST NOT introduce any non-determinism.

---

## 12. Determinism and Reproducibility

MAX-Core V2.5 MUST be fully deterministic.

Determinism is a mandatory architectural and behavioral property.

---

### 12.1 Functional Determinism

Given identical:

- initial StructuralState,
- ParameterSet,
- delta_dim,
- sequence of delta_input values,
- sequence of dt values,
- floating-point environment,

The runtime MUST produce identical:

- S_current sequence,
- S_previous sequence,
- LifecycleContext sequence,
- EventFlag sequence,
- step_counter sequence.

No non-deterministic behavior is permitted.

---

### 12.2 Prohibited Sources of Non-Determinism

The implementation MUST NOT use:

- random number generators,
- system time,
- unordered containers with undefined iteration order,
- multi-threaded mutation,
- platform-dependent math behavior (unless strictly controlled),
- hidden global mutable state.

All computations MUST be explicit and ordered.

---

### 12.3 Floating-Point Discipline

The runtime MUST:

- use IEEE-754 double precision,
- avoid undefined floating-point behavior,
- avoid dependence on extended precision registers,
- not rely on compiler optimizations that alter numerical results.

Recommended constraints:

- disable fast-math,
- enforce strict IEEE compliance,
- avoid fused multiply-add unless consistent across builds.

---

### 12.4 Bitwise Reproducibility

Under identical:

- compiler,
- architecture,
- build flags,
- floating-point environment,

Repeated executions MUST produce bit-identical outputs.

Cross-platform bitwise identity is recommended but not mandatory.

---

### 12.5 Deterministic Error Semantics

If Step() returns ERROR under a given input state,
the same ERROR MUST occur under identical conditions.

ERROR behavior MUST:

- not mutate state,
- not modify lifecycle flags,
- not introduce randomness.

Error handling MUST be deterministic.

---

### 12.6 Deterministic Ordering Guarantee

All internal operations of Step() MUST follow
the execution order defined in Section 5.

Reordering canonical update stages is not permitted.

Determinism is structurally enforced,
not validated post hoc.

---

## 13. Numerical Stability and Error Conditions

MAX-Core V2.5 MUST enforce numerical safety
to prevent divergence, overflow, or undefined behavior.

All numerical checks MUST occur before atomic commit.

---

### 13.1 Stability Condition

For each Step(), the implementation MUST verify:

dt is finite  
dt > 0  

Let:

max_rate = max(
    eta,
    gamma,
    rho,
    lambda_phi,
    lambda_m
)

The condition:

dt * max_rate < 1

MUST be satisfied.

If violated:

- Step() MUST return ERROR.
- No mutation MUST occur.
- LifecycleContext MUST remain unchanged.

---

### 13.2 Finite Arithmetic Requirement

All intermediate and final values during computation MUST remain finite.

If at any point:

- norm2 becomes non-finite,
- Phi_next becomes non-finite,
- Memory_next becomes non-finite,
- Kappa_next becomes non-finite,

Then:

- Step() MUST return ERROR.
- No mutation MUST occur.

---

### 13.3 Overflow Handling

The implementation MUST detect overflow conditions.

If overflow occurs in:

- norm computation,
- multiplication,
- accumulation,
- update equations,

Then:

- Step() MUST return ERROR.
- No mutation MUST occur.

Silent overflow is prohibited.

---

### 13.4 Underflow Handling

Underflow toward zero is permitted,
provided that the result remains finite.

Subnormal values are allowed unless explicitly disabled
by the floating-point environment.

Underflow MUST NOT trigger ERROR
if invariants remain satisfied.

---

### 13.5 Initialization Errors

Initialization MUST fail if:

- ParameterSet contains non-finite values,
- positivity constraints are violated,
- delta_dim == 0,
- initial StructuralState violates invariants.

No partially initialized runtime instance is permitted.

---

### 13.6 Error Isolation Guarantee

If Step() returns ERROR:

- S_current MUST remain unchanged.
- S_previous MUST remain unchanged.
- step_counter MUST remain unchanged.
- terminal MUST remain unchanged.
- collapse_emitted MUST remain unchanged.

The runtime MUST remain valid and usable after ERROR.

---

### 13.7 No Silent Correction

The implementation MUST NOT:

- silently modify dt,
- silently modify ParameterSet,
- silently rescale Delta unless norm guard is explicitly enabled,
- silently adjust structural coordinates beyond defined clamping.

All corrective behavior MUST be explicitly defined
and deterministic.

---

### 13.8 Separation of ERROR and COLLAPSE

ERROR and COLLAPSE are distinct outcomes:

- ERROR indicates validation or numerical violation.
- COLLAPSE indicates Kappa reached zero under canonical dynamics.

ERROR MUST NOT:

- trigger collapse,
- alter Kappa,
- alter terminal flag,
- alter collapse_emitted flag.

Collapse MUST NOT be masked as ERROR.

---

## 14. Compliance and Conformance Requirements

This section defines the mandatory conditions
an implementation MUST satisfy
to claim compliance with MAX-Core V2.5.

---

### 14.1 Structural Compliance

A conforming implementation MUST:

- Implement persistent state exactly as:
  S = (Phi, Memory, Kappa)
- Maintain S_current and S_previous.
- Maintain LifecycleContext.
- Enforce all invariants defined in Section 10.
- Follow canonical update equations exactly as defined in Sections 7–9.
- Use Admissible Delta processing as defined in Section 6.

No alternative dynamics are permitted.

---

### 14.2 Execution Order Compliance

A conforming implementation MUST execute Step()
in the exact order defined in Section 5.3:

1. Input validation
2. Numerical stability check
3. Local candidate state creation
4. Admissible Delta processing
5. Canonical Energy update
6. Canonical Memory update
7. Canonical Stability update
8. Invariant enforcement
9. Collapse detection
10. Atomic commit
11. Return EventFlag

Reordering of these stages constitutes non-compliance.

---

### 14.3 Determinism Compliance

An implementation MUST demonstrate:

- Identical output for identical input sequences.
- No stochastic behavior.
- No time-dependent behavior.
- No hidden mutable global state.
- Single-threaded mutation authority.

Determinism MUST be structural, not accidental.

---

### 14.4 Error Handling Compliance

The implementation MUST:

- Return ERROR for all validation failures.
- Not mutate state on ERROR.
- Preserve S_current and S_previous on ERROR.
- Preserve LifecycleContext on ERROR.
- Keep runtime usable after ERROR.

ERROR MUST be deterministic.

---

### 14.5 Collapse Compliance

An implementation MUST:

- Detect collapse only when Kappa_next == 0.
- Emit COLLAPSE exactly once per lifecycle.
- Set terminal = true after collapse.
- Prevent further mutation after terminal.
- Prevent recovery within lifecycle.

Failure to enforce irreversible collapse
constitutes non-compliance.

---

### 14.6 Numerical Compliance

The implementation MUST:

- Use IEEE-754 double precision.
- Avoid undefined behavior.
- Detect non-finite values.
- Enforce dt stability constraint.
- Avoid silent overflow.
- Avoid hidden corrections beyond defined clamping.

---

### 14.7 Interface Compliance

A conforming runtime MUST expose:

- Explicit initialization interface.
- Explicit Step() function with signature defined in Section 5.
- Deterministic EventFlag return.
- Read-only access to S_current.
- Read-only access to S_previous.
- Read-only access to LifecycleContext.

Implicit initialization or hidden state mutation is prohibited.

---

### 14.8 Version Identification

A conforming implementation MUST clearly identify itself as:

MAX-Core V2.5

If any behavior deviates from this specification,
the implementation MUST NOT claim compliance
with MAX-Core V2.5.

---

## 15. Automatic Fresh Genesis (Optional)

MAX-Core V2.5 defines exactly one finite structural lifecycle.

Collapse terminates the current lifecycle irreversibly.
Within a lifecycle, no recovery is permitted.

Automatic Fresh Genesis is an optional external orchestration policy.
It is not part of the canonical Step() operator.

An external orchestration layer MAY implement Automatic Fresh Genesis
for continuous processing environments.

Automatic Fresh Genesis MUST satisfy the following rules:

Trigger Condition:

If and only if Step() returns EventFlag::COLLAPSE,
the current lifecycle is considered complete.

Immutability Requirement:

The final committed structural state of the completed lifecycle:

- MUST remain immutable.
- MUST NOT be modified.
- MUST NOT be reused as active state.
- MUST remain historically preserved.

Reinstantiation Requirement:

A new MAX-Core instance MUST be constructed
through the canonical initialization interface.

The new instance MUST be initialized with:

Phi    = 0  
Memory = 0  
Kappa  = kappa_max  

LifecycleContext for the new instance MUST satisfy:

step_counter     = 0  
terminal         = false  
collapse_emitted = false  

Isolation Requirement:

Automatic Fresh Genesis MUST:

- NOT modify the committed collapsed instance.
- NOT revive a collapsed lifecycle.
- NOT bypass collapse detection.
- NOT alter canonical equations.
- NOT introduce hidden state transfer.
- NOT mutate ParameterSet implicitly.

Each lifecycle MUST remain:

- finite,
- deterministic,
- ontologically independent.

Automatic Fresh Genesis establishes
a sequence of independent MAX-Core lifecycles
managed externally to the canonical engine.
