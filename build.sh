#!/bin/bash

if [ "$#" -ne 1 ] ; then
        echo "$0: Insert 'phase1' or 'phase2' or 'phase3' as argument"
        exit 3
fi

if [ $1 != "phase1" -a $1 != 'phase2' -a $1 != 'phase3' ] ; then
    echo "$0: Argument not valid, only 'phase1', 'phase2', 'phase3' are allowed"
    exit 1
else
    rm -rf ./build
    cmake -DPHASE=$1 -S ./ -B ./build/
    cd ./build
    make
    exit 0
fi