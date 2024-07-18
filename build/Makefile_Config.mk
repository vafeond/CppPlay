#  This is free and unencumbered software released into the public domain.
#
#  Anyone is free to copy, modify, publish, use, compile, sell, or
#  distribute this software, either in source code form or as a compiled
#  binary, for any purpose, commercial or non-commercial, and by any
#  means.
#
#  In jurisdictions that recognize copyright laws, the author or authors
#  of this software dedicate any and all copyright interest in the
#  software to the public domain. We make this dedication for the benefit
#  of the public at large and to the detriment of our heirs and
#  successors. We intend this dedication to be an overt act of
#  relinquishment in perpetuity of all present and future rights to this
#  software under copyright law.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#  For more information, please refer to <https://unlicense.org>

ifneq (true,$(MAKEFILE_CONFIG_INCLUDED))
MAKEFILE_CONFIG_INCLUDED=true

# ensure we're only building for debug or release
ifneq (,$(filter debug run_debug,$(MAKECMDGOALS)))
ifneq (,$(filter release run_release,$(MAKECMDGOALS)))
$(error Can only build for debug or release, not both!)
endif
endif

# build tool configuration
CPP=g++-13
LINKER=g++-13
ARCHIVER=ar
COVERAGE=gcov-13
COVERAGE_CFLAGS=-fprofile-arcs -ftest-coverage
FORMATTER=clang-format-17
ANALYZER=clang-tidy-17
PYTHON=python3

# general configuration
ifeq (,$(GTEST_VERSION))
GTEST_VERSION=1.15.0
endif
ifeq (,$(GBENCH_VERSION))
GBENCH_VERSION=1.8.5
endif

CFLAGS_COMMON=-std=c++23 -Wall -Wextra -Wpedantic -fstack-protector-strong -fstack-clash-protection -mshstk -fconcepts-diagnostics-depth=2 $(CFLAGS_SANITIZE)
CPPFLAGS_COMMON= 
LFLAGS_COMMON=$(LFLAGS_SANITIZE)
ifeq (,$(filter release run_release test_release bench,$(MAKECMDGOALS)))
# if not release do debug
CONFIG=debug
CFLAGS=$(CFLAGS_COMMON) -g $(CFLAGS_EXTRA)
CPPFLAGS=-DDEBUG $(CPPFLAGS_COMMON) $(CPPFLAGS_EXTRA)
LFLAGS=$(LFLAGS_COMMON)
ifneq (,$(filter coverage,$(MAKECMDGOALS)))
CONFIG:=$(CONFIG)coverage
CFLAGS+=$(COVERAGE_CFLAGS)
SYSLIBS=gcov
endif
else
# otherwise do release
CONFIG=release
CFLAGS+=$(CFLAGS_COMMON) -O3 $(CFLAGS_EXTRA)
CPPFLAGS=-DNDEBUG $(CPPFLAGS_COMMON) $(CPPFLAGS_EXTRA)
LFLAGS=$(LFLAGS_COMMON)
ifneq (,$(filter coverage,$(MAKECMDGOALS)))
$(error 'coverage' is not allowed in release tests)
endif
endif

# add sanitizers, comma separated list passed in like "SANITIZE=address:leak"
SANITIZER_LIST:=$(subst :, ,$(SANITIZE))
ifneq (,$(filter address,$(SANITIZER_LIST)))
CONFIG:=$(CONFIG)sanitizeaddress
CFLAGS_SANITIZE:=$(CFLAGS_SANITIZE) -fsanitize=address
LFLAGS_SANITIZE:=$(LFLAGS_SANITIZE) -fsanitize=address
endif
ifneq (,$(filter leak,$(SANITIZER_LIST)))
CONFIG:=$(CONFIG)sanitizeleak
# CFLAGS_SANITIZE=-fsanitize=leak
LFLAGS_SANITIZE:=$(LFLAGS_SANITIZE) -fsanitize=leak
endif
ifneq (,$(filter thread,$(SANITIZER_LIST)))
CONFIG:=$(CONFIG)sanitizethread
CFLAGS_SANITIZE:=$(CFLAGS_SANITIZE) -fsanitize=thread
LFLAGS_SANITIZE:=$(LFLAGS_SANITIZE) -fsanitize=thread
endif
ifneq (,$(filter useafterscope,$(SANITIZER_LIST)))
CONFIG:=$(CONFIG)sanitizeuseafterscope
CFLAGS_SANITIZE:=$(CFLAGS_SANITIZE) -fsanitize-address-use-after-scope
LFLAGS_SANITIZE:=$(LFLAGS_SANITIZE) -fsanitize-address-use-after-scope
endif

BIN_PATH:=bin
BIN_CONFIG_PATH:=bin/$(CONFIG)
# binary config padding is needed to support extra_sources which has
# a relative path that contains ../, so need to add some depth to
# binary directory structure so the created artifacts (e.g. objects and deps) 
# are properly contained withing the bin/$(CONFIG) directory
# this whole thing is due to how make pattern rules work
# below dervies the needed padding based on the deepest Makefile
MAKEFILE_PATHS_RAW!=find $(BUILD_ROOT).. -name Makefile 
# $(info MAKEFILE_PATHS_RAW=$(MAKEFILE_PATHS_RAW))
MAKEFILE_PATHS_FROM_ROOT:=$(sort $(strip $(subst Makefile,,$(subst /Makefile,,$(subst $(BUILD_ROOT)../,,$(MAKEFILE_PATHS_RAW))))))
# $(info MAKEFILE_PATHS_FROM_ROOT=$(MAKEFILE_PATHS_FROM_ROOT))
MAKEFILE_PATH_LONGEST:=$(lastword $(MAKEFILE_PATHS_FROM_ROOT))
# $(info MAKEFILE_PATH_LONGEST=$(MAKEFILE_PATH_LONGEST))
DERIVED_PADDING=$(subst $(subst ,, ),,$(patsubst %,p/,$(subst /, ,$(MAKEFILE_PATH_LONGEST))))
# $(info DERIVED_PADDING=$(DERIVED_PADDING))
BIN_CONFIG_PAD:=$(DERIVED_PADDING)
# if above starts failing can always hardcode as below, and extend number of p's as needed
# BIN_CONFIG_PAD=p/p/p/
# $(info BIN_CONFIG_PAD=$(BIN_CONFIG_PAD))

BIN_COVERAGE_PATH=$(BIN_CONFIG_PATH)/coverage
BIN_COVERAGE_RESULTS_FILENAME=coverage_results.txt
BIN_COVERAGE_RESULTS_PATH=$(BIN_COVERAGE_PATH)/$(BIN_COVERAGE_RESULTS_FILENAME)

BIN_METRICS_PATH=$(BIN_PATH)/metrics
BIN_METRICS_RESULTS_FILENAME=metrics_results.csv
BIN_METRICS_RESULTS_PATH=$(BIN_METRICS_PATH)/$(BIN_METRICS_RESULTS_FILENAME)

BIN_ANALYZE_PATH=$(BIN_PATH)/analyze
BIN_ANALYZE_RESULTS_FILENAME=analyze_results.csv
BIN_ANALYZE_RESULTS_PATH=$(BIN_ANALYZE_PATH)/$(BIN_ANALYZE_RESULTS_FILENAME)

endif # MAKEFILE_CONFIG_INCLUDED
