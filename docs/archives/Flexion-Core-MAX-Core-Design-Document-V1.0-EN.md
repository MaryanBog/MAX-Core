# Flexion Core — MAX Core Design Document v1.0

## Section 1. Architectural Overview

---

### 1.1 Design Goals

The design of MAX Core must satisfy:

- Deterministic execution
- Strict invariant preservation
- Numerical stability
- Replay reproducibility
- Clean integration boundary
- Separation of evolution and observation

The implementation must be minimal, explicit, and auditable.

---

### 1.2 Architectural Principles

1. Single Source of Truth  
   The structural state X is the only mutable state.

2. Explicit Evolution  
   All state mutation occurs inside Step().

3. Layer Separation  
   Evolution logic must not depend on telemetry logic.

4. Deterministic Isolation  
   The Core must not perform I/O or access external systems.

5. Replayability  
   All behavior must be reproducible from logged inputs.

---

### 1.3 High-Level Architecture

The Core is divided into five internal layers:

1. State Layer  
   Stores and validates X.

2. Evolution Layer  
   Implements Step(), regime logic, and lifecycle transitions.

3. Numerical Guard Layer  
   Handles validation, overflow checks, monotonic enforcement.

4. Telemetry Layer  
   Computes derived operators without mutating state.

5. Interface Layer  
   Exposes API surface for integration.

Each layer has strict responsibility boundaries.

---

### 1.4 Data Flow

External world:

Observation → Encoder → Δ_in → Step()

Internal flow:

Validate → Update Δ → Recompute Φ → Update M → Update κ → Evaluate Collapse → Commit State

Telemetry flow:

X → Derived Operators → Output (non-mutating)

---

### 1.5 Deterministic Execution Model

Execution model is strictly:

Single-threaded state mutation.

If multi-threading is used:

- Mutation must remain single-threaded
- Telemetry may be parallelized only if deterministic

No shared mutable global state is allowed.

---

### 1.6 Component Diagram (Conceptual)

Core Instance

- Configuration
- Current State X
- Regime Engine
- Lifecycle Manager
- Numerical Guard
- Telemetry Engine
- Replay Context

All components operate in-process.

---

### 1.7 Memory Model

The Core maintains:

- Current X
- Optional previous X (for flow calculation)
- Optional previous velocity (for acceleration)

No unbounded history storage is allowed internally.

History must be handled externally.

---

### 1.8 Error Containment

Errors must not propagate undefined state.

If an error occurs:

- State remains unchanged
- Error code returned
- No partial mutation permitted

---

### 1.9 Extensibility Strategy

Extensions may include:

- Additional telemetry operators
- Additional regime parameters
- Additional diagnostic flags

Extensions must not:

- Alter X definition
- Alter collapse rule
- Introduce stochasticity
- Break determinism

---

## Section 2. Internal Module Design

---

### 2.1 Core State Module

Responsibility:

- Store structural state X
- Validate invariants
- Provide read-only access
- Prevent external mutation

Internal Structure:

State {
    vector<double> Δ
    double Φ
    double M
    double κ
    uint64 cycle_id
    uint64 step_counter
}

Rules:

- Φ must never be set directly from outside
- M and κ updates allowed only through Evolution Module
- Direct mutation forbidden

---

### 2.2 Configuration Module

Responsibility:

- Store immutable configuration parameters
- Validate configuration at initialization
- Expose read-only configuration access

Configuration includes:

- DELTA_DIM
- μ
- ν
- regime type
- collapse mode
- tolerance ε
- optional coupling matrix
- Δ input mode
- optional limits (Δ_max, Φ_max, etc.)

Configuration becomes immutable after initialization.

---

### 2.3 Evolution Module

Responsibility:

- Execute Step()
- Apply regime logic
- Update Δ
- Recompute Φ
- Update M
- Update κ
- Trigger lifecycle transitions

Step execution pipeline:

1. Validate inputs
2. Compute Δ_new
3. Apply regime transform
4. Compute Φ_new
5. Update M
6. Update κ
7. Collapse check
8. Commit state atomically

Atomicity rule:

If any phase fails,
state remains unchanged.

---

### 2.4 Regime Engine

Responsibility:

- Apply contractive or divergent modifier to Δ
- Ensure modifier preserves finiteness
- Remain deterministic

Input:

- Δ_new
- dt
- configuration

Output:

- Δ_eff

Regime Engine MUST NOT:

- modify M directly
- modify κ directly

---

### 2.5 Lifecycle Manager

Responsibility:

- Handle Genesis
- Handle Collapse mode
- Manage cycle_id
- Enforce allowed transitions

States:

- PRE_STRUCTURAL
- ACTIVE
- COLLAPSED

Transitions must be explicit and logged.

---

### 2.6 Numerical Guard Module

Responsibility:

- Finite validation
- Overflow checks
- Monotonic enforcement
- Tolerance enforcement

Monotonic enforcement:

M_new = max(M_new, M_prev)
κ_new = max(min(κ_new, κ_prev), 0)

Numerical Guard runs before state commit.

---

### 2.7 Telemetry Module

Responsibility:

- Compute derived operators
- Provide flow values
- Compute stability metrics
- Compute collapse proximity
- Provide diagnostics

Telemetry must:

- Use current state snapshot
- Not modify state
- Not cache mutable state

Optional caching allowed only if deterministic.

---

### 2.8 Replay Context Module

Responsibility:

- Log configuration
- Log Genesis parameters
- Log Δ_in sequence
- Log dt sequence
- Support deterministic re-execution

Replay must:

- Reproduce identical results
- Not depend on runtime environment

---

### 2.9 Multi-Structure Coordinator (Optional)

Responsibility:

- Maintain list of Core instances
- Apply deterministic coupling
- Coordinate global replay

Coupling must occur only via Δ modification.

Coordinator must not break isolation guarantees.

---

## Section 3. Data Structures and Memory Layout

---

### 3.1 Design Intent

Data structures must support:

- determinism
- fixed dimensionality
- stable ABI boundaries (if C API is used)
- minimal allocations
- safe numeric operations

The design must be portable across compilers and platforms.

---

### 3.2 Fixed-Dimension Δ Storage

Δ MUST be stored in a fixed-size contiguous buffer.

Preferred options:

Option A (compile-time dimension):
- std::array<double, DELTA_DIM>

Option B (runtime dimension, fixed after init):
- a single heap allocation at init
- no reallocation after init
- size locked for the lifetime of the instance

Under both options:

- Δ memory MUST be contiguous
- indexing MUST be bounds-safe in debug builds
- no implicit resizing is allowed

---

### 3.3 State Snapshot Structure

Telemetry MUST operate on a state snapshot.

Two valid approaches:

Approach A — Copy Snapshot
- Copy X into a local struct
- Run telemetry on the copy

Approach B — Immutable View
- Expose a read-only pointer/view of state data
- Telemetry reads without mutation

Rule:

Telemetry MUST see a consistent X.

If concurrency exists, snapshotting is mandatory.

---

### 3.4 Minimal Stored History

To compute flow and acceleration, the Core may store:

- X_prev (previous step)
- vΔ_prev (previous velocity)

No additional history is allowed in the Core.

Long-term history is handled externally via replay logs.

---

### 3.5 Configuration Storage

Configuration is immutable after initialization.

Store configuration in one struct:

Config {
  DELTA_DIM
  input_mode
  μ, ν
  regime_id
  regime parameters (λ or α)
  collapse mode
  tolerance ε
  limits (optional)
  coupling matrix (optional)
}

All config values must be finite and validated.

---

### 3.6 Multi-Structure Memory Model

Multi-structure coordinator stores:

- array/vector of Core instances
- coupling matrix C (k×k)

k (number of structures) is fixed at initialization.

No dynamic creation/destruction during evolution unless explicitly defined.

---

### 3.7 ABI Boundary (C-Compatible)

If a C ABI is required:

- define a handle type (opaque pointer)
- expose only functions operating on handles
- never expose internal structs directly as mutable

For telemetry:

- return a packed telemetry packet (read-only)
- or provide field-by-field getter API

ABI must avoid undefined padding assumptions.

---

### 3.8 Allocation Policy

Allocations must be deterministic.

Rules:

- allocations occur only during initialization
- Step MUST NOT allocate memory
- telemetry SHOULD NOT allocate memory
- replay logging occurs outside the Core or via a caller-provided sink

This ensures stable performance and reproducibility.

---

### 3.9 Numerical Safety Utilities

Provide internal utilities for:

- safe norm computation (avoid overflow)
- safe squared norm computation
- finite checks
- clamp functions
- monotonic enforcement

All utilities must be deterministic and pure.

---

### 3.10 Logging and Replay Storage

The Core must not store logs internally.

Instead:

- provide callbacks or sinks for logging
- or require caller to store input streams

Replay format is defined outside this Design Document.

Only the required content is mandated:

(config, genesis params, Δ_in stream, dt stream, regime changes if any)

---

## Section 4. API Design and Integration Surface

---

### 4.1 API Intent

The API must:

- expose a clean deterministic boundary
- support real-world operation (Δ_in + dt stream)
- support telemetry without mutation
- support replay and certification workflows
- remain minimal and stable

The Core must not:

- fetch data
- interpret domain semantics
- embed business logic

---

### 4.2 Instance Lifecycle

The Core instance lifecycle consists of:

1. Create
2. Configure / Initialize
3. Genesis
4. Step loop
5. Optional Reset / Destroy

Creation returns a handle to an opaque Core instance.

Configuration becomes immutable after successful initialization.

---

### 4.3 Minimal Function Set (Conceptual)

Required operations:

- CreateInstance()
- DestroyInstance()

- Init(config)
- Genesis(genesis_params)

- Step(delta_in, dt) -> event_flags

- GetState() -> snapshot / view
- GetTelemetry() -> telemetry packet / snapshot

- GetLastError() -> error code + optional diagnostic info
- ResetError() (optional)

For multi-structure:

- CreateCoordinator(k)
- StepAll(dt, delta_in_list) -> events
- GetAggregateTelemetry()

---

### 4.4 Input Contract

Step inputs:

- delta_in: array of length DELTA_DIM
- dt: positive finite double

Input mode is defined at Init:

- additive or replacement

The API must reject invalid input without mutating state.

---

### 4.5 Output Contract

State outputs must include:

- Δ
- Φ
- M
- κ
- cycle_id
- step_counter

Telemetry outputs may include:

- flow values
- collapse proximity
- stability classification
- structural time metrics
- invariant diagnostics

Event flags must include:

- NORMAL
- COLLAPSE
- GENESIS
- ERROR

---

### 4.6 Error Model and Codes

The API must define stable error codes, including:

- INVALID_INPUT
- INVALID_DT
- NONFINITE_VALUE
- NUMERIC_OVERFLOW
- INVARIANT_VIOLATION
- LIFECYCLE_VIOLATION
- INVALID_CONFIG

Error retrieval must be explicit.

On ERROR:

- state remains unchanged
- error code persists until cleared or next successful call (policy-defined)
- error is reproducible

---

### 4.7 Telemetry Retrieval Policy

Telemetry retrieval must:

- be read-only
- not trigger evolution
- return a consistent snapshot

Two acceptable policies:

Policy A — Pull telemetry:
- caller calls GetTelemetry() after Step

Policy B — Step returns telemetry:
- Step fills caller-provided telemetry buffer

Step must not depend on whether telemetry is requested.

---

### 4.8 Replay Hooks

API must support replay in one of two ways:

A) External replay driver:
- caller logs (config, genesis, delta_in, dt)
- caller replays by feeding them back

B) Replay sink:
- Core emits all required records through a caller-provided deterministic sink

In both cases:

Replay content must be complete and minimal:

- config
- genesis params
- ordered delta_in sequence
- ordered dt sequence
- regime changes (if switching enabled)

---

### 4.9 Threading and Re-Entrancy

The API should assume:

- single-threaded Step calls per instance

If concurrent access is allowed:

- state and telemetry access must be synchronized
- telemetry must operate on snapshots
- determinism must be preserved

Re-entrancy is not required unless explicitly implemented.

---

### 4.10 Versioning and Compatibility

API must include:

- version query
- capability query (feature mask)
- configuration echo

This supports certification and long-term stability.

---

## Section 5. Telemetry Architecture and Operator Pipeline

---

### 5.1 Telemetry Design Intent

Telemetry exists to maximize observability without modifying evolution.

Telemetry must satisfy:

- Non-mutation of X
- Determinism under replay
- Consistent snapshot semantics
- Stable outputs given identical state and config

Telemetry must not introduce hidden evolution influence.

---

### 5.2 Telemetry Pipeline Stages

Telemetry is computed as an ordered pipeline of operator stages.

Recommended stages:

Stage 0 — Snapshot
- capture X (and optionally X_prev) into an immutable snapshot

Stage 1 — Primary Scalars
- ||Δ||
- Φ (from state, or recomputed for validation)
- M, κ
- dt, tick, cycle_id

Stage 2 — Flow Operators
- dX/dt from (X - X_prev)/dt
- vΔ = ||dΔ/dt||

Stage 3 — Higher Kinematics (optional)
- aΔ from (vΔ - vΔ_prev)/dt

Stage 4 — Margins & Collapse Proximity
- κ margin, other optional margins
- collapse proximity function

Stage 5 — Regime & Reversibility
- regime_id
- SRI / SRD (if enabled)
- classification derived only from snapshot

Stage 6 — Structural Time (optional)
- T(X)
- dT/dt from discrete diff

Stage 7 — Diagnostics
- finite checks
- invariant consistency flags

The stage order must be fixed.

---

### 5.3 Snapshot Policy

Telemetry MUST operate on a consistent snapshot.

Preferred policy:

- Step commits X atomically
- Telemetry reads X after commit
- Telemetry reads X_prev for flow calculations
- Telemetry never reads partially-updated state

If concurrency exists, snapshot copying is mandatory.

---

### 5.4 Caching Policy

Caching is allowed only if:

- cache inputs are explicit (X snapshot + config)
- cache does not change results
- cache does not mutate X
- cache invalidation is deterministic

Telemetry caching must never affect Step outcomes.

---

### 5.5 Computation Placement: Inside Step vs Outside Step

Telemetry computations may occur:

A) Outside Step (preferred)
- Step remains minimal mutation engine
- Telemetry computed on demand

B) Inside Step (allowed)
- only if computed from the same snapshot that will be committed
- must not change step order or branching
- must not introduce allocations

Rule:

Whether telemetry is requested MUST NOT change evolution.

---

### 5.6 Operator Library Organization

Telemetry operators should be organized as pure functions:

Operator(snapshot, config, optional prev_snapshot) -> value

Operators must not:

- access global state
- perform I/O
- mutate snapshot
- depend on environment

This ensures auditability and testability.

---

### 5.7 Feature Masking

Telemetry should support a feature mask controlling enabled operators.

If an operator class is disabled:

- outputs are set to 0 or a documented neutral value
- feature bit is cleared
- determinism remains intact

Feature masks must be logged for replay.

---

### 5.8 Telemetry Integrity Tests

Telemetry must be validated by tests:

- invariant flags match state behavior
- flow matches step differences
- proximity is monotonic in κ
- time operator is monotonic in expected directions (if enabled)
- caching does not change results

Telemetry failures should be treated as conformance failures in strict mode.

---

### 5.9 Output Packaging

Telemetry output packaging should be:

- fixed-layout packet (preferred for ABI)
or
- structured getters (acceptable)

Packet content must remain stable across minor versions,
except when explicitly extended with backward-compatible additions.

---

## Section 6. Replay, Logging, and Certification Workflow Design

---

### 6.1 Design Intent

Replay and logging are mandatory for:

- deterministic verification
- debugging
- compliance validation
- certification
- scientific reproducibility

Replay must be minimal, complete, and deterministic.

The Core must not depend on replay infrastructure internally.

Replay is an external orchestration capability built on the API surface.

---

### 6.2 Minimal Replay Record

A valid replay record must include:

1. Configuration snapshot
2. Genesis parameters
3. Ordered sequence of:
   - Δ_in(t)
   - dt(t)
4. Regime changes (if enabled)
5. Multi-structure coupling config (if used)

No additional data is required.

No external timestamps are required.

---

### 6.3 Replay Execution Model

Replay execution consists of:

1. Create new instance
2. Apply configuration
3. Execute Genesis
4. Iterate Step with recorded Δ_in and dt
5. Collect outputs

Replay must produce:

- identical state trajectory
- identical collapse timing
- identical telemetry
- identical error occurrences

---

### 6.4 Golden Trace Generation

For certification, a “golden trace” must be produced.

Golden trace contains:

- X(t) for each step
- event flags per step
- selected telemetry fields
- cycle transitions

Golden trace must be:

- deterministic
- stored in canonical numeric format
- versioned with specification version

Golden trace becomes the reference artifact for regression testing.

---

### 6.5 Logging Architecture

The Core must not perform file I/O.

Instead, it must support:

- callback-based logging
or
- caller-managed logging

Logging must capture:

- configuration
- genesis parameters
- each Step input
- event flags
- optional telemetry (if certification mode)

Logging must be deterministic and ordered.

---

### 6.6 Certification Workflow

Certification workflow consists of:

1. Freeze specification version
2. Freeze configuration profile
3. Generate golden trace
4. Run static analysis
5. Run invariant audit
6. Run deterministic replay comparison
7. Produce compliance matrix

Certification must verify:

- deterministic replay
- invariant preservation
- collapse correctness
- no hidden state mutation

---

### 6.7 Compliance Matrix Binding

Each requirement from Specification must map to:

- implementation component
- validation method
- test case identifier
- pass/fail status

Compliance matrix must be complete.

Unmapped requirements invalidate certification.

---

### 6.8 Version Control of Artifacts

All replay artifacts must be:

- tied to specification version
- tied to build hash
- tied to configuration hash

Any change in:

- state definition
- collapse rule
- update formula
- regime logic

requires regeneration of golden trace.

---

### 6.9 Deterministic Drift Detection

Certification must include drift detection:

Run identical replay across:

- different compilers
- different optimization levels
- different machines (if IEEE-754 compliant)

Results must remain identical within strict equality.

Any drift indicates:

- hidden nondeterminism
- undefined behavior
- floating-point inconsistency

---

### 6.10 Long-Term Reproducibility

Replay format must be:

- text or binary canonical
- endian-safe if binary
- free of locale-dependent formatting

Long-term reproducibility requires:

- fixed numeric formatting
- explicit precision
- stable ordering

Replay artifacts must remain executable years later.

---

## Section 7. Performance and Optimization Strategy

---

### 7.1 Design Philosophy

Performance optimizations must never:

- alter structural semantics
- break determinism
- weaken invariant enforcement
- introduce undefined behavior
- change numeric results

Correctness always overrides speed.

---

### 7.2 Deterministic Optimization Rule

All optimizations must satisfy:

Optimized execution ≡ Reference execution

Bitwise equality of results is required.

If an optimization produces even minimal numeric deviation,
it is invalid.

---

### 7.3 Zero-Allocation Step Policy

The Step function must:

- perform zero dynamic memory allocation
- operate only on pre-allocated buffers
- use fixed-size structures

Allocations are allowed only during:

- initialization
- multi-structure setup

No allocation during runtime evolution.

---

### 7.4 Δ Norm Optimization

Norm computation is performance-critical.

Safe optimization strategies:

- fused multiply-add (if deterministic)
- loop unrolling
- prefetching
- SIMD with strict IEEE-754 compliance

Forbidden optimizations:

- fast-math flags
- relaxed floating-point ordering
- non-deterministic reduction ordering

Reduction order must remain fixed.

---

### 7.5 Branch Minimization

Step execution pipeline should minimize branching.

However:

- invariant checks must not be removed
- error checks must not be bypassed

Branches may be reorganized only if behavior is identical.

---

### 7.6 Cache-Friendly Layout

Δ should be:

- contiguous in memory
- aligned for cache efficiency
- accessed sequentially

Avoid:

- pointer chasing
- indirect indexing
- scattered memory patterns

---

### 7.7 SIMD and Vectorization

Vectorization is allowed only if:

- reduction order is fixed
- compiler does not reorder operations unpredictably
- strict floating-point model is enabled

Parallel reduction with unspecified order is forbidden.

---

### 7.8 Telemetry Optimization

Telemetry may be:

- partially disabled via feature mask
- lazily evaluated
- computed on-demand

However:

- enabling/disabling telemetry must not change evolution
- cached telemetry must not alter state

---

### 7.9 Multi-Structure Scaling

For k structures:

Preferred scaling model:

- loop over structures sequentially
- apply coupling deterministically
- avoid parallel mutation unless explicitly synchronized

Parallel evaluation of independent structures is allowed only if:

- deterministic ordering is enforced
- no shared mutable data races occur

---

### 7.10 Numeric Guard Optimization

Numerical checks may be:

- batched
- vectorized
- combined

But they must:

- remain logically equivalent
- not skip validation
- not depend on undefined behavior

---

### 7.11 Debug vs Production Builds

Debug Build:

- full invariant checks
- strict overflow detection
- verbose diagnostics

Production Build:

- invariant checks preserved
- diagnostics reduced
- no behavior change

Behavior must remain identical across builds.

---

### 7.12 Prohibited Optimizations

The following are explicitly forbidden:

- -ffast-math
- reassociation of floating-point operations
- non-deterministic parallel reductions
- silent saturation arithmetic
- implicit widening/narrowing conversions
- architecture-dependent precision drift

---

### 7.13 Benchmarking Protocol

Benchmarking must be performed:

- using deterministic input sequences
- with replay verification
- after correctness validation

Performance results must never be accepted without replay equality check.