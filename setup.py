#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Tested with boost v1.59.0

import sys
from distutils.core import setup

import numpy as np
from Cython.Build import cythonize

ext_modules = []
if '--help' not in sys.argv[1:] and sys.argv[1] not in (
            '--help-commands', 'egg_info', 'clean', '--version'):
    ext_modules = cythonize("pyodeint/*.pyx", include_path=['./include'],
                            gdb_debug=True)

setup_kwargs = dict(
    ext_modules=ext_modules,
    include_dirs=[np.get_include(), './include']
)

if __name__ == '__main__':
    setup(**setup_kwargs)
