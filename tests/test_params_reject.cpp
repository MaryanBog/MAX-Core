// ==============================
// File: tests/test_params_reject.cpp
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

static maxcore::ParameterSet base_params() {
    using namespace maxcore;
    return ParameterSet{
        1.0,  // alpha
        0.1,  // eta
        0.5,  // beta
        0.1,  // gamma
        0.2,  // rho
        0.1,  // lambda_phi
        0.1,  // lambda_m
        10.0  // kappa_max
    };
}

int main() {
    using namespace maxcore;

    std::cout << "test_params_reject\n";

    const size_t delta_dim = 2;
    const StructuralState init_ok{0.0, 0.0, 10.0};

    // ---- delta_dim == 0 must reject
    {
        ParameterSet p = base_params();
        auto core = MaxCore::Create(p, 0, init_ok);
        expect_true(!core.has_value(), "Create must reject delta_dim == 0");
    }

    // ---- Any parameter <= 0 must reject
    {
        struct Mut { const char* name; int idx; };
        // idx mapping to ParameterSet fields order in base_params():
        // 0 alpha,1 eta,2 beta,3 gamma,4 rho,5 lambda_phi,6 lambda_m,7 kappa_max
        const Mut muts[] = {
            {"alpha", 0}, {"eta", 1}, {"beta", 2}, {"gamma", 3},
            {"rho", 4}, {"lambda_phi", 5}, {"lambda_m", 6}, {"kappa_max", 7}
        };

        for (const auto& m : muts) {
            ParameterSet p = base_params();
            double* arr = &p.alpha;
            arr[m.idx] = 0.0;
            auto core0 = MaxCore::Create(p, delta_dim, init_ok);
            expect_true(!core0.has_value(), "Create must reject param == 0");

            arr[m.idx] = -1.0;
            auto coreN = MaxCore::Create(p, delta_dim, init_ok);
            expect_true(!coreN.has_value(), "Create must reject param < 0");

            // Special: kappa_max also must reject non-finite
            if (m.idx == 7) {
                arr[m.idx] = std::numeric_limits<double>::quiet_NaN();
                auto coreNaN = MaxCore::Create(p, delta_dim, init_ok);
                expect_true(!coreNaN.has_value(), "Create must reject kappa_max NaN");
            }
        }
    }

    // ---- Reject non-finite params (sample a few)
    {
        ParameterSet p = base_params();
        p.alpha = std::numeric_limits<double>::infinity();
        auto core = MaxCore::Create(p, delta_dim, init_ok);
        expect_true(!core.has_value(), "Create must reject alpha = +Inf");
    }
    {
        ParameterSet p = base_params();
        p.lambda_m = std::numeric_limits<double>::quiet_NaN();
        auto core = MaxCore::Create(p, delta_dim, init_ok);
        expect_true(!core.has_value(), "Create must reject lambda_m = NaN");
    }

    // ---- Reject invalid initial_state
    {
        ParameterSet p = base_params();

        StructuralState s = init_ok;
        s.phi = -1.0;
        auto core = MaxCore::Create(p, delta_dim, s);
        expect_true(!core.has_value(), "Create must reject initial phi < 0");
    }
    {
        ParameterSet p = base_params();

        StructuralState s = init_ok;
        s.memory = -1.0;
        auto core = MaxCore::Create(p, delta_dim, s);
        expect_true(!core.has_value(), "Create must reject initial memory < 0");
    }
    {
        ParameterSet p = base_params();

        StructuralState s = init_ok;
        s.kappa = -1.0;
        auto core = MaxCore::Create(p, delta_dim, s);
        expect_true(!core.has_value(), "Create must reject initial kappa < 0");
    }
    {
        ParameterSet p = base_params();

        StructuralState s = init_ok;
        s.kappa = p.kappa_max + 1.0;
        auto core = MaxCore::Create(p, delta_dim, s);
        expect_true(!core.has_value(), "Create must reject initial kappa > kappa_max");
    }
    {
        ParameterSet p = base_params();

        StructuralState s = init_ok;
        s.kappa = std::numeric_limits<double>::quiet_NaN();
        auto core = MaxCore::Create(p, delta_dim, s);
        expect_true(!core.has_value(), "Create must reject initial kappa NaN");
    }

    // ---- delta_max optional validation
    {
        ParameterSet p = base_params();
        auto core = MaxCore::Create(p, delta_dim, init_ok, 0.0);
        expect_true(!core.has_value(), "Create must reject delta_max == 0");
    }
    {
        ParameterSet p = base_params();
        auto core = MaxCore::Create(p, delta_dim, init_ok, -1.0);
        expect_true(!core.has_value(), "Create must reject delta_max < 0");
    }
    {
        ParameterSet p = base_params();
        const double nanv = std::numeric_limits<double>::quiet_NaN();
        auto core = MaxCore::Create(p, delta_dim, init_ok, nanv);
        expect_true(!core.has_value(), "Create must reject delta_max NaN");
    }
    {
        ParameterSet p = base_params();
        const double infv = std::numeric_limits<double>::infinity();
        auto core = MaxCore::Create(p, delta_dim, init_ok, infv);
        expect_true(!core.has_value(), "Create must reject delta_max +Inf");
    }

    // ---- Control: valid create must succeed
    {
        ParameterSet p = base_params();
        auto core = MaxCore::Create(p, delta_dim, init_ok, 5.0);
        expect_true(core.has_value(), "Create(valid) must succeed");
    }

    if (g_fail == 0) {
        std::cout << "[OK] test_params_reject\n";
        return 0;
    }

    std::cout << "[FAIL] test_params_reject: " << g_fail << " failures\n";
    return 2;
}
