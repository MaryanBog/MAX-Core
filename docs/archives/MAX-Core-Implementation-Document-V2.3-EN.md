# MAX-Core Implementation Document
## Version 2.3
### Canonical Deterministic Living Structural Runtime

---

# 1. Implementation Overview

## 1.1 Purpose

This document defines the concrete implementation rules
for MAX-Core V2.3.

It translates:

- MAX-Core Specification V2.3 (normative mathematics)
- MAX-Core Design Document V2.3 (architectural structure)

into an executable deterministic runtime.

This document defines:

- Concrete data layout
- Execution pipeline realization
- Function boundaries
- Error handling rules
- Determinism constraints
- Atomic commit implementation
- Numerical safety enforcement

---

## 1.2 Scope of Implementation

The implementation is responsible exclusively for:

- Maintaining StructuralState
- Maintaining LifecycleContext
- Executing canonical update equations
- Enforcing invariants
- Detecting collapse
- Performing atomic commit
- Returning deterministic EventFlag

The implementation MUST NOT:

- Modify ParameterSet during lifecycle
- Introduce stochastic behavior
- Implement alternative dynamics
- Implement multi-structure logic
- Implement observer logic
- Implement external environment logic

The runtime implements exactly one canonical structural system.

---

## 1.3 Execution Model

The implementation is:

- Single-threaded by contract
- Deterministic
- IEEE-754 double precision
- Strictly ordered
- Free of hidden mutable global state

All mutation MUST occur inside:

    Step(const Delta&, double dt)

No other function may mutate StructuralState.

---

## 1.4 Architectural Alignment

The implementation MUST:

- Follow the execution order defined in Design V2.3
- Implement equations defined in Specification V2.3
- Preserve invariant guarantees
- Preserve collapse semantics
- Preserve lifecycle semantics

No legacy decay-only logic is permitted.

---

## 1.5 Implementation Constraints

The implementation MUST:

- Avoid dynamic memory allocation inside Step()
- Avoid undefined behavior
- Avoid fast-math optimizations
- Validate inputs before mutation
- Validate numerical stability before update
- Enforce atomic commit before exposure

---

## 1.6 Determinism Requirement

Given identical:

- Initial state
- ParameterSet
- Delta sequence
- dt sequence
- Compiler and floating-point environment

The implementation MUST produce bit-identical outputs.

Determinism is a structural requirement, not optional behavior.

---

# 2. Concrete Data Structures (C++ Layout)

This section defines the concrete C++ data layout for MAX-Core V2.3.
The layout MUST remain minimal, deterministic, and free of hidden state.

## 2.1 StructuralState

StructuralState stores the mutable structural coordinates.

struct StructuralState {
    double phi;
    double memory;
    double kappa;
};

Properties:
- Plain old data (POD).
- No virtual functions.
- No dynamic allocation.
- No hidden fields.
- No cached derived metrics.
- All fields MUST remain finite.

Delta is not stored persistently.
Delta is passed per Step() call.

## 2.2 LifecycleContext

LifecycleContext stores runtime metadata.

struct LifecycleContext {
    uint64_t step_counter;
    bool terminal;
    bool collapse_emitted;
};

Rules:
- terminal == (state.kappa == 0)
- collapse_emitted becomes true only once.
- step_counter increments only on successful commit.

No additional lifecycle flags are permitted.

## 2.3 ParameterSet

ParameterSet defines canonical coefficients.

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

Constraints:
- All values MUST be finite.
- All required positivity constraints MUST hold.
- Structure MUST be immutable after initialization.
- No internal mutation allowed during lifecycle.

## 2.4 Working State Buffer

Step() MUST use a local working copy.

StructuralState next_state;

Rules:
- next_state is initialized from current_state.
- current_state MUST NOT be modified until commit.
- All canonical equations operate on next_state.

## 2.5 Core Engine Class

A minimal implementation class MAY be:

class MaxCore {
public:
    explicit MaxCore(const ParameterSet& params);
    EventFlag Step(const std::vector<double>& delta_input, double dt);
    const StructuralState& CurrentState() const;
    const StructuralState& PreviousState() const;
    const LifecycleContext& Lifecycle() const;
private:
    ParameterSet params_;
    StructuralState current_;
    StructuralState previous_;
    LifecycleContext lifecycle_;
};

Requirements:
- params_ is immutable after construction.
- current_ and previous_ are mutated only in Step().
- No hidden static state.
- No global variables.

## 2.6 EventFlag Definition

enum class EventFlag {
    NORMAL,
    COLLAPSE,
    ERROR
};

EventFlag MUST be returned deterministically.

## 2.7 Memory Layout Guarantees

The implementation SHOULD:
- Avoid padding-dependent behavior.
- Avoid reinterpret_cast tricks.
- Avoid non-deterministic container iteration.
- Avoid platform-specific undefined behavior.

The state layout MUST remain stable under identical compiler configuration.

## 2.8 No Additional Hidden Structures

The implementation MUST NOT introduce:
- Hidden accumulators.
- Implicit damping buffers.
- Implicit history arrays.
- Adaptive parameter adjustments.
- Stochastic noise terms.

All structural evolution is fully captured by:
StructuralState
ParameterSet
LifecycleContext

---

### 2.x Core Engine Class Definition

class MaxCore {
private:
    ParameterSet params_;          // immutable after initialization
    size_t delta_dim_;             // immutable Delta dimensionality

    StructuralState current_;
    StructuralState previous_;
    LifecycleContext lifecycle_;

public:
    static Result<MaxCore> Create(
        const ParameterSet& params,
        size_t delta_dim
    );

    StepResult Step(
        const std::vector<double>& delta_input,
        double dt
    );
};

---

delta_dim_ MUST:

- be defined at initialization
- be strictly > 0
- remain constant during lifecycle
- be used to validate delta_input dimensionality at each Step()

---

# 3. Step() Implementation — Detailed Code-Level Pipeline

This section defines the exact code-level realization of Step()
for MAX-Core V2.3.

The execution order is mandatory.
No reordering is permitted.

---

## 3.1 Function Signature

EventFlag MaxCore::Step(const std::vector<double>& delta_input, double dt);

Step() is the only mutation authority.

---

## 3.2 Terminal Short-Circuit

If lifecycle_.terminal == true:

- Return EventFlag::NORMAL
- Do not increment step_counter
- Do not mutate state

Terminal state performs no evolution.

---

### 3.3 Stage 1 — Input Validation

Before any structural computation, the Step() function MUST validate:

1) Runtime initialization status
2) dt > 0
3) delta_input is finite
4) delta_input dimensionality matches configured dimension
5) StructuralState contains only finite values
6) ParameterSet remains valid

The configured Delta dimension (delta_dim) MUST be:

- defined at initialization
- stored as immutable runtime configuration
- constant throughout the lifecycle

If:

    delta_input.size() != delta_dim

Then:

- Step() MUST return ERROR
- No mutation MUST occur
- Lifecycle flags MUST remain unchanged
- step_counter MUST NOT increment

All validation MUST occur before any candidate state
is computed.

No partial computation is permitted prior to validation.

---

## 3.4 Numerical Stability Check

Compute:

double max_rate = std::max({
    params_.eta,
    params_.gamma,
    params_.rho,
    params_.lambda_phi,
    params_.lambda_m
});

If dt * max_rate >= 1.0:
- Return EventFlag::ERROR
- Perform no mutation

---

## 3.5 Local Working Copy

StructuralState next = current_;

No mutation of current_ is allowed before commit.

---

### 3.6 Stage 4 — Admissible Delta

After successful input validation and before canonical updates,
the Step() function MUST compute:

    Delta_step

Delta_step is the deterministic admissible excitation
used in canonical equations.

The Admissible Delta stage MUST:

1) Compute the squared Euclidean norm:

       norm2 = Σ (delta_i * delta_i)

   using double precision and deterministic accumulation order.

2) Verify that norm2 is finite.

3) Optionally enforce a deterministic norm guard:

       if (norm > Delta_max)
           Delta_step = delta_input * (Delta_max / norm)
       else
           Delta_step = delta_input

If norm guard is not enabled:

       Delta_step = delta_input

The Admissible Delta stage:

- MUST be deterministic
- MUST NOT mutate persistent StructuralState
- MUST NOT modify ParameterSet
- MUST NOT modify LifecycleContext
- MUST NOT introduce randomness

Delta_step exists only within the execution of Step().

Delta_step MUST NOT be stored persistently.

If norm2 or any intermediate value becomes non-finite:

- Step() MUST return ERROR
- No mutation MUST occur

---

## 3.7 Energy Update

next.phi =
    current_.phi
    + params_.alpha * norm2
    - params_.eta * current_.phi * dt;

If next.phi < 0.0:
    next.phi = 0.0;

If next.phi is non-finite:
    Return EventFlag::ERROR

## 3.8 Memory Update

next.memory =
    current_.memory
    + params_.beta * next.phi * dt
    - params_.gamma * current_.memory * dt;

If next.memory < 0.0:
    next.memory = 0.0;

If next.memory is non-finite:
    Return EventFlag::ERROR

## 3.9 Stability Update

next.kappa =
    current_.kappa
    + params_.rho * (params_.kappa_max - current_.kappa) * dt
    - params_.lambda_phi * next.phi * dt
    - params_.lambda_m * next.memory * dt;

If next.kappa < 0.0:
    next.kappa = 0.0;

If next.kappa > params_.kappa_max:
    next.kappa = params_.kappa_max;

If next.kappa is non-finite:
    Return EventFlag::ERROR

## 3.10 Invariant Verification

Verify:

next.phi >= 0
next.memory >= 0
0 <= next.kappa <= params_.kappa_max

If violation detected:
- Return EventFlag::ERROR
- Perform no mutation

## 3.11 Collapse Detection

EventFlag flag = EventFlag::NORMAL;

If current_.kappa > 0.0 and next.kappa == 0.0:
    flag = EventFlag::COLLAPSE;

## 3.12 Atomic Commit

previous_ = current_;
current_ = next;

lifecycle_.step_counter += 1;
lifecycle_.terminal = (current_.kappa == 0.0);

If flag == EventFlag::COLLAPSE:
    lifecycle_.collapse_emitted = true;

Return flag;

## 3.13 Mutation Guarantees

Mutation occurs only after:

- Validation
- Numerical stability check
- Canonical updates
- Invariant verification

No partial mutation is permitted.

If ERROR is returned:
- previous_ unchanged
- current_ unchanged
- lifecycle_ unchanged

---

# 4. Atomic Commit Implementation Details

This section defines the precise behavior and guarantees
of the atomic commit mechanism in MAX-Core V2.3.

Atomic commit is the only legal mutation boundary.

## 4.1 Logical Atomicity

Atomic commit MUST be logically indivisible.

This means:

- No externally observable intermediate state.
- No partial assignment.
- No lifecycle flag update before state update.
- No state update before invariant verification.

Atomicity is enforced structurally by ordering,
not by concurrency primitives.

The runtime is single-threaded by contract.

## 4.2 Commit Order

The commit sequence MUST be:

1) previous_ = current_
2) current_ = next
3) lifecycle_.step_counter += 1
4) lifecycle_.terminal = (current_.kappa == 0.0)
5) if collapse occurred → lifecycle_.collapse_emitted = true

This order MUST NOT be changed.

## 4.3 Collapse Emission Guarantee

Collapse is emitted exactly once.

Implementation rule:

If current_.kappa > 0.0 and next.kappa == 0.0:
    flag = COLLAPSE

After commit:

lifecycle_.collapse_emitted = true

collapse_emitted MUST remain true permanently
for the remainder of the lifecycle.

## 4.4 No Commit on ERROR

If Step() returns ERROR:

- previous_ MUST remain unchanged
- current_ MUST remain unchanged
- lifecycle_ MUST remain unchanged

Atomic commit MUST NOT execute.

## 4.5 Terminal State Enforcement

If lifecycle_.terminal == true before Step():

- No commit occurs.
- step_counter does not increment.
- StructuralState remains unchanged.

Terminal state is frozen state.

## 4.6 Previous State Integrity

previous_ MUST always contain the fully committed
state from the immediately preceding successful Step().

previous_ MUST NOT be modified:

- Before invariant validation
- On ERROR
- During terminal short-circuit

previous_ is updated only during atomic commit.

## 4.7 Deterministic Flag Update

Lifecycle flags MUST be derived only from:

- current_.kappa
- collapse detection condition

No additional hidden rules are permitted.

terminal MUST always satisfy:

terminal == (current_.kappa == 0.0)

## 4.8 No Partial Structural Exposure

The implementation MUST NOT expose next
through public interfaces.

Public accessors MUST reference current_ only.

No external code may observe:

- Partially updated state
- Intermediate values
- Working buffers

## 4.9 Structural Consistency Guarantee

After atomic commit:

- All invariants hold
- Lifecycle flags are consistent
- previous_ and current_ are synchronized
- The runtime is safe for the next Step()

Atomic commit is the structural integrity boundary.

---

# 5. Numerical Stability Enforcement in Code

This section defines how numerical stability constraints
are enforced at the implementation level in MAX-Core V2.3.

Numerical safety is mandatory and precedes mutation.

## 5.1 Stability Constraint

Before canonical updates, the implementation MUST verify:

dt * max_rate < 1.0

Where:

double max_rate = std::max({
    params_.eta,
    params_.gamma,
    params_.rho,
    params_.lambda_phi,
    params_.lambda_m
});

If dt * max_rate >= 1.0:

- Step() MUST return EventFlag::ERROR
- No mutation MUST occur

This prevents discrete-time divergence.

## 5.2 Finite Input Enforcement

Before update:

- All delta_input values MUST be finite.
- current_.phi MUST be finite.
- current_.memory MUST be finite.
- current_.kappa MUST be finite.
- All parameters MUST be finite.

Implementation pattern:

if (!std::isfinite(value)) return EventFlag::ERROR;

No silent substitution is permitted.

## 5.3 Energy Overflow Protection

During energy update:

next.phi =
    current_.phi
    + params_.alpha * norm2
    - params_.eta * current_.phi * dt;

After computation:

if (!std::isfinite(next.phi))
    return EventFlag::ERROR;

No overflow recovery is permitted.

## 5.4 Memory Overflow Protection

During memory update:

next.memory =
    current_.memory
    + params_.beta * next.phi * dt
    - params_.gamma * current_.memory * dt;

After computation:

if (!std::isfinite(next.memory))
    return EventFlag::ERROR;

No clamping beyond non-negativity is allowed.

## 5.5 Stability Overflow Protection

During stability update:

next.kappa =
    current_.kappa
    + params_.rho * (params_.kappa_max - current_.kappa) * dt
    - params_.lambda_phi * next.phi * dt
    - params_.lambda_m * next.memory * dt;

After computation:

if (!std::isfinite(next.kappa))
    return EventFlag::ERROR;

Then clamp:

if (next.kappa < 0.0)
    next.kappa = 0.0;

if (next.kappa > params_.kappa_max)
    next.kappa = params_.kappa_max;

## 5.6 Underflow Handling

Underflow toward zero is permitted,
provided the result remains finite.

Subnormal values are allowed
unless the compiler configuration disables them.

No artificial lower bounds are imposed.

## 5.7 No Silent Correction Policy

The implementation MUST NOT:

- Automatically reduce dt
- Automatically rescale Delta
- Automatically damp parameters
- Automatically adjust kappa_max

Any numerical violation results in ERROR.

## 5.8 Deterministic Error Behavior

Numerical errors MUST be:

- Deterministic
- Reproducible
- Non-destructive

The same inputs MUST always produce the same ERROR.

No stochastic fallback is permitted.

## 5.9 Post-Error Safety Guarantee

After ERROR:

- StructuralState remains unchanged
- LifecycleContext remains unchanged
- previous_ remains unchanged
- current_ remains unchanged

The runtime remains valid for subsequent calls.

Numerical stability enforcement precedes atomic commit.

---

# 6. Parameter Validation and Initialization Logic

This section defines how ParameterSet and initial state
are validated and how the runtime is initialized.

Initialization is deterministic and mandatory.

---

### 6.1 Initialization Interface

The runtime MUST NOT rely on constructor-based validation.

Initialization MUST be performed through an explicit factory
or initialization function that returns deterministic status.

Example:

    static Result<MaxCore> Create(
        const ParameterSet& params,
        size_t delta_dim
    );

Initialization MUST:

- Validate ParameterSet
- Validate delta_dim > 0
- Initialize StructuralState
- Initialize LifecycleContext

If validation fails:

- Initialization MUST return ERROR
- No partially constructed runtime instance is permitted

The public runtime contract MUST NOT throw exceptions.
All failures MUST be reported through explicit return values.

---

### 6.2 Parameter Validation

ParameterSet MUST be validated during initialization.

Validation MUST verify:

- All parameters are finite.
- All strictly positive parameters are > 0.
- kappa_max > 0.

If validation fails:

- Initialization MUST return ERROR.
- No runtime instance may be created.

ParameterSet becomes immutable after successful initialization.

---

## 6.3 Initial StructuralState

Initial StructuralState MUST be explicitly defined.

Default initialization MAY be:

current_.phi = 0.0;
current_.memory = 0.0;
current_.kappa = params_.kappa_max;

All initial values MUST be finite.

No random initialization is permitted.

Alternative initial states MAY be provided explicitly,
but MUST satisfy invariants.

## 6.4 Initial LifecycleContext

After initialization:

lifecycle_.step_counter = 0;
lifecycle_.terminal = (current_.kappa == 0.0);
lifecycle_.collapse_emitted = false;

If initial kappa == 0:

- Lifecycle begins in terminal state.

## 6.5 Immutability of ParameterSet

After construction:

params_ MUST NOT change.

Implementation rules:

- Do not expose non-const reference to params_
- Do not provide setter functions
- Do not allow runtime mutation

ParameterSet is read-only for the lifecycle duration.

## 6.6 Reinitialization Rules

If reinitialization is required:

- A new MaxCore instance MUST be created
OR
- A dedicated Reset() function MUST:
    - Validate parameters again
    - Reset StructuralState
    - Reset LifecycleContext

Reinitialization MUST NOT preserve:

- step_counter
- collapse_emitted
- terminal

Unless explicitly documented.

## 6.7 No Hidden Initialization Logic

Initialization MUST NOT:

- Auto-adjust parameters
- Auto-correct invalid inputs
- Inject implicit damping
- Modify coefficients
- Add hidden state

All initialization logic MUST be explicit and deterministic.

## 6.8 Initialization Safety Guarantee

After successful initialization:

- All invariants hold
- All parameters are valid
- Lifecycle flags are consistent
- Runtime is ready for deterministic Step() execution

No evolution occurs during initialization.

---

# 7. Deterministic Testing Hooks and Debug Safety

This section defines optional but recommended mechanisms
for deterministic testing, verification, and debug safety
in MAX-Core V2.3.

Testing facilities MUST NOT alter canonical behavior.

## 7.1 Deterministic State Access

The implementation MUST provide read-only accessors:

const StructuralState& CurrentState() const;
const StructuralState& PreviousState() const;
const LifecycleContext& Lifecycle() const;

Accessors MUST:

- Return const references
- Not expose mutable pointers
- Not allow state mutation
- Not perform hidden computations

Returned state MUST reflect only committed values.

## 7.2 Deterministic Snapshot Export

For testing purposes, the implementation MAY provide:

std::string SerializeState() const;

Serialization MUST:

- Include phi, memory, kappa
- Include step_counter
- Include terminal flag
- Include collapse_emitted flag
- Use deterministic formatting
- Avoid locale-dependent formatting

Serialization MUST NOT mutate state.

## 7.3 Compile-Time Debug Assertions

In debug builds, the implementation MAY assert:

- All invariants after commit
- All values finite before commit
- terminal == (current_.kappa == 0.0)
- collapse_emitted consistency

Assertions MUST NOT modify state.

Assertions MUST NOT alter runtime logic.

Release builds MUST preserve identical behavior
except for assertion removal.

## 7.4 Deterministic Logging (Optional)

If logging is implemented:

- Logging MUST NOT modify state
- Logging MUST NOT introduce timing dependencies
- Logging MUST NOT affect execution order
- Logging MUST NOT allocate memory inside Step() unless deterministic

Logging MUST be fully optional.

## 7.5 Regression Test Targets

The following deterministic scenarios SHOULD be testable:

1) Rest convergence:
   Delta = 0 repeatedly
   Expect phi → 0
   Expect memory → 0
   Expect kappa → kappa_max

2) Constant moderate load:
   Delta constant
   Expect convergence to equilibrium

3) Sustained excessive load:
   Delta large
   Expect collapse

4) Deterministic replay:
   Same input sequence
   Expect identical output sequence

Testing MUST not require modification of internal runtime logic.

## 7.6 No Testing Backdoors

The implementation MUST NOT include:

- Hidden testing-only mutation paths
- Test-only parameter overrides
- Hidden reset logic inside Step()
- Special-case behavior for test inputs

All testing must operate through the public interface.

## 7.7 Deterministic Build Configuration

For reproducibility, the build configuration SHOULD:

- Disable fast-math
- Use consistent optimization level
- Avoid architecture-dependent math transformations
- Use identical compiler flags for regression testing

Testing environment must be controlled.

## 7.8 Debug Safety Guarantee

Debug instrumentation MUST NOT:

- Change floating-point behavior
- Change execution order
- Change rounding behavior
- Introduce timing-dependent state

Debug and release builds MUST preserve canonical semantics.

Testing is observational, never mutational.

---

# 8. Implementation Compliance Criteria (V2.3)

This section defines the mandatory requirements
for an implementation to declare compliance with
MAX-Core Implementation Document V2.3.

All previous sections are normative.

## 8.1 Canonical Equation Compliance

A compliant implementation MUST:

- Implement the exact canonical equations defined in Specification V2.3.
- Preserve update order: Energy → Memory → Stability.
- Use Phi_next in Memory update.
- Use Phi_next and Memory_next in Stability update.
- Allow Kappa to increase or decrease.
- Clamp only as specified.
- Not introduce additional damping terms.

Any modification of equations constitutes non-compliance.

## 8.2 Mutation Authority Compliance

A compliant implementation MUST:

- Mutate StructuralState only inside Step().
- Use atomic commit as the only mutation boundary.
- Prevent partial mutation.
- Prevent state mutation on ERROR.
- Prevent state mutation in terminal state.

No alternative mutation path is permitted.

## 8.3 Collapse Semantics Compliance

A compliant implementation MUST:

- Define collapse strictly as Kappa == 0.
- Emit COLLAPSE exactly once.
- Prevent regeneration after collapse.
- Keep terminal state immutable.

Collapse MUST NOT depend on Phi or Memory thresholds.

## 8.4 Determinism Compliance

A compliant implementation MUST:

- Be single-threaded by contract.
- Avoid hidden mutable global state.
- Avoid stochastic components.
- Avoid non-deterministic containers.
- Enforce strict execution ordering.
- Preserve bitwise reproducibility under identical build conditions.

Any nondeterministic behavior constitutes non-compliance.

## 8.5 Numerical Stability Compliance

A compliant implementation MUST:

- Enforce dt * max_rate < 1.
- Reject non-finite input.
- Reject non-finite intermediate results.
- Reject overflow conditions.
- Never silently modify dt or parameters.
- Never silently rescale Delta.

Numerical violations MUST produce ERROR.

## 8.6 Parameter Integrity Compliance

A compliant implementation MUST:

- Validate ParameterSet at construction.
- Enforce positivity constraints.
- Prevent parameter mutation during lifecycle.
- Prevent hidden parameter adaptation.

ParameterSet MUST remain immutable.

## 8.7 Lifecycle Compliance

A compliant implementation MUST:

- Increment step_counter only on commit.
- Prevent step_counter increment on ERROR.
- Prevent step_counter increment in terminal state.
- Maintain terminal == (current_.kappa == 0).
- Maintain collapse_emitted consistency.

Lifecycle semantics MUST be state-driven.

## 8.8 Prohibited Behaviors

The following behaviors are explicitly prohibited:

- Monotonic Kappa enforcement.
- Functional normative layers.
- Decay-only stability logic.
- Alternative collapse criteria.
- Runtime parameter tuning.
- Adaptive stabilization.
- Hidden structural history arrays.
- Recovery from collapse.
- State mutation outside atomic commit.

Presence of any prohibited behavior voids compliance.

## 8.9 Conformance Declaration

An implementation MAY declare:

"Compliant with MAX-Core V2.3"

only if all mandatory requirements are satisfied.

Partial compliance is not permitted.

Compliance must be binary.

## 8.10 Final Implementation Guarantee

A fully compliant MAX-Core V2.3 implementation guarantees:

- Deterministic structural evolution.
- Regenerative stability dynamics.
- Strict invariant preservation.
- Irreversible collapse semantics.
- Reproducible execution.
- Numerical safety before mutation.

This completes the Implementation Document V2.3.
