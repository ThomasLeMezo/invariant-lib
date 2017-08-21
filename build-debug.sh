# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
sh $INVARIANTLIB_DIR/cmake/build_IbexLib.sh
# sh $INVARIANTLIB_DIR/cmake/build_Ibex4pyIbex.sh # Benoit Desrochers version
# sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
cd $INVARIANTLIB_DIR

mkdir build/build-debug -p
cd build/build-debug
cmake -DCMAKE_INSTALL_PREFIX=${HOME} -DIBEX_ROOT=${HOME}/ibex -DVTK_ROOT=${HOME}/vtk -DCMAKE_BUILD_TYPE=DEBUG -DBUILD_TESTS=ON $INVARIANTLIB_DIR
make -j4
cd ..
