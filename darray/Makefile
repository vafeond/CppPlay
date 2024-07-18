# header only module
# Makefile included solely to enable:
# - gathering of metrics
# - automatic style formatting

METRICS_EXTRA_FILES_RELATIVE=./include/darray.hpp
FORMAT_EXTRA_FILES_RELATIVE=$(METRICS_EXTRA_FILES_RELATIVE)
ANALYZE_EXTRA_FILES_RELATIVE=$(METRICS_EXTRA_FILES_RELATIVE)


# boilerplate for build support
BUILD_ROOT:=$(dir $(wildcard $(addsuffix /build/Makefile_Config.mk, . .. ../.. ../../..)))
ifeq (,$(BUILD_ROOT))
$(error Build system BUILD_ROOT could not be detected! Where is Makefile_Config.mk? )
endif
include $(BUILD_ROOT)Makefile_Config.mk
include $(BUILD_ROOT)Makefile_Metrics.mk
include $(BUILD_ROOT)Makefile_Format.mk
include $(BUILD_ROOT)Makefile_Analyze.mk
