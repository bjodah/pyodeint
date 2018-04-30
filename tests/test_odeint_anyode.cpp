// C++11 source code.
#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main()
#include "catch.hpp"
#include "odeint_anyode.hpp"
#include "testing_utils.hpp"


TEST_CASE( "methods", "[OdeintIntegrator]" ) {
    Decay odesys(1.0);
    double dx0 = 1e-12, atol=1e-8, rtol=1e-8, dx_max=INFINITY;
    auto intgr = odeint_anyode::Integr<Decay>(&odesys, dx0, dx_max, atol, rtol, odeint_anyode::StepType::dopri5, 500);
    std::vector<double> y0 {{1.0}};
    std::vector<double> tout {{0.0, 1.0}};
    std::vector<double> yout(2);
    intgr.predefined(tout.size(), &tout[0], &y0[0], &yout[0]);
    double yref = std::exp(-tout[1]);
    REQUIRE( std::abs(yout[1] - yref) < 1e-7 );
}


TEST_CASE( "decay_adaptive", "[simple_adaptive]" ) {
    Decay odesys(1.0);
    double y0 = 1.0;
    double dx0=1e-9, dx_max=INFINITY;
    long int mxsteps = 500;
    auto tout_yout = odeint_anyode::simple_adaptive(&odesys, 1e-10, 1e-10,
                                                    odeint_anyode::StepType::dopri5,
                                                    &y0, 0.0, 1.0, mxsteps, dx0, dx_max);
    auto& tout = tout_yout.first;
    auto& yout = tout_yout.second;
    REQUIRE( tout.size() == yout.size() );
    for (uint i = 0; i < tout.size(); ++i){
        REQUIRE( std::abs(std::exp(-tout[i]) - yout[i]) < 1e-8 );
    }
    REQUIRE( odesys.current_info.nfo_int["nfev"] > 1 );
    REQUIRE( odesys.current_info.nfo_int["nfev"] < 997 );
}


TEST_CASE( "decay_adaptive_dx_max", "[simple_adaptive]" ) {
    Decay odesys(1.0);
    double y0 = 1.0;
    double dx0=1e-9, dx_max=0.0;
    long int mxsteps = 500;
    auto tout_yout = odeint_anyode::simple_adaptive(&odesys, 1e-10, 1e-10,
                                                    odeint_anyode::StepType::dopri5,
                                                    &y0, 0.0, 1.0, mxsteps, dx0, dx_max); // , 0.0, 1e-3, 1100);
    auto& tout = tout_yout.first;
    auto& yout = tout_yout.second;
    REQUIRE( tout.size() == yout.size() );
    for (uint i = 0; i < tout.size(); ++i){
        REQUIRE( std::abs(std::exp(-tout[i]) - yout[i]) < 1e-8 );
    }
    REQUIRE( odesys.current_info.nfo_int["nfev"] > 50 );
}
