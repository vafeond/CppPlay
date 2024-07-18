# top-level Makefile, supporting:
# - environment-level targets
# - building and testing all sub-projects
# - project-level targets

# boilerplate for build support
BUILD_ROOT:=$(dir $(wildcard $(addsuffix /build/Makefile_Config.mk, . .. ../.. ../../..)))
ifeq (,$(BUILD_ROOT))
$(error Build system BUILD_ROOT could not be detected! Where is Makefile_Config.mk? )
endif
include $(BUILD_ROOT)Makefile_Config.mk

$(info MAKEFILE_PATHS_FROM_ROOT=$(MAKEFILE_PATHS_FROM_ROOT))

.PHONY: debug release clean test coverage


#
# build all but those excluded (opt-out)
#
# BUILD_EXCLUDES is a comma-separated list of relatve Makefile paths
# that should not be invoked by the build targets
BUILD_EXCLUDES=darray
BUILD_DEBUG_EXCLUDES=darray/benchmark
debug:
	$(info Building all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(filter-out $(BUILD_EXCLUDES) $(BUILD_DEBUG_EXCLUDES),$(MAKEFILE_PATHS_FROM_ROOT)))) | xargs --delimiter=: -IARG make -C ARG debug $(COVERAGE_QUALIFIER)

release:
	$(info Building all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(filter-out $(BUILD_EXCLUDES),$(MAKEFILE_PATHS_FROM_ROOT)))) | xargs --delimiter=: -IARG make -C ARG release $(COVERAGE_QUALIFIER)


#
# test all subdirectories that meet test application naming convention of:
# - _utest: unit tests
# - _itest: integration tests
# - _etest: end-to-end tests
# invoke with coverage if requested
#
COVERAGE_QUALIFIER=
ifneq (,$(filter coverage,$(MAKECMDGOALS)))
COVERAGE_QUALIFIER=coverage
endif
TEST_PATHS_RAW!=find . -type d -name _*test 
test:
	$(info Testing all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(TEST_PATHS_RAW))) | xargs --delimiter=: -IARG make -C ARG test $(COVERAGE_QUALIFIER)

test_debug:
	$(info Testing all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(TEST_PATHS_RAW))) | xargs --delimiter=: -IARG make -C ARG test_debug $(COVERAGE_QUALIFIER)

test_release:
	$(info Testing all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(TEST_PATHS_RAW))) | xargs --delimiter=: -IARG make -C ARG test_release

coverage:
ifeq (,$(filter test test_debug,$(MAKECMDGOALS))) # if coverage run with test or test_debug it's run as part of that target, so skip here
	$(info Coverage for all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(TEST_PATHS_RAW))) | xargs --delimiter=: -IARG make -C ARG coverage
endif


#
# metrics for those included (opt-in)
#
# METRICS_INCLUDES is a comma-separated list of relatve Makefile paths
# that should be invoked by the metrics target
METRICS_INCLUDES=darray
metrics:
	$(info Metrics for all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(METRICS_INCLUDES)))  | xargs --delimiter=: -IARG make -C ARG metrics


#
# benchmark for those included (opt-in)
#
# BENCHMARK_INCLUDES is a comma-separated list of relatve Makefile paths
# that should be invoked by the metrics target
BENCHMARK_INCLUDES=darray/benchmark
bench:
	$(info Benchmarks for all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(BENCHMARK_INCLUDES)))  | xargs --delimiter=: -IARG make -C ARG bench


#
# format for those included (opt-in)
#
# FORMAT_INCLUDES is a comma-separated list of relatve Makefile paths
# that should be invoked by the metrics target
FORMAT_INCLUDES=darray darray/_utest
format:
	$(info Format all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(FORMAT_INCLUDES)))  | xargs --delimiter=: -IARG make -C ARG format


#
# analyze for those included (opt-in)
#
# ANALYZE_INCLUDES is a comma-separated list of relatve Makefile paths
# that should be invoked by the metrics target
ANALYZE_INCLUDES=darray
analyze:
	$(info Analyze all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(ANALYZE_INCLUDES)))  | xargs --delimiter=: -IARG make -C ARG analyze

#
# clean all
#
clean:
	$(info Cleaning all...)
	echo -n $(subst $(subst ,, ),:,$(strip $(MAKEFILE_PATHS_FROM_ROOT))) | xargs --delimiter=: -IARG make -C ARG clean


# configure and include development container targets
STACK_PATH=$(CURDIR)
IMAGE_NAME=cpp_patdf_utils-dev
CONTAINER_NAME=cpp_patdf_utils-dev-container
include build/Makefile_DevContainer.mk

# included development IDE container targets if present
IMAGE_NAME_IDE=$(IMAGE_NAME)-ide
CONTAINER_NAME_IDE=$(CONTAINER_NAME)-ide
-include devtools/Makefile_DevContainerIde.mk

