#!/bin/bash
cc -o ../../bin/ModelImporter -lm -mtune=native -march=native -O3 -fomit-frame-pointer -I../../Deps -I../../Include main.c
