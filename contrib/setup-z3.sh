#!/bin/bash

# Z3_VERSION=6cc52e04c3ea7e2534644a285d231bdaaafd8714
Z3_VERSION=8e6f17ebd011a352b3e67fcec03dcd4378a1472c

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
DEPS=$DIR/../deps

mkdir -p $DEPS

if [ ! -d "$DEPS/z3" ]; then
    cd $DEPS
    git clone https://github.com/Z3Prover/z3.git
    chmod -R 777 z3
    cd z3
    git checkout -f $Z3_VERSION
    # a static binary linked to z3 often segfaults due to
    # a pthread related issue
    # compiling with --single-threaded helps, but isn't a real solution
    # see https://github.com/Z3Prover/z3/issues/4554
    ./configure --staticlib --single-threaded
    cd build
    make -j$(nproc)
    cd $DIR
else
    echo "$DEPS/z3 already exists. If you want to rebuild, please remove it manually."
fi
