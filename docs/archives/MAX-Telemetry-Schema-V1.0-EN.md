# FMRT / Flexion Core — MAX Telemetry Schema v1.0

Status: DRAFT (MAX research profile)
Scope: One-step diagnostic output of the structural core.
Core State: X = (Δ, Φ, M, κ)
All values MUST be finite IEEE-754 doubles (no NaN/Inf).

---

## 0) Common conventions

- Δ is a fixed-dimension vector: Δ ∈ R^DELTA_DIM
- Time parameter in the runtime is "step time" (dt); structural time T is derived.
- All "derived" fields MUST be deterministic functions of (X, dX/dt, config limits, and chosen operators).
- If a field is unsupported by a build profile, it MUST be set to 0 and flagged in `features_mask`.

---

## 1) Telemetry packet (single struct)

### 1.1 Header

- u32  version_major         = 1
- u32  version_minor         = 0
- u32  features_mask         // bitmask, see §2
- u32  flags                 // event + validity, see §3

- u64  tick                  // step counter
- u64  cycle_id              // genesis cycle counter
- f64  t                     // runtime time (accumulated dt)
- f64  dt                    // current step dt

- u32  delta_dim             // = DELTA_DIM
- u32  reserved_u32[3]       // = 0

---

### 1.2 Structural state X

- f64  delta[DELTA_DIM]      // Δ
- f64  phi                   // Φ
- f64  mem                   // M
- f64  kappa                 // κ

---

### 1.3 Structural flow (Field) dX/dt = F(X)

- f64  d_delta[DELTA_DIM]    // dΔ/dt
- f64  d_phi                 // dΦ/dt
- f64  d_mem                 // dM/dt
- f64  d_kappa               // dκ/dt

---

### 1.4 Structural acceleration (optional) d²X/dt²

- f64  dd_delta[DELTA_DIM]   // d²Δ/dt²
- f64  dd_phi                // d²Φ/dt²
- f64  dd_mem                // d²M/dt²
- f64  dd_kappa              // d²κ/dt²

---

### 1.5 Collapse / viability margins

Config limits (from core config, echoed here for research reproducibility):
- f64  delta_max_norm        // Δ_max (norm limit)
- f64  phi_max               // Φ_max
- f64  mem_max               // M_max

Computed margins:
- f64  delta_norm            // ||Δ||
- f64  margin_delta          // Δ_max - ||Δ||
- f64  margin_phi            // Φ_max - Φ
- f64  margin_mem            // M_max - M
- f64  margin_kappa          // κ (boundary at 0)

Aggregate:
- f64  margin_min            // min(margins)
- f64  collapse_proximity    // normalized, e.g. clamp01(1 - margin_min / margin_ref)

---

### 1.6 Collapse kinematics (Δ-channel)

- f64  v_delta               // ||dΔ/dt||
- f64  a_delta               // d/dt(||dΔ/dt||) (computed via finite diff or analytic)
- f64  jerk_delta            // d²/dt²(||dΔ/dt||) (optional; 0 if not supported)

---

### 1.7 Reversibility / regime

- f64  sri                   // Structural Reversibility Index
- f64  srd                   // Structural Reversibility Density (optional)
- i32  regime_id             // enum: 0=contractive,1=expansive,2=critical,3=dead
- i32  reserved_i32          // = 0

---

### 1.8 Field geometry (local)

Jacobian of the flow w.r.t. the scalar subspace (Φ,M,κ) always present:
- f64  J_scalar[3][3]        // ∂(dΦ,dM,dκ)/∂(Φ,M,κ)

Optional Δ-block summaries (to avoid huge matrices):
- f64  J_delta_trace         // trace of ∂(dΔ)/∂Δ
- f64  J_delta_fro_norm      // Frobenius norm summary
- f64  div_F                 // divergence proxy (trace of full J, using summaries)

Eigen summaries (optional):
- f64  J_scalar_eig_re[3]    // real parts (if complex, store real parts)
- f64  J_scalar_eig_im[3]    // imag parts (0 if real)
- f64  spectral_radius       // max |eig|

---

### 1.9 Structural space geometry (metric)

Chosen metric weights (echoed):
- f64  w_delta               // weight for Δ
- f64  w_phi
- f64  w_mem
- f64  w_kappa

Derived:
- f64  ds_dt                 // sqrt( wΔ||dΔ/dt||² + wΦ(dΦ/dt)² + wM(dM/dt)² + wκ(dκ/dt)² )
- f64  path_len_accum        // ∫ ds (accumulated by the core)
- f64  traj_curvature        // optional scalar curvature of trajectory in this metric

---

### 1.10 Structural time (FTT layer)

- f64  T                     // T = 𝒯(Δ,Φ,M,κ)
- f64  v_T                   // dT/dt
- f64  a_T                   // d²T/dt²

Gradient:
- f64  dT_dDelta_norm        // ||∂T/∂Δ|| (vector collapsed to norm)
- f64  dT_dPhi               // ∂T/∂Φ
- f64  dT_dMem               // ∂T/∂M
- f64  dT_dKappa             // ∂T/∂κ
- f64  gradT_norm            // ||∇T|| (using above components)

Curvature/Hessian summaries:
- f64  HT_trace              // trace(H_T)
- f64  HT_fro_norm           // ||H_T||_F (summary)
- f64  K_T                   // temporal curvature scalar (core-defined)

---

### 1.11 Risk / resilience indices (derived from FTT metrics)

- f64  ST                    // temporal stability function (core-defined)
- f64  TRI                   // temporal resilience index
- f64  TSI                   // temporal stress index
- f64  R_T                   // temporal risk proxy = 1/(T+eps)

---

### 1.12 Invariant checks (diagnostics)

- f64  inv_mem_monotone      // 1.0 if ΔM >= 0 else 0.0
- f64  inv_kappa_noninc      // 1.0 if Δκ <= 0 else 0.0 (if your core enforces it)
- f64  inv_finite_only       // 1.0 if all finite else 0.0
- f64  inv_reserved          // = 0

---

## 2) features_mask (u32)

bit 0  : FLOW_DXDT               (1.3)
bit 1  : ACCEL_D2XDT2            (1.4)
bit 2  : COLLAPSE_MARGINS        (1.5)
bit 3  : COLLAPSE_KINEMATICS     (1.6)
bit 4  : REVERSIBILITY_REGIME    (1.7)
bit 5  : FIELD_GEOMETRY          (1.8)
bit 6  : SPACE_METRIC            (1.9)
bit 7  : STRUCTURAL_TIME         (1.10)
bit 8  : RISK_RESILIENCE         (1.11)
bit 9  : INVARIANT_CHECKS        (1.12)

bits 10..31 reserved

---

## 3) flags (u32)

Event flags:
bit 0 : EVENT_NORMAL
bit 1 : EVENT_COLLAPSE           // κ <= 0 reached
bit 2 : EVENT_GENESIS            // cycle reset happened this step
bit 3 : EVENT_ERROR              // any runtime error / invalid input

Validity flags:
bit 8 : VALID_STATE
bit 9 : VALID_FLOW
bit 10: VALID_TIME
bit 11: VALID_GEOMETRY
bit 12: VALID_INDICES

bits 13..31 reserved

---

## 4) Notes on determinism

- All finite differences MUST use the same deterministic scheme.
- If using analytic derivatives (Jacobian/Hessian), they MUST be consistent with the exact implemented operators.
- If a component is not supported, set it to 0 and clear the corresponding feature bit.

---

