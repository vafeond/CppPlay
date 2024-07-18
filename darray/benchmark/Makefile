
SOURCE_PATHS=.
INCLUDE_PATHS=../include


# boilerplate for executable build support
BUILD_ROOT:=$(dir $(wildcard $(addsuffix /build/Makefile_Config.mk, . .. ../.. ../../..)))
ifeq (,$(BUILD_ROOT))
$(error Build system BUILD_ROOT could not be detected! Where is Makefile_Config.mk? )
endif
include $(BUILD_ROOT)Makefile_Benchmark.mk

