#!/bin/bash

set -e -x

cd $HOME
for PYBIN in /opt/python/cp3*/bin; do
  
  #if [ "${PYBIN}" != "/opt/python/cp36-cp36m/bin" ]; then
  #  continue
  #fi

  "${PYBIN}/python" -m pip install --upgrade pip
  "${PYBIN}/python" -m pip install pyibex==1.8.1
  mkdir -p build_dir && cd build_dir

  /usr/local/bin/cmake -DCMAKE_INSTALL_PREFIX=${HOME} \
                      -DIBEX_ROOT=${HOME}/ibex-lib \
                      -DVTK_ROOT=${HOME}/vtk \
                      -DPPL_ROOT=${HOME}/ppl \
                      -DGMP_ROOT=${HOME}/gmp \
                      -DNETCDF_ROOT=${HOME}/netcdf \
                      -DPROJ4_ROOT=${HOME}/proj4 \
                      -DPYTHON_EXECUTABLE=${PYBIN}/python \
                      -DPYBIND11_CPP_STANDARD=$CPP_VERSION \
                      -DCMAKE_BUILD_TYPE=RELEASE \
                      -DBUILD_TESTS=OFF \
                      -DWITH_PYTHON=ON \
                      -DWITH_3D=ON \
                      -DWITH_GRAPHIZ=OFF \
                      -DWITH_NETCDF=ON \
                      -DWITH_PYIBEX_VERSION=ON \
                      -DWITH_PPL=ON \
                      -DWITH_PROJ4=ON \
                      -DWITH_EXAMPLES=OFF \
                      -DWITH_IPEGENERATOR=OFF \
                      ${TRAVIS_BUILD_DIR}
  make

  make pip_package
  echo "copy wheel and clean build_dir"
  for whl in *.whl; do
    auditwheel repair "$whl" -w /io/wheelhouse/
  done
  
  "${PYBIN}/python" -m pip install pyinvariant --no-deps --no-index -f /io/wheelhouse
  (cd "$HOME"; "${PYBIN}/python" -m unittest discover tubex_lib.tests)
  cd /io
  rm -fr build_dir

done