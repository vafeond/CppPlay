.DEFAULT_GOAL=test

ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
include $(BUILD_ROOT)Makefile_Config.mk

ifeq (,$(TARGET))
TARGET=test
endif

# include GoogleTest from extra_sources
GTEST_PATH=googletest-$(GTEST_VERSION)/googletest
EXTRA_SOURCE_PATHS+= $(GTEST_PATH)/src
EXTRA_SOURCE_INCLUDE_PATHS+=$(GTEST_PATH) $(GTEST_PATH)/include $(GTEST_PATH)/include/gtest

ifneq (,$(FILTER))
GTEST_FILTER=--gtest_filter=$(FILTER)
endif

EXECUTABLE_VARIANT=Test
include $(BUILD_ROOT)Makefile_Executable.mk

.PHONY: debug release test test_debug test_release clean coverage

test: $(TARGET_FULL_PATH) 
	./$(TARGET_FULL_PATH) $(GTEST_FILTER)

test_debug: test
test_release: test

debug: $(TARGET_FULL_PATH)
release: $(TARGET_FULL_PATH)

coverage:
	$(info )
	$(info [ Coverage ] =============================================)
	$(info OBJECTS_FULL_PATH: $(OBJECTS_FULL_PATH))
	$(info COVERAGE_FILES   : $(COVERAGE_FILES))
	$(info )
	mkdir -p $(BIN_COVERAGE_PATH)
	$(COVERAGE) -H -k -m $(OBJECTS_FULL_PATH) > $(BIN_COVERAGE_PATH)/stdout.txt
	echo -n $(subst $(subst ,, ),:,$(strip $(COVERAGE_FILES))) | xargs --delimiter=: -IARG grep -A 1 "File.*ARG" $(BIN_COVERAGE_PATH)/stdout.txt | tee $(BIN_COVERAGE_RESULTS_PATH)
	mv *.gcov $(BIN_COVERAGE_PATH)
	# echo -n $(subst $(subst ,, ),:,$(strip $(COVERAGE_FILES))) | xargs --delimiter=: -IARG find $(BIN_COVERAGE_PATH) -type f ! -name 'ARG.gcov' -delete

clean:
	rm -rf $(BIN_PATH)
