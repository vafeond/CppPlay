.DEFAULT_GOAL=debug

ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif

ifeq (,$(TARGET))
TARGET=sample
endif

EXECUTABLE_VARIANT=Sample
include $(BUILD_ROOT)Makefile_Executable.mk

.PHONY: debug release run run_debug run_release clean

debug: $(TARGET_FULL_PATH)
release: $(TARGET_FULL_PATH)

run: $(TARGET_FULL_PATH) 
	./$(TARGET_FULL_PATH)

run_debug: run
run_release: run

clean:
	rm -rf $(BIN_PATH)

