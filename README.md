# Cpp Play  

Some basic utilities to experiment with modern c++ features. Includes:  
- `darray`: Dynamic array with optional thread protection.  
    - Thread protection safe guards concurrent reads and mutations via locked Mutex.  
    - When thread protection not enabled it's mechanisms are excluded by the compiler, incurring no cost.  
    - Iterators are not protected, any mutation of the data structure invalidates existing iterators.  
    - Code:
        - Utility source: `darray/include/darray.hpp`  
        - Unit tests: `darray/_utest/*.cc`
        - Benchmarks: `darray/benchmark/main.cc` (no warm up configured, run multiple times)

## Quick Start  
Install dependencies:  
- `Docker`  
- `GNU Make`  

Create and enter Docker environment. From repository root directory:  
- `make cbuild`, build Docker image  
- `make ccreate`, create Docker container  
- `make cexec`, enter running Docker container  

Exercise the code from within the container:  
- `make test`, run unit tests  
- `make test coverage`, run unit tests and display code coverage  
- `make test SANITIZE=address:leak`, run unit tests with address and leak sanitizers
- `make bench`, run benchmarks  
- `make analyze`, perform static analysis  
    - Results stored in `bin/analyze/analyze_results.csv` directory below target  
- `make metrics`, generate metrics 
    - Metrics stored in `bin/metrics/metrics_results.csv` directory below target  
    - Final column is Cyclomatic Complexity  

## Build System  
See `build/README.md`.

## IDE Support    
See `devtools/README.md`.

## TODO  
- Algorithm requiring a random access iterator, such as a heap, search, or sort. Tested with `darray`.

# License  
Everything here is [Unlicense](https://unlicense.org).
If a file doesn't contain the Unlicense header, assume Unlicense unless 
otherwise stated.

So, use it at will. Credit it here if you want.
