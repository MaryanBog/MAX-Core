# Flexion Core — Architecture & Evolution Log V1.0

---

## 0. Purpose of This Document

This document records architectural decisions, structural interpretations,
and engineering translations of the Flexion theoretical framework into
a computational core.

It is not:
- a theory document,
- not a formal specification,
- not an implementation manual.

It is a structural engineering log bridging theory and executable design.

---

## 1. Ontological Scope

The Core models:

- A single structural state X = (Δ, Φ, M, κ, σ).
- Structural evolution under discrete Step().
- Collapse when κ = 0.
- Genesis of a new structure after collapse.
- Continuous Δ input stream.

The Core does NOT model:

- Metaphysical universe-level persistence.
- Memory transfer across collapse.
- Observation layer.
- Social or behavioral systems.

---

## 2. Structural State Definition

At any step t:

X(t) = (Δ(t), Φ(t), M(t), κ(t), σ(t))

Where:

- Δ — structural deviation vector.
- Φ — structural energy.
- M — structural memory.
- κ — viability.
- σ — regime indicator (-1 contractive, +1 expansive).

---

## 3. Input Stream Model

The Core receives a continuous stream of Δ impulses:

Δ_in(t)

These impulses are interpreted as structural shocks
added to the current Δ:

Δ ← Δ + Δ_in

---

## 4. Discrete Evolution Operator

The Core evolves structure using:

Step(X, Δ_in, dt)

The step must deterministically update:

1. Δ
2. Φ
3. M
4. σ
5. κ

All updates must respect Framework invariants.

---

## 5. Collapse Handling

Collapse condition:

κ = 0

When collapse occurs:

- The current structure X is terminated.
- No further evolution is allowed for this X.
- No restoration of κ is permitted.

---

## 6. Genesis Model

After collapse, if Δ stream continues:

A new structure X₀ is created:

Δ₀ = current Δ_in
Φ₀ = Φ(Δ₀)
M₀ = 0
κ₀ = f(Δ₀, structural geometry)

Genesis is clean:
- No memory transfer.
- No κ restoration.
- No continuation of admissible futures.

---

## 7. Action Space and Contractivity

The Core defines:

𝒰(S) — admissible actions.
𝒦(S) — contractive actions.
𝒞(S) = 𝒰 ∩ 𝒦

SRI = |𝒞|
SRD = |𝒞| / |𝒰|

These determine regime σ and influence κ.

---

## 8. Viability Principles

κ must satisfy:

- κ > 0 → structure exists.
- κ = 0 → collapse.
- κ cannot be restored within one X.
- κ₀ after Genesis must be computed,
  not assigned as constant.

---

## 9. Cyclical Structural Engine

The Core operates as:

while (Δ stream active):

    evolve X

    if κ == 0:
        archive X
        create new X₀
        continue

The universe is cyclic.
Each X is finite.

---

## 10. Open Questions

- Exact functional form of κ.
- Exact memory evolution rule.
- Exact Δ flow equation.
- Hysteresis implementation for σ.
- Metric definition for structural space.

---

## 11. Decisions Log

(Each architectural decision must be recorded here with date and reasoning.)

---

End of Document.

---

## 6. Genesis Model (Strict)

### 6.1 Ontological Constraint (Framework)
Collapse is terminal for the current structure X:
- when κ = 0, the structure no longer exists,
- no operator inside the same X may restore κ or reverse collapse,
- post-collapse computation for the same X is meaningless.

Therefore, any continuation after collapse must be modeled as a NEW structure X_new,
not as recovery of X_old.

### 6.2 Genesis Trigger (Cycle Layer)
Genesis is a cycle-layer event that creates a new structure when:
- the current structure has collapsed (κ_old = 0), AND
- the external Δ stream continues to provide non-trivial stimulus.

Trigger rule (minimal):
- If κ_old = 0 and ||Δ_in(t)|| > 0, then Genesis is admissible.

(Thresholding may be introduced later, but is NOT assumed here.)

### 6.3 Clean Genesis (No Transfer)
Genesis MUST be clean:
- no memory transfer: M_new(0) = 0
- no viability transfer: κ_new(0) computed from the new structure only
- no continuation of admissible futures across collapse
- no re-use of Φ_old, σ_old

This preserves collapse irreversibility and prevents “hidden recovery”.

### 6.4 Genesis Initialization Map
Define the Genesis operator 𝓖 that creates X_new at time τ = 0:

Given Δ_seed := Δ_in(t_genesis),

X_new(0) = 𝓖(Δ_seed) = (Δ0, Φ0, M0, κ0, σ0)

with:

- Δ0 := Δ_seed
- Φ0 := Φ(Δ0)
- M0 := 0
- σ0 := +1   (neutral default; regime will be re-evaluated at the next Step)
- κ0 := κ_gen(Δ0, Φ0, geometry(Δ0))

Note: σ0 is a bootstrap choice. It is not a “control input”.
At the first evolution step, σ is computed from SRI/SRD + hysteresis rules.

### 6.5 First Post-Genesis Step
After Genesis, normal evolution continues:

X_new(1) = Step(X_new(0), Δ_in(t_genesis+1), dt)

The system is thus cyclic at the universe level, but each X is finite.

---

## 8. Viability Function Class (κ Discipline)

### 8.1 Framework Requirements
Any κ used in the Core must satisfy:

R1. Existence: κ(X) > 0 implies X exists.
R2. Collapse: κ(X) = 0 implies collapse (terminal for that X).
R3. Non-restoration: within one X, no rule may “restore” κ after it has reached 0.
R4. Boundedness: κ must remain finite and non-negative.

### 8.2 What κ is Allowed to Depend On
Within the Core scope, κ may depend only on the current structural configuration:

κ = κ(Δ, Φ, M, σ, derived-geometry)

It may NOT depend on:
- external observers,
- future information,
- cross-cycle memory,
- any post-collapse state of the terminated X.

### 8.3 Genesis Viability κ0 Must Be Computed (Not Constant)
At Genesis:

κ0 = κ_gen(Δ0, Φ0, derived-geometry(Δ0))

Constraints for κ_gen:

G1. Non-negativity: κ0 ≥ 0
G2. Non-trivial stimulus admissibility:
    if ||Δ0|| = 0 then κ0 = 0 (no symmetry break → no structure)
G3. Contractivity necessity (minimal):
    if SRD(Δ0) = 0 then κ0 = 0 (no contractive capacity → no viable structure)
G4. Finite bound: κ0 ≤ κ_max

### 8.4 Practical Minimal Canon (V1.0 Choice Class)
We do NOT fix a single closed-form κ formula yet.
Instead we define an admissible family:

κ0 = κ_max * A(Δ0) * B(Φ0) * C(SRD0)

where:
- A(Δ0) ∈ {0,1} gates “genesis admissibility” (e.g., ||Δ0||>0)
- B(Φ0) is a decreasing load term (finite, non-negative)
- C(SRD0) is a monotone term in SRD0 with C(0)=0

Examples of admissible shapes (NOT yet selected):
- B(Φ0) = 1 / (1 + Φ0)
- C(SRD0) = SRD0
- A(Δ0) = 1[||Δ0||>0]

Selecting a specific form is an engineering decision recorded in Section 11.

---

## 11. Decisions Log (Initial Entries)

### 2026-02-12 — D1: Core Input Model
Decision:
- The Core receives Δ as an input stream Δ_in(t).
- Δ_in is treated as an impulse added to the internal Δ state.

Rationale:
- Preserves the “Δ is the state” model while allowing streaming stimuli.

Status:
- Accepted.

### 2026-02-12 — D2: Discrete Core Engine
Decision:
- The Core evolution operator is discrete Step(), not continuous ODE.

Rationale:
- Determinism, ABI simplicity, testability.

Status:
- Accepted.

### 2026-02-12 — D3: Collapse Handling
Decision:
- When κ reaches 0, the current X is terminal (no further evolution for that X).

Rationale:
- Required by Framework collapse irreversibility.

Status:
- Accepted.

### 2026-02-12 — D4: Post-Collapse Continuation via Clean Genesis
Decision:
- After collapse, the system continues only by creating a new structure X_new via Genesis.
- Genesis is clean: no memory/viability transfer across collapse.

Rationale:
- Avoids hidden recovery; preserves collapse terminality while allowing cyclic universe-level operation.

Status:
- Accepted.

### 2026-02-12 — D5: κ0 Is Computed, Not Constant
Decision:
- κ0 after Genesis must be computed from the new structure (Δ0 and derived geometry), not assigned as a fixed constant.

Rationale:
- κ is a structural viability measure; Genesis should not create identical viability for all seeds.

Status:
- Accepted.

### 2026-02-12 — D6: κ Discipline as an Admissible Class (Not Single Formula Yet)
Decision:
- In V1.0 log we fix the admissible class/constraints for κ and κ_gen, but do not lock a single closed-form formula yet.

Rationale:
- Prevents premature hard-coding; keeps alignment with theory while engineering choices are still open.

Status:
- Accepted.

---

## 7. Action Space and Contractivity (Computable Form)

This section defines how the Core computes:

- 𝒰(S) — admissible action space
- 𝒦(S) — contractive actions
- 𝒞(S) = 𝒰 ∩ 𝒦
- SRI and SRD

This layer is purely structural and must be deterministic.

---

### 7.1 Discrete Action Space 𝒰(S)

For a structure with deviation vector:

Δ ∈ ℝ^n

Define a finite, discrete action space:

𝒰(S) = { ±h_i e_i | i = 1..n }

Where:

- e_i — canonical basis vectors
- h_i > 0 — fixed structural step size per dimension
- n = dim(Δ)

Cardinality:

|𝒰(S)| = 2n

No action depends on:
- memory,
- external control,
- future state.

This guarantees determinism.

---

### 7.2 Deviation Norm

Deviation magnitude is defined via weighted L1 norm:

||Δ|| = Σ_i w_i |Δ_i|

with:

- w_i > 0
- fixed for the lifetime of a structure X

This norm defines geometric contraction.

---

### 7.3 Contractive Set 𝒦(S)

An action u ∈ 𝒰(S) is contractive if:

||Δ + u|| < ||Δ||

Formally:

𝒦(S) = { u ∈ 𝒰(S) | ||Δ + u|| < ||Δ|| }

This is purely local geometry.
No probabilistic or optimization assumptions are allowed.

---

### 7.4 Contractive Intersection 𝒞(S)

𝒞(S) = 𝒰(S) ∩ 𝒦(S)

Since 𝒦 ⊆ 𝒰 by definition:

𝒞(S) = 𝒦(S)

---

### 7.5 Structural Reversibility Index (SRI)

Since 𝒰 is finite and discrete, measure μ is counting measure.

SRI(S) = |𝒞(S)|

Properties:

- SRI ∈ {0,1,...,2n}
- SRI = 0 → no contractive direction exists

---

### 7.6 Structural Reversibility Density (SRD)

SRD(S) = |𝒞(S)| / |𝒰(S)|

Given |𝒰(S)| = 2n:

SRD(S) = |𝒞(S)| / (2n)

Properties:

- SRD ∈ [0,1]
- SRD = 0 → no contractive geometry
- SRD = 1 → all actions reduce deviation

---

### 7.7 Regime Determination σ

Initial regime rule (minimal canonical form):

If SRI(S) > 0:
    σ = -1   (contractive regime)
Else:
    σ = +1   (expansive regime)

Hysteresis may be added later using γ(M),
but is not required in V1.0.

---

### 7.8 Genesis Viability Dependency

At Genesis:

Compute:

- SRI0
- SRD0

If SRI0 = 0:
    κ0 = 0   (Genesis fails → no viable structure)

If SRI0 > 0:
    κ0 computed via κ_gen class defined in Section 8.

This guarantees:

No contractive geometry → no viable structure.

---

### 7.9 Determinism Requirement

All computations in this section must:

- use finite numbers only
- reject NaN / Inf
- produce identical results for identical Δ

No randomness allowed.

---

## 12. API — Δ Stream Input Contract (V1.0)

### 12.1 Core Principle
The Core accepts ONLY a Δ impulse stream as input.

Δ_in(t) is interpreted as an impulse added to internal deviation state:

Δ ← Δ + Δ_in

The Core does not interpret the semantic meaning of Δ components.
Semantic mapping is handled by external adapters via Delta Profiles.

---

### 12.2 Δ Data Format (ABI)
Δ input is a dense, contiguous vector of float64:

- type: double (IEEE-754)
- layout: C contiguous array
- length: dim (fixed after init)
- values: finite only (reject NaN/Inf)
- ownership: caller-owned pointer valid for the call duration

---

### 12.3 Runtime-Fixed Dimension Model
Dimension is configured at Core initialization:

- dim is provided once via FX_Config
- dim is immutable for the lifetime of the FX_Core instance
- all incoming packets MUST match dim

Rationale:
- supports multiple research domains by instantiating different Core configs
- keeps ABI stable and deterministic
- avoids compile-time rebuild for new Δ profiles

---

### 12.4 Δ Packet (Optional Metadata)
To support research logging and reproducibility, input may include metadata.
The Core may ignore metadata for computation but MUST validate dim.

```c
typedef struct {
  uint32_t dim;          // MUST == core_dim
  const double* v;       // pointer to dim doubles (contiguous)

  uint64_t tick;         // external step index (optional, for logs)
  double dt;             // optional; if <=0 -> use config default
  uint32_t tag;          // optional profile/source tag (no semantics in Core)
  uint32_t flags;        // reserved
} FX_DeltaPacket;

---

12.5 Core Configuration
typedef struct {
  uint32_t dim;          // Δ dimension (runtime-fixed)
  const double* w;       // weights for ||Δ|| (dim) ; w_i>0
  const double* h;       // action steps for 𝒰(S) (dim) ; h_i>0
  double kappa_max;      // upper bound for viability scaling
  double dt_default;     // default dt if packet dt not provided
} FX_Config;


All arrays (w, h) are treated as read-only; the Core may copy them internally.

12.6 Core Lifecycle API (C ABI)
typedef struct FX_Core FX_Core;

FX_Core* FX_Create(const FX_Config* cfg);
void     FX_Destroy(FX_Core* core);

// Create a new structure X0 from a Δ seed (Genesis)
int FX_ResetGenesis(FX_Core* core, const FX_DeltaPacket* seed);

// Perform one evolution step
int FX_Step(FX_Core* core, const FX_DeltaPacket* in);

// Read current state snapshot
int FX_GetState(const FX_Core* core,
                /*out*/ double* delta_out,  // dim
                /*out*/ double* phi_out,
                /*out*/ double* mem_out,
                /*out*/ double* kappa_out,
                /*out*/ int*    sigma_out);


Return convention (minimal):

1 = success

0 = failure (invalid input, NaN/Inf, dim mismatch, etc.)

12.7 Delta Profiles (Outside Core)

A Delta Profile defines the meaning of Δ components for a given study.
Profiles live in adapters, not in the Core.

A profile MUST specify:

profile_id (string/hash)

dim

component mapping: i -> meaning

scaling/normalization rules (if any)

The Core remains domain-agnostic.

11. Decisions Log (Append)
2026-02-12 — D7: Δ Input Shape and Dimension Strategy

Decision:

Δ_in is a dense float64 vector (double*), contiguous.

Δ dimension is runtime-fixed: provided at init, immutable per Core instance.

Rationale:

supports multiple research domains via different Delta Profiles and configs

keeps ABI stable, deterministic, and testable

Status:

Accepted.

---

## 13. Output Architecture — Layered Structural Views (V1.0)

### 13.1 Core Output Philosophy

The Core maintains a single internal structural state:

X(t) = (Δ, Φ, M, κ, σ)

All outputs are projections or diagnostics derived from this state.

The Core does NOT expose multiple independent subsystems.
Instead, it exposes layered views over the same structural evolution.

This guarantees:

- internal consistency,
- determinism,
- no duplicated logic across modules.

---

## 13.2 Output Layers

The Core supports layered output retrieval via a bitmask mechanism.

### Layer A — Structural State (Raw)

Minimal structural snapshot:

- Δ (vector)
- Φ
- M
- κ
- σ
- tick
- dt
- lifecycle flags

This is always the primary output.

---

### Layer B — Derived Structural Observables

Derived from X(t):

- ||Δ|| (weighted L1 norm)
- SRI
- SRD
- Δ_norm change (per step)
- ΔΦ, ΔM, Δκ
- structural event (normal / collapse / genesis)

These values are computed deterministically from the current and previous state.

---

### Layer C — Dynamics View

Represents structural evolution operator diagnostics.

Contains:

- effective step contribution norms
- F(Δ) contribution magnitude
- σ regime indicator
- step classification (contractive / expansive)

Field approximation:

F_estimate ≈ (X(t+1) − X(t)) / dt

No additional theoretical assumptions are introduced.

---

### Layer D — Field View

Field View exposes the discrete structural flow:

- Δ_field = (Δ(t+1) − Δ(t)) / dt
- Φ_field = (Φ(t+1) − Φ(t)) / dt
- M_field = (M(t+1) − M(t)) / dt
- κ_field = (κ(t+1) − κ(t)) / dt

This is a computational approximation of the continuous operator.

---

### Layer E — Space View (Future Extension)

Reserved for:

- structural metric
- curvature
- geometric invariants

Not implemented in V1.0.
Requires formal metric definition.

---

### Layer F — Time View (Future Extension)

Provides:

- internal temporal rate indicators
- M growth rate
- κ decay rate
- regime persistence

External tick remains authoritative time index.

---

## 13.3 Output Mask Mechanism

Output layers are requested using a bitmask:

```c
typedef uint32_t FX_OutMask;

enum {
  FX_OUT_STATE   = 1u << 0,
  FX_OUT_DERIVED = 1u << 1,
  FX_OUT_DYN     = 1u << 2,
  FX_OUT_FIELD   = 1u << 3,
  FX_OUT_SPACE   = 1u << 4,
  FX_OUT_TIME    = 1u << 5
};

13.4 Unified Output Structure
typedef struct {

  // ---- Base State ----
  uint64_t tick;
  double dt;
  uint32_t flags;     // lifecycle flags
  uint32_t dim;

  double* delta;      // caller allocated [dim]
  double phi;
  double mem;
  double kappa;
  int sigma;

  // ---- Derived ----
  double delta_norm;
  int sri;
  double srd;

  double dphi;
  double dmem;
  double dkappa;
  double ddelta_norm;

  uint32_t event;     // 0=normal,1=collapse,2=genesis

  // ---- Dynamics View ----
  double F_delta_norm;
  double F_phi;
  double F_mem;
  double F_kappa;

  // ---- Field View ----
  double delta_field_norm;

  // ---- Reserved for future ----
  double reserved_space;
  double reserved_time;

} FX_Output;

13.5 Lifecycle Flags
enum {
  FX_FLAG_NONE      = 0,
  FX_FLAG_COLLAPSED = 1u << 0,
  FX_FLAG_GENESIS   = 1u << 1,
  FX_FLAG_RESET     = 1u << 2
};


Rules:

If κ becomes 0 during Step → COLLAPSED flag set.

If Genesis is triggered → GENESIS flag set.

No structure continues after collapse without Genesis.

13.6 Core Execution Model
int FX_StepEx(FX_Core* core,
              const FX_DeltaPacket* in,
              FX_OutMask mask,
              FX_Output* out);


The mask determines which layers are computed.

All computations must remain deterministic.

11. Decisions Log (Append)
2026-02-12 — D8: Layered Output Architecture

Decision:

Output is layered via bitmask system.

Single unified FX_Output structure.

All subsystems (Dynamics, Field, Space, Time) are projections of one X.

Rationale:

Ensures consistency.

Allows independent study of subsystems.

Prevents divergence between theory layers.

Status:

Accepted.

---

## 14. Canonical Step Execution Order (V1.0)

This section defines the strict deterministic order of computations
performed during a single Step.

No deviation from this order is allowed.

---

## 14.1 Preconditions

Before Step execution:

- Core must be initialized.
- Structure X must exist (κ > 0), OR
- Genesis must be explicitly triggered if κ = 0.

Input validation:

- Δ_in.dim == core.dim
- All Δ_in values must be finite
- dt > 0 (if provided), else use default dt

If validation fails:
- Step returns failure
- State remains unchanged

---

## 14.2 Step Pipeline

Given:

X(t) = (Δ, Φ, M, κ, σ)

and input Δ_in(t)

The Step execution order is:

---

### (1) Lifecycle Check

If κ == 0:

    If ||Δ_in|| == 0:
        Return (structure remains terminated)
    Else:
        Perform Genesis (see Section 6)
        Set event = GENESIS
        Continue with new X₀

---

### (2) Apply External Stimulus

Δ ← Δ + Δ_in

No normalization.
No scaling.
Pure structural impulse addition.

---

### (3) Compute Structural Norm

Δ_norm ← ||Δ|| (weighted L1)

This defines current deformation magnitude.

---

### (4) Compute Contractive Geometry

Using Section 7 definitions:

- Build 𝒰(S)
- Evaluate 𝒦(S)
- Compute 𝒞(S)
- Compute SRI
- Compute SRD

---

### (5) Determine Regime σ

Minimal rule (V1.0):

If SRI > 0:
    σ = -1   (contractive)
Else:
    σ = +1   (expansive)

(Hysteresis may be added later.)

---

### (6) Update Memory M

M ← M + dt * g(Δ, σ)

In V1.0 minimal form:

g(Δ, σ) must satisfy:
- Non-negative memory accumulation
- Monotonic growth under expansive regime

Exact form to be specified separately.

---

### (7) Update Structural Energy Φ

Φ ← Φ(Δ)

Φ is recomputed from current Δ.
No incremental update is allowed in V1.0.

---

### (8) Update Viability κ

κ ← κ_update(Δ, Φ, M, SRD)

Rules:

- κ must remain ≥ 0
- κ cannot increase beyond κ_max
- If κ becomes 0 → collapse event

If κ == 0:
    Set event = COLLAPSE

---

### (9) Compute Derived Quantities

If requested by mask:

- Δ_norm change
- dΦ
- dM
- dκ
- field approximations
- diagnostics

---

### (10) Finalize Step

Increment tick.
Return success.

---

## 14.3 Invariants

The following must hold after each Step:

1. All state variables finite.
2. κ ≥ 0.
3. If κ == 0 → no further evolution without Genesis.
4. Δ dimension remains constant.
5. Deterministic result for identical inputs.

---

## 11. Decisions Log (Append)

### 2026-02-12 — D9: Canonical Step Order

Decision:
- Fixed deterministic Step execution pipeline.
- κ check first.
- Stimulus applied before geometry computation.
- Φ recomputed from Δ.
- κ updated last.

Rationale:
- Prevents ambiguous evaluation order.
- Guarantees reproducibility.
- Aligns collapse irreversibility with Framework.

Status:
- Accepted.

---

## 15. Canonical Memory Law g(Δ, σ) (V1.0)

We fix g(Δ,σ) as a regime-weighted energy law based on structural energy Φ(Δ).

Structural energy:
Φ(Δ) = Σ_i w_i |Δ_i|

Memory differential form:
dM/dt = g(Δ(t), σ(t))

Canonical choice (regime-weighted):
g(Δ,σ) =
  { +α · Φ(Δ),   if σ = +1   (expansive / stress accumulation)
  { -β · Φ(Δ),   if σ = -1   (contractive / stabilization dissipation)

Discrete update (Step):
M_next_raw = M + dt · g(Δ,σ)
M_next     = clamp(M_next_raw, 0, M_max)

Constraints:
- α ≥ 0, β ≥ 0
- M is always finite
- M ≥ 0 (by clamp)
- Under σ=+1 memory increases with stress magnitude (large Φ)
- Under σ=-1 memory dissipates proportionally to Φ (partial recovery of bias)

Notes:
- This matches the documented intent: memory increases under stress (σ=+1) and decreases under stabilization (σ=-1). 
- Nonlinearity can be introduced later; V1.0 stays linear in Φ.

References:
- Flexion Dynamics School V2.0, A.7 Memory Accumulation (common choices and regime-weighted form).

---

## 16. Canonical Viability Update κ_update(...) (V1.0)

Framework constraint:
κ is a monotonically constrained resource.
No operator at Framework level restores or increases κ once lost.

Therefore κ must be non-increasing for all Steps.

We define κ evolution via a nonnegative loss rate L ≥ 0:

Continuous:
dκ/dt = - L(Δ, Φ, M, SRD, σ, κ)

Discrete (Step):
κ_next_raw = κ - dt · L(Δ, Φ, M, SRD, σ, κ)
κ_next     = max(0, κ_next_raw)

Collapse condition:
If κ_next == 0 -> event = COLLAPSE

Canonical loss function (V1.0):

Let:
stress = Φ(Δ)
irrevers = (1 - SRD)          // 0 (fully reversible) ... 1 (no reversible density)
mem_term = M / (1 + M)        // saturating [0..1)

Define:
L = stress · ( λ0 + λ1·I[σ=+1] + λ2·irrevers + λ3·mem_term )

Where:
- λ0 ≥ 0 : baseline viability burn (existence cost)
- λ1 ≥ 0 : extra burn in expansive regime
- λ2 ≥ 0 : irreversibility penalty (near point-of-no-return)
- λ3 ≥ 0 : memory penalty (history makes structure harder to sustain)

Properties:
- L ≥ 0 always  -> κ never increases (monotone)
- Larger Φ burns κ faster (stress consumes viability)
- Lower SRD (less reversibility) accelerates κ loss
- Larger M accelerates κ loss (path dependence)
- σ=+1 accelerates burn compared to σ=-1

Geometric consistency note:
Collapse can also be interpreted as exiting a viability domain D in Δ-space;
in V1.0 we enforce collapse strictly by κ→0 (single collapse gate).

References:
- Flexion Framework V3.1, 6.2 Monotonic Constraint (κ cannot be restored/increased).
- Flexion Dynamics School V2.0, A.10 Collapse Boundary Geometry (viability domain concept).
