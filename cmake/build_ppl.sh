# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/gmp/lib/libgmp.so" ]; then
  echo 'Installing GMP-lib...';
  GMP_VERSION="6.1.2"
  GMP_FILE_NAME=gmp-${GMP_VERSION}
  GMP_ARCHIVE_NAME=${GMP_FILE_NAME}.tar.xz

  wget https://gmplib.org/download/gmp/${GMP_ARCHIVE_NAME}
  tar xf ${GMP_ARCHIVE_NAME}
  rm ${GMP_ARCHIVE_NAME}
  cd ${GMP_FILE_NAME}

  ./configure --prefix=$HOME/gmp --enable-cxx CXXFLAGS="-O2" CFLAGS="-O2" CPPFLAGS="-O2" --with-gnu-ld --with-pic
  make
  make check
  make install
  cd ..
fi


if [ ! -e "$HOME/ppl/lib/libppl.so" ]; then
  echo 'Installing PPL-lib...';

  if [ ! -e "ppl" ]; then
    git clone git://git.cs.unipr.it/ppl/ppl.git
    cd ppl
    git pull
    git checkout devel
  else
    cd ppl
  fi
  
  autoreconf
  ./configure --prefix=$HOME/ppl --enable-interfaces=cxx --enable-optimization=sspeed --enable-fpmath=default \
      --disable-ppl_lpsol --disable-ppl_lcdd --enable-thread-safe --enable-shared --with-cxxflags=-fPIC --with-cflags=-fPIC \
      --with-gmp=$HOME/gmp --disable-documentation
  make -j4
  make install
fi


