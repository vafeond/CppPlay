# Build System
Build system infrastructure is stored in `<repo root>/build`

Supports:
- Building CppPlay Utils static libraries
- Building and running unit test applications using Google Test framework
- Building and running sample applications
- "extra_sources" that have the following properties:
    - don't form part of the CppPlay Utils but shouldn't be part of any single binary (e.g. would be duplicated)
    - generated objects are best stored alongside the objects of the incorporating binary 

## CppPlay Utils Library Structure
`<repo root>/`
- `<library name>`
  - `src`, source files. Subdirectories can be used to organize large libraries.
  - `include`, public headers needed by consumers of the utility library.

## Targets  
Below are the make targets supported for each supported binary type. The commands can be issued after navigating to the directory containing the binary Makefile. 

### Static Library  
- `make` and `make debug`: Build debug version of the library
- `make release`: Build a release version of the library
- `make clean`: Removes all build artifacts

### Test Application  
- `make`, `make test` and `make test_debug`: Build a debug version of the test application and run it
- `make test_release`: Build a release version of the test application and run it
- `make debug`: Build a debug version of the test application
- `make release`: Build a release version of the test application
- `make clean`: Removes all build artifacts
- To filter by test case name add variable `FILTER` with the filter specification:
  - `make test FILTER=TestCase1`
- `make test coverage`, `make test_debug coverage`: Create code coverage enabled objects, run tests with those objects, and report coverage for selected files/modules
  - `make coverage` can be called to report coverage on a previous run of coverage-enabled debug tests. If those objects and tests have not been run an error is returned.

### Sanitizers
- To enable supported sanitizers set `SANITIZE` variable when invoking `make`, for example:
    - `make test SANITIZE=address:leak` enable address and leak sanitizers
- List supported sanitizers as a colon-separated-value
- Supported sanitizers are:
    - `address`
    - `leak`
    - `thread`
- Some combinations of sanitizers are invalid, generating an error from the compiler or linker if specified together
- Generated objects are stored in `bin` subdirectory specific to the other specified targets (e.g. `release`) and combination of sanitizers
- Selection method intended to be used by developers or combination of runs performed by CI

### Sample Application  
- `make`, `make run` and `make run_debug`: Build a debug version of the test application and run it
- `make run_release`: Build a release version of the test application and run it
- `make debug`: Build a debug version of the sample application
- `make release`: Build a release version of the sample application
- `make clean`: Removes all build artifacts

### Metrics  
- Static metrics can be gathered for any sub-module. Include `Makefile_Metrics.mk` after makefile heirarchy would include `Makefile_Config.mk`
- Metrics will be displayed for all sources gathered for the module.
- Add extra files for metrics, such as header-only implementations, by setting variable `METRICS_EXTRA_FILES_RELATIVE`.
- `make metrics` measures the gathered and specified sources and headers, and gathers their metrics 

### Format Style  
- Style can be automatically formatted for any sub-module. Include `Makefile_Format.mk` after makefile heirarchy would include `Makefile_Config.mk`
- Formatting is applied for all sources gathered for the module.
- Add extra files for formatting, such as header-only implementations, by setting variable `FORMAT_EXTRA_FILES_RELATIVE`.
- `make format` formats the gathered and specified sources and headers in-place 

## Adding a binary  
- Follow an existing example for the type of binary you want to add, cloning and owning the directory structure and Makefiles
- Top-level Makefiles have a specific structure:
  - Set variables required to describe you binary (see below)
  - Copy code to detect the `BUILD_ROOT` (path to the build system Makefiles from you Makefile location)
  - Include the appropriate next-level Makefile:
    - `include $(BUILD_ROOT)Makefile_LibraryStatic.mk`, if creating a static library
    - `include $(BUILD_ROOT)Makefile_Test.mk`, if creating a test application
    - `include $(BUILD_ROOT)Makefile_Sample.mk`, is creating a sample application

## Variables  
The following variables can be used when configuring a top-level build file for a library or executable.

* `TARGET`: The name of the target to be built.
  * For executables this is the full name of the created executable
    * If not set when using `Makefile_Sample.mk` `TARGET` is set to `sample`
    * If not set when using `Makefile_Test.mk` `TARGET` is set to `test`
  * For libraries this is the library "name spec" and the appropriate prefix and suffix are emplaced
    * For example, the static library `TARGET=darray` becomes `libdarray.a`

* `SOURCE_PATHS`: Space-separated list of relative paths where source files to be included into the binary are found
  This excludes source files of dependent CppPlay Utils. See `CPPPLAY_UTIL_LIBS`.

* `INCLUDE_PATHS`: Space-separated list of relative paths where header files needed to build the binary are found
  This excludes header files of dependent CppPlay Utils. See `CPPPLAY_UTIL_LIBS`.

* `CPPPLAY_UTIL_LIBS`: Space-separated list of CppPlay Util libraries to be linked into the binary
  Use the library "name spec". For example, darray.
  Dependent CppPlay libraries are built if they are out of date.
  Dependent CppPlay library header paths are detected and included in the binary build.

* `EXTRA_SOURCE_PATHS`: Paths relative to `extra_sources` for additional source files to include

* `EXTRA_SOURCE_INCLUDE_PATHS`: Paths relative to `extra_sources` for additional source header files

* `GTEST_VERSION`: GoogleTest version to use. If omitted the default version is used.

* `CFLAGS_EXTRA`: Flags specific to the binary that must be passed to the compiler

* `CPPFLAGS_EXTRA`: Preprocessor flags specific to the binary that must be passed to the compiler

* `COVERAGE_FILES`: In Makefiles deriving/including `Makefile_Test.mk`, specify a space-separated list of files to report coverage for

* `METRICS_EXTRA_FILES_RELATIVE`: For module where static metrics are desired, include `Makefile_Metrics.mk` after makefile heirarchy would include `Makefile_Config.mk`, and set this variable to file to be include in metrics, that aren't gathered as part of the module sources.
