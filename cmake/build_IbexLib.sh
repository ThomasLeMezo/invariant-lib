# ==================================================================
#  tubex-lib - IBEX-lib installation
# ==================================================================

#!/bin/bash

#set -x # debugging

if [ ! -e "$HOME/ibex/lib/libibex.a" ]; then
  echo 'Installing the IBEX-lib...';
  git clone https://github.com/ibex-team/ibex-lib.git
  cd ibex-lib

  ./waf configure --prefix=${HOME}/ibex --interval-lib=gaol
  ./waf build install clean -j4
fi