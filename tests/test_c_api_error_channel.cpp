// ==============================
// File: tests/test_c_api_error_channel.cpp
// ==============================
#include <iostream>
#include <limits>

#include "maxcore/c_api.h"

static int g_fail = 0;

static void expect_true(bool cond, const char* msg) {
    if (!cond) {
        std::cout << "[FAIL] " << msg << "\n";
        g_fail += 1;
    }
}

static void expect_nonempty(const char* s, const char* msg) {
    if (!s || s[0] == '\0') {
        std::cout << "[FAIL] " << msg << " (empty)\n";
        g_fail += 1;
    }
}

static void expect_empty(const char* s, const char* msg) {
    if (!s) {
        std::cout << "[FAIL] " << msg << " (null)\n";
        g_fail += 1;
        return;
    }
    if (s[0] != '\0') {
        std::cout << "[FAIL] " << msg << " (not empty: '" << s << "')\n";
        g_fail += 1;
    }
}

int main() {
    std::cout << "test_c_api_error_channel\n";

    // Valid params/state
    maxcore_params p{};
    p.alpha = 1.0;
    p.eta = 0.1;
    p.beta = 0.5;
    p.gamma = 0.1;
    p.rho = 0.2;
    p.lambda_phi = 0.1;
    p.lambda_m = 0.1;
    p.kappa_max = 10.0;

    const size_t delta_dim = 2;

    maxcore_state init{};
    init.phi = 0.0;
    init.memory = 0.0;
    init.kappa = 10.0;

    maxcore_handle* h = maxcore_create(&p, delta_dim, &init, nullptr);
    expect_true(h != nullptr, "maxcore_create must succeed");
    if (!h) return 1;

    // Initially error string must be empty
    expect_empty(maxcore_last_error(h), "last_error must be empty after create");

    // Valid step clears error
    {
        const double delta[2] = {1.0, 2.0};
        const double dt = 0.01;

        const maxcore_event ev = maxcore_step(h, delta, delta_dim, dt);
        expect_true(ev != MAXCORE_EVENT_ERROR, "valid step must not return ERROR");
        expect_empty(maxcore_last_error(h), "last_error must remain empty after valid step");
    }

    // Invalid step must set last_error (dt=NaN) and return ERROR
    {
        const double delta[2] = {1.0, 2.0};
        const double dt_nan = std::numeric_limits<double>::quiet_NaN();

        const maxcore_event ev = maxcore_step(h, delta, delta_dim, dt_nan);
        expect_true(ev == MAXCORE_EVENT_ERROR, "dt=NaN must return ERROR");
        expect_nonempty(maxcore_last_error(h), "last_error must be non-empty after ERROR");
    }

    // Next valid step must clear last_error again
    {
        const double delta[2] = {1.0, 2.0};
        const double dt = 0.01;

        const maxcore_event ev = maxcore_step(h, delta, delta_dim, dt);
        // Could be NORMAL or COLLAPSE depending on internal state, but must not be ERROR for valid input
        expect_true(ev != MAXCORE_EVENT_ERROR, "valid step after error must not return ERROR");
        expect_empty(maxcore_last_error(h), "last_error must be cleared after next non-ERROR step");
    }

    // Null handle should return a stable error string (contract convenience)
    {
        const char* s = maxcore_last_error(nullptr);
        expect_nonempty(s, "last_error(null) must return non-empty diagnostic string");
    }

    maxcore_destroy(h);

    if (g_fail == 0) {
        std::cout << "[OK] test_c_api_error_channel\n";
        return 0;
    }

    std::cout << "[FAIL] test_c_api_error_channel: " << g_fail << " failures\n";
    return 2;
}
