// ==============================
// File: tests/test_canonical.cpp
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

    std::cout << "test_canonical\n";

    // Params chosen to make arithmetic clean and deterministic
    ParameterSet p{
        2.0,   // alpha
        0.5,   // eta
        0.25,  // beta
        0.5,   // gamma
        0.1,   // rho
        0.2,   // lambda_phi
        0.3,   // lambda_m
        10.0   // kappa_max
    };

    const size_t delta_dim = 2;

    StructuralState init{
        1.0,  // phi
        2.0,  // memory
        5.0   // kappa
    };

    auto core_opt = MaxCore::Create(p, delta_dim, init);
    expect_true(core_opt.has_value(), "Create(valid) must succeed");
    if (!core_opt) return 1;

    MaxCore core = *core_opt;

    const double dt = 0.1;
    const double delta[2] = {3.0, 4.0}; // norm2 = 25

    // Expected canonical:
    // phi_next = phi + alpha*norm2 - eta*phi*dt
    // memory_next = memory + beta*phi_next*dt - gamma*memory*dt
    // kappa_next = kappa + rho*(kappa_max-kappa)*dt - lambda_phi*phi_next*dt - lambda_m*memory_next*dt
    const double norm2 = 25.0;

    double phi_next = init.phi + (p.alpha * norm2) - (p.eta * init.phi * dt);
    if (phi_next < 0.0) phi_next = 0.0;

    double memory_next = init.memory + (p.beta * phi_next * dt) - (p.gamma * init.memory * dt);
    if (memory_next < 0.0) memory_next = 0.0;

    double kappa_next =
        init.kappa
        + (p.rho * (p.kappa_max - init.kappa) * dt)
        - (p.lambda_phi * phi_next * dt)
        - (p.lambda_m * memory_next * dt);

    // clamp kappa
    if (kappa_next < 0.0) kappa_next = 0.0;
    if (kappa_next > p.kappa_max) kappa_next = p.kappa_max;

    const EventFlag ev = core.Step(delta, 2, dt);
    expect_true(ev != EventFlag::ERROR, "Step() must not return ERROR");

    const StructuralState cur = core.Current();
    const StructuralState prev = core.Previous();
    const LifecycleContext lc = core.Lifecycle();

    // Previous must be prior committed state
    expect_near(prev.phi, init.phi, "Previous.phi equals init.phi after 1 step");
    expect_near(prev.memory, init.memory, "Previous.memory equals init.memory after 1 step");
    expect_near(prev.kappa, init.kappa, "Previous.kappa equals init.kappa after 1 step");

    // Current must match canonical
    expect_near(cur.phi, phi_next, "Current.phi matches canonical phi_next");
    expect_near(cur.memory, memory_next, "Current.memory matches canonical memory_next");
    expect_near(cur.kappa, kappa_next, "Current.kappa matches canonical kappa_next");

    // Lifecycle increments exactly once
    expect_true(lc.step_counter == 1u, "step_counter == 1 after one successful commit");

    if (g_fail == 0) {
        std::cout << "[OK] test_canonical\n";
        return 0;
    }

    std::cout << "[FAIL] test_canonical: " << g_fail << " failures\n";
    return 2;
}
