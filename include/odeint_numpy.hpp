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


    // value_type is hardcoded to double at the moment (see NPY_DOUBLE and py_arglist)
    typedef double value_type;
    typedef boost::numeric::ublas::vector<value_type> vector_type;
    typedef boost::numeric::ublas::matrix<value_type> matrix_type;

    class PyOdeintRosenbrock4 {
        PyObject *py_f, *py_j;
        void f(const vector_type &yarr, vector_type &dydx, value_type xval) const;
        void j(const vector_type & yarr, matrix_type &Jmat,
               const value_type & xval, vector_type &dfdx) const;
        std::pair<std::function<void(const vector_type &, vector_type &, value_type)>,
                  std::function<void(const vector_type &, matrix_type &,
                                     const value_type &, vector_type &)> > system;

        void obs(const vector_type &yarr, value_type xval) {
            xout.push_back(xval);
            for(size_t i=0 ; i<yarr.size() ; ++i)
                yout.push_back(yarr[i]);
        }
    public:
        const int ny;
        std::vector<value_type> xout;
        std::vector<value_type> yout;

        PyOdeintRosenbrock4(PyObject * py_f, PyObject * py_j, int ny) :
            py_f(py_f), py_j(py_j),
            system(std::make_pair(std::bind(&PyOdeintRosenbrock4::f, this, _1, _2, _3),
                                  std::bind(&PyOdeintRosenbrock4::j, this, _1, _2, _3, _4))),
            ny(ny) {}
        size_t run(PyObject *py_y0, value_type x0, value_type xend,
                   value_type dx0, value_type atol, value_type rtol){
            vector_type y0(ny);
            for (npy_intp i=0; i<ny; ++i)
                y0[i] = *((double*)PyArray_GETPTR1(py_y0, i));
            auto stepper = make_dense_output<rosenbrock4<value_type> >(atol, rtol);
            auto obs_cb = std::bind(&PyOdeintRosenbrock4::obs, this, _1, _2);
            return integrate_adaptive(stepper, this->system, y0, x0, xend, dx0, obs_cb);
        }
    };
}
void
odeint_numpy::PyOdeintRosenbrock4::f(const vector_type &yarr, vector_type &dydx,
                                     value_type xval) const
{
    npy_intp dims[1] { this-> ny };
    PyObject * py_yarr = PyArray_SimpleNewFromData(
        1, dims, NPY_DOUBLE, const_cast<value_type *>(&(yarr.data()[0])));
    PyObject * py_dydx = PyArray_SimpleNewFromData(
        1, dims, NPY_DOUBLE, const_cast<value_type *>(&(dydx.data()[0])));
    PyObject * py_arglist = Py_BuildValue("(dOO)", xval, py_yarr, py_dydx);
    PyObject * py_result = PyEval_CallObject(this->py_f, py_arglist);
    Py_DECREF(py_arglist);
    Py_DECREF(py_dydx);
    Py_DECREF(py_yarr);
    if (py_result == nullptr){
        PyErr_SetString(PyExc_RuntimeError, "f() failed");
        return;
    } else if (py_result != Py_None){
        // py_result is not None
        PyErr_SetString(PyExc_RuntimeError, "f() did not return None");
    }
    Py_DECREF(py_result);
}
void
odeint_numpy::PyOdeintRosenbrock4::j(const vector_type & yarr, matrix_type &Jmat,
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
    PyObject * py_result = PyEval_CallObject(this->py_j, py_arglist);
    Py_DECREF(py_arglist);
    Py_DECREF(py_dfdx);
    Py_DECREF(py_jmat);
    Py_DECREF(py_yarr);
    if (py_result == nullptr){
        PyErr_SetString(PyExc_RuntimeError, "f() failed");
        return;
    } else if (py_result != Py_None){
        // py_result is not None
        PyErr_SetString(PyExc_RuntimeError, "f() did not return None");
    }
    Py_DECREF(py_result);
}

#endif /* ODEINT_NUMPY_H_ND3O2SRHSNG6JEWSGHNN43PK5E */
