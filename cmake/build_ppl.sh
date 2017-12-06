# ==================================================================
#  VTK Installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/ppl/lib/libppl.a" ]; then
  echo 'Installing PPL-lib...';
  VERSION=1.2
  FILE_NAME=ppl-${VERSION}
  ARCHIVE_NAME=${FILE_NAME}.tar.xz

  wget http://bugseng.com/products/ppl/download/ftp/releases/${VERSION}/${ARCHIVE_NAME}
  tar xf ${ARCHIVE_NAME}
  rm ${ARCHIVE_NAME}
  mkdir -p $HOME/ppl
  cd ${FILE_NAME}

  ./configure --prefix=$HOME/ppl --enable-interfaces=cxx --enable-optimization=sspeed --enable-fpmath=default \
    --disable-ppl_lpsol --disable-ppl_lcdd --enable-cxx --enable-thread-safe --enable-shared
  make -j4
  make install
  cd ..
  rm -R ${FILE_NAME}
fi



###

# git clone git://git.cs.unipr.it/ppl/ppl.git
# cd ppl
# git checkout devel
# autoreconf
# ./configure --prefix=$HOME/ppl --enable-interfaces=cxx --enable-optimization=sspeed --enable-fpmath=default \
#     --disable-ppl_lpsol --disable-ppl_lcdd --enable-cxx --enable-shared --enable-thread-safe
# make -j4
# make install