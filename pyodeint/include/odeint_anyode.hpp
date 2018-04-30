#ifndef ODEINT_ANYODE_H_6D2AAAD4880011E6AC5C734FA77443A3
#define ODEINT_ANYODE_H_6D2AAAD4880011E6AC5C734FA77443A3

#include <limits>
#include <string>
#include <unordered_map>
#include <chrono>
#include <iostream>
#include <sstream>
#include <vector>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/odeint.hpp>

#include <anyode/anyode.hpp>


#if !defined(PYODEINT_NO_BOOST_CHECK)
  #if BOOST_VERSION / 100000 == 1
    #if BOOST_VERSION / 100 % 1000 == 60
      #error "Boost v 1.60 has a bug in rosenbrock stepper (see https://github.com/headmyshoulder/odeint-v2/issues/189) set PYODEINT_NO_BOOST_CHECK to ignore"
    #endif
    #if BOOST_VERSION / 100 % 1000 == 61
      #error "Boost v 1.61 has a bug in rosenbrock stepper (see https://github.com/headmyshoulder/odeint-v2/issues/189) set PYODEINT_NO_BOOST_CHECK to ignore"
    #endif
    #if BOOST_VERSION / 100 % 1000 == 62
      #error "Boost v 1.62 has a bug in rosenbrock stepper (see https://github.com/headmyshoulder/odeint-v2/issues/189) set PYODEINT_NO_BOOST_CHECK to ignore"
    #endif
  #endif
#endif

namespace {
    class StreamFmt
    {
        std::stringstream m_s;
    public:
        StreamFmt() {}
        ~StreamFmt() {}

        template <typename T>
        StreamFmt& operator << (const T& v) {
            this->m_s << v;
            return *this;
        }

        std::string str() const {
            return this->m_s.str();
        }
        operator std::string() const {
            return this->m_s.str();
        }

    };
}


namespace odeint_anyode{
    using namespace std::placeholders;

    using boost::numeric::odeint::integrate_adaptive;
    using boost::numeric::odeint::make_dense_output;
    using boost::numeric::odeint::rosenbrock4;
    using boost::numeric::odeint::runge_kutta_dopri5;
    using boost::numeric::odeint::bulirsch_stoer_dense_out;

    // value_type is hardcoded to double at the moment
    using value_type = double;
    using vector_type = boost::numeric::ublas::vector<value_type>;
    using matrix_type = boost::numeric::ublas::matrix<value_type>;

    // using OdeSys_t = AnyODE::OdeSysBase;

    enum class StepType : int { bulirsch_stoer, rosenbrock4, dopri5 };

    StepType styp_from_name(std::string name){
        if (name == "bulirsch_stoer")
            return StepType::bulirsch_stoer;
        else if (name == "rosenbrock4")
            return StepType::rosenbrock4;
        else if (name == "dopri5")
            return StepType::dopri5;
        else
            throw std::runtime_error(StreamFmt() << "Unknown stepper type name: " << name);
    }

    bool requires_jacobian(StepType styp){
        if (styp == StepType::rosenbrock4)
            return true;
        else
            return false;
    }

    vector_type vec_from_ptr(const value_type * const arr, std::size_t len){
        vector_type vec(len);
        for (std::size_t i=0; i<len; ++i)
            vec[i] = arr[i];
        return vec;
    }


    // Integr will be specialzed for: rosenbrock, dopri5 and bulrisch-stoer
    // adaptive and predefined cannot be put here since make_dense_output
    // is a function and bulirsch_stoer_dense_out is a class
    template<class OdeSys>
    struct Integr {
        OdeSys * m_odesys;
        double m_time_cpu = -1.0, m_time_wall = -1.0;
        value_type m_dx0, m_dx_max, m_atol, m_rtol;
        StepType m_styp;
        long int m_mxsteps;
        int m_autorestart;
        long int m_nsteps;
        bool m_return_on_error;

        void rhs(const vector_type &yarr, vector_type &dydx, value_type xval);
        void jac(const vector_type & yarr, matrix_type &Jmat,
                 const value_type & xval, vector_type &dfdx);
        Integr(OdeSys * odesys, value_type dx0, value_type dx_max, value_type atol, value_type rtol, StepType styp,
               long int mxsteps, int autorestart=0, bool return_on_error=false) :
            m_odesys(odesys), m_dx0(dx0), m_dx_max(dx_max), m_atol(atol), m_rtol(rtol), m_styp(styp),
            m_mxsteps(mxsteps), m_autorestart(autorestart), m_return_on_error(return_on_error) {}

        std::pair<std::vector<value_type>, std::vector<value_type> >
        adaptive(const value_type x0,
                 const value_type xend,
                 const value_type * const __restrict__ y0){
            std::time_t cputime0 = std::clock();
            auto t_start = std::chrono::high_resolution_clock::now();
            std::pair<std::vector<value_type>, std::vector<value_type> > result;
            try{
                if ( m_styp == StepType::bulirsch_stoer ) {
                    this->adaptive_bulirsch_stoer(x0, xend, y0);
                } else if ( m_styp == StepType::dopri5 ) {
                    this->adaptive_dopri5(x0, xend, y0);
                } else if ( m_styp == StepType::rosenbrock4 ) {
                    this->adaptive_rosenbrock4(x0, xend, y0);
                } else {
                    goto impossible_adaptive;
                }
            } catch (const std::exception& e) {
                if (m_autorestart > 0){
                    std::cerr << e.what() << std::endl;
                    if (this->m_xout.size() > 0){
                        std::cerr << "odeint_anyode.hpp:" << __LINE__ << ": Autorestart (" << m_autorestart
                                  << ") x=" << this->m_xout.back() << "\n";
                        m_autorestart--;
                        auto c_nsteps = this->m_nsteps;
                        auto c_xout = this->m_xout;
                        auto c_yout = this->m_yout;
                        adaptive(c_xout.back(), xend, &c_yout[c_yout.size() - m_odesys->get_ny()]);
                        c_xout.insert(c_xout.end(), m_xout.begin(), m_xout.end());
                        c_yout.insert(c_yout.end(), m_yout.begin(), m_yout.end());
                        m_xout = c_xout;
                        m_yout = c_yout;
                        m_nsteps += c_nsteps;
                    } else {
                        std::cerr << "odeint_anyode.hpp:" << __LINE__ << ": Autorestart failed." << "\n";
                        if (!m_return_on_error)
                            throw;
                    }
                } else {
                    if (!m_return_on_error)
                        throw;
                }
            }
            this->m_time_cpu = (std::clock() - cputime0) / (double)CLOCKS_PER_SEC;
            this->m_time_wall = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - t_start).count();
            return std::make_pair(this->m_xout, this->m_yout);
        impossible_adaptive:
            throw std::runtime_error("Impossible: unknown StepType!");
        }

        int predefined(const int nx,
                       const value_type * const __restrict__ xout,
                       const value_type * const __restrict__ y0,
                       value_type * const __restrict__ yout){
            int nreached;
            std::time_t cputime0 = std::clock();
            auto t_start = std::chrono::high_resolution_clock::now();
            std::copy(y0, y0 + (this->m_odesys->get_ny()), yout);
            try {
                if ( m_styp == StepType::bulirsch_stoer ) {
                    this->predefined_bulirsch_stoer(nx, xout, y0, yout, &nreached);
                } else if ( m_styp == StepType::dopri5 ) {
                    this->predefined_dopri5(nx, xout, y0, yout, &nreached);
                } else if ( m_styp == StepType::rosenbrock4 ) {
                    this->predefined_rosenbrock4(nx, xout, y0, yout, &nreached);
                } else {
                    goto impossible_predefined;
                }
            } catch (const std::exception& e) {
                if (m_autorestart > 0){
                    std::cerr << e.what() << std::endl;
                    if (this->m_xout.size() > 0) {
                        std::cerr << "odeint_anyode.hpp:" << __LINE__ << ": Autorestart (" << m_autorestart
                                  << ") x=" << this->m_xout.back() << "\n";
                        m_autorestart--;
                        auto c_nsteps = this->m_nsteps;
                        nreached += predefined(nx - nreached, xout + nreached,
                                               yout + nreached*m_odesys->get_ny(),
                                               yout + nreached*m_odesys->get_ny());
                        this->m_nsteps += c_nsteps;
                    } else {
                        std::cerr << "odeint_anyode.hpp:" << __LINE__ << ": Autorestart failed." << "\n";
                        if (!m_return_on_error)
                            throw;
                    }
                } else {
                    if (!m_return_on_error)
                        throw;
                }
            }
            this->m_time_cpu = (std::clock() - cputime0) / (double)CLOCKS_PER_SEC;
            this->m_time_wall = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - t_start).count();
            return nreached;
        impossible_predefined:
            throw std::runtime_error("Impossible: unknown StepType!");
        }
    private:
        std::vector<value_type> m_xout, m_yout;

        void reset() {
            this->m_nsteps = 0;
            this->m_xout.clear();
            this->m_yout.clear();
        }

        void obs_adaptive(const vector_type &yarr, value_type xval){
            this->m_xout.push_back(xval);
            for(int i=0 ; i < this->m_odesys->get_ny() ; ++i)
                this->m_yout.push_back(yarr[i]);
            if (this->m_nsteps == this->m_mxsteps)
                throw std::runtime_error(StreamFmt() << "Maximum number of steps reached: " << this->m_nsteps);
            m_nsteps++;
        }

        void obs_predefined(const vector_type & /* yarr */, value_type /* xval */){
            if (this->m_nsteps == this->m_mxsteps)
                throw std::runtime_error(StreamFmt() << "Maximum number of steps reached: " << this->m_nsteps);
            m_nsteps++;
        }

        void adaptive_bulirsch_stoer(const value_type x0,
                                     const value_type xend,
                                     const value_type * const __restrict__ y0
                                     ){
            const int ny = this->m_odesys->get_ny();
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };
            auto stepper = bulirsch_stoer_dense_out< vector_type, value_type >(
                this->m_atol, this->m_rtol, 1.0, 1.0, this->m_dx_max);
            auto y_ = vec_from_ptr(y0, ny);
            this->reset();
            integrate_adaptive(stepper, f, y_, x0, xend, this->m_dx0,
                               std::bind(&Integr::obs_adaptive, this, _1, _2));
        }

        void predefined_bulirsch_stoer(const int nx,
                                       const value_type * const __restrict__ xout,
                                       const value_type * const __restrict__ y0,
                                       value_type * const __restrict__ yout,
                                       int * nreached){
            *nreached = 0;
            const auto ny = this->m_odesys->get_ny();
            vector_type y_ = vec_from_ptr(y0, ny);
            vector_type xout_ = vec_from_ptr(xout, nx);
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };
            try{
                auto stepper = bulirsch_stoer_dense_out< vector_type, value_type >(
                    this->m_atol, this->m_rtol, 1.0, 1.0, this->m_dx_max);
                for (*nreached=1; *nreached < nx; ++*nreached){
                    const int ix = *nreached;
                    this->reset();
                    integrate_adaptive(stepper, f, y_, xout[ix-1], xout[ix], this->m_dx0,
                                       std::bind(&Integr::obs_predefined, this, _1, _2));
                    for (int iy=0; iy < ny; ++iy)
                        yout[ix*ny + iy] = y_[iy];
                }
            } catch (const std::exception& e) {
                std::cerr << __FILE__ << ":" << __LINE__ << ":";
                std::cerr << e.what() << std::endl;
                nreached--;
                if (!m_return_on_error)
                    throw;
            }
        }

        void adaptive_dopri5(const value_type x0,
                             const value_type xend,
                             const value_type * const __restrict__ y0){
            const int ny = this->m_odesys->get_ny();
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };

            auto stepper = make_dense_output<runge_kutta_dopri5<vector_type, value_type> >(
                this->m_atol, this->m_rtol, this->m_dx_max);
            auto y_ = vec_from_ptr(y0, ny);
            this->reset();
            integrate_adaptive(stepper, f, y_, x0, xend, this->m_dx0,
                               std::bind(&Integr::obs_adaptive, this, _1, _2));
        }

        void predefined_dopri5(const int nx,
                               const value_type * const __restrict__ xout,
                               const value_type * const __restrict__ y0,
                              value_type * const __restrict__ yout,
                              int * nreached){
            *nreached = 0;
            const auto ny = this->m_odesys->get_ny();
            vector_type y_ = vec_from_ptr(y0, ny);
            vector_type xout_ = vec_from_ptr(xout, nx);
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };
            try {
                auto stepper = make_dense_output<runge_kutta_dopri5<vector_type, value_type> >(
                    this->m_atol, this->m_rtol, this->m_dx_max);
                for (*nreached=1; *nreached < nx; ++*nreached){
                    const int ix = *nreached;
                    this->reset();
                    integrate_adaptive(stepper, f, y_, xout[ix - 1], xout[ix], this->m_dx0,
                                       std::bind(&Integr::obs_predefined, this, _1, _2));
                    for (int iy=0; iy < ny; ++iy)
                        yout[ix*ny + iy] = y_[iy];
                }
            } catch (const std::exception& e) {
                std::cerr << __FILE__ << ":" << __LINE__ << ":";
                std::cerr << e.what() << std::endl;
                nreached--;
                if (!m_return_on_error)
                    throw;
            }
        }

        void adaptive_rosenbrock4(const value_type x0,
                                  const value_type xend,
                                  const value_type * const __restrict__ y0){
            const int ny = this->m_odesys->get_ny();
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };
            auto j = [&](const vector_type & yarr, matrix_type &Jmat,
                                     const value_type & xval, vector_type &dfdx) {
                this->m_odesys->dense_jac_rmaj(xval, &(yarr.data()[0]), nullptr, &(Jmat.data()[0]), ny, &(dfdx.data()[0]));
            };
            auto stepper = make_dense_output<rosenbrock4<value_type> >(this->m_atol, this->m_rtol, this->m_dx_max);
            auto y_ = vec_from_ptr(y0, ny);
            this->reset();
            integrate_adaptive(stepper, std::make_pair(f, j), y_, x0, xend, this->m_dx0,
                                              std::bind(&Integr::obs_adaptive, this, _1, _2));
        }

        void predefined_rosenbrock4(const int nx,
                                    const value_type * const __restrict__ xout,
                                    const value_type * const __restrict__ y0,
                                    value_type * const __restrict__ yout,
                                    int * nreached){
            *nreached = 0;
            const auto ny = this->m_odesys->get_ny();
            vector_type y_ = vec_from_ptr(y0, ny);
            vector_type xout_ = vec_from_ptr(xout, nx);
            auto f = [&](const vector_type &yarr, vector_type &dydx, value_type xval) {
                this->m_odesys->rhs(xval, &(yarr.data()[0]), &(dydx.data()[0]));
            };
            auto j = [&](const vector_type & yarr, matrix_type &Jmat,
                                     const value_type & xval, vector_type &dfdx) {
                this->m_odesys->dense_jac_rmaj(xval, &(yarr.data()[0]), nullptr, &(Jmat.data()[0]), ny, &(dfdx.data()[0]));
            };
            try {
                auto stepper = make_dense_output<rosenbrock4<value_type> >(this->m_atol, this->m_rtol, this->m_dx_max);
                for (*nreached=1; *nreached < nx; ++*nreached){
                    const int ix = *nreached;
                    this->reset();
                    integrate_adaptive(stepper, std::make_pair(f, j), y_, xout[ix - 1], xout[ix], this->m_dx0,
                                       std::bind(&Integr::obs_predefined, this, _1, _2));
                    for (int iy=0; iy < ny; ++iy)
                        yout[ix*ny + iy] = y_[iy];
                }
            } catch (const std::exception& e) {
                std::cerr << __FILE__ << ":" << __LINE__ << ":";
                std::cerr << e.what() << std::endl;
                nreached--;
                if (!m_return_on_error)
                    throw;
            }
        }

    };

    template <class OdeSys>
    void set_integration_info(OdeSys * odesys, const Integr<OdeSys>& integrator){
        odesys->current_info.nfo_int["n_steps"] = integrator.m_nsteps;
        odesys->current_info.nfo_int["nfev"] = odesys->nfev;
        odesys->current_info.nfo_int["njev"] = odesys->njev;
        odesys->current_info.nfo_dbl["time_wall"] = integrator.m_time_wall;
        odesys->current_info.nfo_dbl["time_cpu"] = integrator.m_time_cpu;
    }

    template <class OdeSys>
    std::pair<std::vector<double>, std::vector<double> >
    simple_adaptive(OdeSys * const odesys,
                    const double atol,
                    const double rtol,
                    const StepType styp,
                    const double * const y0,
                    const double x0,
                    const double xend,
                    long int mxsteps=0,
                    double dx0=0.0,
                    double dx_max=0.0,
                    int autorestart=0,
                    bool return_on_error=false
                    )
                    //,
                    // const double dx_min=0.0,

                    // long int mxsteps=0)
    {
        if (dx0 == 0.0)
            dx0 = odesys->get_dx0(x0, y0);
        if (dx0 == 0.0){
            if (x0 == 0)
                dx0 = std::numeric_limits<double>::epsilon() * 100;
            else
                dx0 = std::numeric_limits<double>::epsilon() * 100 * x0;
        }
        if (dx_max == 0.0)
            dx_max = odesys->get_dx_max(x0, y0);
        if (mxsteps == 0)
            mxsteps = 500;
        auto integr = Integr<OdeSys>(odesys, dx0, dx_max, atol, rtol, styp, mxsteps, autorestart, return_on_error);
        auto result = integr.adaptive(x0, xend, y0);
        odesys->current_info.clear();
        set_integration_info<OdeSys>(odesys, integr);
        return result;
    }

    template <class OdeSys>
    int simple_predefined(OdeSys * const odesys,
                          const double atol,
                          const double rtol,
                          const StepType styp,
                          const double * const y0,
                          const int nout,
                          const double * const xout,
                          double * const yout,
                          long int mxsteps=0,
                          double dx0=0.0,
                          double dx_max=0.0,
                          int autorestart=0,
                          bool return_on_error=false
                          )
    // const double dx_min=0.0,
    {
        if (dx0 == 0.0)
            dx0 = odesys->get_dx0(xout[0], y0);
        if (dx0 == 0.0){
            if (xout[0] == 0)
                dx0 = std::numeric_limits<double>::epsilon() * 100;
            else
                dx0 = std::numeric_limits<double>::epsilon() * 100 * xout[0];
        }
        if (dx_max == 0.0)
            dx_max = INFINITY;
        if (mxsteps == 0)
            mxsteps = 500;
        auto integr = Integr<OdeSys>(odesys, dx0, dx_max, atol, rtol, styp, mxsteps, autorestart, return_on_error);
        int nreached = integr.predefined(nout, xout, y0, yout);
        odesys->current_info.nfo_int.clear();
        odesys->current_info.nfo_dbl.clear();
        set_integration_info(odesys, integr);
        return nreached;
    }

}

#endif /* ODEINT_ANYODE_H_6D2AAAD4880011E6AC5C734FA77443A3 */
