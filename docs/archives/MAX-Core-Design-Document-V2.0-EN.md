# MAX Core Design Document V2.0
## Deterministic Structural Runtime with Fundamental Operator Architecture

---

# 1. Architectural Overview

## 1.1 Design Objective

MAX Core V2.0 is designed as a two-layer deterministic architecture:

Layer 1 — Structural Evolution Engine
Layer 2 — Fundamental Operator Engine

The design enforces strict separation between:

- Ontological state mutation
- Fundamental operator computation

No operator may influence structural evolution.

---

## 1.2 High-Level Architecture

+------------------------------------------------------+
|                    MAX Core V2.0                     |
+------------------------------------------------------+
|                                                      |
|  Layer 1: Evolution Engine                           |
|    - Step()                                          |
|    - Invariant Enforcement                           |
|    - Collapse Gate                                   |
|    - Lifecycle Management                            |
|                                                      |
|  Layer 2: Fundamental Operator Engine                |
|    - Flow Module                                     |
|    - Space Module                                    |
|    - Time Module                                     |
|    - Field Module                                    |
|    - Observer Module                                 |
|    - Entanglement Module (optional)                  |
|    - Intelligence Module (optional)                  |
|                                                      |
+------------------------------------------------------+

Data flow is strictly:

Step → Commit X → Compute Operator Packs

Never the reverse.

---

## 1.3 Core State Containers

The design uses three main containers:

1) StructuralState
   - Δ[N]
   - Φ
   - M
   - κ

2) LifecycleContext
   - cycle_id
   - step_counter
   - mode
   - terminal flag

3) OperatorContext
   - X_current snapshot
   - X_previous snapshot
   - dt
   - configuration

Only StructuralState is mutable during Step().

OperatorContext is read-only during operator computation.

---

## 1.4 Execution Pipeline

For each external call:

1) Validate inputs
2) Execute Step() (Layer 1)
3) Commit X_new if valid
4) Update previous snapshot
5) Execute Operator Engine (Layer 2)
6) Produce FundamentalPack

Operator Engine MUST run only after successful commit.

---

## 1.5 Atomicity Model

Structural mutation is atomic.

Operator computation is stateless relative to evolution.

Failure in operator computation MUST NOT invalidate
the committed structural state.

Operator errors are diagnostic only.

---

## 1.6 Determinism Boundaries

Determinism applies to:

- Step execution
- Snapshot capture
- Operator pack computation

Design prohibits:

- Dynamic memory allocation during Step
- Hidden mutable caches
- Non-deterministic threading
- Non-deterministic math libraries

All modules operate under identical floating-point configuration.

---

# 2. Layer 1 — Evolution Engine Design

## 2.1 Responsibility

The Evolution Engine is the only module allowed to mutate:

X = (Δ, Φ, M, κ)

It is responsible for:

- Deterministic state update
- Invariant enforcement
- Collapse detection
- Lifecycle state transition

No other module may modify StructuralState.

---

## 2.2 StructuralState Layout

struct StructuralState {
    double Δ[N];   // fixed dimension
    double Φ;
    double M;
    double κ;
};

Design constraints:

- N defined at initialization
- Δ allocated once
- No resizing permitted
- Contiguous memory layout required
- No hidden padding-dependent behavior

---

## 2.3 Step() Internal Pipeline

The engine uses a two-buffer model:

StructuralState X_current;
StructuralState X_next;

Execution order:

1) Copy X_current → X_next
2) Apply Δ update
3) Compute Φ
4) Compute M
5) Compute κ
6) Enforce invariants
7) Validate finiteness
8) Detect collapse
9) Commit X_next → X_current (atomic)

Mutation occurs only at step 9.

---

## 2.4 Invariant Enforcement Module

A dedicated invariant enforcement function MUST:

- Clamp Φ ≥ 0
- Enforce M ≥ previous M
- Clamp κ ≥ 0

Invariant logic MUST be centralized.
No distributed invariant corrections are permitted.

---

## 2.5 Collapse Gate Module

Collapse detection module:

if κ_prev > 0 AND κ_new == 0:
    trigger collapse

Design properties:

- Collapse triggers once per cycle
- Collapse event is deterministic
- No other operator may introduce collapse

Collapse logic is isolated from operator modules.

---

## 2.6 Lifecycle Controller

LifecycleController operates after collapse detection.

Modes supported:

- Frozen
- Strict
- Deterministic Regenesis

LifecycleController MUST:

- Not alter invariant rules
- Not override κ gate
- Not modify operator outputs

Regenesis MUST allocate new StructuralState instance.

---

## 2.7 Time Advancement Control

Evolution Engine owns time advancement.

Structural time advances only when:

- Commit occurs
- No ERROR
- Not in Frozen/Strict block

Operator modules may read time index
but may not advance it.

---

## 2.8 Error Handling Strategy

If any stage fails:

- Abort pipeline
- Discard X_next
- Return ERROR
- Do not update previous snapshot

No partial state must survive.

---

## 2.9 Snapshot Management

After successful commit:

- X_previous ← old X_current
- X_current ← X_next
- step_counter++

Snapshot update occurs before operator execution.

Snapshot integrity is required for derivative computation.

---

# 3. Layer 2 — Fundamental Operator Engine Architecture

## 3.1 Responsibility

The Fundamental Operator Engine computes:

FundamentalPack(t)

containing all enabled operator packs:

- Flow
- Space
- Time
- Field
- Observer
- Collapse
- Entanglement (optional)
- Intelligence (optional)

Operators are derived from:

- committed X_current
- committed X_previous (if available)
- dt
- immutable configuration
- lifecycle meta-state

Operator Engine MUST NOT:

- mutate StructuralState
- influence Step pipeline
- create additional collapse gates
- advance time independently

---

## 3.2 Operator Engine Inputs

Operator Engine receives an immutable evaluation context:

struct OperatorContext {
    const StructuralState* X_current;
    const StructuralState* X_previous;   // may be null for first step
    double dt;
    uint64_t cycle_id;
    uint64_t step_counter;
    bool terminal;
    const Config* cfg;
};

All operator modules MUST treat OperatorContext as read-only.

---

## 3.3 FundamentalPack Output

All operator outputs are written into caller-provided memory:

struct FundamentalPack {
    BasePack base;
    FlowPack flow;
    SpacePack space;
    TimePack time;
    FieldPack field;
    ObserverPack observer;
    CollapsePack collapse;
    EntanglementPack entanglement;   // optional
    IntelligencePack intelligence;   // optional
};

All packs include:

- validity flags
- finite-only values

If a pack fails, it MUST set:

- pack_status = ERROR
- without affecting other packs
- without affecting structural state

---

## 3.4 Execution Ordering

Operator Engine runs only after a successful commit.

Recommended deterministic ordering:

1) BasePack
2) FlowPack
3) SpacePack
4) TimePack
5) FieldPack
6) ObserverPack
7) CollapsePack
8) EntanglementPack (if enabled)
9) IntelligencePack (if enabled)

No pack may depend on values that are not already computed
or explicitly available in OperatorContext.

---

## 3.5 No Hidden Cache Rule

Operator Engine MUST be stateless with respect to evolution.

Allowed memory usage:

- stack buffers
- caller-provided output buffers

Disallowed:

- internal mutable caches
- lazy initialization that changes outputs between calls
- storing previous metrics internally

The only allowed "history" is X_previous,
managed by Layer 1 snapshot discipline.

---

## 3.6 Failure Isolation

If any operator computation fails (non-finite, invalid):

- that pack returns PACK_ERROR
- other packs continue
- FundamentalPack remains partially valid
- Step result remains valid and committed

Operator failure MUST NOT trigger structural ERROR event.

---

## 3.7 Deterministic Math Discipline

All operator modules follow the same numerical constraints:

- IEEE-754 double
- no fast-math
- no non-deterministic transcendental calls

If transcendental functions are required:

- provide deterministic implementation OR
- version-lock math library OR
- disable the metric.

This constraint is strict because FundamentalPack
is replay-verifiable.

---

## 3.8 ABI and Packaging Strategy

The design exposes a stable C ABI:

- Init()
- Step()
- ComputeFundamentalPack() or StepWithPack()

Two supported patterns:

A) StepWithPack:
   Step commits X and outputs FundamentalPack in one call.

B) Two-call:
   Step commits X.
   ComputeFundamentalPack reads snapshots and produces pack.

Pattern A is recommended to avoid mismatch
between step index and computed pack.

Both patterns MUST preserve determinism.

---

# 4. Pack Definitions and Module Contracts

## 4.1 BasePack (MUST)

### 4.1.1 Purpose
BasePack binds operator outputs to the exact committed structural snapshot.

### 4.1.2 Fields
BasePack MUST include:

- spec_version_major, spec_version_minor
- cycle_id (uint64)
- step_counter (uint64)
- dt (double)
- terminal (bool)
- event_flag (NORMAL|COLLAPSE|ERROR) from the same Step
- X snapshot:
  - Δ[N]
  - Φ
  - M
  - κ

### 4.1.3 Rules
- BasePack MUST be computed deterministically.
- BasePack MUST always be valid if Step committed.

---

## 4.2 FlowPack (MUST)

### 4.2.1 Purpose
FlowPack reports discrete structural velocity:

dX/dt = (X(t) - X(t-1)) / dt

### 4.2.2 Fields
- has_prev (bool)
- dΔ_dt[N]
- dΦ_dt
- dM_dt
- dκ_dt

### 4.2.3 Rules
- If X_previous is missing: has_prev=false and all derivatives MUST be 0.
- If terminal=true (collapsed/frozen): derivatives MUST be 0.
- Any non-finite result -> pack_status=PACK_ERROR.

---

## 4.3 SpacePack (MUST)

### 4.3.1 Purpose
SpacePack reports geometry in structural space.

### 4.3.2 Fields (minimum)
- delta_norm = ||Δ||
- kappa_margin = κ
- collapse_boundary_hit = (κ == 0)

Optional (config-enabled):
- delta_margin = Δ_max_norm - ||Δ||
- phi_margin   = Φ_max - Φ
- mem_margin   = M_max - M

Metric-dependent (config-enabled):
- dS (path-length increment)

### 4.3.3 Rules
- Δ_max_norm, Φ_max, M_max are optional; if not configured, margins are omitted or flagged unsupported.
- Metric tensor g_ij (if used) MUST be configuration-defined and deterministic.
- Any non-finite result -> PACK_ERROR.

---

## 4.4 TimePack (MUST)

### 4.4.1 Purpose
TimePack computes Time Theory operators as functions over X.

### 4.4.2 Fields (minimum)
- T_field = T(X)
- gradT:
  - dT_dΔ[N]
  - dT_dΦ
  - dT_dM
  - dT_dκ

Optional (config-enabled):
- HessianT representation (reduced form allowed)
- C_time = 1 - T_field / T_max

### 4.4.3 Rules
- T(X) MUST be implemented exactly as defined by the selected Time model in config.
- If T_max not configured: C_time MUST be omitted or flagged unsupported.
- TimePack MUST NOT define collapse. Collapse remains κ == 0 only.
- Any non-finite result -> PACK_ERROR.

---

## 4.5 FieldPack (MUST)

### 4.5.1 Purpose
FieldPack exposes the structural field decomposition:

𝔽(X) = (FΔ, FΦ, FM, Fκ)

### 4.5.2 Fields
- FΔ[N]
- FΦ
- FM
- Fκ

Optional (config-enabled):
- regime_selector σ (reporting only)

### 4.5.3 Rules
- FieldPack MUST be computed purely from committed X and config.
- FieldPack MUST NOT override invariants or collapse gate.
- Any non-finite result -> PACK_ERROR.

---

## 4.6 ObserverPack (MUST)

### 4.6.1 Purpose
ObserverPack reports observation/projection load as diagnostics only.

### 4.6.2 Fields (minimum)
- obs_load (double)         // projection intensity / cost proxy
- future_multiplicity (double) // deterministic proxy for |𝔽(X)|
Optional (config-enabled):
- obs_frequency (double)

### 4.6.3 Rules
- ObserverPack MUST NOT mutate X.
- ObserverPack MUST NOT advance time.
- Any non-finite result -> PACK_ERROR.

---

## 4.7 CollapsePack (MUST)

### 4.7.1 Purpose
CollapsePack reports collapse geometry around the κ boundary.

### 4.7.2 Fields (minimum)
- collapse = (κ == 0)
- v_boundary = dκ/dt   (from FlowPack if available)
Optional (config-enabled):
- near_collapse = (κ <= κ_warn_threshold)

### 4.7.3 Rules
- Collapse gate is κ == 0 only. No extra gates permitted.
- If FlowPack has_prev=false, v_boundary MUST be 0.
- Any non-finite result -> PACK_ERROR.

---

## 4.8 EntanglementPack (OPTIONAL)

### 4.8.1 Enable Condition
Enabled only in multi-structure mode with explicit coupling configuration.

### 4.8.2 Fields (suggested minimum)
- ES (double in [0,1])
Optional components:
- S_C, S_μ, S_M, S_κ (double in [0,1])
- I_shared (double >= 0)

### 4.8.3 Rules
- Entanglement operators MUST NOT merge structural identities.
- EntanglementPack MUST NOT modify invariants or collapse gate.
- Any non-finite result -> PACK_ERROR.

---

## 4.9 IntelligencePack (OPTIONAL Research Mode)

### 4.9.1 Enable Condition
Enabled only if FIT research mode is configured.

### 4.9.2 Fields (suggested minimum)
- delta_phi_wave (double)
- overload_flag = (|delta_phi_wave| > κ)   // diagnostic only
Optional:
- identity_fixed_point_error (double)
- T_int (double)

### 4.9.3 Rules
- IntelligencePack MUST NOT add new state coordinates.
- IntelligencePack MUST NOT override collapse gate κ == 0.
- Any non-finite result -> PACK_ERROR.

---

## 4.10 Pack Status Contract (Normative)

Each pack MUST expose:

- pack_status ∈ { PACK_OK, PACK_ERROR, PACK_UNSUPPORTED }

Rules:
- PACK_ERROR: computation failed (non-finite / invalid arithmetic).
- PACK_UNSUPPORTED: metric disabled or missing config.
- Pack failures MUST NOT affect Step() result.
- BasePack MUST NEVER be PACK_UNSUPPORTED after successful commit.

---

# 5. ABI Design and Call Patterns

## 5.1 ABI Goals

The ABI MUST:

- Preserve determinism
- Guarantee snapshot consistency
- Avoid mismatch between Step and Operator outputs
- Support C-compatible interface
- Remain stable across minor revisions

---

## 5.2 Primary Recommended Pattern (StepWithPack)

### 5.2.1 Signature

StepResult MAX_StepWithPack(
    StructuralState* X,
    const double* Δ_input,
    double dt,
    LifecycleContext* lifecycle,
    FundamentalPack* out_pack
);

### 5.2.2 Execution Order

1) Validate inputs
2) Execute Step() (Layer 1)
3) Commit X if valid
4) Update snapshot
5) Compute FundamentalPack
6) Return StepResult

### 5.2.3 Determinism Guarantee

StepWithPack ensures:

- OperatorPack corresponds exactly to the committed X
- No temporal desynchronization
- No race condition between Step and pack computation

This is the REQUIRED pattern for production usage.

---

## 5.3 Two-Call Pattern (Optional)

Pattern:

1) MAX_Step(...)
2) MAX_ComputeFundamentalPack(...)

### 5.3.1 Constraints

Two-call pattern MUST:

- Use the same committed snapshot
- Reject calls if no new commit occurred
- Not allow computing pack on partially mutated state

### 5.3.2 Risk

Two-call pattern increases risk of:

- Snapshot mismatch
- Incorrect dt usage
- Replay divergence if misused

It is allowed but not recommended.

---

## 5.4 Initialization Interface

Initialization MUST define:

- Δ dimension N
- Metric configuration
- Time functional model
- Field model
- Observer model parameters
- Optional Entanglement mode
- Optional Intelligence mode
- Collapse warning thresholds

Example:

InitResult MAX_Init(
    const Config* cfg,
    StructuralState* X_initial,
    LifecycleContext* lifecycle
);

Initialization MUST validate invariants.

---

## 5.5 Configuration Model

Configuration MUST be:

- Immutable during runtime
- Deterministically serialized
- Versioned
- Explicit about enabled packs

Config MUST include flags:

- enable_time_pack
- enable_space_pack
- enable_field_pack
- enable_observer_pack
- enable_entanglement_pack
- enable_intelligence_pack

Disabled packs MUST return PACK_UNSUPPORTED.

---

## 5.6 Error Model

StepResult MUST include:

- event_flag (NORMAL|COLLAPSE|ERROR)
- error_code (if ERROR)
- lifecycle_state

Operator packs MUST expose independent pack_status.

Operator failures MUST NOT propagate to StepResult.

---

## 5.7 Threading Policy

The Core is defined as:

- Single-threaded deterministic engine

Parallelization is allowed only if:

- It does not alter floating-point order
- It produces bitwise-identical results
- It does not introduce non-deterministic scheduling

Otherwise, multi-threading is prohibited.

---

## 5.8 Memory Policy

The Core MUST:

- Avoid dynamic allocation during Step
- Avoid dynamic allocation during Operator computation
- Use caller-provided buffers for outputs

Stack or preallocated buffers are allowed.

---

## 5.9 Deterministic Replay Compatibility

ABI MUST guarantee that:

- StepWithPack under identical inputs
  produces identical FundamentalPack

Replay validation MAY compare:

- StructuralState
- FundamentalPack hashes
- EventFlag sequence
- Lifecycle transitions

Mismatch indicates non-compliance.

---

## 5.10 Version Compatibility

ABI MUST expose:

- specification_major
- specification_minor
- build_identifier

Mismatch in major version MUST reject runtime usage.

Minor version differences MAY be allowed
only if deterministic compatibility is guaranteed.
