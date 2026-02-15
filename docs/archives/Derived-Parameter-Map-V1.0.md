# Derived Parameter Map V1.0
## MAX-Core V2.3 -> Fundamental Parameter Projection Catalog

Purpose:
Define a deterministic Derived Layer that projects the MAX-Core runtime outputs
into the full set of fundamental parameters used across the Flexion Universe.
The MAX-Core itself remains minimal (X + lifecycle + event).
All additional parameters are computed as a pure function of an immutable Snapshot.

---

## 0. Inputs and Determinism Contract

### 0.1 Snapshot (immutable input to Derived Layer)

Snapshot S MUST include:

- Core state (committed):
  - Phi
  - Memory
  - Kappa
- Previous core state (previous committed):
  - Phi_prev
  - Memory_prev
  - Kappa_prev
- Step context:
  - dt
  - delta_input (optional)
  - delta_norm2 = ||Delta_step||^2 (recommended to pass from core to avoid recompute differences)
- Parameters:
  - alpha, eta, beta, gamma, rho, lambda_phi, lambda_m, kappa_max
- Lifecycle:
  - step_counter
  - terminal
  - collapse_emitted
- Event:
  - EventFlag (NORMAL / COLLAPSE / ERROR)

Determinism rule:
DerivedFrame D = G(S) MUST be a pure function:
- no mutation
- no hidden state
- no randomness
- no time-based values

---

## 1. Output Frame Definition

DerivedFrame MUST be versioned:

- derived_version = "DPM-V1.0"

DerivedFrame SHOULD be partitioned into groups:

A) Primary canonical values (echoed)
B) Rates and fluxes
C) Normalized invariants and margins
D) Load decomposition
E) Regime classification (derived, deterministic)
F) Equilibrium estimators and stability diagnostics
G) Collapse geometry proxies
H) Time / Space / Field / Observer projections (where computable)
I) Intelligence projections (where computable)

Each parameter entry below declares:
- ID
- Name / Symbol
- Type
- DependsOn (subset of Snapshot S)
- Definition / Compute rule
- Status:
  - COMPUTABLE (from S only)
  - NEEDS_EXTERNAL_MODEL (requires additional state or theory mapping beyond S)

---

## 2. Parameter Catalog

### 2.A Primary Canonical Values (Echo)

DPM.A.001 Phi
- Type: double
- DependsOn: Phi
- Compute: Phi
- Status: COMPUTABLE

DPM.A.002 Memory
- Type: double
- DependsOn: Memory
- Compute: Memory
- Status: COMPUTABLE

DPM.A.003 Kappa
- Type: double
- DependsOn: Kappa
- Compute: Kappa
- Status: COMPUTABLE

DPM.A.004 dt
- Type: double
- DependsOn: dt
- Compute: dt
- Status: COMPUTABLE

DPM.A.005 delta_norm2
- Type: double
- DependsOn: delta_norm2 (or delta_input)
- Compute: delta_norm2 = sum(delta_i^2)
- Status: COMPUTABLE

DPM.A.006 step_counter
- Type: uint64
- DependsOn: step_counter
- Compute: step_counter
- Status: COMPUTABLE

DPM.A.007 terminal
- Type: bool
- DependsOn: terminal (or Kappa)
- Compute: terminal (or Kappa == 0)
- Status: COMPUTABLE

DPM.A.008 collapse_emitted
- Type: bool
- DependsOn: collapse_emitted
- Compute: collapse_emitted
- Status: COMPUTABLE

DPM.A.009 event_flag
- Type: enum
- DependsOn: EventFlag
- Compute: EventFlag
- Status: COMPUTABLE

---

### 2.B Rates and Fluxes (Discrete Derivatives)

Note:
All rates are defined only when dt > 0 and EventFlag != ERROR and not terminal short-circuit.

DPM.B.001 dPhi_dt
- Type: double
- DependsOn: Phi, Phi_prev, dt
- Compute: (Phi - Phi_prev) / dt
- Status: COMPUTABLE

DPM.B.002 dMemory_dt
- Type: double
- DependsOn: Memory, Memory_prev, dt
- Compute: (Memory - Memory_prev) / dt
- Status: COMPUTABLE

DPM.B.003 dKappa_dt
- Type: double
- DependsOn: Kappa, Kappa_prev, dt
- Compute: (Kappa - Kappa_prev) / dt
- Status: COMPUTABLE

DPM.B.004 energy_injection_rate
- Type: double
- DependsOn: alpha, delta_norm2
- Compute: alpha * delta_norm2
- Status: COMPUTABLE

DPM.B.005 energy_dissipation_rate
- Type: double
- DependsOn: eta, Phi_prev
- Compute: eta * Phi_prev
- Status: COMPUTABLE

DPM.B.006 memory_formation_rate
- Type: double
- DependsOn: beta, Phi
- Compute: beta * Phi
- Status: COMPUTABLE

DPM.B.007 memory_decay_rate
- Type: double
- DependsOn: gamma, Memory_prev
- Compute: gamma * Memory_prev
- Status: COMPUTABLE

DPM.B.008 kappa_regen_rate
- Type: double
- DependsOn: rho, kappa_max, Kappa_prev
- Compute: rho * (kappa_max - Kappa_prev)
- Status: COMPUTABLE

DPM.B.009 kappa_load_rate_phi
- Type: double
- DependsOn: lambda_phi, Phi
- Compute: lambda_phi * Phi
- Status: COMPUTABLE

DPM.B.010 kappa_load_rate_memory
- Type: double
- DependsOn: lambda_m, Memory
- Compute: lambda_m * Memory
- Status: COMPUTABLE

DPM.B.011 kappa_net_rate_model
- Type: double
- DependsOn: rho, kappa_max, Kappa_prev, lambda_phi, Phi, lambda_m, Memory
- Compute: rho*(kappa_max - Kappa_prev) - lambda_phi*Phi - lambda_m*Memory
- Status: COMPUTABLE

---

### 2.C Normalized Invariants and Margins (Dimensionless Health)

DPM.C.001 kappa_margin
- Type: double
- DependsOn: Kappa, kappa_max
- Compute: Kappa / kappa_max
- Status: COMPUTABLE

DPM.C.002 collapse_distance_kappa
- Type: double
- DependsOn: Kappa
- Compute: Kappa
- Status: COMPUTABLE

DPM.C.003 collapse_distance_norm
- Type: double
- DependsOn: Kappa, kappa_max
- Compute: Kappa / kappa_max
- Status: COMPUTABLE

DPM.C.004 phi_norm_by_kappa
- Type: double
- DependsOn: Phi, Kappa
- Compute: Phi / max(Kappa, eps)
- Status: COMPUTABLE

DPM.C.005 memory_norm_by_kappa
- Type: double
- DependsOn: Memory, Kappa
- Compute: Memory / max(Kappa, eps)
- Status: COMPUTABLE

DPM.C.006 load_total
- Type: double
- DependsOn: lambda_phi, Phi, lambda_m, Memory
- Compute: lambda_phi*Phi + lambda_m*Memory
- Status: COMPUTABLE

DPM.C.007 regen_minus_load
- Type: double
- DependsOn: rho, kappa_max, Kappa_prev, lambda_phi, Phi, lambda_m, Memory
- Compute: rho*(kappa_max - Kappa_prev) - (lambda_phi*Phi + lambda_m*Memory)
- Status: COMPUTABLE

DPM.C.008 stress_ratio
- Type: double
- DependsOn: load_total, rho, kappa_max, Kappa_prev
- Compute: load_total / max(rho*(kappa_max - Kappa_prev), eps)
- Status: COMPUTABLE

DPM.C.009 terminal_flag_consistency
- Type: bool
- DependsOn: terminal, Kappa
- Compute: terminal == (Kappa == 0)
- Status: COMPUTABLE

---

### 2.D Regime Classification (Deterministic, Derived)

Regime is a derived label (NOT a core state).
Regime rules MUST be deterministic and documented.

Suggested minimal set (can extend later):
- REST
- LIVING
- OVERLOAD
- COLLAPSED
- ERROR

DPM.D.001 regime_id
- Type: enum {REST, LIVING, OVERLOAD, COLLAPSED, ERROR}
- DependsOn: EventFlag, Kappa, Phi, Memory, delta_norm2
- Compute:
  - if EventFlag == ERROR -> ERROR
  - else if Kappa == 0 -> COLLAPSED
  - else if delta_norm2 == 0 and Phi == 0 and Memory == 0 -> REST
  - else:
      - if kappa_net_rate_model >= 0 -> LIVING
      - else -> OVERLOAD
- Status: COMPUTABLE

DPM.D.002 regime_confidence
- Type: double in [0,1]
- DependsOn: |kappa_net_rate_model|, load_total, kappa_regen_rate
- Compute: clamp01( |kappa_net_rate_model| / (|kappa_net_rate_model| + load_total + kappa_regen_rate + eps) )
- Status: COMPUTABLE

---

### 2.E Equilibrium Estimators (Analytic Targets)

These are informative, useful for tests and monitoring.

Let U = delta_norm2.

DPM.E.001 phi_star
- Type: double
- DependsOn: alpha, eta, delta_norm2
- Compute: (alpha/eta) * delta_norm2
- Status: COMPUTABLE

DPM.E.002 memory_star
- Type: double
- DependsOn: beta, gamma, alpha, eta, delta_norm2
- Compute: (beta/gamma) * (alpha/eta) * delta_norm2
- Status: COMPUTABLE

DPM.E.003 kappa_star
- Type: double
- DependsOn: kappa_max, rho, alpha, eta, delta_norm2, lambda_phi, lambda_m, beta, gamma
- Compute:
  kappa_max - (1/rho)*(alpha/eta)*delta_norm2*(lambda_phi + lambda_m*(beta/gamma))
- Status: COMPUTABLE

DPM.E.004 u_critical
- Type: double
- DependsOn: kappa_max, rho, eta, alpha, lambda_phi, lambda_m, beta, gamma
- Compute:
  (kappa_max * rho * eta) / (alpha * (lambda_phi + lambda_m*(beta/gamma)))
- Status: COMPUTABLE

DPM.E.005 living_condition
- Type: bool
- DependsOn: delta_norm2, u_critical
- Compute: delta_norm2 < u_critical
- Status: COMPUTABLE

DPM.E.006 kappa_equilibrium_error
- Type: double
- DependsOn: Kappa, kappa_star
- Compute: Kappa - kappa_star
- Status: COMPUTABLE

DPM.E.007 phi_equilibrium_error
- Type: double
- DependsOn: Phi, phi_star
- Compute: Phi - phi_star
- Status: COMPUTABLE

DPM.E.008 memory_equilibrium_error
- Type: double
- DependsOn: Memory, memory_star
- Compute: Memory - memory_star
- Status: COMPUTABLE

---

### 2.F Collapse Geometry Proxies (From Collapse Theory, but computable here)

These are proxies that do NOT require full manifold geometry,
yet are useful for collapse diagnostics.

DPM.F.001 collapse_channel_phi
- Type: double
- DependsOn: Phi, Kappa
- Compute: Phi / max(Kappa, eps)
- Status: COMPUTABLE

DPM.F.002 collapse_channel_memory
- Type: double
- DependsOn: Memory, Kappa
- Compute: Memory / max(Kappa, eps)
- Status: COMPUTABLE

DPM.F.003 collapse_hazard_simple
- Type: double in [0, +inf)
- DependsOn: load_total, Kappa
- Compute: load_total / max(Kappa, eps)
- Status: COMPUTABLE

DPM.F.004 collapse_imminence
- Type: double in [0,1]
- DependsOn: Kappa, kappa_max
- Compute: clamp01(1 - (Kappa / kappa_max))
- Status: COMPUTABLE

DPM.F.005 collapse_event_step
- Type: uint64 or null
- DependsOn: EventFlag, step_counter
- Compute: if EventFlag == COLLAPSE then step_counter else null
- Status: COMPUTABLE

---

### 2.G Time Theory Projections (What is computable from MAX-Core)

Important:
Full Flexion Time Theory may define additional operators.
Here we include only projections that are definable from (Phi, Memory, Kappa) and their rates.

DPM.G.001 structural_time
- Type: uint64
- DependsOn: step_counter
- Compute: step_counter
- Status: COMPUTABLE

DPM.G.002 memory_time_rate
- Type: double
- DependsOn: dMemory_dt
- Compute: dMemory_dt
- Status: COMPUTABLE

DPM.G.003 time_dilation_proxy
- Type: double
- DependsOn: dMemory_dt, memory_star
- Compute: dMemory_dt / max(memory_star/dt, eps)
- Status: COMPUTABLE

DPM.G.004 time_freeze_flag
- Type: bool
- DependsOn: Kappa
- Compute: (Kappa == 0)
- Status: COMPUTABLE

DPM.G.005 irreversible_time_flag
- Type: bool
- DependsOn: collapse_emitted
- Compute: collapse_emitted
- Status: COMPUTABLE

Parameters below exist in Time Theory but need additional constructs:
- advanced temporal metric tensors
- causal cones
- projection horizons
Status: NEEDS_EXTERNAL_MODEL

---

### 2.H Space Theory / Field Theory Projections (Minimal computable subset)

Space/Field theories may define rich geometry and field operators.
From MAX-Core alone, only proxies can be derived unless a mapping is specified.

DPM.H.001 viability_scalar_field_value
- Type: double
- DependsOn: Kappa
- Compute: Kappa
- Status: COMPUTABLE

DPM.H.002 energy_scalar_field_value
- Type: double
- DependsOn: Phi
- Compute: Phi
- Status: COMPUTABLE

DPM.H.003 memory_scalar_field_value
- Type: double
- DependsOn: Memory
- Compute: Memory
- Status: COMPUTABLE

DPM.H.004 curvature_proxy
- Type: double
- DependsOn: Kappa, kappa_max
- Compute: 1 - (Kappa / kappa_max)
- Status: COMPUTABLE

DPM.H.005 space_metric_g
- Type: object (matrix) or null
- DependsOn: (requires explicit mapping g(X))
- Compute: defined by Space Theory mapping g(X)
- Status: NEEDS_EXTERNAL_MODEL

DPM.H.006 field_tensor_F
- Type: object or null
- DependsOn: (requires explicit mapping F(X))
- Compute: defined by Field Theory mapping F(X)
- Status: NEEDS_EXTERNAL_MODEL

---

### 2.I Observer Theory Projections (Cost and Observation Effects)

Observer theory introduces observation as resource cost and structural interaction.
MAX-Core does not model observer influence directly (by design).
We include only what is computable from snapshots.

DPM.I.001 observation_budget_proxy
- Type: double
- DependsOn: Kappa
- Compute: Kappa (available viability as proxy budget)
- Status: COMPUTABLE

DPM.I.002 observation_cost_proxy
- Type: double
- DependsOn: dKappa_dt
- Compute: max(0, -dKappa_dt)
- Status: COMPUTABLE

Observer coupling operators, measurement operators, and entropic observation costs:
- require explicit observer input stream and coupling model
Status: NEEDS_EXTERNAL_MODEL

---

### 2.J Entanglement Theory Projections (Require multi-structure state)

Entanglement parameters require at least two structures or a coupling state.
MAX-Core V2.3 is single-structure.

All entanglement metrics (examples):
- entanglement_strength
- coupling_phase
- shared_memory_overlap
- mutual viability transfer
Status: NEEDS_EXTERNAL_MODEL

---

### 2.K Intelligence Theory Projections (Require policy / adaptation layer)

Intelligence projections require additional state (policy, goals, control, learning).
MAX-Core is not an intelligence engine.

We include only minimal proxies:

DPM.K.001 adaptation_capacity_proxy
- Type: double
- DependsOn: Kappa, kappa_max
- Compute: Kappa / kappa_max
- Status: COMPUTABLE

All higher intelligence operators:
- require external control surface / agent layer
Status: NEEDS_EXTERNAL_MODEL

---

## 3. Implementation Notes (Derived Layer)

### 3.1 Precision and eps
Define eps as a constant (e.g. 1e-12) to avoid division by zero.
eps MUST be fixed and versioned with DerivedFrame.

### 3.2 Output determinism
- Use fixed formatting for serialization (locale-independent).
- Avoid unordered iteration when producing JSON/CSV.
- If a parameter is NEEDS_EXTERNAL_MODEL, output null plus a reason string.

### 3.3 Versioning and Evolution
When adding parameters:
- bump derived_version (e.g. DPM-V1.1)
- never change meaning of existing IDs
- only extend

---

## 4. Immediate Action Items (to make this truly "ALL parameters")

To make "ALL parameters" literal, we must next:
1) Extract the canonical list of fundamental symbols/fields from:
   - Flexion Framework V3.1
   - Collapse V1.0
   - Time Theory V1.1
   - Space Theory V1.0
   - Field Theory V1.0
   - Observer Theory V1.1
   - Entanglement Theory V1.0
   - Intelligence Theory V3.1
2) For each symbol/field, decide:
   - can it be computed from Snapshot S?
   - or does it require an External Model Adapter?
3) For NEEDS_EXTERNAL_MODEL items, define the adapter interface:
   - inputs it needs
   - outputs it returns
   - determinism constraints

If you say "go", the next deliverable is:
Derived Parameter Map V1.1 = expanded catalog with the explicit symbol list from each fundamental document and adapter specs for every missing block.
