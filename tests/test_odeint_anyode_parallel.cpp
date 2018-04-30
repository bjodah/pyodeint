// C++11 source code.
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"
#include "odeint_anyode_parallel.hpp"
#include "testing_utils.hpp"

TEST_CASE( "decay_adaptive", "[multi_adaptive]" ) {
    std::vector<double> k {{ 2.0, 3.0}};
    Decay odesys1(k[0]);
    Decay odesys2(k[1]);
    std::vector<Decay *> systems {{ &odesys1, &odesys2 }};
    std::vector<double> y0 {{ 5.0, 7.0 }};
    std::vector<double> t0 {{ 1.0, 3.0 }};  // delta = 2
    std::vector<double> tend {{ 2.0, 5.0 }};  // delta = 3
    const int mxsteps = 0;
    std::vector<double> dx0 {{ 0, 0 }};
    std::vector<double> dx_max {{ 2.0, 2.0 }};

    auto result = odeint_anyode_parallel::multi_adaptive(
            systems, {1e-10}, 1e-10, odeint_anyode::StepType::bulirsch_stoer, &y0[0], &t0[0],
            &tend[0], mxsteps, &dx0[0], &dx_max[0]);
    for (int idx=0; idx<2; ++idx){
        const auto& tout = result[idx].first;
        const auto& yout = result[idx].second;
        for (unsigned j=0; j<tout.size(); ++j){
            REQUIRE( std::abs(y0[idx]*std::exp(tout[0]-tout[j]) - yout[j]) < 1e-8 );
        }
        REQUIRE( systems[idx]->current_info.nfo_dbl["time_cpu"] > 1e-9 );
        REQUIRE( systems[idx]->current_info.nfo_dbl["time_wall"] > 1e-9 );
    }
}
