#ifndef ODEINT_NUMPY_H_ND3O2SRHSNG6JEWSGHNN43PK5E
#define ODEINT_NUMPY_H_ND3O2SRHSNG6JEWSGHNN43PK5E
#include <Python.h>
#include <numpy/arrayobject.h>

#include <functional>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/odeint.hpp>

namespace odeint_numpy{
    using namespace std::placeholders;

    using boost::numeric::odeint::integrate_adaptive;
    using boost::numeric::odeint::make_dense_output;
    using boost::numeric::odeint::rosenbrock4;
    using boost::numeric::odeint::runge_kutta_dopri5;


    // value_type is hardcoded to double at the moment (see NPY_DOUBLE and py_arglist)
    typedef double value_type;
    typedef boost::numeric::ublas::vector<value_type> vector_type;
    typedef boost::numeric::ublas::matrix<value_type> matrix_type;

    template<typename system_t, typename stepper_t>
    class PyIntegr {
    protected:
        system_t system;
        PyObject *py_rhs, *py_jac;
        void obs(const vector_type &yarr, value_type xval) {
            xout.push_back(xval);
            for(size_t i=0 ; i<yarr.size() ; ++i)
                yout.push_back(yarr[i]);
        }
    public:
        const int ny;
        std::vector<value_type> xout;
        std::vector<value_type> yout;
        void rhs(const vector_type &yarr, vector_type &dydx, value_type xval) const;
        void jac(const vector_type & yarr, matrix_type &Jmat,
               const value_type & xval, vector_type &dfdx) const;
        PyIntegr(PyObject * py_rhs, PyObject * py_jac, int ny, system_t sys) :
            system(sys), py_rhs(py_rhs), py_jac(py_jac), ny(ny) {}

        size_t adaptive(PyObject *py_y0, value_type x0, value_type xend,
                   value_type dx0, value_type atol, value_type rtol){
            vector_type y0(ny);
            for (npy_intp i=0; i<ny; ++i)
                y0[i] = *((double*)PyArray_GETPTR1(py_y0, i));
            auto stepper = make_dense_output<stepper_t>(atol, rtol);
            auto obs_cb = std::bind(&PyIntegr::obs, this, _1, _2);
            return integrate_adaptive(stepper, this->system, y0, x0, xend, dx0, obs_cb);
        }

        void predefined(PyObject *py_y0, PyObject *py_xout, PyObject *yout,
                        value_type dx0, value_type atol, value_type rtol){
            vector_type y0(ny);
            for (npy_intp i=0; i<ny; ++i)
                y0[i] = *((double*)PyArray_GETPTR1(py_y0, i));

            const npy_intp nx = PyArray_DIMS(py_xout)[0];
            vector_type xout(nx);
            for (npy_intp i=0; i<nx; ++i)
                xout[i] = *((double*)PyArray_GETPTR1(py_xout, i));
            auto stepper = make_dense_output<stepper_t >(atol, rtol);
            for (npy_intp ix=0; ix<nx-1; ++ix){
                integrate_const(stepper, this->system, y0, xout[ix], xout[ix+1], dx0);
                for (npy_intp iy=0; iy<ny; ++iy){
                    auto elem = static_cast<double*>(PyArray_GETPTR2(yout, ix+1, iy));
                    *elem = y0[iy];
                }
            }
        }
    };

    class PyOdeintRosenbrock4 : public PyIntegr<
        std::pair<std::function<void(const vector_type &, vector_type &, value_type)>,
                  std::function<void(const vector_type &, matrix_type &,
                                     const value_type &, vector_type &)> >,
        rosenbrock4<value_type>
        > {
    public:
        PyOdeintRosenbrock4(PyObject * py_rhs, PyObject * py_jac, int ny) :
            PyIntegr(py_rhs, py_jac, ny,
                     std::make_pair(std::bind(&PyIntegr::rhs, this, _1, _2, _3),
                                    std::bind(&PyIntegr::jac, this, _1, _2, _3, _4))) {}
    };

    class PyOdeintDopri5 : public PyIntegr<
        std::function<void(const vector_type &, vector_type &, value_type)>,
        runge_kutta_dopri5< vector_type, value_type >
        > {
    public:
        PyOdeintDopri5(PyObject * py_rhs, int ny) :
            PyIntegr(py_rhs, nullptr, ny, std::bind(&PyIntegr::rhs, this, _1, _2, _3)) {}
    };
}

template<typename T1, typename T2>
void
odeint_numpy::PyIntegr<T1, T2>::rhs(const vector_type &yarr, vector_type &dydx,
                                     value_type xval) const
{
    npy_intp dims[1] { this-> ny };
    PyObject * py_yarr = PyArray_SimpleNewFromData(
        1, dims, NPY_DOUBLE, const_cast<value_type *>(&(yarr.data()[0])));
    PyObject * py_dydx = PyArray_SimpleNewFromData(
        1, dims, NPY_DOUBLE, const_cast<value_type *>(&(dydx.data()[0])));
    PyObject * py_arglist = Py_BuildValue("(dOO)", xval, py_yarr, py_dydx);
    PyObject * py_result = PyEval_CallObject(this->py_rhs, py_arglist);
    Py_DECREF(py_arglist);
    Py_DECREF(py_dydx);
    Py_DECREF(py_yarr);
    if (py_result == nullptr){
        PyErr_SetString(PyExc_RuntimeError, "rhs() failed");
        return;
    } else if (py_result != Py_None){
        // py_result is not None
        PyErr_SetString(PyExc_RuntimeError, "rhs() did not return None");
    }
    Py_DECREF(py_result);
}

template<typename T1, typename T2>
void
odeint_numpy::PyIntegr<T1, T2>::jac(const vector_type & yarr, matrix_type &Jmat,
                                     const value_type & xval, vector_type &dfdx) const
{
    npy_intp ydims[1] { this->ny };
    npy_intp Jdims[2] { this->ny, this->ny };
    PyObject * py_yarr = PyArray_SimpleNewFromData(1, ydims, NPY_DOUBLE,
                                                   const_cast<value_type *>(&(yarr.data()[0])));
    PyObject * py_jmat = PyArray_SimpleNewFromData(2, Jdims, NPY_DOUBLE,
                                                   const_cast<value_type *>(&(Jmat.data()[0])));
    PyObject * py_dfdx = PyArray_SimpleNewFromData(1, ydims, NPY_DOUBLE,
                                                   const_cast<value_type *>(&(dfdx.data()[0])));
    PyObject * py_arglist = Py_BuildValue("(dOOO)", xval, py_yarr, py_jmat, py_dfdx);
    PyObject * py_result = PyEval_CallObject(this->py_jac, py_arglist);
    Py_DECREF(py_arglist);
    Py_DECREF(py_dfdx);
    Py_DECREF(py_jmat);
    Py_DECREF(py_yarr);
    if (py_result == nullptr){
        PyErr_SetString(PyExc_RuntimeError, "jac() failed");
        return;
    } else if (py_result != Py_None){
        // py_result is not None
        PyErr_SetString(PyExc_RuntimeError, "jac() did not return None");
    }
    Py_DECREF(py_result);
}

#endif /* ODEINT_NUMPY_H_ND3O2SRHSNG6JEWSGHNN43PK5E */
