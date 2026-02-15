# MAX-Core Implementation Document
## Version 2.4
### Deterministic Canonical Living Structural Runtime

---

# 1. Implementation Overview

## 1.1 Purpose

This document defines the concrete C++ realization
of MAX-Core Specification V2.4 and Design V2.4.

The implementation MUST:

- Realize persistent state S = (Phi, Memory, Kappa)
- Enforce strict execution ordering
- Enforce numerical safety
- Enforce deterministic atomic commit
- Prevent mutation on ERROR
- Prevent recovery after collapse
- Guarantee reproducibility under identical environment

This implementation describes a single-threaded,
strict IEEE-754 double precision runtime.

---

## 1.2 Persistent Ontology

The only persistent structural state is:

    struct StructuralState {
        double phi;
        double memory;
        double kappa;
    };

No additional structural coordinates are permitted.

Delta is transient and never stored persistently.

---

## 1.3 Lifecycle Context

Lifecycle metadata:

    struct LifecycleContext {
        uint64_t step_counter;
        bool terminal;
        bool collapse_emitted;
    };

Invariants:

    terminal == (current_.kappa == 0)

collapse_emitted:

    false before first collapse
    true permanently after first collapse

step_counter increments only after successful commit.

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
- strictly positive (except none optional)
- validated during initialization

ParameterSet is immutable after successful initialization.

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
        double dt
    );

    const StructuralState& Current() const noexcept;
    const StructuralState& Previous() const noexcept;
    const LifecycleContext& Lifecycle() const noexcept;

private:
    ParameterSet params_;
    size_t delta_dim_;

    std::optional<double> delta_max_;  // Optional norm guard

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

---

## 1.6 EventFlag Definition

    enum class EventFlag {
        NORMAL,
        COLLAPSE,
        ERROR
    };

ERROR never mutates state.

COLLAPSE emitted exactly once per lifecycle.

---

## 1.7 Initialization Rules

Create() MUST:

- Validate ParameterSet
- Validate delta_dim > 0
- Validate all initial_state values finite
- Validate invariants:
      phi ≥ 0
      memory ≥ 0
      0 ≤ kappa ≤ kappa_max

If validation fails:

    return std::nullopt

On success:

    current_  = initial_state
    previous_ = initial_state
    lifecycle_.step_counter = 0
    lifecycle_.terminal = (initial_state.kappa == 0)
    lifecycle_.collapse_emitted = false

No partial instance may exist.

---

## 1.8 Determinism Constraints

Implementation MUST:

- Use IEEE-754 double precision
- Avoid fast-math
- Avoid extended precision dependency
- Avoid hidden mutable globals
- Avoid randomness
- Avoid multi-threaded mutation

All accumulation order MUST be deterministic.

---

# 2. Step() Implementation — Canonical Execution Pipeline

This section defines the exact implementation logic of:

    EventFlag MaxCore::Step(const double* delta_input, double dt)

The execution order defined here is mandatory.
No reordering is permitted.

---

## 2.1 Terminal Short-Circuit

If:

    lifecycle_.terminal == true

Then:

- No mutation occurs.
- lifecycle_.step_counter does NOT increment.
- Return EventFlag::NORMAL.

Terminal state is irreversible within lifecycle.

---

## 2.2 Stage 1 — Input Validation

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

## 2.3 Stage 2 — Numerical Stability Check

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

If not satisfied:

    return EventFlag::ERROR

---

## 2.4 Stage 3 — Norm Computation and Optional Guard

Compute squared Euclidean norm:

    double norm2 = 0.0;

    for (size_t i = 0; i < delta_len; ++i) {
        double v = delta_input[i];
        norm2 += v * v;
    }

If norm2 is not finite:

    return EventFlag::ERROR;

If delta_max_ is set:

    double max2 = delta_max_.value() * delta_max_.value();

    if (norm2 > max2) {
        double norm = std::sqrt(norm2);

        if (!std::isfinite(norm))
            return EventFlag::ERROR;

        double scale = delta_max_.value() / norm;

        // Conceptual Delta_step scaling (direction preserved)
        // No persistent buffer is stored.
        norm2 = delta_max_.value() * delta_max_.value();
    }

This stage MUST:

- Be deterministic.
- Never expand input magnitude.
- Preserve input direction when norm guard is enabled.
- Not modify StructuralState.
- Not modify ParameterSet.
- Not modify LifecycleContext.

---

## 2.6 Stage 5 — Canonical Energy Update

Compute candidate:

    double phi_next =
        current_.phi
        + params_.alpha * norm2
        - params_.eta   * current_.phi * dt;

Clamp:

    if (phi_next < 0.0)
        phi_next = 0.0;

If !std::isfinite(phi_next):

    return EventFlag::ERROR;

---

## 2.7 Stage 6 — Canonical Memory Update

Compute:

    double memory_next =
        current_.memory
        + params_.beta  * phi_next * dt
        - params_.gamma * current_.memory * dt;

Clamp:

    if (memory_next < 0.0)
        memory_next = 0.0;

If !std::isfinite(memory_next):

    return EventFlag::ERROR;

---

## 2.8 Stage 7 — Canonical Stability Update

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

---

## 2.9 Stage 8 — Collapse Detection

Determine:

    bool collapse_now =
        (current_.kappa > 0.0 && kappa_next == 0.0);

EventFlag flag =
    collapse_now ? EventFlag::COLLAPSE
                 : EventFlag::NORMAL;

---

## 2.10 Stage 9 — Atomic Commit

Copy:

    previous_ = current_;

Update:

    current_.phi    = phi_next;
    current_.memory = memory_next;
    current_.kappa  = kappa_next;

Increment:

    lifecycle_.step_counter++;

Update terminal:

    lifecycle_.terminal = (current_.kappa == 0.0);

If collapse_now and not previously emitted:

    if (!lifecycle_.collapse_emitted)
        lifecycle_.collapse_emitted = true;

AtomicCommit MUST occur only after all validation succeeds.

---

## 2.11 Stage 10 — Return

Return:

    flag

No further mutation occurs after return.

---

## 2.12 ERROR Isolation Guarantee

If any stage returns ERROR:

- previous_ MUST remain unchanged
- current_ MUST remain unchanged
- lifecycle_ MUST remain unchanged

No partial update is permitted.

---

# 3. Initialization Implementation Details

This section defines the concrete implementation of:

    static std::optional<MaxCore> MaxCore::Create(...)

Initialization MUST be deterministic and non-partial.

No runtime instance may exist in invalid state.

---

## 3.1 Parameter Validation

Before constructing the instance,
Create() MUST validate ParameterSet.

For each parameter:

    std::isfinite(value) == true

Additionally, the following MUST hold:

    alpha        > 0
    eta          > 0
    beta         > 0
    gamma        > 0
    rho          > 0
    lambda_phi   > 0
    lambda_m     > 0
    kappa_max    > 0

If any condition fails:

    return std::nullopt;

No instance is constructed.

---

## 3.2 delta_dim Validation

delta_dim MUST satisfy:

    delta_dim > 0

If delta_dim == 0:

    return std::nullopt;

delta_dim is immutable after construction.

---

## 3.3 Initial StructuralState Validation

The provided initial_state MUST satisfy:

1) All values finite:
       std::isfinite(phi)
       std::isfinite(memory)
       std::isfinite(kappa)

2) Invariants:
       phi ≥ 0
       memory ≥ 0
       0 ≤ kappa ≤ kappa_max

If any condition fails:

    return std::nullopt;

No instance is constructed.

---

## 3.4 Private Constructor Execution

Only after full validation,
Create() invokes private constructor:

    MaxCore(params, delta_dim, initial_state)

The constructor MUST:

- Copy params into params_
- Store delta_dim into delta_dim_
- Initialize current_ and previous_
- Initialize lifecycle_

The constructor MUST NOT perform validation.

Validation belongs exclusively to Create().

---

## 3.5 Lifecycle Initialization

After construction:

    current_  = initial_state
    previous_ = initial_state

    lifecycle_.step_counter = 0
    lifecycle_.terminal =
        (initial_state.kappa == 0.0)

    lifecycle_.collapse_emitted = false

If initial_state.kappa == 0:

- The lifecycle begins in terminal state.
- No collapse is emitted.
- Step() will short-circuit.

---

## 3.6 Initialization Determinism

Given identical:

- ParameterSet
- delta_dim
- initial_state

Create() MUST produce identical internal state.

Initialization MUST NOT depend on:

- system time
- environment variables
- randomness
- global mutable state

---

## 3.7 No Partial Construction Guarantee

If Create() returns std::nullopt:

- No instance exists.
- No partially initialized object is exposed.
- No side effects occur.

The runtime must be either fully valid or nonexistent.

---

## 3.8 Reinitialization Policy

Reinitialization MUST occur only by:

- Destroying the previous instance.
- Calling Create() again.

State mutation after collapse is prohibited.

Recovery requires new lifecycle construction.

If delta_max is provided:

    std::isfinite(delta_max.value()) == true
    delta_max.value() > 0

If validation fails:

    return std::nullopt;

If delta_max is not provided:

    norm guard is disabled.

---

# 4. Numerical Safety and Floating-Point Discipline

This section defines strict numerical behavior
required for MAX-Core V2.4 implementation.

The goal is to prevent undefined behavior,
silent instability, and platform-dependent divergence.

---

## 4.1 IEEE-754 Requirement

The implementation MUST:

- Use double precision (64-bit IEEE-754).
- Avoid extended precision dependency.
- Avoid non-standard floating-point modes.
- Avoid fast-math compiler flags.

Recommended:

    -fno-fast-math
    -ffp-contract=off
    strict IEEE compliance

Behavior MUST NOT depend on:

- Fused multiply-add variability
- Non-deterministic evaluation order
- Platform-specific math extensions

---

## 4.2 Finite Validation Discipline

At every stage of Step():

All intermediate and final computed values MUST be checked:

    std::isfinite(value)

Specifically:

- dt
- norm2
- phi_next
- memory_next
- kappa_next

If any becomes non-finite:

    return EventFlag::ERROR;

No mutation MUST occur.

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
        return ERROR;

Silent overflow is prohibited.

---

## 4.4 Underflow Handling

Underflow toward zero is permitted
provided the value remains finite.

Subnormal numbers are allowed unless
explicitly disabled by floating-point mode.

Underflow MUST NOT produce ERROR
unless invariants are violated.

---

## 4.5 Deterministic Accumulation

Norm accumulation MUST follow:

    for i = 0 to delta_dim_-1

The order MUST NOT vary.

Parallel reduction is prohibited.

Reordering accumulation changes rounding behavior
and violates determinism.

---

## 4.6 No Implicit Correction

The implementation MUST NOT:

- silently adjust dt
- silently modify ParameterSet
- silently modify delta_input
- silently expand Delta magnitude
- silently modify state outside defined clamping

Only the following clamping is permitted:

    phi    = max(0, phi)
    memory = max(0, memory)
    kappa  = clamp(0, kappa, kappa_max)

No additional correction layers are allowed.

---

## 4.7 dt Stability Enforcement

Before any canonical update:

    double max_rate = max(...)

    if (dt * max_rate >= 1.0)
        return ERROR;

This MUST occur before computing norm2.

This ensures discrete-time stability.

---

## 4.8 Strict Isolation on ERROR

If ERROR is returned at any stage:

- No partial mutation
- No state copy
- No lifecycle flag modification
- No step_counter increment

ERROR is fully isolated from structural evolution.

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

---

## 4.10 No Undefined Behavior

The implementation MUST NOT:

- Dereference null pointers
- Access out-of-bounds delta_input
- Use uninitialized memory
- Rely on unspecified C++ behavior
- Depend on container iteration order

Undefined behavior invalidates determinism.

All safety checks precede mutation.

---

# 5. Determinism Enforcement Checklist

This section provides an implementation-level checklist
ensuring strict determinism in MAX-Core V2.4.

Every conforming implementation MUST satisfy
all items listed below.

---

## 5.1 Single Mutation Authority

✔ Step() is the only function that mutates StructuralState.  
✔ No background threads.  
✔ No asynchronous callbacks.  
✔ No external mutation hooks.  

---

## 5.2 Immutable Configuration

✔ ParameterSet is validated once and never modified.  
✔ delta_dim_ is validated once and never modified.  
✔ No runtime parameter tuning.  
✔ No hidden configuration mutation.  

---

## 5.3 Explicit State Ownership

✔ current_ and previous_ are private members.  
✔ LifecycleContext is private.  
✔ No public setters.  
✔ No friend classes modifying state.  

---

## 5.4 Strict Execution Order

✔ Input validation first.  
✔ Stability check before computation.  
✔ norm2 computed before updates.  
✔ Energy → Memory → Stability update order preserved.  
✔ Collapse detection before commit.  
✔ AtomicCommit last.  

Reordering any stage violates determinism.

---

## 5.5 Deterministic Norm Accumulation

✔ Accumulation order fixed from 0 to delta_dim_-1.  
✔ No parallel reduction.  
✔ No SIMD reordering unless deterministic and identical across builds.  

---

## 5.6 Finite Validation Coverage

✔ dt finite checked.  
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

---

## 5.8 Collapse Semantics

✔ Collapse occurs iff current_.kappa > 0 and kappa_next == 0.  
✔ collapse_emitted set exactly once.  
✔ terminal set immediately upon commit.  
✔ No mutation in terminal state.  

---

## 5.9 Floating-Point Discipline

✔ No fast-math flags.  
✔ No implicit fused multiply-add variability.  
✔ No dependence on extended precision registers.  
✔ Deterministic evaluation order preserved.  

---

## 5.10 No Hidden Global State

✔ No static mutable variables affecting evolution.  
✔ No dependency on system time.  
✔ No random number generators.  
✔ No environment-dependent branching.  

---

## 5.11 Lifecycle Determinism

✔ step_counter increments only after successful commit.  
✔ step_counter does not increment on ERROR.  
✔ step_counter does not increment in terminal state.  
✔ terminal == (current_.kappa == 0).  

---

## 5.12 Reproducibility Validation

Given identical:

- initial state
- ParameterSet
- delta_input sequence
- dt sequence

The sequence of:

- StructuralState
- LifecycleContext
- EventFlag

MUST be identical.

---

## 5.13 Minimal Ontology Preservation

✔ Persistent state contains only:
      phi, memory, kappa  
✔ Delta is transient only.  
✔ No hidden structural coordinates introduced.  

---

## 5.14 Non-Compliance Indicators

An implementation violates Implementation V2.4 if it:

✘ Reorders update stages.  
✘ Mutates state on ERROR.  
✘ Allows recovery after collapse.  
✘ Commits non-finite values.  
✘ Introduces hidden structural state.  
✘ Expands persistent ontology.  
✘ Alters parameter values during lifecycle.  
✘ Allows dt instability.  

---

# 6. Implementation Authority Statement

This Implementation Document V2.4
realizes MAX-Core Specification V2.4
and Design V2.4 exactly.

No deviation is permitted
if compliance with MAX-Core V2.4 is claimed.

Implementation V2.4 supersedes all prior versions.
