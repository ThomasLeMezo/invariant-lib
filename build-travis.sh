# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
# sh $INVARIANTLIB_DIR/cmake/build_IbexLib.sh
sh $INVARIANTLIB_DIR/cmake/build_Ibex4pyIbex.sh # Benoit Desrochers version
# sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
cd $INVARIANTLIB_DIR

mkdir build/build-release -p
cd build/build-release
cmake -DCMAKE_INSTALL_PREFIX=${HOME} \
	  -DIBEX_ROOT=${HOME}/ibex \
	  -DCMAKE_BUILD_TYPE=RELEASE \
	  -DBUILD_TESTS=OFF \
	  -DWITH_PYTHON=ON \
	  -DWITH_3D=OFF \
	  -DWITH_GRAPHIZ=OFF\
	  -DWITH_NETCDF=OFF \
	  -DWITH_PYIBEX_VERSION=ON \
	  $INVARIANTLIB_DIR
make -j2
cd ..
