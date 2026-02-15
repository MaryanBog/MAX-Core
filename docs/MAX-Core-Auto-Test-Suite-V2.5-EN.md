# MAX-Core Auto-Test Suite V2.5
## Deterministic Verification and Invariant Test Specification

---

## 1. Scope

This document defines the complete automatic test suite
for MAX-Core V2.5.

The purpose of the test suite is to verify:

- Correct initialization behavior.
- Strict enforcement of input validation.
- Deterministic execution of canonical equations.
- Proper invariant enforcement.
- Correct collapse detection.
- Strict terminal state behavior.
- ERROR isolation without mutation.
- Deterministic structural time progression.
- Bitwise reproducibility under identical conditions.

The test suite does NOT redefine the specification.
It verifies compliance of the implementation
with Specification V2.5, Design V2.5, and Implementation V2.5.

Test categories include:

- Initialization Tests
- Validation Tests
- Canonical Update Tests
- Invariant Enforcement Tests
- Collapse Tests
- Terminal State Tests
- Determinism Tests
- Error Isolation Tests
- Stress and Boundary Tests

All tests must be deterministic.

No test may rely on randomness,
system time,
or external state.

The test suite is normative for runtime verification of MAX-Core V2.5.

---

## 2. Initialization Test Cases

This section defines automatic tests
verifying initialization correctness
and parameter validation behavior.

All initialization tests MUST confirm:

- Deterministic behavior.
- Strict validation.
- No partial instance exposure.
- No hidden mutation.

---

### 2.1 Valid Initialization Test

**Test ID:** INIT-T1  
**Purpose:** Verify successful deterministic construction.

Input:
- Valid ParameterSet (all coefficients > 0, finite)
- delta_dim > 0
- Valid initial StructuralState satisfying invariants
- Optional valid delta_max

Expected Result:
- Create() returns non-empty instance.
- current_ equals initial_state.
- previous_ equals initial_state.
- step_counter == 0.
- terminal == (initial_state.kappa == 0).
- collapse_emitted == false.

Status:
PASS required.

---

### 2.2 Invalid ParameterSet: Non-Finite Coefficient

**Test ID:** INIT-T2  
**Purpose:** Ensure non-finite parameter rejects construction.

Input:
- ParameterSet containing NaN or Inf.
- Valid delta_dim and initial_state.

Expected Result:
- Create() returns std::nullopt.

No instance created.

---

### 2.3 Invalid ParameterSet: Non-Positive Coefficient

**Test ID:** INIT-T3  
**Purpose:** Ensure zero or negative coefficient rejects construction.

Input:
- ParameterSet with alpha <= 0 (repeat for each coefficient).
- Valid delta_dim and initial_state.

Expected Result:
- Create() returns std::nullopt.

---

### 2.4 Invalid delta_dim

**Test ID:** INIT-T4  
**Purpose:** delta_dim must be strictly positive.

Input:
- delta_dim = 0.
- Otherwise valid configuration.

Expected Result:
- Create() returns std::nullopt.

---

### 2.5 Invalid initial_state: Negative Phi

**Test ID:** INIT-T5  
**Purpose:** Ensure invariant violation rejects construction.

Input:
- initial_state.phi < 0.
- Otherwise valid configuration.

Expected Result:
- Create() returns std::nullopt.

---

### 2.6 Invalid initial_state: Kappa Out of Bounds

**Test ID:** INIT-T6  
**Purpose:** Ensure Kappa outside [0, kappa_max] rejects construction.

Input:
- initial_state.kappa < 0
  or initial_state.kappa > kappa_max.

Expected Result:
- Create() returns std::nullopt.

---

### 2.7 Invalid delta_max

**Test ID:** INIT-T7  
**Purpose:** Ensure invalid delta_max rejects construction.

Input:
- delta_max <= 0
  or non-finite delta_max.

Expected Result:
- Create() returns std::nullopt.

---

### 2.8 Deterministic Initialization Reproducibility

**Test ID:** INIT-T8  
**Purpose:** Ensure deterministic construction.

Procedure:
- Construct two instances with identical inputs.

Expected Result:
- Internal state identical.
- LifecycleContext identical.
- Bitwise equality of state fields.

---

### 2.9 Terminal Initialization Test

**Test ID:** INIT-T9  
**Purpose:** Verify terminal state when initialized with Kappa == 0.

Input:
- initial_state.kappa == 0.

Expected Result:
- terminal == true.
- collapse_emitted == false.
- step_counter == 0.
- Step() immediately short-circuits.

---

### Initialization Test Coverage Summary

The initialization test suite verifies:

- Parameter validation.
- Structural invariant validation.
- Deterministic construction.
- Correct lifecycle initialization.
- Proper terminal detection at initialization.

Initialization correctness is fully test-covered.

---

## 3. Input Validation and ERROR Handling Test Cases

This section defines automatic tests verifying
input validation logic and ERROR isolation behavior.

All validation tests MUST confirm:

- ERROR is returned when required.
- No mutation occurs on ERROR.
- step_counter does not increment.
- previous_ remains unchanged.
- lifecycle flags remain unchanged.

---

### 3.1 Null Delta Pointer

**Test ID:** VAL-T1  
**Purpose:** Ensure null delta pointer triggers ERROR.

Input:
- delta_input = nullptr
- delta_len valid
- dt valid

Expected Result:
- Step() returns ERROR.
- current_ unchanged.
- previous_ unchanged.
- step_counter unchanged.
- lifecycle flags unchanged.

---

### 3.2 Delta Length Mismatch

**Test ID:** VAL-T2  
**Purpose:** Ensure mismatched delta_len triggers ERROR.

Input:
- delta_len != delta_dim_
- delta_input valid
- dt valid

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.3 Non-Finite Delta Component

**Test ID:** VAL-T3  
**Purpose:** Ensure NaN or Inf in delta triggers ERROR.

Input:
- delta_input contains NaN or Inf.
- delta_len correct.
- dt valid.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.4 Invalid dt: Non-Positive

**Test ID:** VAL-T4  
**Purpose:** dt must be strictly positive.

Input:
- dt <= 0.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.5 Stability Constraint Violation

**Test ID:** VAL-T5  
**Purpose:** Ensure dt * max_rate >= 1 triggers ERROR.

Input:
- dt chosen so that dt * max_rate >= 1.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.6 Non-Finite norm2

**Test ID:** VAL-T6  
**Purpose:** Overflow during norm computation triggers ERROR.

Input:
- Extremely large delta causing norm2 overflow.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.7 Non-Finite Phi_next

**Test ID:** VAL-T7  
**Purpose:** Overflow in energy update triggers ERROR.

Input:
- Large Phi and dt causing overflow.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.8 Non-Finite Memory_next

**Test ID:** VAL-T8  
**Purpose:** Overflow in memory update triggers ERROR.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.9 Non-Finite Kappa_next

**Test ID:** VAL-T9  
**Purpose:** Overflow in stability update triggers ERROR.

Expected Result:
- Step() returns ERROR.
- No mutation occurs.

---

### 3.10 ERROR Isolation Integrity

**Test ID:** VAL-T10  
**Purpose:** Verify state unchanged after ERROR.

Procedure:
- Snapshot current_, previous_, lifecycle_.
- Trigger ERROR via invalid input.
- Compare state before and after.

Expected Result:
- All fields identical.
- step_counter unchanged.

---

### Validation and ERROR Handling Coverage Summary

Validation tests confirm:

- All input constraints enforced.
- Numerical stability constraint enforced.
- Non-finite values detected.
- ERROR produces zero mutation.
- Lifecycle integrity preserved on failure.

Input validation and ERROR isolation are fully test-covered.

---

## 4. Canonical Update and Invariant Enforcement Test Cases

This section defines automatic tests verifying
correct implementation of canonical equations
and invariant enforcement.

All canonical update tests MUST confirm:

- Correct numerical update.
- Correct dependency ordering.
- Correct clamping behavior.
- Finite result enforcement.
- No hidden correction.

---

### 4.1 Deterministic Energy Update

**Test ID:** CAN-T1  
**Purpose:** Verify correct Phi update.

Input:
- Known initial state.
- Known delta_input.
- Known dt.
- Known parameters.

Procedure:
- Manually compute expected Phi_next.
- Call Step().
- Compare committed Phi to expected.

Expected Result:
- Phi matches expected value within double precision.

---

### 4.2 Deterministic Memory Update

**Test ID:** CAN-T2  
**Purpose:** Verify Memory uses Phi_next (not Phi_current).

Input:
- Known initial state.
- Known delta_input.
- Known dt.

Procedure:
- Compute expected Phi_next.
- Compute expected Memory_next using Phi_next.
- Compare committed Memory.

Expected Result:
- Memory matches canonical equation.

---

### 4.3 Deterministic Stability Update

**Test ID:** CAN-T3  
**Purpose:** Verify Kappa update matches canonical equation.

Procedure:
- Compute expected Kappa_next manually.
- Compare with committed value.

Expected Result:
- Kappa matches canonical equation.

---

### 4.4 Zero Delta Case

**Test ID:** CAN-T4  
**Purpose:** Verify behavior when delta_input = 0.

Input:
- delta_input all zeros.

Expected Result:
- Phi decays according to eta.
- Memory decays according to gamma.
- Kappa regenerates toward kappa_max.
- No unexpected change occurs.

---

### 4.5 Norm Guard Behavior

**Test ID:** CAN-T5  
**Purpose:** Verify norm guard scales magnitude only.

Input:
- delta_input exceeding delta_max.

Expected Result:
- norm2 equals delta_max^2.
- Direction preserved.
- No sign inversion.
- Deterministic scaling.

---

### 4.6 Invariant Clamp: Phi Non-Negative

**Test ID:** CAN-T6  
**Purpose:** Ensure Phi is clamped at zero.

Input:
- Parameters and dt causing Phi_next < 0 before clamp.

Expected Result:
- Committed Phi == 0.

---

### 4.7 Invariant Clamp: Memory Non-Negative

**Test ID:** CAN-T7  
**Purpose:** Ensure Memory is clamped at zero.

Expected Result:
- Committed Memory == 0 if negative candidate.

---

### 4.8 Invariant Clamp: Kappa Bounds

**Test ID:** CAN-T8  
**Purpose:** Ensure Kappa clamped within bounds.

Input:
- Parameters causing Kappa_next > kappa_max.
- Parameters causing Kappa_next < 0.

Expected Result:
- Committed Kappa ∈ [0, kappa_max].

---

### 4.9 No Hidden Cross-Term Test

**Test ID:** CAN-T9  
**Purpose:** Ensure no unintended coupling exists.

Procedure:
- Set Phi_current = 0, Memory_current = 0.
- Apply zero delta.
- Confirm no unexpected drift in Kappa beyond regeneration.

Expected Result:
- Kappa evolves exactly per regeneration term.

---

### 4.10 Sequential Consistency Test

**Test ID:** CAN-T10  
**Purpose:** Verify sequential dependency ordering.

Procedure:
- Compute one step manually.
- Apply two successive Step() calls.
- Compare expected multi-step result.

Expected Result:
- Multi-step evolution matches iterative canonical computation.

---

### Canonical Update Coverage Summary

The canonical update test suite verifies:

- Correct equation implementation.
- Proper dependency ordering.
- Deterministic arithmetic.
- Invariant enforcement.
- Correct norm guard behavior.
- Absence of hidden terms.

Canonical equation correctness is fully test-covered.

---

## 5. Collapse and Terminal State Test Cases

This section defines automatic tests verifying:

- Correct collapse detection.
- Single collapse emission.
- Proper lifecycle flag updates.
- Strict terminal state behavior.
- Irreversibility of collapse.

All collapse tests MUST confirm:

- Collapse occurs only when Kappa reaches zero.
- Collapse is emitted exactly once.
- Terminal state is absorbing.
- No mutation occurs after collapse.

---

### 5.1 Exact Collapse Detection

**Test ID:** COL-T1  
**Purpose:** Verify collapse emitted when Kappa crosses to zero.

Procedure:
- Configure parameters and inputs so that Kappa_next == 0.
- Call Step().

Expected Result:
- Step() returns COLLAPSE.
- Committed Kappa == 0.
- lifecycle_.terminal == true.
- lifecycle_.collapse_emitted == true.

---

### 5.2 No Collapse When Kappa > 0

**Test ID:** COL-T2  
**Purpose:** Ensure no false collapse.

Procedure:
- Use parameters where Kappa decreases but remains > 0.

Expected Result:
- Step() returns NORMAL.
- lifecycle_.collapse_emitted == false.

---

### 5.3 Single Collapse Emission

**Test ID:** COL-T3  
**Purpose:** Collapse must be emitted exactly once.

Procedure:
- Trigger collapse.
- Call Step() repeatedly afterward.

Expected Result:
- First collapse: COLLAPSE returned.
- Subsequent calls: NORMAL returned.
- collapse_emitted remains true.
- No duplicate COLLAPSE events.

---

### 5.4 Terminal Short-Circuit Behavior

**Test ID:** COL-T4  
**Purpose:** Verify terminal state short-circuit.

Procedure:
- After collapse, call Step() with valid input.

Expected Result:
- Step() returns NORMAL.
- current_ unchanged.
- previous_ unchanged.
- step_counter unchanged.

---

### 5.5 Structural Time Freeze

**Test ID:** COL-T5  
**Purpose:** Ensure step_counter does not increment in terminal state.

Procedure:
- Record step_counter at collapse.
- Call Step() multiple times.

Expected Result:
- step_counter remains constant.

---

### 5.6 No Regeneration After Collapse

**Test ID:** COL-T6  
**Purpose:** Ensure Kappa does not increase after collapse.

Procedure:
- Collapse instance.
- Call Step() repeatedly.

Expected Result:
- Kappa remains exactly zero.
- No regeneration term applied.

---

### 5.7 Collapse Boundary Precision Test

**Test ID:** COL-T7  
**Purpose:** Ensure collapse occurs only at exact zero.

Procedure:
- Configure Kappa_next to be extremely small but positive.

Expected Result:
- No collapse emitted.
- Collapse only occurs when committed Kappa == 0.

---

### 5.8 Collapse Determinism Test

**Test ID:** COL-T8  
**Purpose:** Ensure collapse timing deterministic.

Procedure:
- Run identical input sequence twice.

Expected Result:
- Collapse occurs at identical step.
- LifecycleContext identical.
- Structural state identical.

---

### Collapse and Terminal Test Coverage Summary

The collapse test suite verifies:

- Exact boundary condition enforcement.
- Single collapse emission.
- Strict terminal state isolation.
- Irreversibility of collapse.
- Deterministic collapse timing.
- Structural time freeze after collapse.

Collapse semantics are fully test-covered.

---

## 6. Determinism and Reproducibility Test Cases

This section defines automatic tests verifying:

- Deterministic state evolution.
- Bitwise reproducibility under identical conditions.
- Absence of hidden nondeterminism.
- Structural time determinism.

All determinism tests MUST confirm:

- Identical input sequences produce identical output sequences.
- No mutation occurs outside canonical pathway.
- Structural time progression is deterministic.

---

### 6.1 Identical Run Reproducibility

**Test ID:** DET-T1  
**Purpose:** Verify identical state evolution under identical inputs.

Procedure:
- Initialize two identical instances.
- Apply identical sequence of delta_input and dt.
- Compare state after each Step().

Expected Result:
- current_ identical at every step.
- previous_ identical.
- lifecycle_ identical.
- EventFlag identical.

Bitwise equality required.

---

### 6.2 Multi-Step Reproducibility

**Test ID:** DET-T2  
**Purpose:** Verify reproducibility over long sequences.

Procedure:
- Run N steps (N large).
- Repeat run with identical inputs.
- Compare full state history.

Expected Result:
- Entire structural trajectory identical.

---

### 6.3 ERROR Determinism

**Test ID:** DET-T3  
**Purpose:** ERROR must be deterministic.

Procedure:
- Trigger ERROR using invalid input.
- Repeat identical invalid input.

Expected Result:
- ERROR returned in both cases.
- State remains unchanged in both cases.

---

### 6.4 Collapse Determinism

**Test ID:** DET-T4  
**Purpose:** Collapse must occur at deterministic step.

Procedure:
- Use input sequence known to cause collapse.
- Repeat run.

Expected Result:
- Collapse occurs at identical step.
- Structural time identical.
- Final state identical.

---

### 6.5 Structural Time Monotonicity

**Test ID:** DET-T5  
**Purpose:** Verify structural time increments deterministically.

Procedure:
- Run valid Step() calls.
- Track step_counter.

Expected Result:
- step_counter increments by exactly 1 per commit.
- No increment on ERROR.
- No increment in terminal state.

---

### 6.6 No Hidden State Leakage

**Test ID:** DET-T6  
**Purpose:** Ensure external observation does not affect evolution.

Procedure:
- After each Step(), call Current() and Lifecycle().
- Compare with run without observation.

Expected Result:
- Identical structural evolution.

Observation has no side effects.

---

### 6.7 Cross-Instance Independence

**Test ID:** DET-T7  
**Purpose:** Ensure instances do not interfere.

Procedure:
- Run two instances in interleaved sequence.
- Compare with independent runs.

Expected Result:
- Each instance produces identical trajectory as standalone run.

No cross-instance contamination.

---

### 6.8 Floating-Point Determinism Under Strict Flags

**Test ID:** DET-T8  
**Purpose:** Verify bitwise reproducibility under mandated build flags.

Procedure:
- Compile with strict IEEE flags.
- Run identical input sequence twice.

Expected Result:
- Bitwise identical state and output.

Note:
Test valid only under mandated build constraints.

---

### Determinism Test Coverage Summary

The determinism test suite verifies:

- Deterministic transition function.
- Deterministic collapse timing.
- Deterministic structural time progression.
- ERROR isolation determinism.
- Observer neutrality.
- Cross-instance independence.
- Bitwise reproducibility under strict environment.

Determinism guarantees are fully test-covered.

---

## 7. Stress, Boundary, and Robustness Test Cases

This section defines automatic tests verifying
robustness under extreme, boundary, and long-duration scenarios.

All stress tests MUST confirm:

- No invariant violation.
- No undefined behavior.
- Correct ERROR isolation.
- Deterministic outcomes.
- No silent overflow propagation.

---

### 7.1 Large Delta Magnitude Stress

**Test ID:** STR-T1  
**Purpose:** Verify robustness under large delta input.

Procedure:
- Provide delta_input with extremely large magnitude.
- Ensure norm guard or overflow detection triggers.

Expected Result:
- Either norm guard scales safely
  OR ERROR returned.
- No mutation on ERROR.
- No non-finite commit.

---

### 7.2 Very Small dt

**Test ID:** STR-T2  
**Purpose:** Verify stability under very small dt.

Procedure:
- Use dt approaching machine epsilon.
- Run multiple steps.

Expected Result:
- Stable evolution.
- No numerical instability.
- Deterministic output.

---

### 7.3 dt Near Stability Boundary

**Test ID:** STR-T3  
**Purpose:** Verify enforcement of dt * max_rate < 1.

Procedure:
- Use dt slightly below boundary.
- Then use dt equal to boundary.
- Then slightly above.

Expected Result:
- Below boundary: valid step.
- At or above boundary: ERROR.
- No mutation on ERROR.

---

### 7.4 Large Phi and Memory Values

**Test ID:** STR-T4  
**Purpose:** Verify bounded behavior with large structural values.

Procedure:
- Initialize Phi and Memory near large values.
- Apply moderate inputs.

Expected Result:
- No overflow without detection.
- ERROR if overflow occurs.
- No silent non-finite commit.

---

### 7.5 Near-Zero Kappa Boundary

**Test ID:** STR-T5  
**Purpose:** Verify correct collapse threshold.

Procedure:
- Set Kappa to very small positive value.
- Apply load sufficient to push below zero.

Expected Result:
- Kappa clamped to zero.
- COLLAPSE emitted.
- No negative committed Kappa.

---

### 7.6 Long-Run Stability Test

**Test ID:** STR-T6  
**Purpose:** Verify stability over many steps.

Procedure:
- Run large number of steps with bounded input.
- Monitor invariants.

Expected Result:
- No drift outside bounds.
- No invariant violation.
- Deterministic trajectory.

---

### 7.7 Subnormal Value Handling

**Test ID:** STR-T7  
**Purpose:** Verify correct handling of subnormal values.

Procedure:
- Use very small delta values.
- Observe evolution.

Expected Result:
- No ERROR unless non-finite.
- Deterministic behavior.
- No unexpected clamp beyond canonical rules.

---

### 7.8 Rapid Collapse Scenario

**Test ID:** STR-T8  
**Purpose:** Verify collapse under extreme stress.

Procedure:
- Use high load causing immediate collapse.

Expected Result:
- COLLAPSE emitted.
- Terminal state enforced.
- No mutation afterward.

---

### 7.9 Alternating Valid/Invalid Inputs

**Test ID:** STR-T9  
**Purpose:** Verify robustness under alternating valid and invalid steps.

Procedure:
- Apply valid step.
- Apply invalid step (ERROR).
- Apply valid step again.

Expected Result:
- ERROR step does not corrupt state.
- Subsequent valid step behaves correctly.

---

### 7.10 Multi-Lifecycle Continuous Operation

**Test ID:** STR-T10  
**Purpose:** Verify stability across repeated Fresh Genesis cycles.

Procedure:
- Run until collapse.
- Reinitialize.
- Repeat multiple cycles.

Expected Result:
- Each lifecycle independent.
- No state leakage across cycles.
- Deterministic behavior preserved.

---

### Stress and Boundary Coverage Summary

The stress test suite verifies:

- Numerical robustness.
- Stability constraint enforcement.
- Overflow detection.
- Boundary clamp correctness.
- Collapse boundary precision.
- Long-run stability.
- Deterministic lifecycle repetition.
- ERROR isolation under mixed inputs.

Robustness guarantees are fully test-covered.