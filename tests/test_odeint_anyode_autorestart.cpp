#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"
#include <math.h>
#include <vector>
#include "anyode/anyode.hpp"
#include "odeint_anyode.hpp"
#include "cetsa_case.hpp"


TEST_CASE( "adaptive_autorestart", "[simple_adaptive]" ) {
    std::vector<double> p = {{298.15, 39390, -135.3, 18010, 44960, 48.2, 65919.5, -93.8304, 1780, 3790, 57.44, 19700, -157.4}};
    std::vector<double> y0 = {{8.99937e-07, 0.000693731, 0.000264211, 0.000340312, 4.11575e-05}};
    double t0=0, tend=60;
    OdeSys odesys(&p[0]);
    const long int mxsteps=30;
    const double dx0=0.0;
    int autorestart=2;

    auto tout_yout = odeint_anyode::simple_adaptive(&odesys, 1e-8, 1e-8, odeint_anyode::StepType::rosenbrock4, &y0[0], t0, tend,
                                                    mxsteps, dx0, autorestart);
    auto& tout = tout_yout.first;
    auto& yout = tout_yout.second;
    const int ref = tout.size() * odesys.get_ny();
    // for (unsigned idx=0; idx<tout.size(); ++idx){
    //     std::cout << tout[idx] << " ";
    //     for (unsigned yi=0; yi<5; ++yi)
    //         std::cout << yout[idx*5 + yi] << " ";
    //     std::cout << "\n";
    // }
    REQUIRE( ref == yout.size() );
    REQUIRE( odesys.last_integration_info["n_steps"] > 1 );
    REQUIRE( odesys.last_integration_info["n_steps"] < 997 );
}
