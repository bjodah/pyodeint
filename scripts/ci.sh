#!/bin/bash
set -xeuo pipefail

export PATH="$(compgen -G /opt-2/gcc-??/bin):$PATH"
export CPLUS_INCLUDE_PATH=$(compgen -G "/opt-3/boost-1.*/include")

PKG_NAME=${1:-${CI_REPO_NAME##*/}}

source /opt-3/cpython-v3.11-apt-deb/bin/activate

#################### Install and test python package  ####################

python3 setup.py sdist
CC=gcc CXX=g++ python3 -m pip install --ignore-installed dist/*.tar.gz
(cd /; python3 -m pytest --pyargs $PKG_NAME)
CC=gcc CXX=g++ python3 -m pip install -e .[all]
python3 -m pip install pytest-cov pytest-flakes matplotlib sphinx numpydoc sphinx_rtd_theme
PYTHONPATH=$(pwd) PYTHON=python3 ./scripts/run_tests.sh --cov $PKG_NAME --cov-report html
./scripts/coverage_badge.py htmlcov/ htmlcov/coverage.svg

./scripts/render_notebooks.sh examples/
(cd examples/; ../scripts/render_index.sh *.html)
./scripts/generate_docs.sh

if [[ ! $(python3 setup.py --version) =~ ^[0-9]+.* ]]; then
    set -x
    >&2 echo "Bad version string?: $(python3 setup.py --version)"
    exit 1
fi

#################### Run stand-alone tests under ./tests/ ####################
cd tests/

make clean
make CC=gcc CXX=g++ EXTRA_FLAGS=-D_GLIBCXX_DEBUG

make clean
make CC=gcc CXX=g++ EXTRA_FLAGS=-DNDEBUG


LLVM_ROOT=$(compgen -G "/opt-2/llvm-??")

LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-asan")
make clean
make \
    CXX=clang++ \
    EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" \
    LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" \
    LDLIBS="-lc++" \
    OPENMP_LIB="-Wl,-rpath,${LLVM_ROOT}/lib -lomp" \
    PY_LD_PRELOAD=$(clang++ --print-file-name=libclang_rt.asan.so)

LIBCXX_ROOT=$(compgen -G "/opt-2/libcxx??-debug")
make clean
make \
    CXX=clang++ \
    EXTRA_FLAGS="-fsanitize=address -nostdinc++ -isystem ${LIBCXX_ROOT}/include/c++/v1" \
    LDFLAGS="-nostdlib++ -Wl,-rpath,${LIBCXX_ROOT}/lib -L${LIBCXX_ROOT}/lib" \
    LDLIBS="-lc++" \
    OPENMP_LIB="-Wl,-rpath,${LLVM_ROOT}/lib -lomp" \
    PY_LD_PRELOAD=$(clang++ --print-file-name=libclang_rt.asan.so)

cd -
