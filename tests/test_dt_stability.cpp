// ==============================
// File: tests/test_dt_stability.cpp
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

    std::cout << "test_dt_stability\n";

    // Choose max_rate = 2.0 (eta dominates).
    ParameterSet p{
        1.0,   // alpha
        2.0,   // eta (max)
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

    const double delta[2] = {1.0, 2.0};

    // max_rate = 2.0, so:
    // dt = 0.49 -> dt*max_rate = 0.98 < 1  -> OK
    // dt = 0.5  -> dt*max_rate = 1.0  !< 1 -> ERROR (no mutation)
    const double dt_ok = 0.49;
    const double dt_bad = 0.5;

    // ---- First, a valid step at dt_ok should succeed and mutate once.
    {
        const StructuralState before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        const EventFlag ev = core.Step(delta, delta_dim, dt_ok);
        expect_true(ev != EventFlag::ERROR, "dt_ok must not yield ERROR");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_true(lc_after.step_counter == lc_before.step_counter + 1u, "dt_ok: step_counter increments exactly once");
        // Must actually commit something (not required to change all coords, but should be a valid finite state).
        expect_true(std::isfinite(after.phi) != 0, "dt_ok: phi finite");
        expect_true(std::isfinite(after.memory) != 0, "dt_ok: memory finite");
        expect_true(std::isfinite(after.kappa) != 0, "dt_ok: kappa finite");
        (void)before;
    }

    // ---- Now dt_bad must yield ERROR and MUST NOT mutate anything.
    {
        const StructuralState before = core.Current();
        const StructuralState prev_before = core.Previous();
        const LifecycleContext lc_before = core.Lifecycle();

        const EventFlag ev = core.Step(delta, delta_dim, dt_bad);
        expect_true(ev == EventFlag::ERROR, "dt_bad must yield ERROR (dt*max_rate !< 1)");

        const StructuralState after = core.Current();
        const StructuralState prev_after = core.Previous();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, before.phi, "dt_bad: Current.phi unchanged");
        expect_near(after.memory, before.memory, "dt_bad: Current.memory unchanged");
        expect_near(after.kappa, before.kappa, "dt_bad: Current.kappa unchanged");

        expect_near(prev_after.phi, prev_before.phi, "dt_bad: Previous.phi unchanged");
        expect_near(prev_after.memory, prev_before.memory, "dt_bad: Previous.memory unchanged");
        expect_near(prev_after.kappa, prev_before.kappa, "dt_bad: Previous.kappa unchanged");

        expect_true(lc_after.step_counter == lc_before.step_counter, "dt_bad: step_counter unchanged");
        expect_true(lc_after.terminal == lc_before.terminal, "dt_bad: terminal unchanged");
        expect_true(lc_after.collapse_emitted == lc_before.collapse_emitted, "dt_bad: collapse_emitted unchanged");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_dt_stability\n";
        return 0;
    }

    std::cout << "[FAIL] test_dt_stability: " << g_fail << " failures\n";
    return 2;
}
