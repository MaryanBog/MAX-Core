// ==============================
// File: tests/test_clamp_invariants.cpp
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

int main() {
    using namespace maxcore;

    std::cout << "test_clamp_invariants\n";

    // Choose params that can drive kappa below zero without clamp, and ensure clamps are enforced.
    ParameterSet p{
        1.0,    // alpha
        0.1,    // eta
        0.5,    // beta
        0.1,    // gamma
        0.01,   // rho (small regen)
        5.0,    // lambda_phi (strong load)
        5.0,    // lambda_m (strong load)
        10.0    // kappa_max
    };

    const size_t delta_dim = 2;

    // Start with small kappa so clamp to 0 is reachable
    StructuralState init{0.0, 0.0, 0.1};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.01;
    const double delta[2] = {10.0, 10.0};

    // Run a few steps and assert invariants after each successful commit
    for (int i = 0; i < 50; ++i) {
        const EventFlag ev = core.Step(delta, delta_dim, dt);
        expect_true(ev != EventFlag::ERROR, "Step must not return ERROR for valid inputs");

        const StructuralState s = core.Current();

        // Clamp invariants
        expect_true(s.phi >= 0.0, "Invariant: phi >= 0");
        expect_true(s.memory >= 0.0, "Invariant: memory >= 0");
        expect_true(s.kappa >= 0.0, "Invariant: kappa >= 0");
        expect_true(s.kappa <= p.kappa_max, "Invariant: kappa <= kappa_max");

        // If terminal reached, kappa must be exactly 0 and remain so
        if (core.Lifecycle().terminal) {
            expect_true(s.kappa >= 0.0 && s.kappa < 1e-15, "Terminal: kappa ~ 0");
            // One more step should short-circuit and not change anything
            const StructuralState frozen = core.Current();
            const LifecycleContext lc = core.Lifecycle();

            const EventFlag ev2 = core.Step(delta, delta_dim, dt);
            expect_true(ev2 == EventFlag::NORMAL, "Terminal: Step returns NORMAL");
            expect_true(core.Lifecycle().step_counter == lc.step_counter, "Terminal: step_counter frozen");
            expect_true(core.Lifecycle().terminal == true, "Terminal: terminal stays true");
            expect_true(core.Current().kappa >= 0.0 && core.Current().kappa < 1e-15, "Terminal: kappa still ~0");

            expect_near(core.Current().phi, frozen.phi, "Terminal: phi frozen");
            expect_near(core.Current().memory, frozen.memory, "Terminal: memory frozen");
            expect_near(core.Current().kappa, frozen.kappa, "Terminal: kappa frozen");
            break;
        }
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_clamp_invariants\n";
        return 0;
    }

    std::cout << "[FAIL] test_clamp_invariants: " << g_fail << " failures\n";
    return 2;
}
