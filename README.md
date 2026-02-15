# MAX-Core V2.5

Deterministic Structural Evolution Engine  
Strict IEEE-754 | Collapse-Consistent | Specification-Compliant

---

## 1. Overview

MAX-Core is a deterministic structural evolution engine implementing
a discrete canonical model of state transition:

S = (Phi, Memory, Kappa)

The engine provides:

- Strictly deterministic state evolution
- Collapse-consistent lifecycle semantics
- Atomic commit model
- Bounded numerical invariants
- Pure projection layer (Derived)
- C++ core + C API

The system is designed to be:

- Numerically stable
- Free of undefined behavior
- Strict IEEE-754 compliant
- Fully test-verified

Current status:

- 15/15 unit tests passed
- Full compliance matrix completed
- Static analysis verified
- Strict FP mode enforced

---

## 2. Structural Model

The core evolves a structural state:

Phi     – energy-like accumulation term  
Memory  – structural persistence term  
Kappa   – stability / viability term  

The canonical discrete update:

Phi_next =
    Phi + alpha * ||Δ||²
        - eta * Phi * dt

Memory_next =
    Memory
    + beta * Phi_next * dt
    - gamma * Memory * dt

Kappa_next =
    Kappa
    + rho * (kappa_max - Kappa) * dt
    - lambda_phi * Phi_next * dt
    - lambda_m * Memory_next * dt

Invariant enforcement:

- Phi >= 0
- Memory >= 0
- 0 <= Kappa <= kappa_max

Collapse occurs when:

Kappa reaches 0

After collapse:

- Lifecycle becomes terminal
- No further mutation occurs
- Step() short-circuits
- Collapse event emitted exactly once

---

## 3. Determinism & Numerical Guarantees

MAX-Core V2.5 is designed to be strictly deterministic under identical inputs.

Determinism is guaranteed by:

- Fixed operation ordering
- No hidden global state
- No random number generation
- No time-dependent behavior
- Atomic commit model
- Strict IEEE-754 enforcement

### Strict Floating-Point Mode

The build system enforces deterministic floating-point behavior:

- -fno-fast-math
- -ffp-contract=off
- -fno-unsafe-math-optimizations
- -fno-associative-math

This prevents:

- Instruction reordering
- Algebraic reassociation
- Fused-multiply-add contraction
- Non-deterministic optimizations

### Stability Constraint

A numerical stability guard enforces:

dt * max_rate < 1

If violated:

- Step() returns ERROR
- No mutation occurs

This prevents unstable exponential divergence.

### Norm Guard

If a delta magnitude exceeds delta_max (optional):

- The vector is uniformly scaled
- Direction is preserved
- Magnitude is capped

This guarantees bounded energy injection.

### Atomic Commit Semantics

All state mutation occurs only inside a single commit block:

previous_ = current_
current_  = next
lifecycle_.step_counter++

No partial mutation is possible.

### Collapse Semantics

When Kappa reaches zero:

- Collapse event emitted once
- Lifecycle becomes terminal
- All further Step() calls short-circuit
- No further mutation occurs

This guarantees structural consistency.

---

## 4. Architecture

MAX-Core V2.5 consists of three strictly separated layers:

### 4.1 Core Engine (C++)

Class: MaxCore

Responsibilities:

- Own structural state
- Validate all inputs
- Enforce invariants
- Execute canonical evolution
- Detect collapse
- Maintain lifecycle state

The core:

- Has no global state
- Allocates no dynamic memory
- Exposes only immutable snapshots
- Mutates only via Step()

State exposed:

- Current()
- Previous()
- Lifecycle()

---

### 4.2 Derived Projection Layer

Header: derived.h  
Function: ComputeDerived(...)

DerivedFrame is a pure projection computed from:

- Current state
- Previous state
- Lifecycle
- Parameters
- dt

Properties:

- Deterministic
- Side-effect free
- No mutation
- No persistent memory
- Rejects invalid inputs

This layer computes:

- State deltas
- Rates
- Normalized stability
- Collapse distance
- Instantaneous load/regeneration terms

Derived is optional and does not affect core behavior.

---

### 4.3 C API Layer

Header: c_api.h  
Library: maxcore_capi

Provides:

- maxcore_create
- maxcore_step
- maxcore_get_current
- maxcore_get_previous
- maxcore_get_lifecycle
- maxcore_compute_derived
- maxcore_last_error

The C API:

- Mirrors C++ behavior exactly
- Preserves determinism
- Provides error reporting channel
- Owns handle lifetime

No additional logic is introduced in C API.

---

### 4.4 Lifecycle Model

LifecycleContext contains:

- step_counter
- terminal
- collapse_emitted

Rules:

- step_counter increments on successful commit
- terminal becomes true at collapse
- collapse_emitted becomes true exactly once
- terminal state freezes evolution

The lifecycle model guarantees:

- No resurrection
- No hidden resets
- No implicit fresh genesis

---

## 5. Build & Usage

MAX-Core uses a universal CMake + Ninja workflow.

### 5.1 Build Requirements

- CMake >= 3.20
- Ninja
- C++17 compatible compiler
- IEEE-754 compliant floating-point implementation

Recommended:

- GCC (MinGW64)
- Clang
- MSVC with /fp:strict

---

### 5.2 Strict Deterministic Build

To build with strict floating-point mode enabled:

rm -rf build
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DMAXCORE_STRICT_FP=ON
cmake --build build

Run all tests:

ctest --test-dir build --output-on-failure

Expected result:

100% tests passed

---

### 5.3 Minimal C++ Usage

Example:

ParameterSet p{...};
StructuralState init{0.0, 0.0, p.kappa_max};

auto core_opt = MaxCore::Create(p, delta_dim, init);
if (!core_opt) { /* handle error */ }

MaxCore core = *core_opt;

double delta[2] = {1.0, 2.0};
double dt = 0.01;

EventFlag ev = core.Step(delta, 2, dt);

if (ev == EventFlag::COLLAPSE) {
    // handle collapse
}

---

### 5.4 C API Usage

maxcore_handle* h =
    maxcore_create(&params, delta_dim, &init, NULL);

maxcore_event ev =
    maxcore_step(h, delta, delta_len, dt);

if (ev == MAXCORE_EVENT_ERROR) {
    const char* err = maxcore_last_error(h);
}

maxcore_destroy(h);

---

### 5.5 Fresh Genesis Strategy

The core itself does NOT automatically reset after collapse.

If continuous processing is required:

- External orchestration layer must create a new instance.
- Collapsed instance remains immutable.
- Each lifecycle is ontologically independent.

See example:

examples/pipeline_cpp.cpp

---

## 6. Testing & Verification

MAX-Core V2.5 is verified by a complete unit test suite.

Current status:

- 15/15 tests passed
- Strict FP mode enabled
- No test failures
- Deterministic replay verified

---

### 6.1 Test Coverage Categories

The test suite validates:

Creation & Parameter Validation
- Invalid parameters rejected
- Invalid initial state rejected
- delta_dim validation
- delta_max validation

Step() Contract
- Input validation
- No mutation on ERROR
- dt stability constraint

Canonical Evolution
- Exact formula verification
- Correct use of Phi_next
- Correct Kappa update

Norm Guard
- Preserve direction
- Limit magnitude only

Collapse Semantics
- Collapse event emitted exactly once
- Terminal state freeze
- No resurrection

Determinism
- Replay produces identical trajectories
- No hidden state

Clamp Invariants
- Phi >= 0
- Memory >= 0
- 0 <= Kappa <= kappa_max

Derived Layer
- Pure projection
- Rate consistency
- No mutation

C API
- State parity with C++ core
- Error channel correctness

Long-Run Stability
- 5000-step stress test
- No NaN
- No Inf
- Invariants preserved

---

### 6.2 Atomic Mutation Guarantee

All tests confirm that:

- State mutation occurs only inside commit boundary
- ERROR never mutates state
- Terminal state never mutates state

---

### 6.3 Deterministic Execution

Strict IEEE-754 mode ensures:

- No algebraic reassociation
- No contraction (FMA)
- No unsafe optimizations

Replay tests confirm bitwise-consistent behavior under identical environment.

---

### 6.4 Certification Status

Specification: Verified  
Design: Verified  
Implementation: Verified  
Compliance Matrix: Complete  
Static Analysis: Passed  
Auto-Test Suite: Complete  

Formal contour of the core is closed.

---

## 7. Limitations & Design Boundaries

MAX-Core V2.5 is intentionally minimal.

The engine does NOT:

- Implement automatic fresh genesis
- Provide persistence or serialization
- Manage threading or concurrency
- Perform logging
- Perform I/O
- Allocate dynamic buffers inside the core
- Perform stochastic modeling
- Provide adaptive timestep control

These responsibilities belong to external orchestration layers.

---

### 7.1 No Implicit Resurrection

After collapse:

- The core becomes terminal
- No automatic reset occurs
- No internal lifecycle restart is performed

Fresh Genesis MUST be implemented externally by creating a new instance.

---

### 7.2 Single-State Engine

The core evolves exactly one structural state:

S = (Phi, Memory, Kappa)

It does not:

- Model spatial fields
- Support multi-agent entanglement
- Perform field coupling
- Manage distributed state

Those belong to higher-level frameworks.

---

### 7.3 Determinism Scope

Determinism is guaranteed only if:

- Same compiler
- Same strict FP flags
- Same architecture
- Same input sequence

Cross-platform bitwise identity is not guaranteed without controlled toolchain.

---

### 7.4 Numerical Stability Boundaries

The stability constraint:

dt * max_rate < 1

must be respected by the caller.

The engine will reject unstable configurations but does not auto-adjust dt.

---

### 7.5 Thread Safety

MAX-Core instances are:

- Self-contained
- Not internally synchronized

Concurrent access to the same instance is undefined behavior.

Each thread must own its own instance.

---

### 7.6 Intended Usage Model

MAX-Core is designed to be embedded inside:

- Simulation engines
- Control systems
- Analytical pipelines
- Research frameworks
- Higher-level structural models

It is not intended to be a complete application.

---

# Final Statement

MAX-Core V2.5 provides a:

Deterministic  
Collapse-consistent  
Invariant-enforcing  
Specification-compliant  
Minimal structural evolution kernel.

The implementation is:

- Fully test-verified
- Static-analysis reviewed
- Strict FP deterministic
- Formally compliant

The formal contour of the core is closed.
