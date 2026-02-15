# MAX-Core Static Analysis Report V2.5

Status: Final  
Date: 2026-02-XX  
Determinism Mode: Strict IEEE-754  

---

# 1. Scope

This report verifies:

- Absence of Undefined Behavior (UB)
- Deterministic floating-point behavior
- Memory safety
- State mutation integrity
- Numerical boundedness
- API safety

Target: MAX-Core V2.5

Build Configuration:

- CMake + Ninja
- CMAKE_BUILD_TYPE=Release
- MAXCORE_STRICT_FP=ON
- Flags:
  - -fno-fast-math
  - -ffp-contract=off
  - -fno-unsafe-math-optimizations
  - -fno-associative-math
  - -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wfloat-equal

---

# 2. Undefined Behavior Analysis

## 2.1 Pointer Safety

- Step() validates:
  - delta_input != nullptr
  - delta_len == delta_dim
- Terminal short-circuit occurs before pointer validation.
- No out-of-bounds memory access.

Status: PASS

---

## 2.2 Floating-Point Safety

All arithmetic operations are guarded by:

- isfinite() checks
- dt validation
- dt * max_rate < 1 constraint
- clamp_range enforcement

No division by zero:
- dt > 0 validated before use.
- Derived layer rejects dt <= 0.

No unchecked overflow paths:
- norm2 accumulation validated finite.
- All state transitions checked finite before commit.

Status: PASS

---

## 2.3 Signed / Unsigned Safety

- step_counter is uint64_t.
- No signed overflow.
- No mixed signed/unsigned UB.

Status: PASS

---

# 3. Determinism Analysis

## 3.1 Strict FP Mode

Compiler flags disable:

- fast-math
- reassociation
- contraction (FMA)
- unsafe optimizations

All operations performed in fixed sequence.

Status: PASS

---

## 3.2 State Mutation Boundary

Only mutation point:

Atomic commit block:

previous_ = current_
current_  = next
lifecycle_.step_counter++
lifecycle_.terminal = ...
lifecycle_.collapse_emitted = ...

No hidden mutable global state.

Status: PASS

---

## 3.3 Replay Determinism

Verified by:

- test_determinism.cpp

Identical inputs produce identical trajectories.

Status: PASS

---

# 4. Memory Model

## 4.1 Heap Usage

- MaxCore contains no raw dynamic allocations.
- C API uses new/delete safely.
- No manual memory arithmetic.
- No ownership ambiguity.

Status: PASS

---

## 4.2 Lifetime Integrity

- C API handle encapsulates MaxCore by value.
- Destroy() properly deletes handle.
- No dangling references exposed.

Status: PASS

---

# 5. Numerical Stability

## 5.1 Clamp Guarantees

After every update:

- phi >= 0
- memory >= 0
- 0 <= kappa <= kappa_max

Verified by test_clamp_invariants.cpp

Status: PASS

---

## 5.2 Collapse Boundary

- Collapse when kappa reaches 0.
- After terminal, no further mutation.
- Step short-circuits before validation.

Verified by:
- test_collapse.cpp
- test_terminal_shortcircuit.cpp
- test_initial_terminal.cpp

Status: PASS

---

## 5.3 Long-Run Finite Guarantee

Stress test (5000 steps):

- No NaN
- No Inf
- Invariants preserved

Verified by test_long_run_finite.cpp

Status: PASS

---

# 6. Norm Guard Correctness

Norm limiting implemented as uniform scaling:

- Preserves direction
- Caps magnitude
- Deterministic

Verified by test_norm_guard.cpp

Status: PASS

---

# 7. Derived Layer Safety

Derived projection:

- Pure function
- No mutation
- Finite checks
- dt > 0 enforced

Verified by test_derived_consistency.cpp

Status: PASS

---

# 8. C API Safety

- Parity with C++ core verified.
- last_error channel correctly set/cleared.
- Null handle safe.

Verified by:
- test_c_api_parity.cpp
- test_c_api_error_channel.cpp

Status: PASS

---

# 9. Final Safety Assessment

The MAX-Core V2.5 implementation:

- Contains no observable undefined behavior.
- Enforces strict numerical invariants.
- Guarantees deterministic evolution under strict FP mode.
- Maintains atomic state commit semantics.
- Prevents mutation after collapse.
- Preserves bounded state space.

---

# Certification Statement

MAX-Core V2.5 is:

✔ Deterministic  
✔ Numerically stable  
✔ Memory safe  
✔ Collapse-consistent  
✔ Specification-compliant  

Formal contour of the core is closed.
