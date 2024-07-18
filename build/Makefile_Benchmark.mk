.DEFAULT_GOAL=bench

ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
include $(BUILD_ROOT)Makefile_Config.mk

ifeq (,$(TARGET))
TARGET=bench
endif

# include GoogleTest from extra_sources
GBENCH_PATH=benchmark-$(GBENCH_VERSION)
EXTRA_SOURCE_PATHS+= $(GBENCH_PATH)/src
EXTRA_SOURCE_INCLUDE_PATHS+=$(GBENCH_PATH)/include

ifneq (,$(FILTER))
GTEST_FILTER=--benchmark_filter=$(FILTER)
endif

EXECUTABLE_VARIANT=Benchmark
include $(BUILD_ROOT)Makefile_Executable.mk

.PHONY: bench

bench: $(TARGET_FULL_PATH) 
	./$(TARGET_FULL_PATH) $(GBENCH_FILTER)

clean:
	rm -rf $(BIN_PATH)
