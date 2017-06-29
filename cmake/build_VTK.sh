# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/vtk/bin/vtkH5detect" ]; then
  echo 'Installing VTK-lib...';
  git clone https://gitlab.kitware.com/vtk/vtk
  cd vtk
  git checkout v8.0.0.rc2
  BUILD_DIR=$HOME/vtk

  mkdir -p ${BUILD_DIR}
  cmake -DCMAKE_BUILD_DIRECTORY=${BUILD_DIR}

  cd ${BUILD_DIR}
  make -j4
fi
