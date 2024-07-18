
INCLUDE_PATHS=../include
COVERAGE_FILES=darray.hpp
METRICS_EXTRA_FILES_RELATIVE=../include/darray.hpp


# boilerplate for build support
BUILD_ROOT:=$(dir $(wildcard $(addsuffix /build/Makefile_Config.mk, . .. ../.. ../../..)))
ifeq (,$(BUILD_ROOT))
$(error Build system BUILD_ROOT could not be detected! Where is Makefile_Config.mk? )
endif
include $(BUILD_ROOT)Makefile_Test.mk
include $(BUILD_ROOT)Makefile_Format.mk

