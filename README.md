# File Compressor in C

## A full file compressor and decompressor written in C using the Huffman coding algorithm!

This is a project for [Stardance](https://stardance.hackclub.com/projects/17270) written in C! You can compress and decompress your files using the Huffman algorithm, which is a lossless compression algorithm.

## Table of Contents

- [How to use](#how-to-use)
  - [1. Clone this repository](#1-clone-this-repository)
  - [2. Install the necessary tools](#2-install-the-necessary-tools)
  - [3. Compile the code](#3-compile-the-code)
  - [4. Usage](#4-usage)
- [How it works](#how-it-works)
- [Issues](#issues)
- [License](#license)

## How to use

### 1. Clone this repository

- **HTTPS**
```bash
git clone https://github.com/danielbarseghian/file-compressor.git
cd file-compressor
```

- **SSH**
```bash
git clone git@github.com:danielbarseghian/file-compressor.git
cd file-compressor
```

### 2. Install the necessary tools

You'll need `gcc` (or another C compiler) and `make` installed.

- **Windows**

  1. Install [MSYS2](https://www.msys2.org/).
  2. Open the MSYS2 terminal and run:
     ```bash
     pacman -Syu
     pacman -S mingw-w64-ucrt-x86_64-gcc make
     ```
  3. Add MSYS2's `ucrt64/bin` folder to your system `PATH` so `gcc` and `make` are available from any terminal.

  Alternatively, you can install [WSL](https://learn.microsoft.com/en-us/windows/wsl/install) and follow the Linux instructions below.

- **Linux**

  - Ubuntu / Debian:
    ```bash
    sudo apt update
    sudo apt install build-essential
    ```
  - Arch:
    ```bash
    sudo pacman -S base-devel
    ```
  - Fedora:
    ```bash
    sudo dnf groupinstall "Development Tools"
    ```

- **Mac**

  Install Xcode Command Line Tools, which include `gcc`/`clang` and `make`:

  ```bash
  xcode-select --install
  ```

### 3. Compile the code

```bash
make compressor
make decompressor
```

### 4. Usage

- **Compress**

  ```bash
  ./compressor uncompressed out-name
  ```

- **Decompress**

  ```bash
  ./decompressor compressed out-name
  ```

> ⚠️ The input and output files must use the same file extension.

## Issues

If you encounter ANY issues, please report them in the [Issues](../../issues) tab.