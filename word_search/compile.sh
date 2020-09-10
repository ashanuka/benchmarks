#!/usr/bin/env bash

if [ "$GEM5_ROOT" != "" ];
then
    GEM5_INC="-I$GEM5_ROOT/include -DGEM5"
    GEM5_LIB="$GEM5_ROOT/util/m5/libm5.a"
else
    GEM5_INC=""
    GEM5_LIB=""
fi

if [ "$VL_ROOT" != "" ];
then
    VL_INC="-I$VL_ROOT/vl"
    VL_LIB="$VL_ROOT/libvl/libvl.a"
    HASVL=true
else
    VL_INC=""
    VL_LIB=""
    HASVL=false
fi

g++ -std=gnu++17 `pkg-config --cflags raftlib` $GEM5_INC -O0 -g $1.cpp -o $1_std `pkg-config --libs raftlib` $GEM5_LIB
if [ $HASVL ];
then
    g++ -std=gnu++17 `pkg-config --cflags raftlib` $GEM5_INC $VL_INC -O0 -g $1.cpp -o $1_vl -DVL `pkg-config --libs raftlib` $GEM5_LIB $VL_LIB
fi
