# ==================================================================
#  NetCDF Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/netcdf/lib/libnetcdf_c++4.a" ]; then

  git clone https://github.com/Unidata/netcdf-cxx4.git

  cd netcdf-cxx4
  autoreconf -if
  ./configure --prefix=$HOME/netcdf

  make -j2
  make check
  make install
  cd ..
fi