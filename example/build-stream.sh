#!/bin/bash

cd STREAM
make stream_c.exe
cd ../..
make
cd example
cp ../noise .
cp STREAM/stream_c.exe .
