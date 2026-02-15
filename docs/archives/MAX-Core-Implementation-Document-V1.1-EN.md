# MAX Core Implementation Document V1.1
## Deterministic Structural Evolution Engine — Reference Implementation

---

# 1. Implementation Scope

## 1.1 Objective

This document defines the reference implementation
of the MAX Core deterministic structural evolution engine.

The implementation MUST conform to:

- MAX-Core-Specification-V1.1-EN
- MAX-Core-Design-Document-V1.1-EN

It provides:

- deterministic Step() execution,
- invariant enforcement,
- collapse lifecycle management,
- telemetry isolation,
- replay integrity.

---

## 1.2 Structural State Representation

The structural state is implemented as:

struct StructuralState {
    double Δ[N];   // fixed-size contiguous buffer
    double Φ;
    double M;
    double κ;
};

Constraints:

- N is fixed per cycle.
- Δ is allocated once at Init().
- No resizing is permitted.
- No heap allocation during Step().

No additional fields belong to StructuralState.

---

## 1.3 Runtime Context (Meta-State)

Meta-state is separated from structural state:

struct RuntimeContext {
    uint64_t cycle_id;
    uint64_t step_counter;
    PostCollapseMode mode;
    bool terminal;
};

Meta-state MUST NOT influence structural computations.

---

## 1.4 Deterministic Compilation Requirements

The implementation MUST:

- use IEEE-754 double precision,
- disable fast-math optimizations,
- avoid undefined behavior,
- avoid non-deterministic parallelism,
- avoid non-deterministic library calls.

Transcendental functions MUST be version-locked
or replaced by deterministic implementations.

---

## 1.5 Finite State Guarantee

Before and after each Step():

- all values in StructuralState MUST be finite,
- no NaN or Inf is permitted.

If non-finite values are detected:

- Step() MUST return ERROR,
- state MUST remain unchanged.

---

## 1.6 Atomic Step Implementation Pattern

Step() MUST follow atomic mutation logic:

1. Copy current state into local buffer X_next.
2. Apply deterministic evolution pipeline to X_next.
3. Validate finiteness and invariants.
4. If valid → commit X_next to X.
5. If invalid → discard X_next and return ERROR.

Partial state mutation is prohibited.

---

## 1.7 Replay Determinism

For identical:

- binary build,
- configuration,
- initial state,
- input sequence,

the implementation MUST produce
bitwise-identical structural trajectories.

Replay determinism applies within the same
floating-point implementation.

---

# 2. Step() Implementation Details

## 2.1 Canonical Step Signature

EventFlag Step(
    StructuralState* X,
    const double* Δ_input,
    RuntimeContext* ctx
);

Where:

- X is mutable structural state,
- Δ_input is a fixed-length stimulus vector of dimension N,
- ctx contains meta-state only,
- EventFlag ∈ { NORMAL, COLLAPSE, ERROR }.

---

## 2.2 Pre-Validation

Before any mutation:

1. Verify all X fields are finite.
2. Verify all Δ_input elements are finite.
3. If any value is non-finite → return ERROR.

No mutation occurs during pre-validation.

---

## 2.3 Local Working Copy

Create local working buffer:

StructuralState X_next = *X;

All computations MUST use X_next.

X MUST NOT be mutated directly
until final commit.

---

## 2.4 Deterministic Evolution Pipeline

The pipeline MUST execute in the following strict order:

### Stage 1 — Δ Update

for i in [0, N):
    X_next.Δ[i] = X->Δ[i] + Δ_input[i]

### Stage 2 — Optional Regime Transform

If regime enabled:

    RegimeTransform(X_next.Δ, config)

RegimeTransform MUST:

- be deterministic,
- preserve finiteness,
- not allocate memory.

If RegimeTransform fails → return ERROR.

### Stage 3 — Φ Computation

X_next.Φ = ComputePhi(X_next.Δ, config)

If X_next.Φ < 0:
    X_next.Φ = 0

### Stage 4 — M Update

double M_candidate =
    X->M + ComputeMemoryIncrement(X_next.Δ, X_next.Φ, config)

X_next.M = max(M_candidate, X->M)

### Stage 5 — κ Update

double κ_candidate =
    X->κ - ComputeKappaDecay(X_next.Δ, X_next.Φ, X_next.M, config)

if κ_candidate < 0:
    κ_candidate = 0

X_next.κ = κ_candidate

---

## 2.5 Invariant Enforcement

After pipeline:

- Ensure X_next.Φ ≥ 0
- Ensure X_next.M ≥ X->M
- Ensure X_next.κ ≥ 0

No global rule enforces κ monotonic decrease.
κ may increase or decrease,
but κ == 0 remains terminal.

---

## 2.6 Collapse Detection

If X_next.κ == 0 AND X->κ > 0:

    collapse_triggered = true

Else:

    collapse_triggered = false

Collapse MUST trigger exactly once per cycle.

---

## 2.7 Post-Collapse Handling

If collapse_triggered:

    ctx->terminal = true

    if ctx->mode == AUTO_REGENESIS:
        attempt deterministic reinitialization (see Section 4)

    else:
        commit X_next
        increment step_counter
        return COLLAPSE

If ctx->terminal is true and mode == FROZEN:

    return NORMAL (no mutation)

If ctx->terminal is true and mode == STRICT:

    return ERROR

---

## 2.8 Atomic Commit

If no ERROR and not blocked by lifecycle:

    *X = X_next
    ctx->step_counter++

Return:

- COLLAPSE if collapse_triggered
- NORMAL otherwise

---

## 2.9 Time Advancement Rule

Internal structural time advances
only when X is committed.

If no commit occurs (ERROR or FROZEN):

- step_counter MUST NOT increment,
- no derived time-based metrics advance.

---

# 3. Telemetry Implementation Details

## 3.1 Telemetry Signature

TelemetryStatus Telemetry(
    const StructuralState* X,
    const RuntimeContext* ctx,
    const TelemetryHistory* history,   // optional
    TelemetryOutput* out
);

Where:

- X is read-only,
- ctx is read-only,
- history contains prior committed snapshots (optional),
- out is caller-provided,
- TelemetryStatus ∈ { TELEMETRY_OK, TELEMETRY_ERROR }.

Telemetry MUST NOT mutate X or ctx.

---

## 3.2 Snapshot Discipline

Telemetry MUST operate only on:

- the current committed StructuralState,
- committed history snapshots (if provided).

Telemetry MUST NOT:

- access intermediate pipeline state,
- access partially mutated buffers,
- infer values from uncommitted state.

---

## 3.3 Deterministic Derived Metrics

All metrics MUST be computed deterministically.

Examples:

### Deviation Norm

out->delta_norm = sqrt(sum_i (X->Δ[i]^2))

### Structural Time Functional (if defined)

out->T = ComputeStructuralTime(X)

### Velocity (if history available)

if history contains previous committed state:

    out->vΔ = (X->Δ - history->prev.Δ) / Δt

else:

    out->vΔ = 0 or undefined (policy-defined)

---

## 3.4 Time Advancement Constraint

Structural time MUST advance only when Step()
commits a new state.

If:

- ctx->terminal == true, or
- Step() did not commit state,

then Telemetry MUST NOT infer time advancement.

Metrics depending on ΔX/Δt MUST:

- return 0, or
- set explicit undefined flag.

No artificial time progression is allowed.

---

## 3.5 Post-Collapse Telemetry

When κ == 0:

- StructuralState is immutable.
- Telemetry MAY compute static diagnostics.
- Flow-based metrics MUST NOT indicate evolution.

If velocity/acceleration require ΔX:

- return 0, or
- report undefined.

Telemetry MUST NOT imply ongoing structure.

---

## 3.6 Non-Finite Handling

If any derived metric computation results in:

- NaN
- ±Inf

Telemetry MUST:

- return TELEMETRY_ERROR,
- leave all outputs unchanged or flagged invalid,
- NOT affect structural state.

---

## 3.7 No Hidden Caches

Telemetry MUST NOT:

- store mutable internal caches,
- modify hidden state inside the core,
- influence future Step() behavior.

If history is required,
it MUST be managed by Lifecycle Manager
as replay-deterministic snapshots.

---

## 3.8 Deterministic Math Discipline

If transcendental functions (e.g., exp) are used:

- implementation MUST be version-locked,
- rounding behavior MUST be deterministic.

If deterministic behavior cannot be guaranteed,
the metric MUST be removed or replaced.

---

# 4. Lifecycle and Auto Regenesis Implementation

## 4.1 Lifecycle State Machine

The lifecycle of a structural instance is defined as:

ACTIVE → COLLAPSED → (FROZEN | STRICT | AUTO_REGENESIS)

States are tracked in RuntimeContext:

ctx->terminal (bool)
ctx->mode     (PostCollapseMode)

StructuralState remains separate.

---

## 4.2 Collapse Transition

Collapse is detected when:

X_next.κ == 0 AND X->κ > 0

On first detection:

- ctx->terminal = true
- collapse event is emitted
- state is committed with κ == 0

Collapse MUST trigger exactly once per cycle.

Subsequent Step() calls MUST NOT emit additional COLLAPSE events.

---

## 4.3 Frozen Mode (Mode A)

If ctx->terminal == true and ctx->mode == FROZEN:

- Step() MUST NOT mutate StructuralState.
- Step() MUST return NORMAL.
- ctx->step_counter MUST NOT increment.
- No internal time advancement occurs.

Telemetry remains allowed (static only).

---

## 4.4 Strict Mode (Mode B)

If ctx->terminal == true and ctx->mode == STRICT:

- Step() MUST return ERROR.
- No mutation occurs.
- step_counter MUST NOT increment.

Telemetry remains allowed.

---

## 4.5 Auto Regenesis Mode (Mode C)

If ctx->terminal == true and ctx->mode == AUTO_REGENESIS:

A deterministic reinitialization MAY occur,
but only if Genesis admissibility conditions are satisfied.

---

## 4.6 Genesis Admissibility Gate

Auto Regenesis MUST verify:

1. Target κ_init > 0
2. Δ_init finite and dimension N preserved
3. Φ_init ≥ 0
4. M_init ≥ 0
5. All invariants satisfiable

If admissibility fails:

- Step() MUST return ERROR, OR
- remain in COLLAPSED state (policy-defined)

Auto Regenesis MUST NOT restore the collapsed instance.
It MUST create a new independent structural cycle.

---

## 4.7 Deterministic Reinitialization

When admissible:

1. Create new StructuralState X_new.
2. Assign deterministic initial values.
3. Increment ctx->cycle_id.
4. Set ctx->terminal = false.
5. Reset ctx->step_counter = 0.
6. Commit X_new.

All parameters MUST come from configuration.

No randomness is permitted unless
explicit deterministic seed is provided.

---

## 4.8 κ < 0 Handling (Non-Structure Protection)

If κ_candidate < 0 before clamping:

- κ_candidate MUST be clamped to 0,
- collapse detection proceeds normally.

If κ computation produces NaN/Inf:

- Step() MUST return ERROR,
- no mutation occurs.

κ < 0 MUST never be stored in StructuralState.

---

## 4.9 No Resurrection Guarantee

Within a single cycle:

If X->κ == 0:

- κ MUST NOT become > 0.
- Any attempt to increase κ MUST result in ERROR.

Only reinitialization creates κ > 0.

---

## 4.10 Replay Integrity Across Cycles

Cycle transitions MUST be deterministic.

Replay log MUST include:

- cycle_id
- mode
- collapse event
- reinitialization parameters

Replaying the same input sequence
MUST reproduce identical cycle transitions.

---

# 5. Replay Logging and Compliance Tests

## 5.1 Replay Logging Requirements

To guarantee deterministic reproducibility,
the implementation MUST support structured replay logging.

Each committed Step() MUST log:

- cycle_id
- step_counter
- input Δ_input
- StructuralState before Step (or cryptographic hash)
- StructuralState after Step (or cryptographic hash)
- EventFlag (NORMAL | COLLAPSE | ERROR)
- Post-collapse mode (if applicable)

Logging MUST occur only after atomic commit.

No log entry is permitted for partial or failed mutation.

---

## 5.2 Replay Procedure

Replay is defined as:

1. Load serialized StructuralState.
2. Load RuntimeContext.
3. Load configuration.
4. Apply recorded Δ_input sequence.
5. Compare resulting StructuralState after each Step.

Replay MUST produce bitwise-identical results
for identical binary and configuration.

Mismatch at any step indicates non-compliance.

---

## 5.3 Deterministic Hashing (Optional)

Implementations MAY use deterministic hashing
to validate replay integrity.

If used:

- hashing MUST operate on raw IEEE-754 binary representation,
- endianness MUST be fixed,
- hash algorithm MUST be version-locked.

Hashing MUST NOT influence structural evolution.

---

## 5.4 Compliance Test Suite Requirements

A compliant implementation MUST pass:

1. Invariant Preservation Tests
   - Φ ≥ 0 always
   - M monotonic non-decreasing
   - κ ≥ 0 always

2. Collapse Tests
   - collapse triggers exactly once
   - κ == 0 is terminal within cycle
   - no resurrection within same cycle

3. Post-Collapse Mode Tests
   - Frozen: no mutation, no time advancement
   - Strict: Step returns ERROR
   - Auto Regenesis: new cycle created only if admissible

4. Telemetry Isolation Tests
   - Telemetry does not mutate state
   - Telemetry does not advance time
   - Telemetry errors do not affect Step

5. Replay Determinism Tests
   - identical runs produce identical trajectories
   - serialization round-trip produces identical future evolution

6. Non-Finite Protection Tests
   - NaN/Inf inputs cause ERROR
   - no NaN/Inf stored in StructuralState

---

## 5.5 Safety Assertions

The implementation MUST assert at runtime (debug builds):

- finiteness of all structural values,
- invariant satisfaction after Step,
- no mutation outside Step,
- no κ increase after collapse.

Assertions MUST NOT alter release behavior.

---

## 5.6 Structural Integrity Statement

The MAX Core implementation:

- preserves structural invariants,
- enforces collapse irreversibility,
- separates evolution from observation,
- guarantees deterministic replay,
- prevents hidden state mutation,
- maintains ontological purity of X.

This concludes MAX-Core-Implementation-Document-V1.1-EN.
