# -*- coding: utf-8 -*-
from __future__ import (absolute_import, division, print_function)

from math import exp


def test_PyDecay():
    import pyximport
    pyximport.install()
    from _odeint_anyode import PyDecay

    pd = PyDecay(1.0)
    tout, yout = pd.adaptive(1.0, 1.0)
    for t, y in zip(tout, yout):
        assert abs(y - exp(-t)) < 2e-9


def test_PyDecay_mxsteps():
    import pyximport
    pyximport.install()
    from _odeint_anyode import PyDecay

    import pytest
    pd = PyDecay(1.0)
    with pytest.raises(Exception):
        tout, yout = pd.adaptive(1.0, 1.0, mxsteps=1)


if __name__ == '__main__':
    test_PyDecay()
    test_PyDecay_mxsteps()
