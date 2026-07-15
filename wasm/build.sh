#!/bin/bash
mkdir -p dist

emcc ../core/decompressor.c \
  -O2 \
  -o Decompressor/decompressor.js \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='decompressor' \
  -s INVOKE_RUN=0 \
  -s EXPORTED_RUNTIME_METHODS='["callMain", "FS"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s FORCE_FILESYSTEM=1