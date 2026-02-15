// ==============================
// File: tests/test_initial_terminal.cpp
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

    std::cout << "test_initial_terminal\n";

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

    // Initial terminal state: kappa == 0
    StructuralState init{0.0, 0.0, 0.0};

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create(valid terminal init) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    // Must be terminal immediately
    expect_true(core.Lifecycle().step_counter == 0u, "Initial terminal: step_counter == 0");
    expect_true(core.Lifecycle().terminal == true, "Initial terminal: terminal == true");
    expect_true(core.Lifecycle().collapse_emitted == false, "Initial terminal: collapse_emitted == false");
    expect_near(core.Current().kappa, 0.0, "Initial terminal: kappa == 0");

    // Step() MUST short-circuit BEFORE validation.
    // Provide intentionally invalid inputs; must return NORMAL and not mutate.
    {
        const double* delta_ptr = nullptr;
        const size_t delta_len = 999;
        const double dt_nan = std::numeric_limits<double>::quiet_NaN();

        const EventFlag ev = core.Step(delta_ptr, delta_len, dt_nan);
        expect_true(ev == EventFlag::NORMAL, "Initial terminal: Step returns NORMAL even with invalid inputs");

        expect_true(core.Lifecycle().step_counter == 0u, "Initial terminal: step_counter frozen");
        expect_true(core.Lifecycle().terminal == true, "Initial terminal: terminal remains true");
        expect_true(core.Lifecycle().collapse_emitted == false, "Initial terminal: collapse_emitted remains false");

        expect_near(core.Current().phi, init.phi, "Initial terminal: phi frozen");
        expect_near(core.Current().memory, init.memory, "Initial terminal: memory frozen");
        expect_near(core.Current().kappa, init.kappa, "Initial terminal: kappa frozen");

        expect_near(core.Previous().phi, init.phi, "Initial terminal: previous phi frozen");
        expect_near(core.Previous().memory, init.memory, "Initial terminal: previous memory frozen");
        expect_near(core.Previous().kappa, init.kappa, "Initial terminal: previous kappa frozen");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_initial_terminal\n";
        return 0;
    }

    std::cout << "[FAIL] test_initial_terminal: " << g_fail << " failures\n";
    return 2;
}
