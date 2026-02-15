// ==============================
// File: tests/test_collapse.cpp
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

    std::cout << "test_collapse\n";

    // Params tuned to reach collapse quickly and deterministically.
    ParameterSet p{
        1.0,    // alpha
        0.1,    // eta
        0.5,    // beta
        0.1,    // gamma
        0.05,   // rho
        0.25,   // lambda_phi
        0.25,   // lambda_m
        10.0    // kappa_max
    };

    const size_t delta_dim = 2;
    StructuralState init{0.0, 0.0, p.kappa_max};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create() must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.01;
    const double delta[2] = {1.0, 2.0};

    bool saw_collapse = false;
    uint64_t collapse_step_counter = 0;

    StructuralState before_collapse{};
    LifecycleContext lc_before_collapse{};

    StructuralState collapsed_state{};
    LifecycleContext lc_collapsed{};

    // Run until collapse
    for (int i = 0; i < 500; ++i) {
        const StructuralState cur_before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        const EventFlag ev = core.Step(delta, 2, dt);

        if (ev == EventFlag::ERROR) {
            expect_true(false, "Unexpected ERROR before collapse");
            break;
        }

        if (ev == EventFlag::COLLAPSE) {
            saw_collapse = true;
            collapse_step_counter = core.Lifecycle().step_counter;

            before_collapse = cur_before;
            lc_before_collapse = lc_before;

            collapsed_state = core.Current();
            lc_collapsed = core.Lifecycle();
            break;
        }
    }

    expect_true(saw_collapse, "Must reach COLLAPSE");
    if (!saw_collapse) {
        std::cout << "[FAIL] did not collapse in guard\n";
        return 2;
    }

    // ---- Collapse invariants
    expect_true(lc_collapsed.terminal == true, "On collapse: terminal true");
    expect_true(lc_collapsed.collapse_emitted == true, "On collapse: collapse_emitted true");
    expect_true(collapsed_state.kappa >= 0.0 && collapsed_state.kappa < 1e-15, "On collapse: kappa ~ 0");
    expect_true(collapse_step_counter == lc_before_collapse.step_counter + 1u, "On collapse: step_counter incremented exactly once");

    // ---- After terminal, Step() MUST short-circuit:
    // returns NORMAL, no mutation, no step_counter increment, collapse_emitted stays true
    {
        const EventFlag ev2 = core.Step(delta, 2, dt);
        expect_true(ev2 == EventFlag::NORMAL, "After terminal: Step returns NORMAL");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_true(lc_after.step_counter == lc_collapsed.step_counter, "After terminal: step_counter frozen");
        expect_true(lc_after.terminal == true, "After terminal: terminal stays true");
        expect_true(lc_after.collapse_emitted == true, "After terminal: collapse_emitted stays true");

        expect_near(after.phi, collapsed_state.phi, "After terminal: phi frozen");
        expect_near(after.memory, collapsed_state.memory, "After terminal: memory frozen");
        expect_near(after.kappa, collapsed_state.kappa, "After terminal: kappa frozen");
    }

    // ---- collapse_emitted must be true and remain true; no second collapse event should ever appear
    // We do a few more steps and assert no COLLAPSE is returned.
    {
        for (int j = 0; j < 10; ++j) {
            const EventFlag ev = core.Step(delta, 2, dt);
            expect_true(ev != EventFlag::COLLAPSE, "After terminal: MUST NOT emit COLLAPSE again");
            expect_true(ev != EventFlag::ERROR, "After terminal: MUST NOT emit ERROR for valid inputs");
        }
        expect_true(core.Lifecycle().collapse_emitted == true, "After terminal: collapse_emitted still true");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_collapse\n";
        return 0;
    }

    std::cout << "[FAIL] test_collapse: " << g_fail << " failures\n";
    return 2;
}
