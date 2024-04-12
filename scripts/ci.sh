#!/bin/bash -xeu

export CPLUS_INCLUDE_PATH=$(compgen -G "/opt-3/boost-1.*/include")

PKG_NAME=${1:-${CI_REPO_NAME##*/}}


cd tests/; make CXX=clang++ EXTRA_FLAGS="-fsanitize=address"; make clean; cd -
cd tests/; make CXX=clang++ EXTRA_FLAGS="-fsanitize=undefined"; make clean; cd -

export CC=gcc
export CXX=g++

cd tests/; make EXTRA_FLAGS=-D_GLIBCXX_DEBUG; make clean; cd -
cd tests/; make EXTRA_FLAGS=-DNDEBUG; make clean; cd -




source /opt-3/cpython-v3.11-apt-deb/bin/activate

python3 setup.py sdist
python3 -m pip install --ignore-installed dist/*.tar.gz
(cd /; python3 -m pytest --pyargs $PKG_NAME)
python3 -m pip install --user -e .[all]
PYTHON=python3 ./scripts/run_tests.sh --cov $PKG_NAME --cov-report html
./scripts/coverage_badge.py htmlcov/ htmlcov/coverage.svg

./scripts/render_notebooks.sh examples/
(cd examples/; ../scripts/render_index.sh *.html)
./scripts/generate_docs.sh

if [[ ! $(python3 setup.py --version) =~ ^[0-9]+.* ]]; then
    set -x
    >&2 echo "Bad version string?: $(python3 setup.py --version)"
    exit 1
fi
