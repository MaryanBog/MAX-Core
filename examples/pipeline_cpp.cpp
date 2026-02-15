// ==============================
// File: examples/pipeline_cpp.cpp
// ==============================
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

#include "maxcore/maxcore.h"
#include "maxcore/derived.h"

static const char* EventToStr(maxcore::EventFlag ev) noexcept {
    switch (ev) {
        case maxcore::EventFlag::NORMAL:   return "NORMAL";
        case maxcore::EventFlag::COLLAPSE: return "COLLAPSE";
        case maxcore::EventFlag::ERROR:    return "ERROR";
        default:                           return "UNKNOWN";
    }
}

static void WriteHeader(std::ofstream& out) {
    out
        << "t"
        << ",lifecycle"
        << ",step_counter"
        << ",event"
        << ",terminal"
        << ",collapse_emitted"
        << ",phi"
        << ",memory"
        << ",kappa"
        << ",d_phi"
        << ",d_memory"
        << ",d_kappa"
        << ",phi_rate"
        << ",memory_rate"
        << ",kappa_rate"
        << ",kappa_ratio"
        << ",kappa_distance"
        << ",load_term"
        << ",regen_term"
        << "\n";
}

int main(int argc, char** argv) {
    using namespace maxcore;

    const std::string out_path = (argc >= 2) ? std::string(argv[1]) : std::string("out_pipeline_cpp.csv");

    // ---- Pipeline config (deterministic)
    const size_t delta_dim = 2;
    const double dt = 0.01;
    const int total_steps = 220;

    // Choose params so collapse is reachable, to exercise Fresh Genesis.
    ParameterSet p{
        1.0,    // alpha
        0.1,    // eta
        0.5,    // beta
        0.1,    // gamma
        0.05,   // rho (regen)
        0.25,   // lambda_phi (load)
        0.25,   // lambda_m (load)
        10.0    // kappa_max
    };

    // Deterministic constant delta input
    double delta[delta_dim] = {1.0, 2.0};

    auto make_fresh = [&]() -> std::optional<MaxCore> {
        // Fresh Genesis initial state (external orchestration):
        // Phi = 0, Memory = 0, Kappa = kappa_max
        StructuralState init{0.0, 0.0, p.kappa_max};
        return MaxCore::Create(p, delta_dim, init);
    };

    std::optional<MaxCore> core_opt = make_fresh();
    if (!core_opt) {
        std::cerr << "Create() failed\n";
        return 1;
    }

    // IMPORTANT: keep the core inside std::optional and reset it on genesis.
    // This avoids any accidental stale state and makes lifecycle resets explicit.
    std::optional<MaxCore> core = std::move(core_opt);

    uint64_t lifecycle_id = 0;

    std::ofstream out(out_path, std::ios::out | std::ios::trunc);
    if (!out) {
        std::cerr << "Cannot open output file: " << out_path << "\n";
        return 2;
    }
    out.setf(std::ios::fixed);
    out << std::setprecision(10);
    WriteHeader(out);

    std::cout << "=== example_pipeline_cpp ===\n";
    std::cout << "out=" << out_path << " steps=" << total_steps << " dt=" << dt << "\n";

    // Print initial state (t = -1 snapshot) to prove genesis resets core state.
    {
        const auto& st = core->Current();
        const auto& lc = core->Lifecycle();
        std::cout
            << "[init] lifecycle=" << lifecycle_id
            << " Phi=" << st.phi
            << " M=" << st.memory
            << " K=" << st.kappa
            << " sc=" << lc.step_counter
            << " term=" << (lc.terminal ? 1 : 0)
            << " collapse_emitted=" << (lc.collapse_emitted ? 1 : 0)
            << "\n";
    }

    for (int t = 0; t < total_steps; ++t) {
        // Defensive: should never be empty during loop.
        if (!core) {
            std::cerr << "Internal error: core is nullopt at t=" << t << "\n";
            return 5;
        }

        const EventFlag ev = core->Step(delta, delta_dim, dt);

        // Derived projection is read-only and must succeed for dt>0 and finite state.
        auto d = ComputeDerived(core->Current(), core->Previous(), core->Lifecycle(), p, dt);
        if (!d) {
            std::cerr << "ComputeDerived failed at t=" << t << "\n";
            return 3;
        }

        out
            << t
            << "," << lifecycle_id
            << "," << core->Lifecycle().step_counter
            << "," << EventToStr(ev)
            << "," << (core->Lifecycle().terminal ? 1 : 0)
            << "," << (core->Lifecycle().collapse_emitted ? 1 : 0)
            << "," << core->Current().phi
            << "," << core->Current().memory
            << "," << core->Current().kappa
            << "," << d->d_phi
            << "," << d->d_memory
            << "," << d->d_kappa
            << "," << d->phi_rate
            << "," << d->memory_rate
            << "," << d->kappa_rate
            << "," << d->kappa_ratio
            << "," << d->kappa_distance
            << "," << d->load_term
            << "," << d->regen_term
            << "\n";

        if (t < 5 || ev == EventFlag::COLLAPSE) {
            std::cout
                << "[t=" << t << "] lifecycle=" << lifecycle_id
                << " ev=" << EventToStr(ev)
                << " Phi=" << core->Current().phi
                << " M=" << core->Current().memory
                << " K=" << core->Current().kappa
                << " sc=" << core->Lifecycle().step_counter
                << " term=" << (core->Lifecycle().terminal ? 1 : 0)
                << " collapse_emitted=" << (core->Lifecycle().collapse_emitted ? 1 : 0)
                << "\n";
        }

        if (ev == EventFlag::ERROR) {
            std::cout << "STOP: ERROR\n";
            break;
        }

        if (ev == EventFlag::COLLAPSE) {
            // External Fresh Genesis (new lifecycle instance).
            lifecycle_id += 1;

            std::optional<MaxCore> fresh = make_fresh();
            if (!fresh) {
                std::cerr << "Fresh Genesis Create() failed\n";
                return 4;
            }

            // Replace instance explicitly.
            core.reset();
            core = std::move(fresh);

            // Print reset proof immediately after genesis.
            {
                const auto& st = core->Current();
                const auto& lc = core->Lifecycle();
                std::cout << "=== Fresh Genesis -> lifecycle=" << lifecycle_id << " ===\n";
                std::cout
                    << "[genesis] lifecycle=" << lifecycle_id
                    << " Phi=" << st.phi
                    << " M=" << st.memory
                    << " K=" << st.kappa
                    << " sc=" << lc.step_counter
                    << " term=" << (lc.terminal ? 1 : 0)
                    << " collapse_emitted=" << (lc.collapse_emitted ? 1 : 0)
                    << "\n";
            }
        }
    }

    out.flush();
    out.close();

    std::cout << "Wrote: " << out_path << "\n";
    return 0;
}
