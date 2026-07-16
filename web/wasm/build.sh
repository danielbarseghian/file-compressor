#!/bin/bash

emcc ../../core/compressor.c \
  -O2 \
  -o Compressor/compressor.js \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='compressor' \
  -s INVOKE_RUN=0 \
  -s EXPORTED_RUNTIME_METHODS='["callMain", "FS"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s MAXIMUM_MEMORY=2GB \
  -s FORCE_FILESYSTEM=1 \
  -s WASMFS=1