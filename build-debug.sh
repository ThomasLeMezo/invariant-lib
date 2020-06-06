# ==================================================================
#  invariant-lib - build script
# ==================================================================

#!/bin/bash

SCRIPT="$(readlink -f "$0")"
INVARIANTLIB_DIR="$(dirname "$SCRIPT")"

cd ..
sh $INVARIANTLIB_DIR/cmake/build_ibex.sh
sh $INVARIANTLIB_DIR/cmake/build_VTK.sh
sh $INVARIANTLIB_DIR/cmake/build_ppl.sh
sh $INVARIANTLIB_DIR/cmake/build_netcdf.sh
sh $INVARIANTLIB_DIR/cmake/build_proj4.sh
cd $INVARIANTLIB_DIR

mkdir build/build-debug -p
cd build/build-debug
cmake -DCMAKE_INSTALL_PREFIX=${HOME} \
	  -DIBEX_ROOT=${HOME}/ibex \
	  -DVTK_ROOT=${HOME}/vtk \
	  -DPPL_ROOT=${HOME}/ppl \
	  -DGMP_ROOT=${HOME}/gmp \
	  -DPROJ4_ROOT=${HOME}/proj4 \
	  -DNETCDF_ROOT=${HOME}/netcdf \
	  -DCMAKE_BUILD_TYPE=DEBUG \
	  -DBUILD_TESTS=OFF \
	  -DWITH_PYTHON=OFF \
	  -DWITH_3D=ON \
	  -DWITH_GRAPHIZ=ON \
	  -DWITH_NETCDF=ON \
  	  -DWITH_PYIBEX_VERSION=ON \
  	  -DWITH_PROJ4=ON\
  	  -DWITH_EXAMPLES=ON \
  	  -DWITH_PPL=ON \
	  $INVARIANTLIB_DIR

if [ -z "$1" ]; then
	make -j4
else
	make -j4 "$1"
fi
cd ..
