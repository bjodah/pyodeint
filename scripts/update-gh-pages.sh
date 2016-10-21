#!/bin/bash -x
#
# Usage:
#
#    $ ./scripts/update-gh-pages.sh v0.6.0 origin
#

tag=${1:-master}
remote=${2:-origin}
hold="dist build"
ori_branch=$(git rev-parse --symbolic-full-name --abbrev-ref HEAD)
tmpdir=$(mktemp -d)
cleanup() {
    rm -r $tmpdir
}
trap cleanup INT TERM

cp -r doc/_build/html/ $tmpdir
mv $hold $tmpdir
if [[ -d .gh-pages-skeleton ]]; then
    cp -r .gh-pages-skeleton $tmpdir
fi

git checkout gh-pages
if [[ $? -ne 0 ]]; then
    git checkout --orphan gh-pages
    if [[ $? -ne 0 ]]; then
        >&2 echo "Failed to switch to 'gh-pages' branch."
        cleanup
        exit 1
    fi
    preexisting=0
else
    preexisting=1
fi

if [[ $preexisting == 1 ]]; then
    while [[ "$(git log -1 --pretty=%B)" == Volatile* ]]; do
        # overwrite previous docs
        git reset --hard HEAD~1
    done
fi

shopt -s extglob
git clean -xfd
if [[ $preexisting == 1 ]]; then
    git rm -rf !(v*) > /dev/null
fi
cp -r $tmpdir/html/ $tag
if [[ -d $tmpdir/.gh-pages-skeleton ]]; then
    cp -r $tmpdir/.gh-pages-skeleton/. .
fi
if [[ "$tag" == v* ]]; then
    ln -s $tag latest
    commit_msg="Release docs for $tag"
else
    if [[ $preexisting == 1 ]]; then
        commit_msg="Volatile ($tag) docs"
    else
        commit_msg="Initial commit"
    fi
fi
git add -f . >/dev/null
git commit -m "$commit_msg"
if [[ $preexisting == 1 ]]; then
    git push -f $remote gh-pages
else
    git push --set-upstream $remote gh-pages
fi
git checkout $ori_branch
for f in $hold; do
    mv $tmpdir/$f .
done
cleanup