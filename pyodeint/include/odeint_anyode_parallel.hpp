#pragma once

#include "anyode/anyode_parallel.hpp"
#include "odeint_anyode.hpp"

namespace odeint_anyode_parallel {

    using odeint_anyode::StepType;
    using odeint_anyode::simple_adaptive;
    using odeint_anyode::simple_predefined;

    using sa_t = std::pair<std::vector<double>, std::vector<double> >;

    template <class OdeSys>
    std::vector<sa_t>
    multi_adaptive(std::vector<OdeSys *> odesys, // vectorized
                   const double atol,
                   const double rtol,
                   const StepType styp,
                   const double * const y0,  // vectorized
                   const double * t0,  // vectorized
                   const double * tend,  // vectorized
                   const long int mxsteps=0,
                   const double dx0=0.0
                   ){
        const int ny = odesys[0]->get_ny();
        const int nsys = odesys.size();
        auto results = std::vector<sa_t>(nsys);

        anyode_parallel::ThreadException te;
        #pragma omp parallel for
        for (int idx=0; idx<nsys; ++idx){
            sa_t local_result;
            te.run([&]{
                local_result = simple_adaptive<OdeSys>(
                    odesys[idx], atol, rtol, styp, y0 + idx*ny, t0[idx], tend[idx],
                    mxsteps, dx0);
            });
            results[idx] = local_result;
        }
        te.rethrow();

        return results;
    }

    template <class OdeSys>
    void
    multi_predefined(std::vector<OdeSys *> odesys,  // vectorized
                     const double atol,
                     const double rtol,
                     const StepType styp,
                     const double * const y0, // vectorized
                     const std::size_t nout,
                     const double * const tout, // vectorized
                     double * const yout,  // vectorized
                     const long int mxsteps=0,
                     const double dx0=0.0
                     ){
        const int ny = odesys[0]->get_ny();
        const int nsys = odesys.size();

        anyode_parallel::ThreadException te;
        #pragma omp parallel for
        for (int idx=0; idx<nsys; ++idx){
            te.run([&]{
                simple_predefined<OdeSys>(odesys[idx], atol, rtol, styp, y0 + idx*ny,
                                          nout, tout + idx*nout, yout + idx*ny*nout,
                                          mxsteps, dx0);
            });
        }
        te.rethrow();
    }

}
