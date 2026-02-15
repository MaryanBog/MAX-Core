# MAX-Core Compliance Matrix V2.5

Status: Verified  
Normative keywords: MUST, MUST NOT, SHALL, MAY

---

## 1. Creation & Parameter Validation

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-CR-1 | delta_dim MUST be > 0 | Unit test | test_params_reject.cpp | PASS |
| CM-CR-2 | All parameters MUST be finite and > 0 | Unit test | test_params_reject.cpp | PASS |
| CM-CR-3 | kappa_max MUST be finite and > 0 | Unit test | test_params_reject.cpp | PASS |
| CM-CR-4 | Initial phi, memory, kappa MUST be finite | Unit test | test_params_reject.cpp | PASS |
| CM-CR-5 | Initial phi >= 0 | Unit test | test_params_reject.cpp | PASS |
| CM-CR-6 | Initial memory >= 0 | Unit test | test_params_reject.cpp | PASS |
| CM-CR-7 | Initial kappa in [0, kappa_max] | Unit test | test_params_reject.cpp | PASS |
| CM-CR-8 | delta_max (if provided) MUST be finite and > 0 | Unit test | test_params_reject.cpp | PASS |

---

## 2. Step() Input Validation

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-ST-1 | dt MUST be finite and > 0 | Unit test | test_validation.cpp | PASS |
| CM-ST-2 | delta_len MUST equal delta_dim | Unit test | test_validation.cpp | PASS |
| CM-ST-3 | delta_input MUST be finite | Unit test | test_validation.cpp | PASS |
| CM-ST-4 | On invalid input, Step() MUST return ERROR | Unit test | test_validation.cpp | PASS |
| CM-ST-5 | On ERROR, no mutation MUST occur | Unit test | test_validation.cpp | PASS |

---

## 3. dt Stability Constraint

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-DT-1 | dt * max_rate MUST be < 1 | Unit test | test_dt_stability.cpp | PASS |
| CM-DT-2 | If violated, Step() MUST return ERROR | Unit test | test_dt_stability.cpp | PASS |
| CM-DT-3 | No mutation on dt instability | Unit test | test_dt_stability.cpp | PASS |

---

## 4. Canonical Evolution

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-CAN-1 | Phi update MUST follow canonical formula | Analytical comparison | test_canonical.cpp | PASS |
| CM-CAN-2 | Memory update MUST use Phi_next | Analytical comparison | test_canonical.cpp | PASS |
| CM-CAN-3 | Kappa update MUST follow canonical equation | Analytical comparison | test_canonical.cpp | PASS |

---

## 5. Norm Guard (Preserve Direction)

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-NG-1 | Norm guard MUST preserve direction | Direction ratio comparison | test_norm_guard.cpp | PASS |
| CM-NG-2 | Guard MUST limit magnitude only | Analytical comparison | test_norm_guard.cpp | PASS |

---

## 6. Collapse Semantics

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-COL-1 | Collapse occurs when kappa reaches 0 | Unit test | test_collapse.cpp | PASS |
| CM-COL-2 | Collapse MUST emit exactly one COLLAPSE event | Unit test | test_collapse.cpp | PASS |
| CM-COL-3 | Lifecycle.terminal MUST become true | Unit test | test_collapse.cpp | PASS |
| CM-COL-4 | collapse_emitted MUST become true | Unit test | test_collapse.cpp | PASS |
| CM-COL-5 | After terminal, Step() MUST short-circuit | Unit test | test_terminal_shortcircuit.cpp | PASS |
| CM-COL-6 | After terminal, no further COLLAPSE events | Unit test | test_collapse.cpp | PASS |

---

## 7. Clamp Invariants

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-INV-1 | Phi MUST be >= 0 | Unit test | test_clamp_invariants.cpp | PASS |
| CM-INV-2 | Memory MUST be >= 0 | Unit test | test_clamp_invariants.cpp | PASS |
| CM-INV-3 | Kappa MUST be in [0, kappa_max] | Unit test | test_clamp_invariants.cpp | PASS |

---

## 8. Determinism

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-DET-1 | Identical inputs MUST produce identical trajectories | Replay comparison | test_determinism.cpp | PASS |
| CM-DET-2 | No hidden state | Replay comparison | test_determinism.cpp | PASS |

---

## 9. Derived Projection Layer

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-DER-1 | Derived MUST be deterministic | Replay comparison | test_derived_consistency.cpp | PASS |
| CM-DER-2 | Derived MUST not mutate core | Snapshot comparison | test_derived_consistency.cpp | PASS |
| CM-DER-3 | Derived rates MUST match delta/dt | Analytical comparison | test_derived_consistency.cpp | PASS |

---

## 10. Initial Terminal State

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-INIT-1 | If initial kappa == 0, terminal MUST be true | Unit test | test_initial_terminal.cpp | PASS |
| CM-INIT-2 | Step() MUST short-circuit immediately | Unit test | test_initial_terminal.cpp | PASS |

---

## 11. Long-Run Numerical Stability

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-NUM-1 | No NaN during long run | Stress test | test_long_run_finite.cpp | PASS |
| CM-NUM-2 | No Inf during long run | Stress test | test_long_run_finite.cpp | PASS |
| CM-NUM-3 | Invariants preserved under long run | Stress test | test_long_run_finite.cpp | PASS |

---

## 12. C API Compliance

| ID | Requirement | Verification Method | Evidence | Status |
|----|------------|--------------------|----------|--------|
| CM-CAPI-1 | C API state equals C++ state | Cross comparison | test_c_api_parity.cpp | PASS |
| CM-CAPI-2 | maxcore_last_error MUST set on ERROR | Unit test | test_c_api_error_channel.cpp | PASS |
| CM-CAPI-3 | last_error MUST clear after valid step | Unit test | test_c_api_error_channel.cpp | PASS |
| CM-CAPI-4 | Null handle MUST return diagnostic string | Unit test | test_c_api_error_channel.cpp | PASS |

---

# Final Certification Status

Total Requirements Verified: 36  
Total Failures: 0  
Test Suite Result: 15/15 Passed  
Determinism Mode: Strict FP Enabled  

MAX-Core V2.5 is fully compliant with its Specification and Design documents.
