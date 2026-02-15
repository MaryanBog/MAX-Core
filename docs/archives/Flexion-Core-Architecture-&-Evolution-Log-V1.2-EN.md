# Flexion Core — Architecture & Evolution Log
## Version 1.2 (Theory-Aligned)

---

## 1. Structural Ontology

The Flexion Core models the evolution of a single structural entity.

At any discrete step t:

X(t) = (Δ(t), Φ(t), M(t), κ(t))

Where:

Δ(t) ∈ ℝ^n  
    Structural deviation vector.

Φ(t) ≥ 0  
    Structural energy derived strictly from Δ.

M(t) ≥ 0  
    Accumulated structural memory (irreversible path accumulation).

κ(t) ≥ 0  
    Structural viability (finite existence resource).

No additional coordinate is fundamental.

---

### 1.1 Non-State Quantities

The following quantities are NOT part of X:

- Regime σ(t)
- Structural Reversibility Index (SRI)
- Structural Reversibility Density (SRD)
- Norm ||Δ||
- Any diagnostic or derived metric

All such quantities are deterministic functions of X(t).

They introduce no additional state.

---

### 1.2 Structural Separation Principle

The structure X is fully defined by its four coordinates.

No hidden variables are allowed.

No auxiliary internal memory beyond M is permitted.

No regime state is stored.

---

### 1.3 Structural Finite Existence

Each structure X exists only while:

κ(t) > 0

When κ(t) = 0:

- The structure is terminated.
- No further evolution of that structure is allowed.
- Only a new structure may be created via Genesis.

---

### 1.4 Cycle-Level Continuity

Structural cycles may repeat indefinitely under continuous Δ input.

However:

- No state transfers across cycles.
- No memory transfer.
- No viability restoration.
- No regime inheritance.

Each cycle corresponds to an independent structural instance.

---

### 1.5 Deterministic Closure

The state X(t) must be sufficient to determine:

- future evolution,
- regime,
- memory growth,
- viability loss,
- collapse timing.

No external hidden dependency is allowed.

This completes the ontological definition of the structural state.

---

## 2. Structural Energy Definition

Structural energy Φ is a deterministic functional of Δ.

Energy is not an independent coordinate.
It is recomputed from Δ at every step.

---

### 2.1 Canonical Energy Form

In Version 1.2 the canonical form is:

Φ(Δ) = Σ_i w_i |Δ_i|

Where:

- w_i > 0
- w_i are fixed at initialization
- dimension n is fixed at initialization

---

### 2.2 Energy Properties

Φ satisfies:

- Φ ≥ 0
- Φ = 0 if and only if Δ = 0
- Φ is finite if Δ is finite
- Φ is strictly increasing with respect to |Δ_i|

Energy must not:

- accumulate independently
- be incrementally updated
- drift from Δ

---

### 2.3 Energy Consistency Rule

At every step:

Φ(t) = Φ(Δ(t))

If Δ changes,
Φ must be recomputed immediately.

No caching inconsistencies are allowed.

---

### 2.4 Structural Meaning

Φ represents instantaneous structural load.

It does not represent:

- accumulated stress,
- historical burden,
- future prediction.

Memory M represents accumulated effect.
Φ represents instantaneous magnitude.

---

### 2.5 Energy and Viability Coupling

Φ directly influences:

- memory accumulation rate,
- viability loss rate,
- collapse timing.

However, Φ alone does not determine collapse.
Collapse requires κ to reach zero.

---

### 2.6 Finite Energy Requirement

If any component of Δ becomes non-finite:

- Φ becomes invalid,
- Step must abort,
- State must remain unchanged.

Energy must remain finite for structural validity.

---

## 3. Structural Geometry

Structural geometry determines reversibility properties of the current deviation Δ.

Geometry depends only on:

- Δ
- fixed configuration parameters (w_i, h_i)

Memory M and viability κ do not influence geometry.

---

### 3.1 Action Space

Define discrete admissible action space:

𝒰 = { ±h_i e_i | i = 1..n }

Where:

- e_i are canonical basis vectors,
- h_i > 0 are fixed step magnitudes,
- n is the dimension of Δ.

Cardinality:

|𝒰| = 2n

Action space is fixed at initialization.

---

### 3.2 Deviation Norm

Norm is defined as:

||Δ|| = Σ_i w_i |Δ_i|

This norm is identical to Φ.

No alternative norm is permitted in Version 1.2.

---

### 3.3 Contractive Set

An action u ∈ 𝒰 is contractive if:

||Δ + u|| < ||Δ||

Define:

𝒦(Δ) = { u ∈ 𝒰 | ||Δ + u|| < ||Δ|| }

Contractivity is evaluated locally at current Δ.

---

### 3.4 Structural Reversibility Index

SRI(Δ) = |𝒦(Δ)|

Properties:

- SRI ∈ {0, 1, ..., 2n}
- SRI = 0 implies no contractive direction exists

---

### 3.5 Structural Reversibility Density

SRD(Δ) = SRI(Δ) / (2n)

SRD ∈ [0, 1]

Interpretation:

- SRD = 0 → fully irreversible geometry
- SRD = 1 → fully contractive geometry

---

### 3.6 Geometry Determinism

For identical Δ:

- SRI must be identical.
- SRD must be identical.

Geometry must not depend on:

- memory M,
- viability κ,
- time index,
- past states.

Geometry is purely a function of current deviation Δ.

---

### 3.7 Geometry and Collapse

Geometry alone does not trigger collapse.

Collapse occurs only through viability exhaustion (κ = 0).

However, SRD influences viability loss rate.

---

## 4. Regime Determination

Regime is a derived structural property.

Regime is not stored.
Regime is not part of X.
Regime is recomputed at every step.

---

### 4.1 Canonical Regime Rule

Regime depends only on structural geometry.

If:

SRI(Δ) > 0

Then:

σ = -1   (contractive regime)

Else:

σ = +1   (expansive regime)

---

### 4.2 Regime Properties

Regime must satisfy:

- Deterministic computation
- No dependence on memory M
- No dependence on viability κ
- No hysteresis
- No stochastic switching

Regime depends only on current Δ via geometry.

---

### 4.3 Structural Meaning

Contractive regime (σ = -1):

- At least one action reduces structural norm.
- Geometry admits local stabilization.

Expansive regime (σ = +1):

- No contractive action exists.
- Geometry admits only non-reducing directions.

---

### 4.4 Regime and Evolution

Regime influences:

- memory growth rate,
- viability loss amplification.

Regime does not directly modify Δ.

---

### 4.5 Regime Consistency Constraint

For identical Δ:

σ must be identical.

Any implementation that produces different σ
for identical Δ violates structural determinism.

---

## 5. Memory Discipline

Memory M represents accumulated structural irreversibility.

Memory is a structural coordinate.
Memory is monotonically non-decreasing.

---

### 5.1 Memory Domain

M ≥ 0

Optional upper bound:

0 ≤ M ≤ M_max

If M_max is defined, it must be fixed at initialization.

Memory must remain finite.

---

### 5.2 Accumulation Law

Memory accumulates as a function of structural energy.

Canonical discrete update:

M_next = M + dt * α * Φ

Where:

- α ≥ 0
- dt > 0
- Φ = Φ(Δ)

No negative increment is allowed.

---

### 5.3 Monotonicity Constraint

For all steps:

M(t+1) ≥ M(t)

Memory cannot decrease.
Memory cannot reset during evolution.

Memory resets only at Genesis.

---

### 5.4 Structural Interpretation

Memory represents:

- accumulated deviation history,
- structural bias,
- path dependence.

Memory does not represent:

- reversible fluctuation,
- instantaneous load,
- geometric reversibility.

Those belong to Φ and SRD.

---

### 5.5 Memory and Viability Coupling

Memory contributes to viability loss via:

M / (1 + M)

As M increases:

- structural fragility increases,
- viability burns faster.

Memory does not influence geometry or regime.

---

### 5.6 Numerical Stability

Memory update must ensure:

- finite arithmetic,
- no overflow,
- no negative values,
- deterministic accumulation.

Memory must not drift due to floating inconsistency.

---

## 6. Viability Discipline

Viability κ represents the finite structural capacity for continued existence.

κ is strictly monotonic non-increasing during the lifetime of a structure.

---

### 6.1 Viability Domain

0 ≤ κ ≤ κ_max

Where:

- κ_max > 0 is fixed at initialization.
- κ = 0 defines collapse.
- κ > 0 allows evolution.

---

### 6.2 Monotonic Constraint

For all steps:

κ(t+1) ≤ κ(t)

Viability cannot increase.
Viability cannot be restored.
No internal mechanism may raise κ.

---

### 6.3 Viability Loss Rate

Define non-negative loss function:

L(Δ, SRD, σ, M) ≥ 0

Canonical Version 1.2 form:

L = Φ * ( λ0
        + λ1 * I[σ = +1]
        + λ2 * (1 - SRD)
        + λ3 * M / (1 + M) )

Where:

- Φ = Φ(Δ)
- SRD = SRD(Δ)
- σ derived from geometry
- M memory
- λ_i ≥ 0 fixed parameters

---

### 6.4 Discrete Update

κ_next = max(0, κ - dt * L)

Properties:

- κ_next ≥ 0
- κ_next ≤ κ
- Collapse occurs if κ_next = 0

---

### 6.5 Structural Interpretation

Viability is consumed by:

- instantaneous structural load (Φ),
- geometric irreversibility (1 - SRD),
- expansive regime,
- accumulated memory.

Viability does not regenerate.
Collapse is irreversible.

---

### 6.6 Collapse Condition

If κ_next = 0:

- Collapse event is raised.
- Structure terminates.
- No further evolution allowed.
- Only Genesis may create new structure.

---

### 6.7 Numerical Safety

Implementation must ensure:

- No negative κ values.
- Exact clamp at zero upon collapse.
- No floating underflow.
- Deterministic arithmetic.

Violation of monotonic constraint invalidates the state.

---

## 7. Collapse

Collapse is the terminal event of a structural instance.

Collapse occurs if and only if:

κ = 0

No other condition defines collapse.

---

### 7.1 Collapse Properties

When κ reaches zero:

- The structure ceases to exist.
- No further Step evolution is permitted.
- Memory M is discarded.
- Regime becomes undefined.
- Geometry is irrelevant.

Collapse is irreversible.

---

### 7.2 Post-Collapse Discipline

After collapse:

- The internal state is frozen.
- Any Step call must first evaluate Genesis condition.
- If Genesis is not triggered, state remains terminated.

No silent continuation is allowed.

---

### 7.3 Collapse Is Not Error

Collapse is a valid structural outcome.

It must not be treated as:

- runtime failure,
- exception state,
- numeric instability.

Collapse is a structural lifecycle event.

---

### 7.4 Collapse and Geometry

Geometry does not directly cause collapse.

Even if SRD = 0:

- Collapse occurs only through κ exhaustion.

Geometry influences viability loss,
but κ = 0 is the only collapse gate.

---

### 7.5 Single Collapse Gate Principle

There exists exactly one collapse gate:

κ = 0

No secondary collapse mechanisms are allowed.

This ensures strict lifecycle clarity.

---

## 8. Genesis

Genesis creates a new structural instance after collapse.

Genesis is not recovery.
Genesis is not continuation.
Genesis is the creation of a new X.

---

### 8.1 Genesis Trigger Conditions

Genesis may occur only if:

1. κ_current = 0
2. ||Δ_in|| > 0
3. SRD(Δ_in) > 0

All three conditions must hold.

If SRD(Δ_in) = 0:

Genesis does not occur.
No structure is created.

---

### 8.2 Genesis Initialization

Given Δ_seed = Δ_in:

Δ₀ = Δ_seed  
Φ₀ = Φ(Δ₀)  
M₀ = 0  
κ₀ = κ_max * SRD(Δ₀)

Properties:

- κ₀ > 0 (since SRD > 0)
- κ₀ ≤ κ_max
- No previous state is reused

---

### 8.3 Genesis Independence Principle

Between structures:

- No transfer of memory.
- No transfer of viability.
- No regime inheritance.
- No derived quantity transfer.

Each structure is independent.

---

### 8.4 Cycle Identifier

Each Genesis increments:

cycle_id ∈ ℕ

cycle_id must:

- increase monotonically,
- uniquely identify structural lifetime,
- be exposed in output.

---

### 8.5 Zero Impulse After Collapse

If:

κ = 0
AND
||Δ_in|| = 0

Then:

No Genesis occurs.
State remains terminated.

---

### 8.6 Structural Meaning

Genesis represents the birth of a new structural configuration
under new deviation conditions.

Collapse ends a structure.
Genesis begins another.

There is no resurrection.
There is only replacement.

---

## 9. Input Discipline and Step Execution Order

This section defines the external impulse model
and the canonical deterministic execution sequence.

---

### 9.1 External Impulse Model

Δ evolves only through external impulse.

At each step:

Δ ← Δ + Δ_in

There is no internal Δ dynamics.

If Δ_in = 0:

Δ remains unchanged.

---

### 9.2 Input Validity Conditions

Before applying Step:

- Δ_in dimension must equal n.
- All components of Δ_in must be finite.
- dt must be strictly positive.

If any condition fails:

- Step must abort.
- State must remain unchanged.

---

### 9.3 Canonical Step Order

For κ > 0:

1. Validate input.
2. Apply impulse: Δ ← Δ + Δ_in.
3. Recompute energy Φ.
4. Compute geometry (SRI, SRD).
5. Determine regime σ.
6. Update memory M.
7. Compute viability loss L.
8. Update viability κ.
9. Check collapse.
10. Increment tick.

No reordering is permitted.

---

### 9.4 Post-Collapse Handling

If κ = 0 at step entry:

1. Evaluate Genesis conditions.
2. If Genesis valid:
       Create new structure.
   Else:
       Return without evolution.

No other behavior allowed.

---

### 9.5 Derived Quantities

After state update:

Derived metrics may be computed:

- ||Δ||
- SRI
- SRD
- regime σ
- ΔM
- Δκ
- field approximations

Derived values must not modify state.

---

### 9.6 Deterministic Closure

Given identical:

- initial state,
- configuration,
- Δ input sequence,

The trajectory of X must be identical.

No stochastic component is allowed.

This completes the operational definition of the Core.

---

## 10. Invariants and Integrity Constraints

This section defines structural invariants that must hold
at all times during execution.

Violation of any invariant invalidates the structure.

---

### 10.1 State Domain Invariants

At every step:

- Δ components must be finite.
- Φ must equal Φ(Δ).
- M ≥ 0.
- κ ≥ 0.
- κ ≤ κ_max.
- Dimension of Δ remains constant.

---

### 10.2 Monotonic Viability Invariant

For any structure:

κ(t+1) ≤ κ(t)

If κ(t) = 0:

No further evolution is allowed for that structure.

---

### 10.3 Monotonic Memory Invariant

For any structure:

M(t+1) ≥ M(t)

Memory must not decrease.

Memory resets only at Genesis.

---

### 10.4 Energy Consistency Invariant

Energy must always satisfy:

Φ(t) = Σ_i w_i |Δ_i(t)|

Energy must not accumulate independently.
Energy must not drift from Δ.

---

### 10.5 Geometry Determinism Invariant

For identical Δ:

- SRI must be identical.
- SRD must be identical.
- Regime must be identical.

Geometry must not depend on:

- memory,
- viability,
- time,
- previous states.

---

### 10.6 Collapse Integrity Invariant

If κ = 0:

- Collapse must be raised.
- State must not evolve further.
- Only Genesis may create a new structure.

---

### 10.7 Genesis Integrity Invariant

Upon Genesis:

- Δ₀ = Δ_seed.
- Φ₀ = Φ(Δ₀).
- M₀ = 0.
- κ₀ = κ_max * SRD(Δ₀).
- κ₀ > 0.
- cycle_id increments.

No previous state influence is permitted.

---

### 10.8 Deterministic Evolution Invariant

For identical:

- configuration,
- initial state,
- Δ input sequence,

The entire structural trajectory must be identical.

Any deviation violates Core integrity.

---

## 11. Configuration Parameters

This section defines all configuration parameters required
to instantiate a Flexion Core instance.

Configuration is immutable after initialization.

---

### 11.1 Structural Dimension

n > 0

- Dimension of Δ.
- Fixed at initialization.
- Cannot change during runtime.

---

### 11.2 Energy Weights

w_i > 0 for i = 1..n

Used in:

Φ(Δ) = Σ_i w_i |Δ_i|

Constraints:

- All weights must be finite.
- All weights must be strictly positive.
- Weights remain constant for structure lifetime.

---

### 11.3 Action Step Magnitudes

h_i > 0 for i = 1..n

Used to construct action space:

𝒰 = { ±h_i e_i }

Constraints:

- All h_i must be finite.
- All h_i strictly positive.
- No adaptive modification allowed.

---

### 11.4 Memory Parameter

α ≥ 0

Used in memory update:

M_next = M + dt * α * Φ

Optional:

M_max ≥ 0

If M_max is defined, memory must be clamped to [0, M_max].

---

### 11.5 Viability Parameters

κ_max > 0

λ0 ≥ 0  
λ1 ≥ 0  
λ2 ≥ 0  
λ3 ≥ 0  

Used in viability loss function:

L = Φ * ( λ0
        + λ1 * I[σ = +1]
        + λ2 * (1 - SRD)
        + λ3 * M / (1 + M) )

All parameters must be finite and non-negative.

---

### 11.6 Time Parameter

dt_default > 0

Used if external dt not provided.

dt must always satisfy:

dt > 0

---

### 11.7 Configuration Validity Rules

Initialization must fail if:

- n ≤ 0
- any w_i ≤ 0
- any h_i ≤ 0
- κ_max ≤ 0
- any λ_i < 0
- α < 0
- dt_default ≤ 0

Core instance must not be created with invalid configuration.

---

### 11.8 Immutability Principle

After initialization:

- No configuration parameter may change.
- Any change requires a new Core instance.

This guarantees structural reproducibility.

---

## 12. Output Architecture

This section defines how structural information is exposed
outside the Core.

Output must be a deterministic projection of X(t).

No hidden variables may be exposed.

---

### 12.1 Primary Structural State Output

The Core must be able to return:

- Δ
- Φ
- M
- κ
- cycle_id
- tick
- dt

These quantities define the full structural state.

---

### 12.2 Derived Structural Metrics

The following quantities may be returned as derived values:

- ||Δ|| (equal to Φ)
- SRI
- SRD
- regime σ
- ΔM (memory increment)
- Δκ (viability decrement)

Derived metrics:

- must be computed from current X,
- must not modify X,
- must be deterministic.

---

### 12.3 Lifecycle Flags

Output must expose lifecycle events:

- NORMAL
- COLLAPSED
- GENESIS

Lifecycle flags must reflect structural transitions exactly.

No silent state transitions are permitted.

---

### 12.4 Field Approximation (Optional)

Discrete field approximation may be exposed:

Δ_field = (Δ(t) − Δ(t−1)) / dt  
Φ_field = (Φ(t) − Φ(t−1)) / dt  
M_field = (M(t) − M(t−1)) / dt  
κ_field = (κ(t) − κ(t−1)) / dt  

Field quantities are computational diagnostics.
They introduce no additional state.

---

### 12.5 Output Determinism

For identical inputs and configuration:

- Output must be identical.
- Floating-point behavior must be stable.
- No platform-dependent branching allowed.

---

### 12.6 Output Integrity Constraint

Reported values must satisfy:

- Φ = Φ(Δ)
- κ ≥ 0
- M ≥ 0
- Derived metrics consistent with geometry

Any mismatch between internal state and reported output
violates Core integrity.

---

## 13. Error Handling and Failure Modes

This section defines how the Core handles invalid inputs,
numerical instability, and structural violations.

The Core must fail explicitly.
Silent corruption is forbidden.

---

### 13.1 Input Validation Failure

Step must abort without state mutation if:

- Δ_in dimension mismatch
- Any Δ_in component is non-finite
- dt ≤ 0
- Core not initialized
- Configuration invalid

In such case:

- State remains unchanged.
- Error code is returned.
- No lifecycle flags are modified.

---

### 13.2 Numerical Instability

If during computation any of the following becomes non-finite:

- Δ
- Φ
- M
- κ
- SRD
- L

Then:

- Step must abort.
- Previous valid state must be preserved.
- Error code must be returned.

No partial state mutation is allowed.

---

### 13.3 Invariant Violation

If any invariant defined in Section 10 is violated:

- Structure becomes invalid.
- Further evolution is prohibited.
- Explicit failure must be raised.

Violation examples:

- κ increases
- M decreases
- Φ inconsistent with Δ
- Non-deterministic regime

---

### 13.4 Collapse Is Not Failure

Collapse is a valid structural event.

Collapse must:

- Set κ = 0
- Raise COLLAPSED flag
- Preserve deterministic behavior

Collapse must not return error.

---

### 13.5 Genesis Failure

Genesis must fail if:

- ||Δ_seed|| = 0
- SRD(Δ_seed) = 0

If Genesis fails:

- No new structure is created.
- State remains collapsed.
- No silent fallback permitted.

---

### 13.6 No Partial Mutation Rule

At any Step:

Either:

- Entire update succeeds,

Or:

- State remains unchanged.

No intermediate mutation is permitted.

---

### 13.7 Error vs Lifecycle Separation

Errors represent:

- invalid input
- numeric instability
- invariant violation

Lifecycle events represent:

- collapse
- genesis

Errors and lifecycle events must be distinguishable.

---

## 14. Versioning and Structural Stability Policy

This section defines how the Flexion Core architecture may evolve
while preserving theoretical integrity.

Versioning is structural, not cosmetic.

---

### 14.1 Version Levels

Major Version (X.0)

A major version increment is required if:

- The definition of X changes.
- A new structural coordinate is introduced.
- The monotonic viability constraint is modified.
- Collapse definition changes.
- Genesis logic changes.
- Memory ceases to be accumulation-only.
- Determinism requirement is altered.

Minor Version (X.Y)

A minor version increment is allowed if:

- Documentation clarity improves.
- Invariants are tightened.
- Numeric stability rules are refined.
- Output format expands without altering state.
- Parameter validation rules are strengthened.

Patch Version (X.Y.Z)

Patch increment applies only to:

- Typographical corrections.
- Formatting corrections.
- Non-structural clarifications.

---

### 14.2 Backward Structural Compatibility

Minor and patch revisions must preserve:

- State definition X(t) = (Δ, Φ, M, κ)
- Accumulation-only memory law
- Monotonic non-increasing κ
- Collapse defined strictly as κ = 0
- Genesis defined strictly via SRD(Δ_seed) > 0
- Deterministic Step order

Any behavioral change requires major version increment.

---

### 14.3 Structural Audit Requirement

Any architectural modification must be evaluated against:

1. Ontological consistency.
2. Deterministic closure.
3. Monotonic viability.
4. Memory monotonicity.
5. Collapse irreversibility.
6. Cycle independence.

If any criterion fails, modification must be rejected
or escalated to major version.

---

### 14.4 Theory Alignment Requirement

All future modifications must remain consistent with:

- Flexion Framework
- Flexion Dynamics School

If a theoretical conflict arises:

- Theoretical revision must precede Core modification.
- Core must not contradict established theory.

---

### 14.5 Stability Objective

The purpose of this policy is:

- To prevent theoretical drift.
- To protect structural minimalism.
- To ensure reproducibility.
- To preserve lifecycle clarity.
- To maintain strict collapse–genesis separation.

Version 1.2 establishes a theory-aligned structural baseline.

---

## 15. Deterministic and Numerical Discipline

This section fixes the execution discipline required for research-grade reproducibility.

---

### 15.1 Determinism Guarantee

For identical:

- configuration
- initial state
- Δ input sequence
- dt sequence

the Core MUST produce identical:

- X(t) trajectory
- SRI/SRD trajectory
- regime σ trajectory
- collapse timing
- genesis timing
- outputs

No randomness is permitted.

---

### 15.2 Finite Arithmetic Requirement

All computations MUST remain finite.

Forbidden values:

- NaN
- +Inf
- -Inf

If any non-finite value is detected during Step:

- Step MUST abort
- state MUST remain unchanged
- an error MUST be returned

---

### 15.3 Clamp Discipline

The Core MUST enforce:

- κ_next = max(0, κ_next)
- M_next = max(0, M_next)

If M_max is configured:

- M_next = min(M_next, M_max)

No underflow below zero is permitted.

---

### 15.4 No Partial Mutation Rule

Step MUST be atomic:

Either:

- the full state update succeeds,

or:

- state remains unchanged.

Partial updates are forbidden.

---

### 15.5 Configuration Immutability

After initialization the following MUST NOT change:

- n
- w_i
- h_i
- κ_max
- α
- λ0..λ3
- dt_default
- optional M_max

Any change requires a new Core instance.

---

## 16. Lifecycle Model and Event Semantics

This section defines how the Core represents structure lifetime and events.

---

### 16.1 Lifecycle States

A Core instance may be in exactly one of:

- ACTIVE      (κ > 0)
- COLLAPSED   (κ = 0)

Genesis is an event that transitions COLLAPSED -> ACTIVE.

---

### 16.2 Event Codes

Each Step MUST emit exactly one event code:

- EVENT_NORMAL
- EVENT_COLLAPSE
- EVENT_GENESIS
- EVENT_ERROR

Rules:

- If κ becomes 0 during Step: EVENT_COLLAPSE
- If κ = 0 at entry and Genesis succeeds: EVENT_GENESIS
- If validation/numerics fail: EVENT_ERROR
- Otherwise: EVENT_NORMAL

---

### 16.3 cycle_id Discipline

The Core MUST maintain:

cycle_id ∈ ℕ

Rules:

- cycle_id increments only on Genesis success
- cycle_id never decreases
- cycle_id is exposed in output

---

### 16.4 Post-Collapse Behavior

If state is COLLAPSED (κ = 0):

- Step MUST NOT apply Δ impulse to the terminated structure
- Step MUST first evaluate Genesis conditions
- If Genesis fails, state remains COLLAPSED and no evolution occurs

---

## 17. Extension and Non-Scope Boundaries

This section prevents uncontrolled drift while keeping the Core usable as a kernel for multiple domains.

---

### 17.1 What Is Fixed in Version 1.2

Fixed and non-negotiable in 1.2:

- X(t) = (Δ, Φ, M, κ)
- Δ evolves only via Δ_in impulse
- Φ(Δ) is deterministic and recomputed from Δ every step
- geometry (SRI/SRD) depends only on Δ and configuration
- regime depends only on geometry (Δ)
- memory is accumulation-only (monotonic non-decreasing)
- κ is monotonic non-increasing
- collapse gate is κ = 0 only
- Genesis requires SRD(Δ_seed) > 0 and sets κ₀ = κ_max * SRD(Δ₀)

---

### 17.2 Allowed Minor Extensions (Do Not Change State)

Allowed in minor versions if theory-consistent:

- additional derived metrics in output
- additional diagnostics (e.g., L components)
- stricter input validation and error codes
- numerical stability refinements (epsilon handling) that do not change semantics
- alternative output packing methods (without altering returned values)

---

### 17.3 Changes Requiring Major Version

Major version is required for:

- adding any new fundamental coordinate to X
- allowing memory decay or reset during evolution
- allowing κ to increase
- adding additional collapse gates
- making regime depend on memory or history
- introducing stochastic evolution
- changing Genesis definition or κ₀ rule

---

## 18. Research Usage Model

This section explains how the Core is intended to be used as a research kernel (not domain logic).

---

### 18.1 Domain Neutrality

The Core does not interpret Δ component semantics.

All semantic mapping is external:

- adapters encode domain data into Δ_in
- adapters define component meaning and scaling
- the Core remains purely structural

---

### 18.2 Studying Subsystems Separately

Because outputs are layered projections of one X, it is valid to study:

- dynamics diagnostics (σ, SRD, loss L) independently
- memory trajectory M(t) independently
- viability decay κ(t) independently
- lifecycle event statistics (collapse/genesis) independently

without changing the Core.

---

### 18.3 Studying Subsystems Together

It is also valid to study coupled behavior:

- SRD vs κ decay rate
- Φ vs M growth rate
- regime distribution vs collapse timing
- Genesis frequency vs Δ stream properties

All such studies are performed via outputs and logs, not by modifying core semantics.

---

## 19. Structural Interpretation

This section fixes the minimal interpretation of what the Core computes.

---

### 19.1 Structural Process

The Core computes a structural lifecycle under impulse:

Δ_in → Δ → Φ → (SRI, SRD) → σ → M → κ → {collapse | continue} → {genesis if allowed}

---

### 19.2 What the Core Is

The Core is:

- a deterministic structural evolution kernel
- a lifecycle engine with collapse and genesis separation
- a domain-agnostic research instrument driven by Δ stream input

---

### 19.3 What the Core Is Not

The Core is not:

- a prediction system
- a trading system
- a domain simulator with semantic meaning
- an optimizer or controller

All such layers must remain external.
