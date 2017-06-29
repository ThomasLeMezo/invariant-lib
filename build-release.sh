# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
sh $INVARIANTLIB_DIR/cmake/build_IbexLib.sh
sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
cd $INVARIANTLIB_DIR

mkdir build/build-release -p
cd build/build-release
cmake -DCMAKE_INSTALL_PREFIX=~/ -DIBEX_ROOT=~/ibex -DVTK_DIR=${HOME}/vtk -DCMAKE_BUILD_TYPE=RELEASE $INVARIANTLIB_DIR
make -j
cd ..
