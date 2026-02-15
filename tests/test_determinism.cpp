// ==============================
// File: tests/test_determinism.cpp
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

static void expect_near(double a, double b, const char* msg) {
    if (!almost_equal(a, b)) {
        std::cout << "[FAIL] " << msg << " (a=" << a << " b=" << b << ")\n";
        g_fail += 1;
    }
}

int main() {
    using namespace maxcore;

    std::cout << "test_determinism\n";

    ParameterSet p{
        1.0,
        0.1,
        0.5,
        0.1,
        0.2,
        0.1,
        0.1,
        10.0
    };

    const size_t delta_dim = 2;
    StructuralState init{0.0, 0.0, p.kappa_max};

    auto coreA_opt = MaxCore::Create(p, delta_dim, init);
    auto coreB_opt = MaxCore::Create(p, delta_dim, init);

    expect_true(coreA_opt.has_value(), "Create(A) must succeed");
    expect_true(coreB_opt.has_value(), "Create(B) must succeed");
    if (!coreA_opt || !coreB_opt) return 1;

    MaxCore coreA = *coreA_opt;
    MaxCore coreB = *coreB_opt;

    const double dt = 0.01;
    const int steps = 100;

    for (int i = 0; i < steps; ++i) {
        // Deterministic but varying delta sequence
        const double delta[2] = {
            1.0 + 0.01 * i,
            2.0 - 0.02 * i
        };

        const EventFlag evA = coreA.Step(delta, 2, dt);
        const EventFlag evB = coreB.Step(delta, 2, dt);

        expect_true(evA == evB, "Event flags must match");

        const StructuralState a = coreA.Current();
        const StructuralState b = coreB.Current();

        expect_near(a.phi, b.phi, "phi must match");
        expect_near(a.memory, b.memory, "memory must match");
        expect_near(a.kappa, b.kappa, "kappa must match");

        const LifecycleContext lcA = coreA.Lifecycle();
        const LifecycleContext lcB = coreB.Lifecycle();

        expect_true(lcA.step_counter == lcB.step_counter, "step_counter must match");
        expect_true(lcA.terminal == lcB.terminal, "terminal flag must match");
        expect_true(lcA.collapse_emitted == lcB.collapse_emitted, "collapse_emitted must match");

        if (evA == EventFlag::COLLAPSE) {
            // After collapse both must freeze identically
            break;
        }
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_determinism\n";
        return 0;
    }

    std::cout << "[FAIL] test_determinism: " << g_fail << " failures\n";
    return 2;
}
