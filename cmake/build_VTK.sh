# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

  VTK_MAJOR_VERSION=8.0
  VTK_SUB_VERSION=0
  VTK_VERSION=${VTK_MAJOR_VERSION}.${VTK_SUB_VERSION}
  VTK_FILE_NAME=VTK-${VTK_VERSION}
  VTK_ARCHIVE_NAME=${VTK_FILE_NAME}.tar.gz


if [ ! -e "$HOME/vtk/lib/libvtkCommonCore-${VTK_MAJOR_VERSION}.a" ]; then
  echo 'Installing VTK-lib...';

  echo "http://www.vtk.org/files/release/${VTK_MAJOR_VERSION}/${VTK_ARCHIVE_NAME}"
  wget http://www.vtk.org/files/release/${VTK_MAJOR_VERSION}/${VTK_ARCHIVE_NAME}
  tar xf ${VTK_ARCHIVE_NAME}
  cd ${VTK_FILE_NAME}

  mkdir -p build
  cd build

  BUILD_DIR=$HOME/vtk
  mkdir -p "${BUILD_DIR}"
  cmake -DCMAKE_INSTALL_PREFIX="${BUILD_DIR}" \
								-DCMAKE_BUILD_TYPE=Release \
								-DVTK_SMP_IMPLEMENTATION_TYPE=OpenMP \
								-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
								-DBUILD_SHARED_LIBS=OFF \
								-DVTK_Group_Rendering=OFF \
								-VTK_Group_Imaging=OFF \
								-VTK_Group_MPI=OFF \
								-VTK_Group_Qt=OFF \
								-VTK_Group_Rendering=OFF \
								-VTK_Group_Tk=OFF \
								-VTK_Group_Views=OFF \
								-VTK_Group_Web=OFF \
								-VTK_IOS_BUILD=OFF \
								../
  make -j4
  make install
  # cd ../..
  # rm -R ${VTK_FILE_NAME}
fi
