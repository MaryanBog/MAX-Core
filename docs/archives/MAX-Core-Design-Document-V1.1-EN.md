# MAX Core Design Document V1.1
## Deterministic Structural Evolution Architecture

---

# 1. Architectural Overview

## 1.1 Purpose

This document defines the software architecture of the MAX Core,
a deterministic structural evolution engine implementing:

X = (Δ, Φ, M, κ)

The Design strictly conforms to:

- MAX-Core-Specification-V1.1-EN
- Collapse irreversibility
- Separation of evolution and observation
- Deterministic replay guarantees

---

## 1.2 Structural State Principle

The structural state X is the only ontological mutable state.

X = (Δ, Φ, M, κ)

Only Step() is permitted to mutate X.

All other mutable data (e.g., cycle counters, replay markers)
are classified as meta-state and:

- MUST NOT affect structural evolution,
- MUST NOT affect deterministic replay,
- MUST NOT alter invariants.

Meta-state is not part of X.

---

## 1.3 Architectural Modules

The MAX Core is composed of:

1. Core Evolution Engine
2. Regime Transform Module (optional)
3. Collapse Gate
4. Telemetry Module (read-only)
5. Lifecycle Manager
6. Serialization Layer

Each module is strictly separated.

No module except the Evolution Engine
may mutate structural state.

---

## 1.4 Deterministic Execution Model

The execution pipeline for Step() is:

Δ update
→ optional Regime transform
→ Φ computation
→ M update
→ κ update
→ invariant enforcement
→ collapse gate

No module may reorder these stages.

---

## 1.5 Memory Discipline

During Step():

- No dynamic memory allocation is allowed.
- Δ storage must be fixed-size and contiguous.
- No resizing is permitted.
- No hidden mutable buffers may influence evolution.

All structural memory is explicit.

---

## 1.6 Collapse Semantics

When κ becomes 0:

- The structural cycle is terminal.
- Evolution halts.
- The collapsed state remains immutable.
- Regeneration is permitted only via explicit new cycle.

The collapsed instance is never restored.

---

## 1.7 Deterministic Replay Boundary

Replay determinism is guaranteed for:

- identical binary build,
- identical configuration,
- identical input sequence.

Cross-platform bitwise identity
is not guaranteed unless math implementation is identical.

---

## 1.8 Non-Goals

The MAX Core does not:

- perform control,
- perform optimization,
- perform interpretation,
- expand admissible futures,
- increase viability after collapse,
- modify structural ontology.

It is a deterministic evolution engine only.

---

# 2. Core State Architecture

## 2.1 Canonical Structural State

The structural state X is defined strictly as:

X = (Δ, Φ, M, κ)

Where:

- Δ — fixed-dimension deviation vector
- Φ — non-negative structural energy
- M — non-negative structural memory
- κ — non-negative structural viability

No additional fields belong to X.

---

## 2.2 State Representation

The Core MUST represent X as:

struct StructuralState {
    double Δ[N];      // fixed dimension
    double Φ;
    double M;
    double κ;
};

Where:

- N is defined at initialization.
- Δ is stored in a contiguous fixed-size buffer.
- The size of Δ MUST NOT change during a cycle.
- No reallocation of Δ is permitted.

If N is runtime-defined, the buffer must be allocated once at Init()
and locked for the duration of the cycle.

std::vector with dynamic resizing is prohibited.

---

## 2.3 Meta-State Separation

Meta-state MAY include:

- cycle_id
- step_counter
- replay markers
- audit flags

Meta-state MUST:

- not influence structural evolution,
- not alter Step() behavior,
- not alter collapse detection,
- not participate in invariant enforcement.

Meta-state is stored outside StructuralState.

Example:

struct RuntimeContext {
    uint64_t cycle_id;
    uint64_t step_counter;
    PostCollapseMode mode;
};

StructuralState remains ontologically pure.

---

## 2.4 Invariant Enforcement Layer

Invariant enforcement is part of the Evolution Engine,
not part of state storage.

After computing Φ, M, κ:

- Φ is clamped to ≥ 0
- M is clamped to ≥ previous M
- κ is clamped to ≥ 0

No structural rule enforces κ_new ≤ κ_prev.
Monotonic κ decrease is not assumed as a universal law.
Collapse is triggered strictly by κ reaching 0.

This removes artificial structural death constraints.

---

## 2.5 Finite Arithmetic Discipline

All fields in StructuralState MUST always remain finite.

If non-finite arithmetic occurs during Step():

- mutation MUST be aborted,
- StructuralState MUST remain unchanged,
- ERROR event is returned.

StructuralState must never contain NaN or Inf.

---

## 2.6 State Immutability Outside Step()

Outside Step():

- StructuralState is read-only.
- Telemetry MUST access state via const reference.
- Serialization MUST not mutate state.

Mutation of X outside Step() is a design violation.

---

## 2.7 Atomicity Guarantee

Step() MUST operate atomically:

- Either the entire StructuralState is updated,
- Or no mutation occurs.

Partial mutation is prohibited.

This ensures deterministic replay integrity.

---

# 3. Evolution Engine Architecture

## 3.1 Core Evolution Engine Role

The Evolution Engine is responsible for:

- applying Step() updates to StructuralState,
- enforcing invariants deterministically,
- generating structural event flags,
- detecting collapse via κ == 0.

It is the only module allowed to mutate X.

---

## 3.2 Step Interface

The canonical evolution call is:

Step(StructuralState* X, const double* Δ_input, RuntimeContext* ctx) -> EventFlag

Where:

- X is mutated only if the step succeeds,
- Δ_input is a fixed-length stimulus vector of dimension N,
- ctx contains meta-state (cycle_id, step_counter, mode),
- EventFlag ∈ { NORMAL, COLLAPSE, ERROR }.

The ABI may wrap this signature, but semantics are identical.

---

## 3.3 Deterministic Evolution Pipeline

Step() MUST apply the following stages strictly in order:

1. Δ update:
   Δ_new = Δ_prev + Δ_input

2. Optional regime transform:
   Δ_eff = RegimeTransform(Δ_new)  (if enabled)
   else Δ_eff = Δ_new

3. Φ computation:
   Φ_new = Φ(Δ_eff), clamped to Φ_new ≥ 0

4. M update:
   M_candidate = M_prev + M_increment(Δ_eff, Φ_new)
   M_new = max(M_candidate, M_prev)

5. κ update:
   κ_candidate = κ_prev - κ_decay(Δ_eff, Φ_new, M_new)
   κ_new = max(κ_candidate, 0)

6. Invariant enforcement:
   Φ_new ≥ 0
   M_new ≥ M_prev
   κ_new ≥ 0

7. Collapse gate:
   if κ_new == 0 -> COLLAPSE else NORMAL

No reordering is permitted.

---

## 3.4 Atomic Update Strategy

To guarantee atomicity:

- compute next state into a local buffer X_next,
- validate finiteness and invariants on X_next,
- commit X_next to X only on success.

If any failure occurs:

- X remains unchanged,
- Step returns ERROR.

---

## 3.5 ERROR Handling

Step MUST return ERROR when:

- any intermediate computation becomes non-finite,
- Δ_input contains non-finite values,
- state X contains non-finite values at entry,
- safe invariant enforcement cannot be performed.

ERROR MUST NOT:

- partially mutate state,
- emit collapse,
- alter meta-state beyond safe logging (optional).

---

## 3.6 Step Counter Semantics (Meta-State)

If ctx->step_counter is used:

- it increments only when Step succeeds (NORMAL or COLLAPSE),
- it MUST NOT affect evolution computations,
- it MUST NOT affect event classification.

The counter is meta-state only.

---

## 3.7 Deterministic No-Allocation Rule

During Step():

- no heap allocations are allowed,
- no resizing of buffers is allowed,
- no I/O is allowed,
- no randomness is allowed.

All behavior is purely deterministic.

---

# 4. Regime Transform Module

## 4.1 Purpose

The Regime Transform Module provides an optional deterministic mapping:

RegimeTransform : Δ → Δ

Its purpose is to apply a configured structural transform to Δ
before Φ, M, and κ updates.

The Regime Transform:

- MUST be deterministic,
- MUST be pure (no side effects),
- MUST NOT allocate memory during Step(),
- MUST NOT depend on external mutable state,
- MUST preserve finiteness.

---

## 4.2 Position in the Pipeline

If enabled, RegimeTransform MUST be applied exactly once
after Δ update and before Φ computation:

Δ_new = Δ_prev + Δ_input  
Δ_eff = RegimeTransform(Δ_new)  
Φ_new = Φ(Δ_eff)  
...

If disabled:

Δ_eff = Δ_new

No other stage is permitted to apply regime logic.

---

## 4.3 Interface

A compliant interface is:

RegimeTransform(Δ_in[N], Δ_out[N], config) -> status

Where:

- Δ_in and Δ_out are fixed-size buffers of equal dimension N,
- config is immutable for the cycle,
- status indicates success/failure.

Failure MUST result in Step() returning ERROR
with no mutation of structural state.

---

## 4.4 Safety Rules

The Regime Transform MUST reject or fail if:

- any input element is non-finite,
- any output element becomes non-finite.

A Regime Transform MUST NOT:

- change the dimension N,
- change buffer ownership,
- write outside bounds,
- introduce randomization.

---

## 4.5 Deterministic Guarantees

For identical binary build and configuration:

RegimeTransform(Δ) MUST be bitwise deterministic.

If the transform uses floating-point operations,
it MUST follow the same floating-point discipline as Step()
(no fast-math, no non-deterministic library calls).

---

## 4.6 Non-Goals

The Regime Transform does NOT:

- classify regimes,
- predict outcomes,
- optimize states,
- interpret structure.

It is a configured deterministic transform only.

---

# 5. Telemetry Module Architecture

## 5.1 Purpose and Ontological Status

Telemetry provides read-only diagnostics computed from StructuralState.

Telemetry is observational and MUST NOT:

- mutate StructuralState,
- influence Step() behavior,
- affect event flags,
- alter collapse semantics.

Telemetry outputs have no ontological status.
They are derived functionals for monitoring and analysis.

---

## 5.2 Telemetry Interface

A compliant interface is:

Telemetry(const StructuralState* X, const RuntimeContext* ctx, TelemetryOut* out)
    -> TelemetryStatus

Where:

- X is read-only,
- ctx is read-only meta-state,
- out is a caller-provided output structure,
- TelemetryStatus ∈ { TELEMETRY_OK, TELEMETRY_ERROR }.

Telemetry MUST NOT emit NORMAL/COLLAPSE/ERROR events.

---

## 5.3 Derived Metrics (Examples)

Telemetry MAY compute any of the following (non-exhaustive):

- Deviation norm: ||Δ||
- Structural time functional: T(X)
- Deviation velocity: vΔ (if sufficient history exists)
- Curvature proxies (implementation-defined)
- Reversibility metrics (e.g., SRI) if deterministic

All metrics MUST be computed exclusively from:

- current state X
- optional prior states stored in meta-state history (see 5.6)

Telemetry MUST NOT attempt to reconstruct or evolve X.

---

## 5.4 Error Isolation

If Telemetry computation encounters:

- NaN / Inf
- invalid derived arithmetic
- unsupported computation due to missing history

Telemetry MUST:

- return TELEMETRY_ERROR,
- leave X unchanged,
- leave ctx unchanged,
- NOT affect event trajectory.

Telemetry errors are observational and non-fatal.

---

## 5.5 Determinism Requirements

For identical:

- binary build,
- configuration,
- state X,
- meta-state history (if used),

Telemetry outputs MUST be bitwise deterministic.

If transcendental functions are used (e.g., exp),
their implementations MUST be fixed and deterministic
within the build configuration.

If this cannot be guaranteed, the metric MUST be removed
or replaced by a deterministic approximation.

---

## 5.6 Optional History for Telemetry

Telemetry MAY require history (e.g., to compute vΔ or acceleration).

History, if used, MUST be implemented as:

- snapshot copies of prior StructuralState values,
- stored in meta-state (Lifecycle Manager),
- updated deterministically after successful Step(),
- never influencing Step() computations.

History MUST NOT be maintained inside Telemetry
as hidden mutable internal state.

Telemetry reads history but does not manage it.

---

## 5.7 Post-Collapse Telemetry

After collapse (κ == 0):

- StructuralState is terminal and immutable.
- Telemetry MAY still be computed as a static diagnostic.
- Any time-like or flow-like telemetry values MUST NOT be interpreted
  as ongoing evolution.

If a metric depends on motion (e.g., vΔ),
Telemetry MUST report:

- 0, or
- “undefined” via a status flag,
depending on the metric contract.

Telemetry MUST NOT imply continuation of structural time
after collapse.

---

# 6. Lifecycle Manager and Post-Collapse Modes

## 6.1 Role of the Lifecycle Manager

The Lifecycle Manager governs:

- structural cycle identity,
- post-collapse behavior,
- deterministic reinitialization,
- meta-state bookkeeping.

It MUST NOT:

- mutate StructuralState directly,
- alter Step() internal computations,
- bypass invariant enforcement.

Lifecycle management is strictly external to structural evolution.

---

## 6.2 Structural Cycle Definition

A structural cycle is defined as:

- initialization of StructuralState with κ > 0,
- deterministic evolution under Step(),
- termination when κ == 0.

Each cycle has a unique cycle_id stored in meta-state.

cycle_id:

- MUST remain constant within a cycle,
- MUST increment only during explicit reinitialization,
- MUST NOT influence evolution.

---

## 6.3 Post-Collapse Modes

The Lifecycle Manager MUST support the following modes:

### Mode A — Frozen

After first COLLAPSE event:

- Step() MUST NOT mutate StructuralState,
- Step() MUST return NORMAL,
- A terminal-state indicator MAY be set in meta-state,
- COLLAPSE MUST NOT be emitted again.

### Mode B — Strict Terminal

After collapse:

- Step() MUST immediately return ERROR,
- No mutation is permitted,
- Telemetry remains allowed.

### Mode C — Auto Regenesis

Upon collapse:

- The current cycle remains terminal,
- A new StructuralState is created deterministically,
- cycle_id MUST increment,
- Step() continues with the new cycle.

Auto Regenesis MUST NOT restore the collapsed state.
It creates a new independent structural trajectory.

---

## 6.4 No Resurrection Enforcement

If κ == 0 within a cycle:

- Any attempt to increase κ MUST result in ERROR,
- No mutation is allowed within the same cycle.

Only reinitialization creates a new cycle with κ > 0.

---

## 6.5 Deterministic Reinitialization

Reinitialization MUST:

- reset Δ, Φ, M, κ,
- assign new cycle_id,
- preserve deterministic replay guarantees.

Reinitialization MUST NOT depend on:

- wall-clock time,
- hardware randomness,
- non-deterministic inputs.

If a seed is used, it MUST be explicitly provided.

---

## 6.6 Meta-State Discipline

Meta-state MAY include:

- cycle_id
- step_counter
- terminal flag
- telemetry history snapshots

Meta-state MUST:

- be replay-deterministic,
- never alter structural computations,
- never modify invariants.

Meta-state corruption MUST NOT propagate into StructuralState.

---

# 7. Serialization and Replay Architecture

## 7.1 Purpose

Serialization enables:

- exact replay of structural evolution,
- deterministic audit of structural trajectories,
- state persistence across execution contexts.

Replay integrity is a core design requirement.

---

## 7.2 Serializable Components

The following MUST be serializable:

StructuralState:
- Δ[ N ]
- Φ
- M
- κ

RuntimeContext:
- cycle_id
- step_counter
- post-collapse mode
- telemetry history snapshots (if enabled)

Configuration:
- Δ dimension N
- Φ parameters
- M increment parameters
- κ decay parameters
- regime configuration
- deterministic flags

No implicit state is permitted.

---

## 7.3 Deterministic Serialization Format

Serialization MUST be:

- deterministic,
- byte-order defined,
- IEEE-754 consistent,
- fully reversible.

Floating-point values MUST be serialized
as raw IEEE-754 binary representation.

Text-based formats MAY be used only if
they preserve exact binary values.

---

## 7.4 Replay Procedure

Replay requires:

1. Load serialized StructuralState.
2. Load RuntimeContext.
3. Load configuration.
4. Apply identical input sequence.
5. Execute Step() deterministically.

The resulting trajectory MUST be bitwise-identical
to the original execution.

---

## 7.5 Snapshot Discipline

Snapshots MUST be taken:

- only after successful Step(),
- only after invariant enforcement,
- before any telemetry computation (optional but recommended).

Snapshots MUST NOT be taken mid-pipeline.

---

## 7.6 History Integrity

If telemetry history is enabled:

- prior states MUST be stored as full snapshots,
- history updates MUST occur only after successful Step(),
- history MUST be replay-deterministic.

History is meta-state only and MUST NOT influence evolution.

---

## 7.7 Versioning

Serialized state MUST include:

- specification version identifier,
- configuration version identifier.

Deserialization MUST reject incompatible versions
unless explicit migration logic is provided.

Migration logic MUST be deterministic.

---

## 7.8 Safety Guarantees

Serialization and deserialization MUST:

- validate finiteness of all values,
- reject NaN or Inf,
- reject invalid κ < 0,
- reject M < 0,
- reject corrupted buffers.

Invalid serialized data MUST NOT produce partial state mutation.
