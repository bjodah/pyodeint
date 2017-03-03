#pragma once

#include <cstdlib>
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
                   const long int mxsteps,
                   const double * dx0,  // vectorized
                   const double * dx_max,  // vectorized
                   int autorestart=0,
                   bool return_on_error=false
                   ){
        const int ny = odesys[0]->get_ny();
        const int nsys = odesys.size();
        auto results = std::vector<sa_t>(nsys);

        anyode_parallel::ThreadException te;
        char * num_threads_var = std::getenv("ANYODE_NUM_THREADS");
        int nt = (num_threads_var) ? std::atoi(num_threads_var) : 1;
        if (nt < 0)
            nt = 1;
        #pragma omp parallel for num_threads(nt) // OMP_NUM_THREADS should be 1 for openblas LU (small matrices)
        for (int idx=0; idx<nsys; ++idx){
            sa_t local_result;
            te.run([&]{
                local_result = simple_adaptive<OdeSys>(
                    odesys[idx], atol, rtol, styp, y0 + idx*ny, t0[idx], tend[idx],
                    mxsteps, dx0[idx], dx_max[idx], autorestart, return_on_error);
            });
            results[idx] = local_result;
        }
        te.rethrow();

        return results;
    }

    template <class OdeSys>
    std::vector<int>
    multi_predefined(std::vector<OdeSys *> odesys,  // vectorized
                     const double atol,
                     const double rtol,
                     const StepType styp,
                     const double * const y0, // vectorized
                     const std::size_t nout,
                     const double * const tout, // vectorized
                     double * const yout,  // vectorized
                     const long int mxsteps,
                     const double * dx0,  // vectorized
                     const double * dx_max,  // vectorized
                     int autorestart=0,
                     bool return_on_error=false
                     ){
        const int ny = odesys[0]->get_ny();
        const int nsys = odesys.size();
        std::vector<int> result(nsys);

        anyode_parallel::ThreadException te;
        #pragma omp parallel for
        for (int idx=0; idx<nsys; ++idx){
            te.run([&]{
                result[idx] = simple_predefined<OdeSys>(odesys[idx], atol, rtol, styp, y0 + idx*ny,
                                                        nout, tout + idx*nout, yout + idx*ny*nout,
                                                        mxsteps, dx0[idx], dx_max[idx], autorestart, return_on_error);
            });
        }
        te.rethrow();
        return result;
    }

}
