# MAX Core Implementation Document V2.2
## Deterministic Single-Structure Normative Runtime

---

## 1.X Canonical Update Implementation

The structural update MUST follow this order:

1) Compute admissible Delta_step
2) Update Phi
3) Update Memory
4) Update Kappa
5) Enforce invariants
6) Detect collapse
7) Atomic commit

---

### 1.X.1 Phi Update

    phi_next =
        phi_current
        + alpha * norm2(delta_step)
        - eta * phi_current * dt;

    if (phi_next < 0.0)
        phi_next = 0.0;

---

### 1.X.2 Memory Update

    memory_next =
        memory_current
        + beta  * phi_next * dt
        - gamma * memory_current * dt;

    if (memory_next < 0.0)
        memory_next = 0.0;

---

### 1.X.3 Kappa Update

    kappa_next =
        kappa_current
        + rho * (kappa_max - kappa_current) * dt
        - lambda_phi * phi_next * dt
        - lambda_m   * memory_next * dt;

    if (kappa_next < 0.0)
        kappa_next = 0.0;

    if (kappa_next > kappa_max)
        kappa_next = kappa_max;

---

### 1.X.4 Collapse Detection

    if (kappa_next == 0.0)
        event_flag = COLLAPSE;
    else
        event_flag = NORMAL;

---

### 1.X.5 Parameter Set

All coefficients MUST be provided as a deterministic parameter set P:

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

The parameter set MUST:

- contain only finite values
- satisfy all positivity constraints
- remain constant during a lifecycle

---

# 1. Implementation Overview

## 1.1 Purpose

This document defines the concrete implementation requirements
for MAX Core V2.2 as specified in:

- MAX Core Specification V2.2
- MAX Core Design Document V2.2

The implementation MUST:

- Be deterministic.
- Enforce all structural invariants.
- Guarantee atomic commit semantics.
- Prevent κ increase.
- Detect collapse exclusively via κ == 0.
- Maintain strict separation between evolution and diagnostics.

This document defines how these guarantees are implemented in code.

---

## 1.2 Language and Toolchain Requirements

Recommended implementation language:

- C++17 or newer (C++20 preferred)

Mandatory properties:

- IEEE-754 double precision (binary64)
- No fast-math optimizations
- No unsafe floating-point reassociation
- No extended precision registers
- No undefined behavior

Recommended compiler flags:

- -std=c++20
- -O2 or -O3 (without fast-math)
- -Wall
- -Wextra
- -Wpedantic
- -Werror
- -fno-fast-math

Floating-point environment must remain fixed
throughout execution.

---

## 1.3 Core Public Interface

The runtime MUST expose a minimal public API:

Initialization:

    Init(const Config&, const StructuralState& initial)

Step execution:

    StepResult Step(const Vector& delta_input, double dt)

State access:

    const StructuralState& GetCurrent() const
    const StructuralState& GetPrevious() const

Version access:

    const char* GetCoreSpecVersion()

The public API MUST NOT expose:

- Internal buffers
- Mutable references
- Diagnostic mutation handles
- Direct κ mutation functions

---

## 1.4 StructuralState Implementation

StructuralState MUST be implemented as a fixed-layout structure:

    struct StructuralState {
        std::array<double, N> delta;
        double phi;
        double memory;
        double kappa;
    };

Requirements:

- No virtual functions.
- No inheritance.
- No dynamic memory.
- No hidden padding-dependent behavior.
- All values initialized explicitly.

StructuralState MUST remain POD-like.

---

## 1.5 LifecycleContext Implementation

    struct LifecycleContext {
        uint64_t step_counter;
        bool terminal;
        bool collapse_emitted;
    };

Requirements:

- `terminal` MUST be a *derived* lifecycle flag and MUST satisfy:

      terminal == (X_current.kappa == 0)

  at all observable times after Init() and after every successful commit.

- `terminal` MUST be set during Init() as:

      terminal = (X_initial.kappa == 0)

- `terminal` MUST be updated only during atomic commit, and only according to
  the committed `X_current.kappa` value.

- `collapse_emitted` MUST become true at most once per lifecycle, exactly when
  the runtime returns `EventFlag == COLLAPSE` for the first time.

- No hidden transitions.
- No mutation outside Step().
- No inference from diagnostics.

---

## 1.6 Config Implementation

Config MUST contain all parameters required by:

- AdmissibleDelta
- EnergyFunction
- Memory evolution
- Viability decay
- Numeric safety thresholds
- Diagnostic enable flags

Config MUST:

- Be immutable after Init().
- Be stored by value or const reference.
- Be serializable.
- Contain no derived runtime state.

---

## 1.7 Step Implementation Pattern

The Step() method MUST follow this exact pattern:

1) Pre-validate inputs (finite Δ_input, dt > 0).
2) If (X_current.kappa == 0) → return NORMAL with:
   - no mutation of any structural coordinate
   - no step_counter increment
   - no time advancement
3) Copy X_current into local X_next.
4) Compute admissible delta.
5) Update energy.
6) Update memory.
7) Update kappa.
8) Enforce invariants.
9) Detect collapse (exclusively via kappa reaching 0).
10) Atomic commit.
11) Execute diagnostics (if enabled) on committed immutable snapshot only.
12) Return StepResult.

All mutation MUST occur on X_next before commit.

---

## 1.8 Atomic Commit Implementation

Atomic commit MUST:

- Copy X_current into X_previous.
- Copy X_next into X_current.
- Increment step_counter by 1.

Then update lifecycle flags deterministically from the committed state:

- terminal MUST be set as:

      terminal = (X_current.kappa == 0)

- collapse_emitted update rule:
  - If EventFlag == COLLAPSE and collapse_emitted == false → set collapse_emitted = true
  - Otherwise collapse_emitted MUST remain unchanged

Commit MUST occur only after invariant validation.
No partial mutation is allowed.

---

## 1.9 Invariant Enforcement Implementation

Invariant enforcement MUST check:

- std::isfinite for all components.
- phi >= 0
- memory >= 0
- kappa >= 0
- kappa <= previous kappa

If any check fails:

- Abort.
- Return ERROR.
- Do not modify state.

Invariant enforcement must be centralized.

---

## 1.10 Collapse Implementation

Collapse detection condition:

    if (previous.kappa > 0 && current.kappa == 0)

Then:

- lifecycle.terminal = true
- lifecycle.collapse_emitted = true
- return COLLAPSE

Collapse must be emitted exactly once.

Subsequent Step() calls must:

- Perform no mutation.
- Return NORMAL.

---

## 1.11 Diagnostic Execution Implementation

Diagnostics MUST:

- Execute after atomic commit.
- Receive const references only.
- Not throw exceptions.
- Not modify state.
- Be isolated from Step failure.

Diagnostic errors MUST NOT affect StepResult.

---

## 1.12 Replay Support

Implementation MUST support:

- Serialization of StructuralState
- Serialization of LifecycleContext
- Serialization of Config
- Deterministic reload
- Identical replay results

Replay mismatch invalidates compliance.

---

## 1.13 Memory Allocation Rules

During Step():

- No dynamic allocation.
- No heap growth.
- No reallocation of delta vector.
- No std::vector resizing.

All buffers must be preallocated.

---

## 1.14 Prohibited Implementation Patterns

The following are prohibited:

- Global mutable state.
- Static mutable variables.
- Thread-local mutation.
- Hidden random number generators.
- Floating-point environment mutation.
- Parallel floating-point reduction.
- Exception-driven control flow altering determinism.

---

## 1.15 Implementation Integrity Statement

A compliant implementation MUST:

- Follow the Step pipeline exactly.
- Enforce invariants strictly.
- Prevent κ increase.
- Detect collapse solely via κ == 0.
- Separate evolution and diagnostics.
- Guarantee deterministic replay.

This concludes the implementation overview section.

---

# 2. Detailed Step() Implementation Walkthrough

## 2.1 StepResult and Status Mapping

The implementation MUST expose the specification-level EventFlag:

- NORMAL
- COLLAPSE
- ERROR

If an internal status code exists, it MUST map deterministically:

- OK              → NORMAL
- OK + first κ→0  → COLLAPSE
- Any abort        → ERROR

No additional externally visible terminal codes are permitted.

---

## 2.2 Required Inputs and Preconditions

Inputs to Step():

- Δ_input: fixed-size vector of dimension N
- dt: double, dt > 0

Preconditions:

- The Core instance was successfully initialized.
- Config is immutable and valid.
- X_current and X_previous satisfy invariants.
- No concurrent Step() invocation occurs on the same instance.

---

## 2.3 Canonical Step() Pseudocode (Normative Order)

The following pseudocode is normative for implementation structure.

```text
function Step(delta_input, dt):

  # Stage 1: Pre-validation
  if dt <= 0:
      return ERROR

  if any component of delta_input is not finite:
      return ERROR

  if any component of X_current is not finite:
      return ERROR

  # Stage 12: Terminal handling
  if lifecycle.terminal == true or X_current.kappa == 0:
      return NORMAL

  # Stage 2: Local state copy
  X_next = X_current

  # Stage 3: Admissible delta
  delta_step = AdmissibleDelta(delta_input, X_current, Config)
  if any component of delta_step is not finite:
      return ERROR

  # Enforce "no expansion" defensively (implementation guard)
  if Norm(delta_step) > Norm(delta_input) + eps_guard:
      return ERROR

  X_next.delta = X_current.delta + delta_step
  if any component of X_next.delta is not finite:
      return ERROR

  # Stage 4: Energy update
  phi_candidate = EnergyFunction(X_next.delta, X_current, Config)
  if phi_candidate is not finite:
      return ERROR
  X_next.phi = max(phi_candidate, 0)

  # Stage 5: Memory evolution
  fm = FM_normative(X_current, X_next, Config)
  dm = D_M_normative(X_current, X_next, Config)
  if fm is not finite or dm is not finite:
      return ERROR
  if fm < 0 or dm < 0:
      return ERROR

  m_candidate = X_current.memory + dt * (fm - dm)
  if m_candidate is not finite:
      return ERROR
  X_next.memory = max(m_candidate, 0)

  # Stage 6: Viability decay
  fk = Fkappa_normative(X_current, X_next, Config)
  if fk is not finite:
      return ERROR
  if fk > 0:
      return ERROR   # normative constraint: fkappa <= 0

  k_candidate = X_current.kappa + dt * fk
  if k_candidate is not finite:
      return ERROR

  X_next.kappa = max(k_candidate, 0)

  # Enforce monotonic non-increase
  if X_next.kappa > X_current.kappa:
      X_next.kappa = X_current.kappa

  # Stage 7: Invariant enforcement
  if X_next.phi < 0:
      X_next.phi = 0

  if X_next.memory < 0:
      X_next.memory = 0

  if X_next.kappa < 0:
      X_next.kappa = 0

  if X_next.kappa > X_current.kappa:
      return ERROR

  if any component of X_next is not finite:
      return ERROR

  # Stage 8: Collapse detection
  collapse_now = (X_current.kappa > 0 and X_next.kappa == 0)

  # Stage 9: Atomic commit
  X_previous = X_current
  X_current  = X_next
  lifecycle.step_counter += 1

  if collapse_now:
      if lifecycle.collapse_emitted == false:
          lifecycle.collapse_emitted = true
          lifecycle.terminal = true
          event = COLLAPSE
      else:
          # Defensive: should never happen
          lifecycle.terminal = true
          event = NORMAL
  else:
      event = NORMAL

  # Stage 10: Diagnostics (optional, post-commit only)
  if Config.diagnostics_enabled:
      RunDiagnostics(X_previous, X_current, dt, Config)
      # Diagnostic failure must not affect event

  return event
```
This order MUST be preserved.
No mutation to X_current may occur prior to atomic commit.

---

## 2.4 Implementation Guardrails

The implementation MUST include guardrails that enforce:

- No dynamic allocation during `Step()`.
- No exceptions escaping `Step()`.
- No branching on nondeterministic signals.
- No platform-dependent math shortcuts.

All guard thresholds (`eps_guard`, `eps_norm`, numeric caps, etc.) MUST be explicitly defined in `Config`.

No hidden hardcoded tolerances are permitted.

---

## 2.5 Deterministic Norm and Comparison

The norm function used for admissibility checks MUST be:

- Deterministic.
- Based on a fixed evaluation order.
- Free of parallel reduction.

### Recommended Implementation Strategy

- Compute sum of squares strictly in index order.
- Use `std::sqrt` only on verified finite values.
- Apply deterministic capping if required by `Config`.

Floating-point comparisons MUST:

- Use explicit deterministic tolerances only when mandated by `Config`.
- Avoid implicit platform-dependent epsilon assumptions.
- Avoid reliance on unordered comparisons.

---

## 2.6 Terminal State Consistency

Once `lifecycle.terminal` becomes `true`:

- `Step()` MUST become a no-op.
- `step_counter` MUST NOT increment.
- Diagnostics MAY run in static mode only.
- No structural coordinate may change.

Terminal state MUST remain irreversible until explicit reinitialization.

---

## 2.7 Failure Outcomes and Non-Mutation Guarantee

If `Step()` returns `ERROR`:

- `X_current` MUST remain unchanged.
- `X_previous` MUST remain unchanged.
- `lifecycle.step_counter` MUST NOT increment.
- `lifecycle.terminal` MUST NOT change.
- `lifecycle.collapse_emitted` MUST NOT change.

This non-mutation guarantee is mandatory.

---

## 2.8 Implementation Notes (Non-Normative)

The following recommendations do not alter normative requirements:

- Implement Step stages as private inline functions.
- Centralize invariant checks in a dedicated internal function.
- Ensure all normative evolution functions are pure.
- Keep diagnostic execution fully isolated from mutation logic.

These recommendations improve clarity and maintainability
but are not part of compliance validation.

---

# 3. Normative Function Implementations

## 3.1 General Requirements

All normative evolution functions MUST:

- Be pure functions.
- Have no side effects.
- Allocate no memory.
- Depend only on explicit inputs.
- Produce finite outputs for all admissible inputs.
- Contain no static mutable state.
- Contain no hidden global state.

Normative functions MUST NOT:

- Access LifecycleContext.
- Modify StructuralState.
- Trigger collapse directly.
- Perform commit operations.
- Call diagnostics.

All normative logic is executed through Step() only.

---

## 3.2 AdmissibleDelta Implementation

### 3.2.1 Purpose

AdmissibleDelta enforces structural contraction.

It prevents deviation expansion and ensures viability-dependent scaling.

### 3.2.2 Required Properties

The implementation MUST guarantee:

- ||Δ_step|| ≤ ||Δ_input||
- Δ_step == 0 when κ == 0
- Deterministic scaling
- No expansion beyond admissible domain
- Finite output

### 3.2.3 Recommended Implementation Pattern

A safe deterministic implementation pattern:

```text
alpha = ComputeAdmissibilityFactor(X_current, Config)

if alpha < 0:
    alpha = 0
if alpha > 1:
    alpha = 1

Δ_step = alpha * Δ_input
```
Constraints:
alpha MUST be deterministic.
alpha MUST be non-decreasing in κ.
alpha MUST be bounded [0, 1].
alpha MUST NOT depend on hidden history.

---

## 3.3 EnergyFunction Implementation

### 3.3.1 Purpose

`EnergyFunction` computes structural energy `Φ_next`
from `Δ_next` and the previous structural state.

---

### 3.3.2 Required Properties

`EnergyFunction` MUST:

- Produce finite output.
- Be deterministic.
- Never produce `NaN` or `Inf`.
- Not depend on uninitialized memory.
- Not depend on hidden or global state.

---

### 3.3.3 Implementation Constraints

`EnergyFunction` MAY:

- Use norms of `Δ`.
- Use weighted quadratic forms.
- Use bounded nonlinear functions.

`EnergyFunction` MUST NOT:

- Modify `κ`.
- Modify `memory`.
- Access `LifecycleContext`.
- Perform commit operations.

Negative output MUST be clamped to zero inside `Step()`.

---

## 3.4 Memory Evolution Implementation

### 3.4.1 FM_normative

`FM_normative` MUST:

- Be ≥ 0.
- Be finite.
- Depend only on `X_current` and `X_next`.
- Not modify state.
- Not depend on diagnostics.

Example admissible patterns include:

- Proportional to motion magnitude.
- Proportional to energy level.
- Bounded by caps defined in `Config`.

---

### 3.4.2 D_M_normative

`D_M_normative` MUST:

- Be ≥ 0.
- Be finite.
- Remain deterministic.
- Depend only on structural coordinates.

`D_M_normative` MAY increase under structural strain if defined.

Memory degradation MUST NOT depend on diagnostic outputs.

---

## 3.5 Viability Decay Implementation

### 3.5.1 Fkappa_normative

`Fkappa_normative` MUST:

- Be ≤ 0.
- Be finite.
- Be deterministic.
- Depend only on `X_current` and `X_next`.
- Not modify state.

If implementation produces `Fkappa_normative > 0`:

- `Step()` MUST treat this as `ERROR`.

---

### 3.5.2 Monotonic Enforcement

After computing:

k_candidate = κ_prev + dt * fk

Implementation MUST:

- Clamp to zero if negative.
- Clamp to `κ_prev` if positive.

Monotonic non-increase of `κ` is mandatory and enforced at runtime.

---

## 3.6 Numerical Stability Rules

All normative functions MUST:

- Guard divisions by small denominators.
- Cap exponentials deterministically.
- Bound nonlinear growth.
- Reject overflow before propagation.
- Validate intermediate results for finiteness.

All caps and thresholds MUST be defined in `Config`.

Hardcoded magic constants are prohibited.

---

## 3.7 Deterministic Evaluation Order

When computing:

- Vector norms
- Sums
- Dot products

Implementation MUST:

- Iterate in fixed index order.
- Avoid parallel reduction.
- Avoid compiler-driven reordering optimizations.
- Avoid architecture-dependent math intrinsics.

Floating-point determinism requires fixed evaluation sequence.

---

## 3.8 Normative Function Integrity Checks

Implementation SHOULD include optional debug assertions to verify:

- Admissibility contraction.
- κ non-increase.
- Memory non-negativity.
- Finite-only state components.

Debug checks MUST:

- Not alter execution order.
- Not modify state.
- Not introduce side effects.
- Be removable in release builds without altering logic.

---

## 3.9 Separation from Diagnostics

Normative functions MUST NOT:

- Access diagnostic data.
- Trigger diagnostic recomputation.
- Depend on diagnostic results.
- Share mutable state with the diagnostic layer.

Normative and diagnostic layers MUST remain strictly isolated.

---

## 3.10 Implementation Integrity Statement

Normative functions define the structural evolution mechanics of the system.

They MUST:

- Remain pure.
- Remain deterministic.
- Remain bounded.
- Respect all invariants.
- Be executed exclusively through `Step()`.

Violation of these requirements invalidates V2.2 compliance.

---

# 4. Serialization and Replay Implementation

## 4.1 Purpose

Serialization and replay mechanisms ensure deterministic reproducibility of structural evolution.

The implementation MUST support:

- Full state serialization
- Deterministic state restoration
- Exact replay under identical inputs

Replay consistency is mandatory for V2.2 compliance.

---

## 4.2 Serializable Components

The following components MUST be serialized:

1) StructuralState (X_current)
2) StructuralState (X_previous)
3) LifecycleContext
4) Config
5) Core specification version string

No other hidden runtime state may influence replay.

---

## 4.3 StructuralState Serialization

The serialization format MUST:

- Preserve raw IEEE-754 bit representation
- Preserve fixed delta dimension N
- Preserve exact value ordering
- Use fixed endianness

Serialization order MUST be:

1) delta[0] ... delta[N-1]
2) phi
3) memory
4) kappa

No padding bytes may be included.

---

## 4.4 LifecycleContext Serialization

LifecycleContext MUST serialize:

1) step_counter
2) terminal
3) collapse_emitted

Booleans MUST use fixed-size representation.

Serialization must not depend on compiler bool layout.

---

## 4.5 Config Serialization

Config MUST be serialized in full.

Requirements:

- All parameters written explicitly
- No derived runtime values
- No omitted default fields
- Deterministic field order
- Version-compatible format

Changing Config invalidates replay equivalence.

---

## 4.6 Version Tagging

The runtime MUST embed:

core_spec_version = "MAX-Core-Spec-V2.2"

The version string MUST:

- Be serialized
- Be exposed through API
- Be included in replay validation
- Remain immutable

Replay MUST fail if version mismatch occurs.

---

## 4.7 Replay Procedure

Replay MUST follow this exact order:

1) Load serialized Config
2) Load serialized StructuralState
3) Load LifecycleContext
4) Validate invariants
5) Execute identical sequence of:
   - delta_input
   - dt
6) Compare final StructuralState

Replay is valid only if:

- All committed states match exactly
- Collapse occurs at identical step
- Event sequence is identical

Bitwise identity is required under identical environment.

---

## 4.8 Replay Validation Strategy

Implementation SHOULD support:

- Deterministic state hashing
- Snapshot comparison
- Collapse step index verification

State hash MUST:

- Be deterministic
- Be based only on serialized data
- Not depend on memory addresses

---

## 4.9 Serialization Format Constraints

Serialization MUST:

- Be deterministic
- Avoid platform-dependent structures
- Avoid pointer serialization
- Avoid STL container metadata
- Avoid locale-dependent formatting

Binary format is recommended.

Text format is allowed only if strictly defined and deterministic.

---

## 4.10 Floating-Point Environment Consistency

Replay guarantees require identical:

- Compiler
- Optimization flags
- Floating-point mode
- Rounding mode
- Standard library implementation

Cross-platform replay is not guaranteed unless environment is identical.

---

## 4.11 Failure Handling in Replay

If during replay:

- Invariants fail
- Version mismatch occurs
- Config mismatch occurs
- Structural mismatch detected

Replay MUST abort deterministically.

Silent divergence is prohibited.

---

## 4.12 Serialization Integrity Statement

A V2.2 compliant implementation MUST guarantee:

- Complete state capture
- Deterministic restoration
- Identical structural evolution under identical inputs
- No hidden state influencing behavior

Replay integrity is mandatory for certification.

---

# 5. Diagnostic Layer Implementation

## 5.1 Purpose

The Diagnostic Layer provides deterministic structural telemetry.

It operates strictly after atomic commit.

It MUST NOT:

- Modify StructuralState
- Modify LifecycleContext
- Influence admissibility
- Influence viability decay
- Trigger collapse
- Affect StepResult

Diagnostics are observational only.

---

## 5.2 Execution Order

Diagnostics MUST execute only after successful atomic commit.

Execution order:

1) Receive const reference to X_previous
2) Receive const reference to X_current
3) Receive dt
4) Compute derived metrics
5) Store diagnostic outputs
6) Return control to caller

Diagnostics MUST NOT execute if Step() returns ERROR.

---

## 5.3 Diagnostic Isolation

Implementation MUST enforce:

- Const access only
- No mutable references
- No access to Step Engine internals
- No access to Snapshot Manager mutation methods
- No access to Lifecycle mutation

Diagnostic layer must compile independently of mutation logic.

---

## 5.4 Diagnostic Modules

The following diagnostic modules MAY be implemented:

- Flow Diagnostics
- Space Diagnostics
- Time Diagnostics
- Field Diagnostics
- Intelligence Diagnostics

Each module MUST:

- Be deterministic
- Allocate no memory during execution
- Produce finite outputs
- Fail independently

Modules MUST NOT depend on each other implicitly.

---

## 5.5 Flow Diagnostics Implementation

Flow diagnostics compute:

dX/dt = (X_current − X_previous) / dt

Requirements:

- Deterministic evaluation
- Finite-only results
- Zero output if dt == 0 (should not occur)
- Zero output if terminal state

No mutation allowed.

---

## 5.6 Space Diagnostics Implementation

Space diagnostics MAY compute:

- Norm of Δ
- Distance increments
- Viability margin
- Configured structural bounds

Norm calculations MUST follow deterministic rules from Section 2.5.

---

## 5.7 Time Diagnostics Implementation

Time diagnostics MAY compute:

- Structural time field
- Time gradients
- Bounded temporal projections

If κ == 0:

- Time value MUST be zero
- Time derivatives MUST be zero

Time diagnostics MUST NOT redefine collapse.

---

## 5.8 Field Diagnostics Implementation

Field diagnostics MAY compute:

F(X_current)

Requirements:

- Deterministic
- Finite
- Consistent sign with normative κ decay
- No mutation

Field diagnostics are descriptive only.

---

## 5.9 Diagnostic Failure Handling

If a diagnostic module produces:

- NaN
- Inf
- Overflow

Then:

- Output MUST be replaced with zero
- Module marked as failed
- StepResult remains unchanged

Diagnostic failure MUST NOT propagate.

---

## 5.10 Deterministic Output Storage

Diagnostic outputs MUST:

- Use fixed memory layout
- Avoid dynamic allocation
- Be stored separately from StructuralState
- Be serializable if replay validation requires

Diagnostic data MUST NOT affect replay equivalence of structural evolution.

---

## 5.11 Optional Diagnostic Disable

If Config disables diagnostics:

- No diagnostic code may execute
- No performance branch must affect evolution
- Step execution order must remain unchanged

Diagnostic enable flag must not affect mutation logic.

---

## 5.12 Diagnostic Integrity Statement

Diagnostic layer MUST:

- Remain isolated
- Remain deterministic
- Remain subordinate
- Never influence structural evolution

Violation invalidates V2.2 compliance.

---

# 6. Compliance and Self-Test Infrastructure

## 6.1 Purpose

This section defines the internal mechanisms required to validate
that an implementation conforms to MAX Core V2.2.

Compliance verification MUST ensure:

- Deterministic evolution
- Strict invariant enforcement
- Monotonic κ behavior
- Correct collapse semantics
- Atomic snapshot guarantees
- Replay reproducibility
- Diagnostic non-interference

Compliance infrastructure is mandatory for certification.

---

## 6.2 Self-Test Categories

The implementation MUST provide automated self-tests
covering the following categories:

1) Initialization tests
2) Step determinism tests
3) Invariant enforcement tests
4) Collapse detection tests
5) Terminal state tests
6) Error handling tests
7) Replay validation tests
8) Diagnostic isolation tests

Each category MUST be independently executable.

---

## 6.3 Initialization Tests

Initialization tests MUST verify:

- Rejection of non-finite inputs
- Rejection of negative κ
- Acceptance of κ == 0
- Proper lifecycle initialization
- step_counter == 0
- terminal flag correctly set

Invalid initialization MUST not partially mutate state.

---

## 6.4 Deterministic Step Tests

Deterministic tests MUST verify:

- Identical input sequence produces identical state trajectory
- Identical build produces identical collapse step
- Repeated execution yields identical results

Bitwise comparison of serialized state is recommended.

---

## 6.5 Invariant Enforcement Tests

Tests MUST confirm:

- κ never increases
- κ never becomes negative
- memory never becomes negative
- phi never becomes negative
- No committed state contains NaN or Inf

Attempted invariant violations MUST trigger ERROR.

---

## 6.6 Collapse Detection Tests

Collapse tests MUST verify:

- Collapse emitted exactly once
- Collapse only when κ transitions to zero
- No collapse if κ > 0
- No repeated collapse emission
- Terminal state remains immutable

Collapse timing MUST be deterministic.

---

## 6.7 Terminal State Tests

Terminal tests MUST verify:

- Step() becomes no-op after collapse
- step_counter does not increment
- StructuralState remains unchanged
- Diagnostics may execute without mutation

Terminal state MUST be irreversible.

---

## 6.8 Error Handling Tests

Error tests MUST verify:

- ERROR return prevents commit
- State remains unchanged on ERROR
- LifecycleContext unchanged on ERROR
- Diagnostics do not execute on ERROR

Each error path MUST be covered.

---

## 6.9 Replay Validation Tests

Replay tests MUST:

1) Serialize state mid-run
2) Reload state
3) Continue execution
4) Compare final state

Replay MUST reproduce:

- Identical StructuralState
- Identical collapse timing
- Identical step_counter
- Identical event sequence

Replay mismatch invalidates compliance.

---

## 6.10 Diagnostic Isolation Tests

Diagnostic isolation tests MUST verify:

- Diagnostics cannot mutate StructuralState
- Diagnostics cannot modify κ
- Diagnostics cannot alter lifecycle
- Diagnostic failure does not affect Step result

Diagnostic enable/disable MUST not change evolution outcome.

---

## 6.11 Stress and Boundary Tests

Stress tests SHOULD include:

- Large Δ_input magnitude
- Near-zero κ
- Very small dt
- Long execution sequences
- Edge-case Config parameters

System MUST remain deterministic and finite.

---

## 6.12 Compliance Reporting

Implementation SHOULD provide:

- Test summary report
- Determinism verification log
- Replay verification result
- Version declaration
- Build configuration record

Compliance report MUST include:

- Spec version
- Compiler version
- Build flags
- Platform description

---

## 6.13 Continuous Integration Requirement

For production certification:

- All tests MUST run in CI pipeline
- Deterministic replay test MUST be mandatory
- Build flags must be fixed
- Fast-math must be disallowed

CI must fail if any invariant test fails.

---

## 6.14 Compliance Integrity Statement

A V2.2 compliant implementation MUST demonstrate:

- Structural determinism
- Strict invariant enforcement
- Normative κ decay
- Correct collapse semantics
- Atomic snapshot behavior
- Diagnostic isolation
- Full replay reproducibility

Failure in any category invalidates compliance.