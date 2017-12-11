# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/vtk/lib/libvtkCommonCore-8.0.so" ]; then
  echo 'Installing VTK-lib...';
  # git clone https://gitlab.kitware.com/vtk/vtk
  # cd vtk
  # git checkout v8.0.0
  
  VTK_MAJOR_VERSION=8.0
  VTK_SUB_VERSION=1
  VTK_VERSION=${VTK_MAJOR_VERSION}.${VTK_SUB_VERSION}
  VTK_FILE_NAME=VTK-${VTK_VERSION}
  VTK_ARCHIVE_NAME=${VTK_FILE_NAME}.tar.gz

  echo "http://www.vtk.org/files/release/${VTK_MAJOR_VERSION}/${VTK_ARCHIVE_NAME}"
  if [ ! -e "${VTK_ARCHIVE_NAME}" ]; then
    wget http://www.vtk.org/files/release/${VTK_MAJOR_VERSION}/${VTK_ARCHIVE_NAME}
    tar xf ${VTK_ARCHIVE_NAME}
  fi
  cd ${VTK_FILE_NAME}

  mkdir -p build
  cd build

  BUILD_DIR=$HOME/vtk
  mkdir -p "${BUILD_DIR}"
  cmake -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}" \
  					  -DCMAKE_BUILD_TYPE=Release \
  					  -DVTK_SMP_IMPLEMENTATION_TYPE=OpenMP \
              -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
              ../
  make -j4
  make install
  # cd ../..
  # rm -R ${VTK_FILE_NAME}
fi
