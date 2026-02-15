# MAX-Core Implementation Document
## Version 2.5
### Deterministic Canonical Living Structural Runtime

---

# 1. Implementation Overview

## 1.1 Purpose

This document defines the concrete C++ realization
of MAX-Core Specification V2.5 and
MAX-Core Design Document V2.5.

The implementation MUST:

- Realize persistent state S = (Phi, Memory, Kappa)
- Enforce strict execution ordering exactly as defined in Design V2.5
- Enforce numerical safety before mutation
- Enforce deterministic atomic commit
- Prevent any mutation on ERROR
- Prevent any mutation in terminal state
- Emit collapse exactly once per lifecycle
- Guarantee bitwise reproducibility under identical environment

This implementation describes a single-threaded,
strict IEEE-754 double precision runtime.

No deviation from Specification V2.5 is permitted.

---

## 1.2 Persistent Ontology

The only persistent structural state is:

    struct StructuralState {
        double phi;
        double memory;
        double kappa;
    };

No additional structural coordinates are permitted.

The implementation MUST NOT store:

- cached derived metrics
- auxiliary correction terms
- historical Delta values
- hidden accumulators
- alternative stability variables

Delta is transient input and MUST NEVER be stored persistently.

Persistent state MUST contain only:

    phi
    memory
    kappa

Minimal ontology is mandatory.

---

## 1.3 Lifecycle Context

Lifecycle metadata:

    struct LifecycleContext {
        uint64_t step_counter;
        bool terminal;
        bool collapse_emitted;
    };

Lifecycle invariants:

    terminal == (current_.kappa == 0.0)

collapse_emitted:

    false before first collapse
    true permanently after first collapse

step_counter:

    increments only after successful atomic commit
    does NOT increment on ERROR
    does NOT increment in terminal state

LifecycleContext MUST be mutated only during AtomicCommit.

---

## 1.4 ParameterSet

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

All parameters MUST be:

- finite
- strictly positive
- validated during initialization

ParameterSet is immutable after successful initialization.

No runtime parameter mutation is permitted.

---

## 1.5 Core Class Definition

class MaxCore {
public:
    static std::optional<MaxCore> Create(
        const ParameterSet& params,
        size_t delta_dim,
        const StructuralState& initial_state,
        std::optional<double> delta_max = std::nullopt
    );

    EventFlag Step(
        const double* delta_input,
        size_t delta_len,
        double dt
    );

    const StructuralState& Current() const noexcept;
    const StructuralState& Previous() const noexcept;
    const LifecycleContext& Lifecycle() const noexcept;

private:
    ParameterSet params_;
    size_t delta_dim_;
    std::optional<double> delta_max_;

    StructuralState current_;
    StructuralState previous_;
    LifecycleContext lifecycle_;

    explicit MaxCore(
        const ParameterSet& params,
        size_t delta_dim,
        const StructuralState& initial_state,
        std::optional<double> delta_max
    );
};

Step() is the only mutation authority.

No other method may modify:

    current_
    previous_
    lifecycle_

---

## 1.6 EventFlag Definition

    enum class EventFlag {
        NORMAL,
        COLLAPSE,
        ERROR
    };

Semantics:

NORMAL
    Successful evolution without collapse.

COLLAPSE
    First transition where:
        previous_.kappa > 0
        current_.kappa == 0

ERROR
    Validation or numerical violation.
    No mutation occurs.

ERROR MUST NOT alter structural state.

COLLAPSE MUST be emitted exactly once per lifecycle.

---

## 1.7 Determinism Constraints

The implementation MUST:

- Use IEEE-754 double precision
- Avoid fast-math
- Avoid extended precision dependency
- Avoid fused multiply-add variability
- Avoid hidden mutable globals
- Avoid randomness
- Avoid multi-threaded mutation
- Avoid platform-dependent evaluation order

All accumulation order MUST be deterministic.

Bitwise reproducibility under identical compiler,
architecture, and build flags is mandatory.

---

## 1.8 Strict Execution Authority

Mutation authority is restricted to:

    EventFlag MaxCore::Step(...)

No constructor, accessor, or helper may mutate state
outside AtomicCommit.

No hidden mutation channel is permitted.

Implementation V2.5 enforces strict separation between:

- Configuration
- Persistent structural state
- Transient computation
- Lifecycle metadata

---

# 2. Step() Implementation — Canonical Execution Pipeline

This section defines the exact implementation logic of:

    EventFlag MaxCore::Step(
        const double* delta_input,
        size_t delta_len,
        double dt
    );

The execution order defined here is mandatory.
No reordering of stages is permitted.

All mutation MUST occur exclusively through AtomicCommit.

---

## 2.0 Terminal Short-Circuit (Pre-Validation)

If:
    current_.kappa == 0.0

Then:

- No validation is performed.
- No numerical computation is performed.
- No mutation occurs.
- lifecycle_.step_counter does NOT increment.
- Return EventFlag::NORMAL.

Terminal state is irreversible within lifecycle.

This short-circuit MUST occur before any other stage.

---

## 2.1 Stage 1 — Input Validation

The following conditions MUST be validated before any computation:

1) delta_input != nullptr
2) delta_len == delta_dim_
3) std::isfinite(dt)
4) dt > 0
5) For all i in [0, delta_len):
       std::isfinite(delta_input[i])
6) std::isfinite(current_.phi)
7) std::isfinite(current_.memory)
8) std::isfinite(current_.kappa)
9) All ParameterSet values are finite

If any condition fails:

    return EventFlag::ERROR;

No mutation is permitted during validation.

---

## 2.2 Stage 2 — Numerical Stability Check

Compute:

    double max_rate = std::max({
        params_.eta,
        params_.gamma,
        params_.rho,
        params_.lambda_phi,
        params_.lambda_m
    });

Verify:

    dt * max_rate < 1.0

If the condition is violated:

    return EventFlag::ERROR;

This check MUST occur before:

- norm computation
- any canonical update
- any candidate state creation

---

## 2.3 Stage 3 — Local Candidate Copy

Create working copy:

    StructuralState next = current_;

No mutation of current_ may occur
before AtomicCommit.

All subsequent updates operate on:

    next

---

## 2.4 Stage 4 — Admissible Delta Processing

Compute squared Euclidean norm deterministically:

    double norm2 = 0.0;

    for (size_t i = 0; i < delta_len; ++i) {
        double v = delta_input[i];
        norm2 += v * v;
    }

If !std::isfinite(norm2):

    return EventFlag::ERROR;

If delta_max_ is enabled:

    double max2 = delta_max_.value() * delta_max_.value();

    if (norm2 > max2) {

        double norm = std::sqrt(norm2);

        if (!std::isfinite(norm))
            return EventFlag::ERROR;

        double scale = delta_max_.value() / norm;

        // Conceptual scaling of Delta_step
        // No persistent buffer is stored.
        // Direction is preserved.
        // Magnitude is reduced deterministically.

        norm2 = max2;
    }

This stage MUST:

- Never expand input magnitude.
- Preserve direction when scaling.
- Not modify StructuralState.
- Not modify ParameterSet.
- Not modify LifecycleContext.
- Not allocate persistent buffers.

Delta_step is conceptual and exists only within this scope.

---

## 2.5 Stage 5 — Canonical Energy Update

Compute:

    double phi_next =
        current_.phi
        + params_.alpha * norm2
        - params_.eta   * current_.phi * dt;

Clamp lower bound:

    if (phi_next < 0.0)
        phi_next = 0.0;

If !std::isfinite(phi_next):

    return EventFlag::ERROR;

Assign:

    next.phi = phi_next;

---

## 2.6 Stage 6 — Canonical Memory Update

Compute:

    double memory_next =
        current_.memory
        + params_.beta  * phi_next * dt
        - params_.gamma * current_.memory * dt;

Clamp lower bound:

    if (memory_next < 0.0)
        memory_next = 0.0;

If !std::isfinite(memory_next):

    return EventFlag::ERROR;

Assign:

    next.memory = memory_next;

---

## 2.7 Stage 7 — Canonical Stability Update

Compute:

    double kappa_next =
        current_.kappa
        + params_.rho * (params_.kappa_max - current_.kappa) * dt
        - params_.lambda_phi * phi_next    * dt
        - params_.lambda_m   * memory_next * dt;

Clamp lower bound:

    if (kappa_next < 0.0)
        kappa_next = 0.0;

Clamp upper bound:

    if (kappa_next > params_.kappa_max)
        kappa_next = params_.kappa_max;

If !std::isfinite(kappa_next):

    return EventFlag::ERROR;

Assign:

    next.kappa = kappa_next;

---

## 2.8 Stage 8 — Invariant Enforcement

Verify candidate state:

    next.phi    >= 0.0
    next.memory >= 0.0
    0.0 <= next.kappa <= params_.kappa_max
    std::isfinite(next.phi)
    std::isfinite(next.memory)
    std::isfinite(next.kappa)

If any violation detected:

    return EventFlag::ERROR;

No additional correction beyond defined clamping is permitted.

---

## 2.9 Stage 9 — Collapse Detection

Determine:

    bool collapse_now =
        (current_.kappa > 0.0 && next.kappa == 0.0);

EventFlag flag =
    collapse_now ? EventFlag::COLLAPSE
                 : EventFlag::NORMAL;

Collapse MUST be emitted exactly once per lifecycle.

---

## 2.10 Stage 10 — Atomic Commit

AtomicCommit MUST occur only after all validation succeeds.

Operations MUST execute in this exact order:

1) previous_ = current_;

2) current_  = next;

3) lifecycle_.step_counter++;

4) lifecycle_.terminal =
       (current_.kappa == 0.0);

5) If collapse_now and lifecycle_.collapse_emitted == false:
       lifecycle_.collapse_emitted = true;

AtomicCommit MUST be logically indivisible.

If any prior stage returned ERROR,
AtomicCommit MUST NOT execute.

---

## 2.11 Stage 11 — Return

Return:

    flag;

No further mutation occurs after return.

---

# 3. Initialization Implementation Details

This section defines the concrete implementation of:

    static std::optional<MaxCore> MaxCore::Create(
        const ParameterSet& params,
        size_t delta_dim,
        const StructuralState& initial_state,
        std::optional<double> delta_max
    );

Initialization MUST be deterministic and non-partial.

No runtime instance may exist in invalid state.

---

## 3.1 Parameter Validation

Before constructing the instance,
Create() MUST validate ParameterSet.

For each parameter:

    std::isfinite(value) == true

Additionally, the following MUST hold:

    params.alpha        > 0
    params.eta          > 0
    params.beta         > 0
    params.gamma        > 0
    params.rho          > 0
    params.lambda_phi   > 0
    params.lambda_m     > 0
    params.kappa_max    > 0

If any condition fails:

    return std::nullopt;

No instance is constructed.

ParameterSet MUST NOT be modified during validation.

---

## 3.2 delta_dim Validation

delta_dim MUST satisfy:

    delta_dim > 0

If:

    delta_dim == 0

Then:

    return std::nullopt;

delta_dim is immutable after successful construction.

---

## 3.3 Optional Delta Guard Validation

If delta_max.has_value():

The following MUST hold:

    std::isfinite(delta_max.value()) == true
    delta_max.value() > 0.0

If validation fails:

    return std::nullopt;

If delta_max is not provided:

Norm guard is disabled.

delta_max_ MUST remain immutable after construction.

---

## 3.4 Initial StructuralState Validation

The provided initial_state MUST satisfy:

1) Finite values:

    std::isfinite(initial_state.phi)
    std::isfinite(initial_state.memory)
    std::isfinite(initial_state.kappa)

2) Invariants:

    initial_state.phi    >= 0.0
    initial_state.memory >= 0.0
    0.0 <= initial_state.kappa <= params.kappa_max

If any condition fails:

    return std::nullopt;

No instance is constructed.

---

## 3.5 Private Constructor Execution

Only after full validation,
Create() invokes private constructor:

    MaxCore(params, delta_dim, initial_state, delta_max);

The private constructor MUST:

- Copy params into params_
- Store delta_dim into delta_dim_
- Store delta_max into delta_max_
- Initialize current_
- Initialize previous_
- Initialize lifecycle_

The constructor MUST NOT perform validation.

All validation belongs exclusively to Create().

---

## 3.6 Lifecycle Initialization

After construction:

    current_  = initial_state;
    previous_ = initial_state;

    lifecycle_.step_counter = 0;

    lifecycle_.terminal =
        (initial_state.kappa == 0.0);

    lifecycle_.collapse_emitted = false;

If initial_state.kappa == 0.0:

- Lifecycle begins in terminal state.
- No collapse event is emitted.
- Step() will short-circuit.
- step_counter remains 0.

Initialization MUST NOT emit COLLAPSE.

---

## 3.7 Initialization Determinism

Given identical:

- ParameterSet
- delta_dim
- initial_state
- delta_max

Create() MUST produce identical internal state.

Initialization MUST NOT depend on:

- system time
- randomness
- global mutable state
- environment variables

Initialization is purely functional.

---

## 3.8 No Partial Construction Guarantee

If Create() returns std::nullopt:

- No instance exists.
- No partially initialized object is exposed.
- No side effects occur.
- No internal state leaks.

The runtime MUST be either:

- Fully valid
- Or nonexistent

Partial construction is strictly prohibited.

---

## 3.9 Reinitialization Policy

Reinitialization MUST occur only by:

- Destroying the previous instance.
- Calling Create() again.

State mutation after collapse is prohibited.

Recovery requires a new lifecycle.

No internal reset mechanism is permitted.

---

# 4. Numerical Safety and Floating-Point Discipline

This section defines strict numerical behavior
required for MAX-Core V2.5 implementation.

The goal is to prevent:

- Undefined behavior
- Silent instability
- Platform-dependent divergence
- Non-deterministic rounding behavior

Numerical safety MUST be enforced before AtomicCommit.

---

## 4.1 IEEE-754 Requirement

The implementation MUST:

- Use double precision (64-bit IEEE-754).
- Avoid extended precision dependency.
- Avoid non-standard floating-point modes.
- Avoid fast-math compiler flags.

Recommended compiler constraints:

    -fno-fast-math
    -ffp-contract=off
    strict IEEE compliance

Behavior MUST NOT depend on:

- Fused multiply-add variability
- Non-deterministic evaluation order
- Platform-specific math extensions

Floating-point mode MUST remain consistent
for the entire lifecycle.

---

## 4.2 Finite Validation Discipline

At every stage of Step(),
all critical values MUST be validated using:

    std::isfinite(value)

Specifically:

- dt
- norm2
- phi_next
- memory_next
- kappa_next

If any value becomes non-finite:

    return EventFlag::ERROR;

No mutation MUST occur.

Finite validation MUST precede AtomicCommit.

---

## 4.3 Overflow Handling

Overflow during:

- norm2 accumulation
- multiplication
- canonical update equations

will produce:

    ±Inf

Detection rule:

    if (!std::isfinite(value))
        return EventFlag::ERROR;

Silent overflow is prohibited.

Overflow MUST NOT be masked or corrected silently.

---

## 4.4 Underflow Handling

Underflow toward zero is permitted
provided the result remains finite.

Subnormal numbers are allowed unless
explicitly disabled by floating-point mode.

Underflow MUST NOT produce ERROR
unless invariants are violated.

Underflow MUST NOT trigger implicit correction.

---

## 4.5 Deterministic Accumulation

Norm accumulation MUST follow:

    for (size_t i = 0; i < delta_dim_; ++i)

The iteration order MUST NOT vary.

Parallel reduction is prohibited.

SIMD vectorization that changes rounding order
is prohibited unless bit-identical across builds.

Reordering accumulation violates determinism.

---

## 4.6 dt Stability Enforcement

Before norm computation and canonical updates:

    double max_rate = std::max({
        params_.eta,
        params_.gamma,
        params_.rho,
        params_.lambda_phi,
        params_.lambda_m
    });

    if (dt * max_rate >= 1.0)
        return EventFlag::ERROR;

This check MUST occur before:

- norm computation
- candidate state creation
- any canonical update

Discrete-time stability enforcement is mandatory.

---

## 4.7 No Implicit Correction

The implementation MUST NOT:

- silently adjust dt
- silently modify ParameterSet
- silently modify delta_input
- silently expand Delta magnitude
- silently alter canonical update equations

Only the following clamping is permitted:

    phi    = max(0.0, phi)
    memory = max(0.0, memory)
    kappa  = clamp(0.0, kappa, kappa_max)

No additional correction layers are allowed.

---

## 4.8 Strict ERROR Isolation

If EventFlag::ERROR is returned at any stage:

- previous_ MUST remain unchanged
- current_ MUST remain unchanged
- lifecycle_ MUST remain unchanged
- step_counter MUST NOT increment
- terminal MUST NOT change
- collapse_emitted MUST NOT change

ERROR MUST be fully isolated from structural evolution.

---

## 4.9 Bitwise Reproducibility Guarantee

Under identical:

- Compiler
- Architecture
- Build flags
- Floating-point environment

The implementation MUST produce bit-identical results.

Cross-platform bitwise identity is recommended
but not mandatory.

Bitwise reproducibility is required for:

- StructuralState sequence
- LifecycleContext sequence
- EventFlag sequence

---

## 4.10 No Undefined Behavior

The implementation MUST NOT:

- Dereference null pointers
- Access out-of-bounds delta_input
- Use uninitialized memory
- Rely on unspecified C++ behavior
- Depend on container iteration order
- Invoke undefined math operations

All safety checks MUST precede mutation.

Undefined behavior invalidates compliance with V2.5.

---

# 5. Determinism Enforcement Checklist

This section defines a strict implementation-level checklist
ensuring full compliance with MAX-Core V2.5 determinism requirements.

Every conforming implementation MUST satisfy
all items listed below.

Failure of any item constitutes non-compliance.

---

## 5.1 Single Mutation Authority

✔ Step() is the only function that mutates StructuralState.  
✔ No background threads mutate state.  
✔ No asynchronous callbacks modify state.  
✔ No external mutation hooks exist.  
✔ No diagnostic layer may alter StructuralState.  

---

## 5.2 Immutable Configuration

✔ ParameterSet is validated once and never modified.  
✔ delta_dim_ is validated once and never modified.  
✔ delta_max_ (if present) is immutable.  
✔ No runtime parameter tuning.  
✔ No hidden configuration mutation.  

---

## 5.3 Explicit State Ownership

✔ current_ is private.  
✔ previous_ is private.  
✔ lifecycle_ is private.  
✔ No public setters.  
✔ No friend classes modifying state.  

---

## 5.4 Strict Execution Order

✔ Terminal short-circuit occurs before validation.  
✔ Input validation occurs before stability check.  
✔ Stability check occurs before norm computation.  
✔ Norm computed before canonical updates.  
✔ Energy → Memory → Stability update order preserved.  
✔ Invariants checked before collapse detection.  
✔ Collapse detection occurs before AtomicCommit.  
✔ AtomicCommit occurs before return.  

Reordering any stage violates determinism.

---

## 5.5 Deterministic Norm Accumulation

✔ Accumulation order fixed from 0 to delta_dim_-1.  
✔ No parallel reduction.  
✔ No dynamic reordering of accumulation.  
✔ No hardware-dependent vector reordering.  

---

## 5.6 Finite Validation Coverage

✔ dt finite checked.  
✔ dt > 0 checked.  
✔ dt * max_rate < 1 enforced.  
✔ norm2 finite checked.  
✔ phi_next finite checked.  
✔ memory_next finite checked.  
✔ kappa_next finite checked.  

No computed value may bypass finite validation.

---

## 5.7 ERROR Isolation

✔ ERROR does not modify current_.  
✔ ERROR does not modify previous_.  
✔ ERROR does not modify lifecycle_.  
✔ ERROR does not increment step_counter.  
✔ ERROR does not emit collapse.  
✔ ERROR does not alter terminal flag.  

---

## 5.8 Collapse Semantics

✔ Collapse occurs iff:
      current_.kappa > 0
      next.kappa == 0

✔ collapse_emitted set exactly once.  
✔ terminal set immediately upon commit.  
✔ No mutation in terminal state.  
✔ No recovery within lifecycle.  

---

## 5.9 Lifecycle Determinism

✔ step_counter increments only after successful commit.  
✔ step_counter does not increment on ERROR.  
✔ step_counter does not increment in terminal state.  
✔ terminal == (current_.kappa == 0).  
✔ collapse_emitted remains true permanently after first collapse.  

---

## 5.10 Floating-Point Discipline

✔ No fast-math flags.  
✔ No implicit fused multiply-add variability.  
✔ No dependence on extended precision registers.  
✔ Deterministic evaluation order preserved.  
✔ No implicit compiler reordering that changes rounding behavior.  

---

## 5.11 No Hidden Global State

✔ No static mutable variables affecting evolution.  
✔ No dependency on system time.  
✔ No random number generators.  
✔ No environment-dependent branching.  
✔ No hidden caching layer affecting updates.  

---

## 5.12 Minimal Ontology Preservation

✔ Persistent state contains only:
      phi, memory, kappa  

✔ Delta is transient only.  
✔ No hidden structural coordinates introduced.  
✔ No additional persistent buffers used for Delta.  

---

## 5.13 Reproducibility Validation

Given identical:

- initial StructuralState  
- ParameterSet  
- delta_input sequence  
- dt sequence  
- compiler and build flags  

The sequence of:

- StructuralState  
- LifecycleContext  
- EventFlag  

MUST be bit-identical.

---

## 5.14 Non-Compliance Indicators

An implementation violates V2.5 if it:

✘ Reorders update stages.  
✘ Mutates state on ERROR.  
✘ Allows recovery after collapse.  
✘ Commits non-finite values.  
✘ Introduces hidden structural state.  
✘ Expands persistent ontology.  
✘ Alters parameter values during lifecycle.  
✘ Skips dt stability enforcement.  
✘ Fails to enforce strict short-circuit in terminal state.  

---

## 5.15 Authority Statement

This Implementation Document V2.5
realizes MAX-Core Specification V2.5
and Design V2.5 exactly.

All determinism guarantees are enforced structurally.

Implementation V2.5 supersedes all prior versions.

No deviation is permitted
if compliance with MAX-Core V2.5 is claimed.

---

# 6. Implementation Authority Statement

This document defines the authoritative reference implementation
of MAX-Core V2.5.

The implementation described herein:

- Realizes MAX-Core Specification V2.5 exactly.
- Conforms structurally to MAX-Core Design Document V2.5.
- Enforces strict execution ordering.
- Enforces deterministic atomic commit.
- Enforces numerical stability before mutation.
- Enforces strict ERROR isolation.
- Enforces irreversible collapse semantics.
- Enforces minimal persistent ontology.
- Enforces bitwise reproducibility under identical environment.

No behavior outside this document
may be claimed as compliant with MAX-Core V2.5.

---

## 6.1 Supersession

Implementation V2.5 supersedes:

- Implementation V2.4
- All prior draft versions
- Any experimental variants

Only Version 2.5 may claim full compliance
with Specification V2.5 and Design V2.5.

---

## 6.2 Non-Compliance Definition

An implementation is non-compliant if it:

- Alters canonical update equations
- Reorders execution stages
- Mutates state on ERROR
- Allows mutation in terminal state
- Allows recovery after collapse
- Commits non-finite values
- Violates dt stability constraint
- Introduces hidden persistent state
- Modifies ParameterSet during lifecycle
- Breaks bitwise determinism under identical environment

Any such deviation voids compliance claims.

---

## 6.3 Deterministic Runtime Identity

A conforming runtime MUST identify itself as:

    MAX-Core V2.5

Version identification MUST be explicit
in documentation and binary metadata if applicable.

---

## 6.4 Final Conformance Guarantee

MAX-Core Implementation V2.5 guarantees:

- Deterministic structural evolution
- Strict invariant preservation
- Single mutation authority
- Irreversible collapse semantics
- Explicit atomic commit semantics
- Isolation of numerical and validation errors
- Minimal and closed structural ontology

All guarantees are enforced by structural design,
not by runtime heuristics.

---

### 7.0 Fresh Genesis Runtime Pattern (External Orchestration)

Fresh Genesis is implemented outside MAX-Core.

Example (conceptual):

flag = core.Step(delta, dt);

if (flag == EventFlag::COLLAPSE)
{
    archive(core);

    auto new_core = MaxCore::Create(
        ParameterSet,
        delta_dim,
        initial_state = {
            Phi    = 0,
            Memory = 0,
            Kappa  = kappa_max
        }
    );

    core = std::move(new_core.value());
}

Fresh Genesis MUST:

- Not modify the committed collapsed instance.
- Not revive a collapsed lifecycle.
- Not bypass collapse detection.
- Not alter canonical equations.
- Not mutate StructuralState directly.

All lifecycle transitions occur through destruction
and re-instantiation of MAX-Core.
