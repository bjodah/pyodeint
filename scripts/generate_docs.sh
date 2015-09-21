#!/bin/bash -xe
PKG=$(find . -maxdepth 2 -name __init__.py -print0 | xargs -0 -n1 dirname | xargs basename)
AUTHOR=$(head -n 1 AUTHORS)
sphinx-apidoc --full --force -A "$AUTHOR" --doc-version=$(python setup.py --version) -F -o doc $PKG/
sed -i 's/Contents/.. include:: ..\/README.rst\n\nContents/g' doc/index.rst
sed -i "s/'sphinx.ext.viewcode',/'sphinx.ext.viewcode',\n    'numpydoc',/g" doc/conf.py
( cd doc; make html )
