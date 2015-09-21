v0.4.1
======
- jacobian callback may now be None for DOPRI5 and Bulirsch-Stoer (not used anyway).

v0.4
====
- Breaking Python API change: ny no longer needed in integrate_predefined/integrate_adaptive
- Excepctions thrown from C++ are now propagated to Python.

v0.3
====
- Breaking Python API change: "stepper" kwarg renamed to "method" (to follow SciPy better)
- Cython source rendered from template.

v0.2
====
- 'steppers' keyword added to integrate_adaptive
- integrate_predefined added
- integrate_adaptive have had its arguments reordered
- dopri5 stepper also available

v0.1
====
- Integration using adaptive step-size and the Rosenbrock4 stepper supported.
