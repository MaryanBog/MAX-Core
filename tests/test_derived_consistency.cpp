// ==============================
// File: tests/test_derived_consistency.cpp
// ==============================
#include <cmath>
#include <cstdint>
#include <iostream>
#include <optional>

#include "maxcore/maxcore.h"
#include "maxcore/derived.h"

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

static void expect_finite(double x, const char* msg) {
    if (!(std::isfinite(x) != 0)) {
        std::cout << "[FAIL] " << msg << " (not finite)\n";
        g_fail += 1;
    }
}

int main() {
    using namespace maxcore;

    std::cout << "test_derived_consistency\n";

    ParameterSet p{
        1.0,    // alpha
        0.1,    // eta
        0.5,    // beta
        0.1,    // gamma
        0.2,    // rho
        0.1,    // lambda_phi
        0.1,    // lambda_m
        10.0    // kappa_max
    };

    const size_t delta_dim = 2;
    StructuralState init{0.0, 0.0, p.kappa_max};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.01;
    const double delta[2] = {1.0, 2.0};

    // Do two successful steps so we have meaningful previous/current delta.
    {
        const EventFlag ev1 = core.Step(delta, delta_dim, dt);
        expect_true(ev1 != EventFlag::ERROR, "Step#1 must not ERROR");

        const EventFlag ev2 = core.Step(delta, delta_dim, dt);
        expect_true(ev2 != EventFlag::ERROR, "Step#2 must not ERROR");
    }

    const StructuralState cur = core.Current();
    const StructuralState prev = core.Previous();
    const LifecycleContext lc = core.Lifecycle();

    auto d = ComputeDerived(cur, prev, lc, p, dt);
    expect_true(d.has_value(), "ComputeDerived must succeed");
    if (!d) return 2;

    // --- Check finiteness
    expect_finite(d->d_phi, "d_phi finite");
    expect_finite(d->d_memory, "d_memory finite");
    expect_finite(d->d_kappa, "d_kappa finite");

    expect_finite(d->phi_rate, "phi_rate finite");
    expect_finite(d->memory_rate, "memory_rate finite");
    expect_finite(d->kappa_rate, "kappa_rate finite");

    expect_finite(d->kappa_ratio, "kappa_ratio finite");
    expect_finite(d->kappa_distance, "kappa_distance finite");

    expect_finite(d->load_term, "load_term finite");
    expect_finite(d->regen_term, "regen_term finite");

    // --- Check deltas match committed snapshots
    expect_near(d->d_phi, cur.phi - prev.phi, "d_phi == cur.phi - prev.phi");
    expect_near(d->d_memory, cur.memory - prev.memory, "d_memory == cur.memory - prev.memory");
    expect_near(d->d_kappa, cur.kappa - prev.kappa, "d_kappa == cur.kappa - prev.kappa");

    // --- Check rates
    expect_near(d->phi_rate, (cur.phi - prev.phi) / dt, "phi_rate == d_phi/dt");
    expect_near(d->memory_rate, (cur.memory - prev.memory) / dt, "memory_rate == d_memory/dt");
    expect_near(d->kappa_rate, (cur.kappa - prev.kappa) / dt, "kappa_rate == d_kappa/dt");

    // --- kappa ratio and distance
    expect_near(d->kappa_ratio, std::max(0.0, std::min(1.0, cur.kappa / p.kappa_max)), "kappa_ratio == clamp(cur.kappa/kappa_max)");
    expect_near(d->kappa_distance, cur.kappa, "kappa_distance == cur.kappa");

    // --- canonical instantaneous terms (must use CURRENT snapshot)
    const double load_term = (p.lambda_phi * cur.phi) + (p.lambda_m * cur.memory);
    const double regen_term = p.rho * (p.kappa_max - cur.kappa);

    expect_near(d->load_term, load_term, "load_term matches canonical");
    expect_near(d->regen_term, regen_term, "regen_term matches canonical");

    // --- lifecycle mirrors
    expect_true(d->step_counter == lc.step_counter, "derived step_counter mirrors lifecycle");
    expect_true(d->terminal == lc.terminal, "derived terminal mirrors lifecycle");
    expect_true(d->collapse_emitted == lc.collapse_emitted, "derived collapse_emitted mirrors lifecycle");

    if (g_fail == 0) {
        std::cout << "[OK] test_derived_consistency\n";
        return 0;
    }

    std::cout << "[FAIL] test_derived_consistency: " << g_fail << " failures\n";
    return 2;
}
