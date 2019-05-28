#!/bin/bash
make clean
if make; then
if [[ $1 == 1 ]] ; then
cd apps
./restart.sh
cd ..
fi
fi
