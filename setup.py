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
from setuptools.command.build_ext import build_ext
from setuptools.extension import Extension
try:
    import cython
except ImportError:
    _HAVE_CYTHON = False
else:
    _HAVE_CYTHON = True
    assert cython  # silence pep8

pkg_name = 'pyodeint'


def _path_under_setup(*args):
    return os.path.join(*args)

_src = {ext: _path_under_setup(pkg_name, '_odeint.' + ext) for ext in "cpp pyx".split()}
if _HAVE_CYTHON and os.path.exists(_src["pyx"]):
    # Possible that a new release of Python needs a re-rendered Cython source,
    # or that we want to include possible bug-fix to Cython, disable by manually
    # deleting .pyx file from source distribution.
    USE_CYTHON = True
    if os.path.exists(_src['cpp']):
        os.unlink(_src['cpp'])  # ensure c++ source is re-generated.
else:
    USE_CYTHON = False

package_include = os.path.join(pkg_name, 'include')

ext_modules = []
if len(sys.argv) > 1 and '--help' not in sys.argv[1:] and sys.argv[1] not in (
        '--help-commands', 'egg_info', 'clean', '--version'):
    import numpy as np
    sources = [_src["pyx" if USE_CYTHON else "cpp"]]
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
    if (boost_root := os.environ.get('Boost_ROOT', '')) != '':
        ext_modules[0].include_dirs += [boost_root.rstrip('/') + '/include']

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
                __version__ = re.sub(r'v([0-9.]+)-(\d+)-(\w+)', r'\1.post\2+\3', _git_version)  # .dev < '' < .post

class BuildExt(build_ext):
    """A custom build extension for adding compiler-specific options."""
    c_opts = {
        'msvc': ['/EHsc'],
        'unix': [],
    }

    def build_extensions(self):
        ct = self.compiler.compiler_type
        opts = self.c_opts.get(ct, [])
        if ct == 'unix':
            opts.append('-DVERSION_INFO="%s"' % self.distribution.get_version())
            opts.append('-std=c++17')
            if sys.platform == 'darwin' and re.search("clang", self.compiler.compiler[0]) is not None:
                opts += ['-stdlib=libc++', '-mmacosx-version-min=10.7']
        elif ct == 'msvc':
            opts.append('/DVERSION_INFO=\\"%s\\"' % self.distribution.get_version())
        for ext in self.extensions:
            ext.extra_compile_args = opts
        build_ext.build_extensions(self)


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
    packages=[pkg_name, f'{pkg_name}.include'] + tests,
    include_package_data=True,
    install_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
    setup_requires=['numpy'] + (['cython'] if USE_CYTHON else []),
    extras_require={'docs': ['Sphinx', 'sphinx_rtd_theme']},
    ext_modules=ext_modules,
    cmdclass={'build_ext': BuildExt}
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
