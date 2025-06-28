# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with
code in this repository.

## Project Overview

Colm (COmputer Language Machinery) is a programming language and compiler
construction toolkit designed for analysis and transformation of computer
languages. It's written in C/C++ and generates C code that compiles to native
executables.

## Build Commands

```bash
# Initial setup (only needed once)
./autogen.sh
./configure

# Build the project
make

# Run tests
make check

# Install colm
make install

# Clean build artifacts
make clean
```

## Development Commands

```bash
# Compile a Colm program
colm file.lm

# Compile and run immediately
colm -r file.lm

# Compile only (don't produce binary)
colm -c file.lm

# Generate dot format for visualization
colm -V file.lm
```

## Testing

```bash
# Run all tests
make check

# Run tests directly with parallel execution
./test/runtests

# Run specific test suite
cd test/colm.d && ../../test/runtests
```

## Code Architecture

### Core Components

1. **Compiler Pipeline** (src/):
   - `parsedata.cc/h`: Main parser data structures
   - `parsetree.cc/h`: Parse tree construction
   - `synthesis.cc`: Code synthesis and generation
   - `bytecode.cc/h`: Bytecode generation for the VM
   - `pdarun.c`: Runtime parser driver

2. **Runtime System** (src/):
   - `colm.c`: Main runtime entry point
   - `tree.c`: Tree manipulation runtime
   - `input.c`: Input stream handling
   - `program.c`: Program execution runtime

3. **Code Generation** (src/):
   - `libfsm/`: Finite state machine library for code generation
   - `cgil/`: Colm sources for code generation intermediate language

### Key Abstractions

- **Parse Trees**: Central data structure for language processing
- **Virtual Machine**: Stack-based VM for executing transformations
- **Input Streams**: Abstraction for handling various input sources
- **Patterns**: Grammar-based pattern matching for transformations

### File Extensions

- `.lm`: Colm language source files
- `.cgil`: Code generation intermediate language files
- `.rl`: Ragel state machine files

## Coding Conventions

- Use tabs for indentation
- Spaces for alignment after first non-whitespace character
- Maximum 100 characters per line
- No trailing whitespace
- Function blocks start on new line
- C-style comments for documentation
- C++ style comments for disabled code

## Important Notes

- The project uses GNU autotools (autoconf, automake, libtool)
- Colm is self-hosting - it uses itself to build parts of the compiler
- The test suite is comprehensive and uses expected output comparison
- Runtime requires GCC as Colm generates C code
- The project includes example grammars for C++, Go, Python, Rust, and other languages in the `grammar/` directory
