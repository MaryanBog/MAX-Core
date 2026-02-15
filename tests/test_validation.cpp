// ==============================
// File: tests/test_validation.cpp
// ==============================
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
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

static maxcore::MaxCore make_core() {
    using namespace maxcore;

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
    StructuralState init{0.0, 0.0, 10.0};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    if (!core_opt) {
        std::cout << "[FAIL] internal: Create(valid) failed\n";
        g_fail += 1;
        // hard stop path; return dummy to satisfy compiler
        return MaxCore::Create(p, delta_dim, init).value();
    }
    return *core_opt;
}

int main() {
    using namespace maxcore;

    std::cout << "test_validation\n";

    // -------------------------
    // Case A: dt == 0 -> ERROR and no mutation
    // -------------------------
    {
        MaxCore core = make_core();
        const StructuralState before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        const double delta[2] = {1.0, 2.0};
        const EventFlag ev = core.Step(delta, 2, 0.0);

        expect_true(ev == EventFlag::ERROR, "dt==0 must yield ERROR");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, before.phi, "dt==0: Current.phi unchanged");
        expect_near(after.memory, before.memory, "dt==0: Current.memory unchanged");
        expect_near(after.kappa, before.kappa, "dt==0: Current.kappa unchanged");
        expect_true(lc_after.step_counter == lc_before.step_counter, "dt==0: step_counter unchanged");
        expect_true(lc_after.terminal == lc_before.terminal, "dt==0: terminal unchanged");
        expect_true(lc_after.collapse_emitted == lc_before.collapse_emitted, "dt==0: collapse_emitted unchanged");
    }

    // -------------------------
    // Case B: dt is NaN -> ERROR and no mutation
    // -------------------------
    {
        MaxCore core = make_core();
        const StructuralState before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        const double delta[2] = {1.0, 2.0};
        const double dt_nan = std::numeric_limits<double>::quiet_NaN();

        const EventFlag ev = core.Step(delta, 2, dt_nan);
        expect_true(ev == EventFlag::ERROR, "dt=NaN must yield ERROR");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, before.phi, "dt=NaN: Current.phi unchanged");
        expect_near(after.memory, before.memory, "dt=NaN: Current.memory unchanged");
        expect_near(after.kappa, before.kappa, "dt=NaN: Current.kappa unchanged");
        expect_true(lc_after.step_counter == lc_before.step_counter, "dt=NaN: step_counter unchanged");
    }

    // -------------------------
    // Case C: delta_len mismatch -> ERROR and no mutation
    // -------------------------
    {
        MaxCore core = make_core();
        const StructuralState before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        const double delta[2] = {1.0, 2.0};
        const double dt = 0.01;

        const EventFlag ev = core.Step(delta, 1, dt);
        expect_true(ev == EventFlag::ERROR, "delta_len mismatch must yield ERROR");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, before.phi, "delta_len mismatch: Current.phi unchanged");
        expect_near(after.memory, before.memory, "delta_len mismatch: Current.memory unchanged");
        expect_near(after.kappa, before.kappa, "delta_len mismatch: Current.kappa unchanged");
        expect_true(lc_after.step_counter == lc_before.step_counter, "delta_len mismatch: step_counter unchanged");
    }

    // -------------------------
    // Case D: delta contains NaN -> ERROR and no mutation
    // -------------------------
    {
        MaxCore core = make_core();
        const StructuralState before = core.Current();
        const LifecycleContext lc_before = core.Lifecycle();

        double delta[2] = {1.0, std::numeric_limits<double>::quiet_NaN()};
        const double dt = 0.01;

        const EventFlag ev = core.Step(delta, 2, dt);
        expect_true(ev == EventFlag::ERROR, "delta contains NaN must yield ERROR");

        const StructuralState after = core.Current();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, before.phi, "delta NaN: Current.phi unchanged");
        expect_near(after.memory, before.memory, "delta NaN: Current.memory unchanged");
        expect_near(after.kappa, before.kappa, "delta NaN: Current.kappa unchanged");
        expect_true(lc_after.step_counter == lc_before.step_counter, "delta NaN: step_counter unchanged");
    }

    // -------------------------
    // Case E: terminal short-circuit behavior (SPEC V2.5)
    //
    // After COLLAPSE occurs (kappa becomes 0), lifecycle is terminal.
    // In terminal, Step() MUST:
    //   - perform no mutation
    //   - not increment step_counter
    //   - keep LifecycleContext unchanged
    //   - return NORMAL (short-circuit)
    // -------------------------
    {
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
        StructuralState init{0.0, 0.0, 10.0};
        auto core_opt = MaxCore::Create(p, delta_dim, init);
        expect_true(core_opt.has_value(), "Create(collapse-reachable) must succeed");
        if (!core_opt) return 1;

        MaxCore core = *core_opt;

        const double dt = 0.01;
        const double delta[2] = {1.0, 2.0};

        EventFlag ev = EventFlag::NORMAL;
        int guard = 0;
        while (ev != EventFlag::COLLAPSE && ev != EventFlag::ERROR && guard < 500) {
            ev = core.Step(delta, 2, dt);
            guard += 1;
        }

        expect_true(ev == EventFlag::COLLAPSE, "Must reach COLLAPSE in guard steps");
        expect_true(core.Lifecycle().terminal == true, "After COLLAPSE: terminal true");
        expect_true(core.Current().kappa >= 0.0 && core.Current().kappa < 1e-15, "After COLLAPSE: kappa is ~0");
        expect_true(core.Lifecycle().collapse_emitted == true, "After COLLAPSE: collapse_emitted true");

        const StructuralState collapsed = core.Current();
        const LifecycleContext lc_collapsed = core.Lifecycle();

        // Next step in terminal MUST short-circuit and return NORMAL (SPEC)
        const EventFlag ev2 = core.Step(delta, 2, dt);
        expect_true(ev2 == EventFlag::NORMAL, "After terminal, Step() MUST return NORMAL (short-circuit)");
        expect_true(core.Lifecycle().step_counter == lc_collapsed.step_counter, "After terminal: step_counter frozen");
        expect_true(core.Lifecycle().terminal == true, "After terminal: terminal stays true");
        expect_true(core.Lifecycle().collapse_emitted == lc_collapsed.collapse_emitted, "After terminal: collapse_emitted unchanged");

        const StructuralState after = core.Current();
        expect_near(after.phi, collapsed.phi, "After terminal: phi frozen");
        expect_near(after.memory, collapsed.memory, "After terminal: memory frozen");
        expect_near(after.kappa, collapsed.kappa, "After terminal: kappa frozen");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_validation\n";
        return 0;
    }

    std::cout << "[FAIL] test_validation: " << g_fail << " failures\n";
    return 2;
}
