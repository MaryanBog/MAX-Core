// ==============================
// File: tests/test_terminal_shortcircuit.cpp
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

int main() {
    using namespace maxcore;

    std::cout << "test_terminal_shortcircuit\n";

    // Parameters tuned so collapse is reachable quickly.
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
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.01;
    const double delta[2] = {1.0, 2.0};

    // Drive to collapse
    EventFlag ev = EventFlag::NORMAL;
    int guard = 0;
    while (ev != EventFlag::COLLAPSE && ev != EventFlag::ERROR && guard < 500) {
        ev = core.Step(delta, delta_dim, dt);
        guard += 1;
    }

    expect_true(ev == EventFlag::COLLAPSE, "Must reach COLLAPSE");
    expect_true(core.Lifecycle().terminal == true, "After COLLAPSE: terminal true");
    expect_true(core.Current().kappa >= 0.0 && core.Current().kappa < 1e-15, "After COLLAPSE: kappa ~ 0");

    const StructuralState collapsed = core.Current();
    const StructuralState prev_collapsed = core.Previous();
    const LifecycleContext lc_collapsed = core.Lifecycle();

    // Now we intentionally provide invalid inputs. If short-circuit occurs BEFORE validation,
    // Step() MUST return NORMAL and MUST NOT mutate.
    {
        const double dt_nan = std::numeric_limits<double>::quiet_NaN();
        const EventFlag ev2 = core.Step(nullptr, 999, dt_nan);

        expect_true(ev2 == EventFlag::NORMAL, "Terminal short-circuit must return NORMAL even for invalid inputs");

        const StructuralState after = core.Current();
        const StructuralState prev_after = core.Previous();
        const LifecycleContext lc_after = core.Lifecycle();

        expect_near(after.phi, collapsed.phi, "Terminal: phi frozen");
        expect_near(after.memory, collapsed.memory, "Terminal: memory frozen");
        expect_near(after.kappa, collapsed.kappa, "Terminal: kappa frozen");

        expect_near(prev_after.phi, prev_collapsed.phi, "Terminal: Previous.phi unchanged");
        expect_near(prev_after.memory, prev_collapsed.memory, "Terminal: Previous.memory unchanged");
        expect_near(prev_after.kappa, prev_collapsed.kappa, "Terminal: Previous.kappa unchanged");

        expect_true(lc_after.step_counter == lc_collapsed.step_counter, "Terminal: step_counter frozen");
        expect_true(lc_after.terminal == lc_collapsed.terminal, "Terminal: terminal unchanged");
        expect_true(lc_after.collapse_emitted == lc_collapsed.collapse_emitted, "Terminal: collapse_emitted unchanged");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_terminal_shortcircuit\n";
        return 0;
    }

    std::cout << "[FAIL] test_terminal_shortcircuit: " << g_fail << " failures\n";
    return 2;
}
