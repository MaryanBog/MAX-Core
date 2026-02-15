// ==============================
// File: tests/test_norm_guard.cpp
// ==============================
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>

#include "maxcore/maxcore.h"

static int g_fail = 0;

static void expect_true(bool cond, const char* msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << "\n";
        g_fail += 1;
    }
}

static bool almost_equal(double a, double b, double rel_eps = 1e-12, double abs_eps = 1e-15) {
    const double diff = std::fabs(a - b);
    if (diff <= abs_eps) return true;
    const double scale = std::max(std::fabs(a), std::fabs(b));
    return diff <= rel_eps * scale;
}

static void expect_near(double a, double b, const char* msg, double rel_eps = 1e-12, double abs_eps = 1e-15) {
    if (!almost_equal(a, b, rel_eps, abs_eps)) {
        std::cout << "[FAIL] " << msg << " (a=" << a << " b=" << b << ")\n";
        g_fail += 1;
    }
}

static double norm2_of(const double* v, size_t n) {
    double s = 0.0;
    for (size_t i = 0; i < n; ++i) {
        s += v[i] * v[i];
    }
    return s;
}

int main() {
    using namespace maxcore;

    std::cout << "test_norm_guard\n";

    // Canonical parameters (valid and stable for dt=0.01)
    ParameterSet p{
        1.0,  // alpha
        0.1,  // eta
        0.5,  // beta
        0.1,  // gamma
        0.2,  // rho
        0.1,  // lambda_phi
        0.1,  // lambda_m
        10.0  // kappa_max
    };

    const size_t delta_dim = 2;
    StructuralState init{0.0, 0.0, p.kappa_max};

    const double dt = 0.01;

    // Raw delta (norm >> delta_max) to activate norm-guard.
    const double delta_raw[2] = {100.0, 200.0};

    const double delta_max = 5.0;

    // Core A: guarded (delta_max enabled)
    auto core_guarded_opt = MaxCore::Create(p, delta_dim, init, delta_max);
    expect_true(core_guarded_opt.has_value(), "Create(guarded) must succeed");
    if (!core_guarded_opt) return 1;
    MaxCore core_guarded = *core_guarded_opt;

    // Core B: unguarded, but we feed an explicitly scaled delta preserving direction
    auto core_scaled_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_scaled_opt.has_value(), "Create(unguarded) must succeed");
    if (!core_scaled_opt) return 1;
    MaxCore core_scaled = *core_scaled_opt;

    const double n2 = norm2_of(delta_raw, delta_dim);
    expect_true(std::isfinite(n2) != 0, "raw norm2 finite");
    expect_true(n2 > delta_max * delta_max, "Precondition: raw norm2 > delta_max^2");

    const double n = std::sqrt(n2);
    expect_true(std::isfinite(n) != 0 && n > 0.0, "sqrt(norm2) finite and > 0");

    const double scale = delta_max / n;
    expect_true(std::isfinite(scale) != 0 && scale > 0.0, "scale finite and > 0");

    const double delta_scaled[2] = { delta_raw[0] * scale, delta_raw[1] * scale };

    const EventFlag ev_a = core_guarded.Step(delta_raw, delta_dim, dt);
    const EventFlag ev_b = core_scaled.Step(delta_scaled, delta_dim, dt);

    expect_true(ev_a != EventFlag::ERROR, "guarded Step must not return ERROR");
    expect_true(ev_b != EventFlag::ERROR, "scaled Step must not return ERROR");

    // Since the canonical model uses only norm2 downstream,
    // guarded(norm-clamped) must match unguarded(explicitly-scaled).
    const StructuralState a = core_guarded.Current();
    const StructuralState b = core_scaled.Current();

    expect_near(a.phi, b.phi, "phi matches guarded vs scaled");
    expect_near(a.memory, b.memory, "memory matches guarded vs scaled");
    expect_near(a.kappa, b.kappa, "kappa matches guarded vs scaled");

    if (g_fail == 0) {
        std::cout << "[OK] test_norm_guard\n";
        return 0;
    }

    std::cout << "[FAIL] test_norm_guard: " << g_fail << " failures\n";
    return 2;
}
