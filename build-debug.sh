# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
sh $INVARIANTLIB_DIR/cmake/build_Ibex4pyIbex.sh # Benoit Desrochers version
sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
sh $INVARIANTLIB_DIR/cmake/build_ppl.sh
cd $INVARIANTLIB_DIR

mkdir build/build-debug -p
cd build/build-debug
cmake -DCMAKE_INSTALL_PREFIX=${HOME} \
	  -DIBEX_ROOT=${HOME}/ibex \
	  -DVTK_ROOT=${HOME}/vtk \
	  -DPPL_ROOT=${HOME}/ppl \
	  -DGMP_ROOT=${HOME}/gmp \
	  -DCMAKE_BUILD_TYPE=DEBUG \
	  -DBUILD_TESTS=OFF \
	  -DWITH_PYTHON=OFF \
	  -DWITH_3D=ON \
	  -DWITH_GRAPHIZ=OFF \
	  -DWITH_NETCDF=ON \
  	  -DWITH_PYIBEX_VERSION=ON \
  	  -DWITH_EXAMPLES=ON \
  	  -DWITH_PPL=ON \
	  $INVARIANTLIB_DIR
make -j4 "$1"
cd ..
