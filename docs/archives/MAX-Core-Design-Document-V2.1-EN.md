# MAX Core Design Document V2.1
## Deterministic Single-Structure Structural Runtime

---

# 1. Architectural Foundation

## 1.1 Design Scope

MAX Core Design V2.1 defines the concrete runtime architecture
implementing MAX Core Specification V2.1.

The runtime is:

- Deterministic
- Single-structure
- Collapse-strict
- Snapshot-based
- Two-layered (Evolution + Diagnostics)

This document describes internal architecture,
module boundaries, data ownership, and execution flow.

---

## 1.2 Ontological Boundary

The runtime operates on exactly one structural state:

X = (Δ, Φ, M, κ)

Where:

- Δ ∈ ℝ^N
- Φ ≥ 0
- M ≥ 0
- κ ≥ 0

No additional ontological coordinates exist.

No lifecycle modes exist.

Collapse is defined exclusively as:

κ == 0

No alternative collapse condition is permitted.

---

## 1.3 Layer Separation

The runtime is divided into two strictly separated layers:

Layer 1 — Structural Evolution Engine  
Layer 2 — Operator Diagnostics Engine  

Layer 1:

- Owns StructuralState
- Executes Step()
- Enforces invariants
- Detects collapse
- Performs atomic commit

Layer 2:

- Observes committed state only
- Computes derived diagnostics
- Has no mutation authority
- Has no lifecycle authority

Layer 1 must not depend on Layer 2.

Layer 2 must not modify Layer 1 state.

---

## 1.4 Single-Structure Constraint

The runtime maintains exactly one StructuralState per cycle.

The following are explicitly out of scope:

- Multi-structure evolution
- Cross-structure coupling
- Structural entanglement
- Joint collapse semantics

Any multi-structure capability requires a different runtime design.

---

## 1.5 Determinism Principle

For identical:

- Initial state
- Configuration
- Δ_input sequence
- dt sequence
- Binary build
- Floating-point environment

The runtime MUST produce identical:

- Structural trajectory
- Collapse timing
- EventFlag sequence
- Operator outputs

Determinism is a mandatory architectural property.

---

# 2. Runtime State Model

## 2.1 StructuralState Representation

The runtime maintains the canonical structural state:

struct StructuralState {
    double* delta;     // pointer to fixed-size array of length N
    double  phi;       // Φ
    double  memory;    // M
    double  kappa;     // κ
};

Rules:

- N (dimension of delta) is fixed at initialization.
- Memory for delta is allocated once during MAX_Init.
- No reallocation of delta is permitted during runtime.
- StructuralState contains no hidden fields.

---

## 2.2 Snapshot Pair

The runtime maintains exactly two committed states:

StructuralState X_current;
StructuralState X_previous;

In addition, the runtime maintains:

uint64_t step_counter;
bool collapse_emitted;

Definitions:

- X_current is the active committed state.
- X_previous is the immediately preceding committed state.
- step_counter increments only on successful commit.
- collapse_emitted prevents duplicate COLLAPSE emission.

No additional lifecycle flags are stored.

Terminal state is defined strictly as:

X_current.kappa == 0

---

## 2.3 Ownership and Lifetime

Ownership rules:

- CoreEngine exclusively owns X_current and X_previous.
- OperatorEngine receives read-only access.
- No external component may hold mutable references.

Lifetime rules:

- StructuralState memory is allocated during MAX_Init.
- Memory is released only during MAX_Destroy.
- No dynamic allocation is allowed inside Step().

---

## 2.4 State Integrity

At all times, committed states MUST satisfy:

- phi >= 0
- memory >= previous_memory
- 0 <= kappa <= previous_kappa
- All components finite

Invariant validation occurs:

- Before commit
- After deserialization
- During initialization

Committed state MUST always remain valid.

---

# 3. Step Execution Architecture

## 3.1 Step Entry

The Step() function is the only mutation entry point.

Signature conceptually:

EventFlag Step(double* delta_input, double dt)

Preconditions:

- dt > 0
- delta_input length == N
- All X_current components are finite
- All delta_input components are finite

If any precondition fails:

- Return ERROR
- Do not mutate state
- Do not increment step_counter

---

## 3.2 Terminal Guard

If X_current.kappa == 0 at Step entry:

- No mutation is performed
- step_counter is not incremented
- EventFlag returned is NORMAL

No further evolution is permitted within the cycle.

New cycle requires explicit reinitialization.

---

## 3.3 Local Computation Buffer

Step execution uses a local buffer:

StructuralState X_next;

Procedure:

1) Copy X_current into X_next.
2) Apply all mutations to X_next only.
3) Validate invariants on X_next.
4) Commit atomically.

X_current and X_previous must not be modified
before successful validation.

---

## 3.4 Deterministic Update Pipeline

The update pipeline MUST execute strictly in this order:

1) Δ update:
   X_next.delta[i] += delta_input[i]

2) Φ recomputation:
   X_next.phi = ComputePhi(X_next.delta)
   X_next.phi = max(X_next.phi, 0)

3) Memory update:
   m_candidate =
       X_current.memory +
       MemoryIncrement(X_next, dt)

   X_next.memory =
       max(m_candidate, X_current.memory)

4) κ update:
   k_candidate =
       X_current.kappa -
       KappaDecay(X_next, dt)

   X_next.kappa =
       max(k_candidate, 0)

Reordering of these stages is not permitted.

---

## 3.5 Invariant Validation

Before commit, the runtime MUST verify:

- X_next.phi >= 0
- X_next.memory >= X_current.memory
- 0 <= X_next.kappa <= X_current.kappa
- All fields finite

If validation fails:

- Abort Step
- Return ERROR
- Do not commit

---

## 3.6 Atomic Commit Protocol

Commit sequence:

1) X_previous = X_current
2) X_current  = X_next
3) step_counter++

This sequence must be atomic from the perspective
of external observers.

No operator execution may begin before commit completes.

---

## 3.7 Collapse Emission Logic

After commit:

If:

- X_previous.kappa > 0
- X_current.kappa == 0
- collapse_emitted == false

Then:

- collapse_emitted = true
- return COLLAPSE

Else:

- return NORMAL

Collapse is emitted at most once per cycle.

---

# 4. Operator Engine Architecture

## 4.1 Invocation Boundary

OperatorEngine executes only after a successful atomic commit.

Execution conditions:

- Step() returned NORMAL or COLLAPSE
- No ERROR occurred
- Snapshot is stable

OperatorEngine must never execute on uncommitted state.

---

## 4.2 Immutable Context

Operators receive immutable context:

struct OperatorContext {
    const StructuralState* current;
    const StructuralState* previous;
    double dt;
    uint64_t step_counter;
    const CoreConfig* config;
};

Rules:

- All pointers are read-only.
- No operator may store mutable references.
- No operator may modify current or previous.
- No operator may modify config.

Terminal state is derived strictly from:

current->kappa == 0

No separate terminal flag exists.

---

## 4.3 Execution Order

Operator packs MUST execute in fixed deterministic order:

1) FlowPack
2) SpacePack
3) TimePack
4) FieldPack
5) ObserverPack
6) CollapseDiagnostics
7) IntelligenceDiagnostics (if enabled)

Disabled packs are skipped deterministically.

Execution order must never depend on runtime conditions.

---

## 4.4 Memory Discipline

OperatorEngine:

- Uses preallocated output buffers.
- Performs no dynamic allocation during Step().
- Uses no static mutable globals.
- Does not persist state across steps.

All operator outputs are recomputed from snapshot only.

---

## 4.5 Non-Interference Enforcement

OperatorEngine MUST NOT:

- Modify StructuralState
- Modify step_counter
- Modify collapse_emitted
- Influence EventFlag
- Call Step()
- Trigger lifecycle transitions

Layer 2 has zero mutation authority.

---

## 4.6 Failure Isolation

If any operator produces:

- NaN
- Inf
- Invalid numeric state

Then:

- That operator's outputs are zeroed.
- A pack_error flag may be set.
- Execution continues for remaining packs.

Operator failure must not:

- Affect StructuralState
- Affect lifecycle
- Affect collapse logic
- Abort Step()

Operator errors are strictly diagnostic.

---

# 5. Detailed Pack Design

## 5.1 FlowPack

FlowPack computes first-order discrete derivatives.

Inputs:

- current
- previous
- dt
- step_counter

Computation:

If step_counter == 0:
    All derivatives = 0

Else if current.kappa == 0:
    All derivatives = 0

Else:
    dΔ[i] = (current.delta[i] - previous.delta[i]) / dt
    dΦ    = (current.phi - previous.phi) / dt
    dM    = (current.memory - previous.memory) / dt
    dκ    = (current.kappa - previous.kappa) / dt

Constraints:

- dt must be the same dt used in Step().
- No smoothing or historical buffering allowed.
- All results must be finite.
- No mutation of structural state.

FlowPack depends strictly on snapshot pair.

---

## 5.2 SpacePack

SpacePack computes geometric diagnostics.

Inputs:

- current
- FlowPack outputs
- config

Outputs:

- delta_norm = sqrt(sum_i current.delta[i]^2)
- kappa_margin = current.kappa

Optional:

- delta_margin
- phi_margin
- memory_margin
- path_length_increment

If path_length_increment is enabled:

dS = sqrt( g_ij * v_i * v_j ) * dt

Where:

- v_i are FlowPack derivatives
- g_ij is a positive-definite metric from config

Constraints:

- g_ij is immutable after initialization.
- No singular metric allowed.
- No gating semantics.
- No influence on evolution.

---

## 5.3 TimePack

TimePack computes bounded structural time diagnostics.

Inputs:

- current
- config

Outputs:

- T
- gradient_T
- optional Hessian_T

Design constraints:

- Time function must be bounded.
- No 1/kappa singularity allowed.
- All divisions must guard against zero.
- All outputs must be finite.

TimePack must not:

- Modify kappa
- Define collapse
- Influence Step()

---

## 5.4 FieldPack

FieldPack computes structural drift proxies:

F = (FΔ, FΦ, FM, Fκ)

Inputs:

- current
- config

Properties:

- Deterministic functions of state only.
- No dependency on hidden state.
- No mutation of StructuralState.
- No invariant override.

Fκ must not redefine collapse.
Collapse remains strictly kappa == 0.

---

## 5.5 ObserverPack

ObserverPack computes bounded observation load.

Inputs:

- current
- config

Constraints:

- No singularity at kappa -> 0
- All computations finite
- No direct or indirect modification of kappa
- No lifecycle influence

Optional cumulative metrics must:

- Be stored inside OperatorEngine only
- Not affect structural state
- Reset on MAX_Init

---

## 5.6 CollapseDiagnostics

CollapseDiagnostics computes:

- kappa_margin
- kappa_velocity

Optional predictive estimates are allowed,
but must remain diagnostic only.

Collapse detection remains exclusively inside Layer 1.

No operator may emit COLLAPSE.

---

## 5.7 IntelligenceDiagnostics

If enabled, computes:

- activity proxy
- overload flag
- stability ratio

Constraints:

- All values bounded and finite
- No comparison may redefine collapse
- No mutation of structural state
- No lifecycle influence

IntelligenceDiagnostics is strictly observational.

---

# 6. Determinism and Replay Architecture

## 6.1 Deterministic Execution Model

The runtime is strictly single-threaded.

No concurrent mutation of StructuralState is permitted.

All state transitions occur only inside Step().

Operator execution occurs only after atomic commit.

No asynchronous callbacks are allowed.

---

## 6.2 Floating-Point Discipline

The implementation MUST:

- Use IEEE-754 double precision
- Disable fast-math optimizations
- Disable unsafe reordering
- Avoid non-deterministic intrinsics
- Avoid fused-multiply-add unless identical across builds

The floating-point environment must remain constant
throughout runtime execution.

---

## 6.3 Finite-Only State Policy

Committed StructuralState must never contain:

- NaN
- +Inf
- -Inf

Validation occurs:

- Before commit
- After deserialization
- During initialization

If non-finite value is detected:

- Step() returns ERROR
- No mutation occurs

Operator outputs must also remain finite.

---

## 6.4 Replay Inputs

Replay is fully determined by:

- Initial StructuralState
- CoreConfig
- Ordered sequence of delta_input
- Ordered sequence of dt

No hidden state is allowed.

No randomness is allowed.

No system time access is allowed.

---

## 6.5 Snapshot Determinism

The pair:

- X_previous
- X_current

Fully determines:

- Flow derivatives
- Operator diagnostics
- Collapse emission logic

Replay must reproduce identical snapshot sequence.

Any divergence indicates non-compliance.

---

## 6.6 Serialization Requirements

Serializer must store:

- X_current
- X_previous
- step_counter
- collapse_emitted
- CoreConfig
- delta dimension

Serialization must:

- Preserve raw IEEE-754 bit patterns
- Use fixed endianness
- Validate invariants on load
- Reject incompatible versions

Deserialization must restore exact runtime state.

---

## 6.7 Version Identity

The runtime must expose:

core_design_version = "MAX-Core-Design-V2.1"

Serialized state must include version string.

Version mismatch must prevent load unless explicitly supported.

---

## 6.8 Compliance Condition

An implementation is compliant with Design V2.1 if:

- It enforces strict layer separation
- It guarantees atomic commit
- It prevents lifecycle policy branching
- It removes entanglement support
- It enforces deterministic replay
- It maintains invariant integrity
- It prevents hidden mutation paths

This completes MAX Core Design Document V2.1.
