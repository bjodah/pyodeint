#!/bin/bash -xeu
PKG_NAME=${1:-${CI_REPO##*/}}
if [[ "$CI_BRANCH" =~ ^v[0-9]+.[0-9]?* ]]; then
    eval export ${PKG_NAME^^}_RELEASE_VERSION=\$CI_BRANCH
    echo ${CI_BRANCH} | tail -c +2 > __conda_version__.txt
fi

cd tests/; make CXX=clang++-10 EXTRA_FLAGS=-fsanitize=address; make clean; cd -
cd tests/; make CXX=clang++-10 EXTRA_FLAGS=-fsanitize=undefined; make clean; cd -

export CC=gcc-10
export CXX=g++-10

cd tests/; make EXTRA_FLAGS=-D_GLIBCXX_DEBUG; make clean; cd -
cd tests/; make EXTRA_FLAGS=-DNDEBUG; make clean; cd -

python3 setup.py sdist
python3 -m pip install --ignore-installed dist/*.tar.gz
(cd /; python3 -m pytest --pyargs $PKG_NAME)
python3 -m pip install --user -e .[all]
PYTHON=python3 ./scripts/run_tests.sh --cov $PKG_NAME --cov-report html
./scripts/coverage_badge.py htmlcov/ htmlcov/coverage.svg

./scripts/render_notebooks.sh examples/
(cd examples/; ../scripts/render_index.sh *.html)
./scripts/generate_docs.sh
