#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Tested with boost v1.59.0

import io
import os
import re
import shutil
import subprocess
import sys
import warnings
from setuptools import setup
from setuptools.extension import Extension


pkg_name = 'pyodeint'


def _path_under_setup(*args):
    return os.path.join(os.path.dirname(__file__), *args)

USE_CYTHON = os.path.exists(_path_under_setup('pyodeint', '_odeint.pyx'))
package_include = os.path.join(pkg_name, 'include')

# Cythonize .pyx file if it exists (not in source distribution)
ext_modules = []
if len(sys.argv) > 1 and '--help' not in sys.argv[1:] and sys.argv[1] not in (
        '--help-commands', 'egg_info', 'clean', '--version'):
    import numpy as np
    ext = '.pyx' if USE_CYTHON else '.cpp'
    sources = [os.path.join(pkg_name, '_odeint' + ext)]
    ext_modules = [Extension('%s._odeint' % pkg_name, sources)]
    if USE_CYTHON:
        from Cython.Build import cythonize
        ext_modules = cythonize(ext_modules, include_path=[
            package_include,
            os.path.join('external', 'anyode', 'cython_def')
        ])
    ext_modules[0].language = 'c++'
    ext_modules[0].extra_compile_args = ['-std=c++11']
    ext_modules[0].define_macros = [('ANYODE_NO_LAPACK', '1')]
    ext_modules[0].include_dirs = [package_include, np.get_include(),
                                   os.path.join('external', 'anyode', 'include')]

RELEASE_VERSION = os.environ.get('%s_RELEASE_VERSION' % pkg_name.upper(), '')

# http://conda.pydata.org/docs/build.html#environment-variables-set-during-the-build-process
CONDA_BUILD = os.environ.get('CONDA_BUILD', '0') == '1'
if CONDA_BUILD:
    try:
        RELEASE_VERSION = 'v' + open(
            '__conda_version__.txt', 'rt').readline().rstrip()
    except IOError:
        pass

release_py_path = _path_under_setup(pkg_name, '_release.py')

if len(RELEASE_VERSION) > 1 and RELEASE_VERSION[0] == 'v':
    TAGGED_RELEASE = True
    __version__ = RELEASE_VERSION[1:]
else:
    TAGGED_RELEASE = False
    # read __version__ attribute from _release.py:
    exec(open(release_py_path).read())
    if __version__.endswith('git'):
        try:
            _git_version = subprocess.check_output(
                ['git', 'describe', '--dirty']).rstrip().decode('utf-8').replace('-dirty', '.dirty')
        except subprocess.CalledProcessError:
            warnings.warn("A git-archive is being installed - version information incomplete.")
        else:
            if 'develop' not in sys.argv:
                warnings.warn("Using git to derive version: dev-branches may compete.")
                __version__ = re.sub('v([0-9.]+)-(\d+)-(\w+)', r'\1.post\2+\3', _git_version)  # .dev < '' < .post

classifiers = [
    "Development Status :: 4 - Beta",
    'License :: OSI Approved :: BSD License',
    'Operating System :: OS Independent',
    'Topic :: Scientific/Engineering',
    'Topic :: Scientific/Engineering :: Mathematics',
]

tests = [
    'pyodeint.tests',
]

with io.open(_path_under_setup(pkg_name, '__init__.py'), 'rt',
             encoding='utf-8') as f:
    short_description = f.read().split('"""')[1].split('\n')[1]
assert 10 < len(short_description) < 255
long_descr = io.open(_path_under_setup('README.rst'), encoding='utf-8').read()
assert len(long_descr) > 100

setup_kwargs = dict(
    name=pkg_name,
    version=__version__,
    description=short_description,
    long_description=long_descr,
    classifiers=classifiers,
    author='Björn Dahlgren',
    author_email='bjodah@DELETEMEgmail.com',
    license='BSD',
    url='https://github.com/bjodah/' + pkg_name,
    packages=[pkg_name] + tests,
    include_package_data=True,
    install_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
    setup_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
    extras_require={'docs': ['Sphinx', 'sphinx_rtd_theme']},
    ext_modules=ext_modules,
)

if __name__ == '__main__':
    try:
        if TAGGED_RELEASE:
            # Same commit should generate different sdist
            # depending on tagged version (set PYODEINT_RELEASE_VERSION)
            # this will ensure source distributions contain the correct version
            shutil.move(release_py_path, release_py_path+'__temp__')
            open(release_py_path, 'wt').write(
                "__version__ = '{}'\n".format(__version__))
        setup(**setup_kwargs)
    finally:
        if TAGGED_RELEASE:
            shutil.move(release_py_path+'__temp__', release_py_path)
