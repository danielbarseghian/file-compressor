#!/bin/bash
mkdir -p dist

emcc ../core/compressor.c \
  -O2 \
  -o dist/compressor.js \
  -s MODULARIZE=1 \
  -s EXPORT_NAME='compressor' \
  -s INVOKE_RUN=0 \
  -s EXPORTED_RUNTIME_METHODS='["callMain", "FS"]' \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s FORCE_FILESYSTEM=1