# MAX Core Specification V1.1
## Deterministic Structural Evolution Engine

---

# 1. Scope and Purpose

## 1.1 Objective

The MAX Core defines a deterministic structural evolution engine
operating on the canonical Flexion state vector:

X = (Δ, Φ, M, κ)

where:

- Δ — structural deviation vector
- Φ — structural energy (non-negative scalar)
- M — structural memory (monotonic non-decreasing scalar)
- κ — structural viability (monotonic non-increasing scalar)

The Core implements structural evolution strictly according to
deterministic rules, preserving invariant constraints and collapse semantics.

---

## 1.2 Design Principles

The MAX Core MUST satisfy:

1. Determinism  
   Identical binary build + identical configuration + identical input sequence  
   MUST produce bitwise-identical structural state trajectory.

2. Invariant Preservation  
   - Φ ≥ 0  
   - M is monotonic non-decreasing  
   - κ is monotonic non-increasing  
   - κ = 0 is terminal for the current structural cycle  

3. Separation of Evolution and Observation  
   - Step() mutates structural state.  
   - Telemetry functions MUST NOT mutate structural state.  

4. Collapse Irreversibility  
   Once κ becomes 0, the current structural instance
   is irreversibly collapsed.

---

## 1.3 Non-Scope

The MAX Core does NOT:

- interpret structure,
- perform optimization,
- perform control,
- perform learning,
- restore collapsed instances,
- modify admissible futures,
- expand viability.

It is strictly a structural evolution engine.

---

## 1.4 Version Alignment

Version 1.1 resolves:

- monotonicity enforcement ambiguity,
- telemetry error separation,
- collapse mode clarification,
- deterministic replay boundaries.

This version supersedes V1.0.

---

# 2. Structural State Definition

## 2.1 Canonical State Vector

The structural state X is defined as:

X = (Δ, Φ, M, κ)

where:

- Δ ∈ ℝ^N            — deviation vector (fixed dimension N)
- Φ ∈ ℝ≥0            — structural energy
- M ∈ ℝ≥0            — structural memory
- κ ∈ ℝ≥0            — structural viability

The dimension N of Δ MUST be fixed at initialization
and MUST NOT change during the lifetime of the structural cycle.

---

## 2.2 Storage Requirements

The Core MUST:

- store Δ in a fixed-size contiguous buffer,
- prevent dynamic resizing after initialization,
- avoid memory reallocation during Step(),
- ensure deterministic memory layout.

State mutation MUST occur only inside Step().

---

## 2.3 State Validity Constraints

At all times the following MUST hold:

1. Φ ≥ 0
2. M ≥ 0
3. κ ≥ 0
4. M_new ≥ M_prev
5. κ_new ≤ κ_prev

If numerical drift causes violation of (4) or (5),
the Core MUST deterministically enforce:

- M_new = max(M_new, M_prev)
- κ_new = min(κ_new, κ_prev)

Such enforcement MUST NOT trigger ERROR.

---

## 2.4 Finite Arithmetic Requirement

All structural values MUST remain finite IEEE-754 double values.

If any intermediate computation produces:

- NaN
- ±Inf

during Step(),

the Core MUST:

- abort the mutation,
- leave the state unchanged,
- return ERROR event.

---

## 2.5 Structural Cycle Identity

Each structural instance belongs to a structural cycle.

A cycle is defined as the evolution of X from initialization
until κ becomes 0.

When κ reaches 0:

- the current cycle is terminal,
- the state MUST NOT evolve further,
- the cycle identity remains immutable.

New cycles MAY be created only through explicit
reinitialization (see Section 6).

---

# 3. Evolution Operator (Step Function)

## 3.1 Step Definition

The structural evolution operator is defined as:

Step(X, Δ_input) → event_flag

where:

- X is the current structural state,
- Δ_input is an external deviation stimulus,
- event_flag ∈ { NORMAL, COLLAPSE, ERROR }.

Step() is the only function permitted to mutate X.

---

## 3.2 Deterministic Evolution Order

The evolution order MUST be strictly:

1. Δ update
2. Optional regime transform
3. Φ update
4. M update
5. κ update
6. Monotonic enforcement
7. Collapse gate evaluation

No reordering is permitted.

---

## 3.3 Δ Update

Δ_new = Δ_prev + Δ_input

If regime transform is enabled:

Δ_eff = RegimeTransform(Δ_new)

Otherwise:

Δ_eff = Δ_new

Δ_eff MUST be used for all subsequent computations.

---

## 3.4 Φ Update

Φ_candidate = Φ(Δ_eff)

If Φ_candidate < 0 due to numerical drift:

Φ_candidate MUST be clamped to 0.

Φ_new = Φ_candidate

---

## 3.5 M Update

M_candidate = M_prev + M_increment(Δ_eff, Φ_new)

M_new = max(M_candidate, M_prev)

Memory MUST be monotonic non-decreasing.

---

## 3.6 κ Update

κ_candidate = κ_prev - κ_decay(Δ_eff, Φ_new, M_new)

If κ_candidate < 0:

κ_candidate = 0

κ_new = min(κ_candidate, κ_prev)

Viability MUST be monotonic non-increasing.

---

## 3.7 Collapse Gate

Collapse is triggered when:

κ_new == 0

after monotonic clamping.

If collapse is triggered:

- κ_new MUST remain 0,
- event_flag MUST be COLLAPSE,
- the structural cycle becomes terminal.

Otherwise:

event_flag MUST be NORMAL.

---

## 3.8 ERROR Conditions

Step MUST return ERROR if:

- any intermediate arithmetic result is non-finite,
- internal invariant enforcement cannot be performed safely.

When ERROR is returned:

- X MUST remain unchanged,
- no partial mutation is permitted.

---

## 3.9 Deterministic Replay

For identical:

- binary build,
- configuration,
- input sequence,

Step() MUST produce bitwise-identical state transitions.

Cross-platform determinism is not guaranteed unless
floating-point implementation and math libraries are identical.

---

# 4. Telemetry and Observation

## 4.1 Observational Nature

Telemetry is observational and MUST NOT:

- mutate structural state X,
- influence structural evolution,
- modify event trajectory,
- affect collapse semantics.

Telemetry is strictly read-only.

---

## 4.2 Telemetry Definition

Telemetry(X) → telemetry_struct

Telemetry MAY compute derived structural metrics such as:

- structural time functional T(X),
- deviation velocity vΔ,
- curvature estimates,
- structural reversibility index (SRI),
- regime indicators.

All telemetry values MUST be derived exclusively from the
current structural state.

---

## 4.3 No Evolutionary Side Effects

Calling Telemetry():

- MUST NOT alter X,
- MUST NOT alter hidden state affecting Step(),
- MUST NOT emit NORMAL/COLLAPSE/ERROR events.

Structural events are reserved exclusively for Step().

---

## 4.4 Telemetry Error Isolation

If telemetry computation encounters:

- non-finite arithmetic,
- invalid derived state,

the Core MUST:

- return telemetry_status = TELEMETRY_ERROR,
- leave X unchanged,
- NOT emit structural ERROR event,
- NOT alter event trajectory.

Telemetry errors are observational and
do not affect structural integrity.

---

## 4.5 Deterministic Telemetry

For identical:

- binary build,
- configuration,
- structural state,

Telemetry() MUST produce bitwise-identical outputs.

If transcendental functions (e.g., exp) are used,
their implementation MUST be fixed and deterministic
within the build configuration.

---

## 4.6 Optional Telemetry Cache

The Core MAY maintain internal cache values
for previously computed telemetry metrics,
provided that:

- cache does not influence structural evolution,
- cache is replay-deterministic,
- cache is fully derivable from prior structural states.

Such cache has no ontological status.

---

# 5. Collapse Semantics and Post-Collapse Modes

## 5.1 Collapse Definition

A structural cycle collapses when:

κ_new == 0

after monotonic enforcement.

Collapse is terminal for the current structural instance.

Collapse MUST be irreversible within the same cycle.

---

## 5.2 First Collapse Event

When κ becomes 0 for the first time:

- event_flag MUST be COLLAPSE,
- state X MUST be committed with κ = 0,
- no further structural evolution is permitted
  within the current cycle.

---

## 5.3 Terminal State Behavior

After collapse, the Core MUST operate in one
of the configured post-collapse modes.

Collapse does NOT imply memory reset.
Collapsed state remains structurally valid but terminal.

---

## 5.4 Post-Collapse Modes

The Core MUST support the following modes:

### Mode A — Frozen

- Step() MUST NOT mutate X.
- Step() MUST return NORMAL
  with terminal-state indicator.
- Repeated COLLAPSE events MUST NOT be emitted.

### Mode B — Strict Terminal

- Step() MUST return ERROR.
- No mutation is permitted.

### Mode C — Auto Regenesis

- The collapsed cycle remains terminal.
- A new structural cycle is created.
- State is reinitialized deterministically.
- cycle_id MUST increment.
- The new cycle is independent of the previous one.

Auto Regenesis MUST NOT restore the collapsed instance.
It creates a new structural trajectory.

---

## 5.5 Cycle Identity

Each structural cycle MUST have a cycle identifier.

The identifier MUST:

- remain constant within a cycle,
- increment only during explicit reinitialization,
- never change during normal evolution.

---

## 5.6 No Resurrection Principle

Under no circumstance may κ increase
from 0 to a positive value within
the same structural cycle.

Any such attempt MUST result in ERROR.

---

# 6. Initialization and Reinitialization

## 6.1 Initialization

The Core MUST provide deterministic initialization:

Init(config, seed) → X₀

Initialization MUST:

- set Δ to defined initial deviation,
- set Φ ≥ 0,
- set M ≥ 0,
- set κ > 0,
- assign cycle_id = initial value.

All initial values MUST be finite.

Initialization MUST NOT depend on:

- system clock,
- random device,
- external mutable state,

unless explicitly provided through deterministic seed.

---

## 6.2 Deterministic Configuration

All structural behavior MUST be defined by configuration parameters:

- Δ dimension N
- Φ function parameters
- M increment parameters
- κ decay parameters
- regime configuration
- post-collapse mode

Configuration MUST be immutable
after initialization of a cycle.

---

## 6.3 Explicit Reinitialization

Reinitialization is defined as:

Reinit(new_config, seed) → new cycle

Reinitialization MUST:

- create a new structural state,
- increment cycle_id,
- reset Δ, Φ, M, κ,
- clear terminal status.

Reinitialization MUST NOT:

- modify prior cycle state,
- reuse κ from collapsed cycle,
- merge memory between cycles.

Each cycle is ontologically independent.

---

## 6.4 Auto Regenesis (Mode C)

If Mode C is enabled and collapse occurs:

- The Core MUST internally invoke deterministic reinitialization.
- The previous cycle remains terminal and immutable.
- A new cycle_id MUST be assigned.
- Evolution continues in the new cycle.

Auto Regenesis MUST be deterministic.

---

## 6.5 State Serialization

The Core MUST support serialization of:

- Δ
- Φ
- M
- κ
- cycle_id
- configuration parameters

Serialization MUST be:

- deterministic,
- reversible,
- sufficient for exact replay.

Deserialized state MUST reproduce
bitwise-identical future evolution
under identical inputs.

---

# 7. Determinism, Safety, and Compliance

## 7.1 Deterministic Replay Guarantee

For identical:

- binary build,
- configuration,
- initial state,
- input sequence,

the Core MUST produce:

- bitwise-identical structural states,
- identical event trajectory,
- bitwise-identical telemetry outputs.

Replay determinism applies within the same
floating-point implementation and math library.

---

## 7.2 Floating-Point Discipline

The Core MUST:

- use IEEE-754 double precision,
- disable fast-math optimizations,
- avoid undefined behavior,
- avoid non-deterministic parallel execution.

Transcendental functions (e.g., exp)
MUST be version-locked or replaced
with deterministic implementations.

---

## 7.3 Memory and Allocation Constraints

During Step():

- no dynamic memory allocation is permitted,
- no resizing of Δ is permitted,
- no mutation outside X is permitted.

Memory layout MUST remain stable
throughout a structural cycle.

---

## 7.4 Safety Guarantees

The Core MUST guarantee:

1. Invariant Preservation  
   Φ ≥ 0  
   M monotonic non-decreasing  
   κ monotonic non-increasing  

2. Collapse Irreversibility  
   κ cannot increase from 0 within a cycle.

3. Observation Separation  
   Telemetry cannot mutate structural state.

4. Atomic Step  
   Step() is atomic:
   either full mutation succeeds,
   or state remains unchanged (ERROR).

---

## 7.5 Compliance Criteria

An implementation is compliant with
MAX Core Specification V1.1 if and only if:

- All sections of this specification are satisfied,
- Deterministic replay tests pass,
- Invariant preservation tests pass,
- Collapse mode tests pass,
- Serialization round-trip tests pass,
- No undefined behavior is detected under sanitizers.

---

## 7.6 Structural Integrity Statement

The MAX Core is a deterministic structural evolution engine.

It:

- does not interpret structure,
- does not modify admissible futures,
- does not expand viability,
- does not restore collapsed instances,
- does not perform control or optimization.

It preserves structural invariants,
collapse semantics,
and deterministic evolution.

---

End of Specification
MAX-Core-Specification-V1.1-EN
