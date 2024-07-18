
ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included after Makefile_Config.mk)
endif

ANALYZER_BUILD_FLAGS=-std=c++23 -I/usr/include/c++/13 -I/usr/include/x86_64-linux-gnu/c -D__cplusplus=202300L -D__cpp_concepts=202002L

analyze:
	$(info )
	$(info [ Analysis ] =============================================)
	$(info SOURCES                     : $(SOURCES))
	$(info ANALYZE_EXTRA_FILES_RELATIVE: $(ANALYZE_EXTRA_FILES_RELATIVE))
	$(info )
	mkdir -p $(BIN_ANALYZE_PATH)
	$(ANALYZER) $(abspath $(ANALYZE_EXTRA_FILES_RELATIVE)) $(abspath $(SOURCES)) -- $(ANALYZER_BUILD_FLAGS) | tee $(BIN_ANALYZE_RESULTS_PATH)

.PHONY: clean analyze

# include clean jic metrics is included by itself
# when included in a heirarchy that defines a clean target
# like Makefile_Executable.mk, an ignore/override warning
# will be issues by Make
# Ensure Makefile heirarchy is ordered such that the desired
# clean is picked up
clean:
	rm -rf $(BIN_PATH)
