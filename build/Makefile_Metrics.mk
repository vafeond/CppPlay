
ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included after Makefile_Config.mk)
endif

metrics:
	$(info )
	$(info [ Metrics ] =============================================)
	$(info SOURCES                     : $(SOURCES))
	$(info METRICS_EXTRA_FILES_RELATIVE: $(METRICS_EXTRA_FILES_RELATIVE))
	$(info )
	mkdir -p $(BIN_METRICS_PATH)
	cd $(BIN_METRICS_PATH);$(PYTHON) /home/metrixplusplus/metrixplusplus-1.7.1/metrix++.py collect --std.code.complexity.cyclomatic $(abspath $(METRICS_EXTRA_FILES_RELATIVE)) $(abspath $(SOURCES))
	cd $(BIN_METRICS_PATH);$(PYTHON) /home/metrixplusplus/metrixplusplus-1.7.1/metrix++.py export $(abspath $(METRICS_EXTRA_FILES_RELATIVE)) $(abspath $(SOURCES)) > $(BIN_METRICS_RESULTS_FILENAME)

.PHONY: clean metrics

# include clean jic metrics is included by itself
# when included in a heirarchy that defines a clean target
# like Makefile_Executable.mk, an ignore/override warning
# will be issues by Make
# Ensure Makefile heirarchy is ordered such that the desired
# clean is picked up
clean:
	rm -rf $(BIN_PATH)
