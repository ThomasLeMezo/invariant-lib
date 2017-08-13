# ==================================================================
#  tubex-lib - IBEX-lib installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/ibex/lib/libibex.a" ]; then
  echo 'Installing the IBEX-lib...';
  git clone https://github.com/ibex-team/ibex-lib.git
  cd ibex-lib
  git checkout ibex-lib-v2.4.3

  ./waf configure --prefix=${HOME}/ibex --with-ensta-robotics --enable-shared
  ./waf build install clean
fi