# ==================================================================
#  Proj4 Installation
# ==================================================================

#!/bin/bash

#set -x # debugging
# apt install sqlite3
# -DBUILD_LIBPROJ_SHARED=off

if [ ! -e "$HOME/proj4/lib/libproj.a" ]; then

  echo 'Installing Proj4...';
  PROJ4_VERSION="6.0.0"
  PROJ4_FILE_NAME=proj-${PROJ4_VERSION}
  PROJ4_ARCHIVE_NAME=${PROJ4_FILE_NAME}.tar.gz

  wget https://download.osgeo.org/proj/${PROJ4_ARCHIVE_NAME}

  tar xf ${PROJ4_ARCHIVE_NAME}
  rm ${PROJ4_ARCHIVE_NAME}
  cd ${PROJ4_FILE_NAME}

  mkdir build
  cd build
  cmake -DBUILD_LIBPROJ_SHARED=off \
        -DCMAKE_INSTALL_PREFIX=${HOME}/proj4 \
        -DCMAKE_BUILD_TYPE=RELEASE \
        -DBUILD_CCT=OFF \
        -DBUILD_CS2CS=OFF \
        -DBUILD_GEOD=OFF \
        -DBUILD_GIE=OFF \
        -DBUILD_GMOCK=OFF \
        -DPROJ_TESTS=OFF \
        -DBUILD_PROJ=OFF \
        -DBUILD_PROJINFO=OFF \
         ..
  cmake --build . -- -j 4
  cd ..
  make install
fi