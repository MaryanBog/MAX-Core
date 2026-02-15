// ==============================
// File: tests/test_init.cpp
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

static void expect_finite(double x, const char* msg) {
    if (!(std::isfinite(x) != 0)) {
        std::cout << "[FAIL] " << msg << " (not finite)\n";
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

    std::cout << "test_init\n";

    // ---- Valid params
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
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) {
        return 1;
    }

    MaxCore core = *core_opt;

    // ---- Initial snapshot invariants
    {
        const StructuralState cur = core.Current();
        const StructuralState prev = core.Previous();
        const LifecycleContext lc = core.Lifecycle();

        expect_finite(cur.phi, "Current().phi finite");
        expect_finite(cur.memory, "Current().memory finite");
        expect_finite(cur.kappa, "Current().kappa finite");

        expect_near(cur.phi, init.phi, "Current.phi equals init.phi");
        expect_near(cur.memory, init.memory, "Current.memory equals init.memory");
        expect_near(cur.kappa, init.kappa, "Current.kappa equals init.kappa");

        // Previous should match current right after create (baseline committed)
        expect_near(prev.phi, cur.phi, "Previous.phi == Current.phi at start");
        expect_near(prev.memory, cur.memory, "Previous.memory == Current.memory at start");
        expect_near(prev.kappa, cur.kappa, "Previous.kappa == Current.kappa at start");

        // Lifecycle baseline
        expect_true(lc.step_counter == 0u, "Lifecycle.step_counter == 0 at start");
        expect_true(lc.terminal == false, "Lifecycle.terminal == false at start");
        expect_true(lc.collapse_emitted == false, "Lifecycle.collapse_emitted == false at start");
    }

    // ---- One step with finite dt
    {
        const double dt = 0.01;
        const double delta[2] = {1.0, 2.0};

        const EventFlag ev = core.Step(delta, 2, dt);
        expect_true(ev != EventFlag::ERROR, "Step must not return ERROR for valid inputs");

        const StructuralState cur = core.Current();
        const StructuralState prev = core.Previous();
        const LifecycleContext lc = core.Lifecycle();

        expect_finite(cur.phi, "After Step: Current().phi finite");
        expect_finite(cur.memory, "After Step: Current().memory finite");
        expect_finite(cur.kappa, "After Step: Current().kappa finite");

        expect_true(lc.step_counter == 1u, "Lifecycle.step_counter increments to 1");

        // Derived must compute for valid dt
        auto d = ComputeDerived(cur, prev, lc, p, dt);
        expect_true(d.has_value(), "ComputeDerived must succeed for valid dt");
        if (d) {
            expect_finite(d->d_phi, "Derived.d_phi finite");
            expect_finite(d->d_memory, "Derived.d_memory finite");
            expect_finite(d->d_kappa, "Derived.d_kappa finite");

            expect_finite(d->phi_rate, "Derived.phi_rate finite");
            expect_finite(d->memory_rate, "Derived.memory_rate finite");
            expect_finite(d->kappa_rate, "Derived.kappa_rate finite");

            expect_finite(d->kappa_ratio, "Derived.kappa_ratio finite");
            expect_true(d->kappa_ratio >= 0.0 && d->kappa_ratio <= 1.0, "Derived.kappa_ratio in [0,1]");

            expect_finite(d->load_term, "Derived.load_term finite");
            expect_finite(d->regen_term, "Derived.regen_term finite");
        }
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_init\n";
        return 0;
    }

    std::cout << "[FAIL] test_init: " << g_fail << " failures\n";
    return 2;
}
