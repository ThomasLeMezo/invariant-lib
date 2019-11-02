# ==================================================================
#  NetCDF Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/netcdf/lib/libnetcdf_c++4.a" ]; then
  nc-config --has-nc4

  git clone https://github.com/Unidata/netcdf-cxx4.git

  cd netcdf-cxx4
  git checkout v4.3.1
  autoreconf -if
  ./configure --prefix=$HOME/netcdf --with-gnu-ld

  make -j2
  make check
  make install
  cd ..

  # -DCMAKE_INSTALL_PREFIX=${HOME} -
fi