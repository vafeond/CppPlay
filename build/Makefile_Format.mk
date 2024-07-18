
ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif
ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included after Makefile_Config.mk)
endif

format:
	$(info )
	$(info [ Format ] =============================================)
	$(info SOURCES                    : $(SOURCES))
	$(info FORMAT_EXTRA_FILES_RELATIVE: $(FORMAT_EXTRA_FILES_RELATIVE))
	$(info )
	$(FORMATTER) -i --style=llvm $(abspath $(FORMAT_EXTRA_FILES_RELATIVE)) $(abspath $(SOURCES))

.PHONY: format
