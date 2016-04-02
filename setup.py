#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Tested with boost v1.59.0

import io
import os
import shutil
import sys
from setuptools import setup
from setuptools.extension import Extension


pkg_name = 'pyodeint'


def _path_under_setup(*args):
    return os.path.join(os.path.dirname(__file__), *args)


def missing_or_any_other_newer(path, other_paths):
    """
    Investigate if path is non-existant or older than any
    provided reference paths.

    Parameters
    ----------
    path: string
        Path to path which might be missing or too old.
    other_paths: iterable of strings
        Reference paths.

    Returns
    -------
    True if path is older or missing.
    """
    if not os.path.exists(path):
        return True
    path_mtime = os.path.getmtime(path)
    for other_path in map(os.path.abspath, other_paths):
        if os.path.getmtime(other_path) - 1e-6 >= path_mtime:
            # 1e-6 is needed beacuse of:
            # http://stackoverflow.com/questions/17086426/
            return True
    return False

USE_CYTHON = os.path.exists(_path_under_setup(
    'pyodeint', '_odeint_numpy.pyx.template'))

# Cythonize .pyx file if it exists (not in source distribution)
ext_modules = []
if '--help' not in sys.argv[1:] and sys.argv[1] not in (
        '--help-commands', 'egg_info', 'clean', '--version'):
    import numpy as np
    if USE_CYTHON:
        source = _path_under_setup('pyodeint', '_odeint_numpy.pyx')
        # render source from template:
        if missing_or_any_other_newer(source, [source + '.template',
                                               source + '.methods']):
            io.open(source, 'wt', encoding='utf-8').write(
                '# rendered from template, do not edit\n' +
                io.open(source + '.template', 'rt',
                        encoding='utf-8').read().replace(
                            '${INTEGRATOR_METHODS}',
                            io.open(source + '.methods', 'rt',
                                    encoding='utf-8').read()))
    else:
        source = _path_under_setup('pyodeint', '_odeint_numpy.cpp')

    ext_modules = [Extension('pyodeint._odeint_numpy',
                             [source],
                             language='c++',
                             extra_compile_args=['-std=c++11'],
                             include_dirs=[np.get_include(), './include'])]
    if USE_CYTHON:
        from Cython.Build import cythonize
        ext_modules = cythonize(ext_modules, include_path=['./include'],
                                gdb_debug=True)

RELEASE_VERSION = os.environ.get('PYODEINT_RELEASE_VERSION', '')

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
    install_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
    setup_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
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
