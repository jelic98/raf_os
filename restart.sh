#!/bin/bash
make clean
if make; then
if [[ $1 == 1 ]] ; then
cd apps
./run_tools.sh
cd ..
fi
fi
