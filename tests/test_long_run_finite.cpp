// ==============================
// File: tests/test_long_run_finite.cpp
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

static void expect_finite(double x, const char* msg) {
    if (!(std::isfinite(x) != 0)) {
        std::cout << "[FAIL] " << msg << " (not finite)\n";
        g_fail += 1;
    }
}

int main() {
    using namespace maxcore;

    std::cout << "test_long_run_finite\n";

    // Stable, moderate params
    ParameterSet p{
        1.0,   // alpha
        0.1,   // eta
        0.5,   // beta
        0.1,   // gamma
        0.2,   // rho
        0.1,   // lambda_phi
        0.1,   // lambda_m
        10.0   // kappa_max
    };

    const size_t delta_dim = 2;
    StructuralState init{0.0, 0.0, p.kappa_max};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.01;
    const int steps = 5000;

    // Deterministic, bounded delta sequence (no RNG)
    for (int i = 0; i < steps; ++i) {
        const double d0 = 1.0 + 0.001 * (i % 100);
        const double d1 = 2.0 - 0.001 * (i % 100);
        const double delta[2] = {d0, d1};

        const EventFlag ev = core.Step(delta, delta_dim, dt);
        expect_true(ev != EventFlag::ERROR, "Long run: must not produce ERROR for valid inputs");

        const StructuralState s = core.Current();

        expect_finite(s.phi, "phi finite");
        expect_finite(s.memory, "memory finite");
        expect_finite(s.kappa, "kappa finite");

        // Invariants always
        expect_true(s.phi >= 0.0, "phi >= 0");
        expect_true(s.memory >= 0.0, "memory >= 0");
        expect_true(s.kappa >= 0.0, "kappa >= 0");
        expect_true(s.kappa <= p.kappa_max, "kappa <= kappa_max");

        if (ev == EventFlag::COLLAPSE) {
            // After collapse, terminal must be true and kappa ~0
            expect_true(core.Lifecycle().terminal == true, "On collapse: terminal true");
            expect_true(core.Lifecycle().collapse_emitted == true, "On collapse: collapse_emitted true");
            expect_true(s.kappa >= 0.0 && s.kappa < 1e-15, "On collapse: kappa ~0");

            // Verify freeze for a few more calls (still valid inputs)
            const StructuralState frozen = core.Current();
            const LifecycleContext lc = core.Lifecycle();
            for (int j = 0; j < 10; ++j) {
                const EventFlag ev2 = core.Step(delta, delta_dim, dt);
                expect_true(ev2 == EventFlag::NORMAL, "Post-collapse: Step returns NORMAL");
                expect_true(core.Lifecycle().step_counter == lc.step_counter, "Post-collapse: step_counter frozen");
                expect_true(core.Lifecycle().terminal == true, "Post-collapse: terminal stays true");
                expect_true(core.Lifecycle().collapse_emitted == true, "Post-collapse: collapse_emitted stays true");

                const StructuralState s2 = core.Current();
                expect_true(std::isfinite(s2.phi) != 0, "Post-collapse: phi finite");
                expect_true(std::isfinite(s2.memory) != 0, "Post-collapse: memory finite");
                expect_true(std::isfinite(s2.kappa) != 0, "Post-collapse: kappa finite");
                expect_true(s2.kappa >= 0.0 && s2.kappa < 1e-15, "Post-collapse: kappa ~0");

                // exact freeze within strict eps
                expect_true(s2.phi == frozen.phi, "Post-collapse: phi exact freeze");
                expect_true(s2.memory == frozen.memory, "Post-collapse: memory exact freeze");
                expect_true(s2.kappa == frozen.kappa, "Post-collapse: kappa exact freeze");
            }
            break;
        }
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_long_run_finite\n";
        return 0;
    }

    std::cout << "[FAIL] test_long_run_finite: " << g_fail << " failures\n";
    return 2;
}
