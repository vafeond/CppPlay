
ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif

include $(BUILD_ROOT)Makefile_Config.mk
include $(BUILD_ROOT)Makefile_GatherSources.mk
include $(BUILD_ROOT)Makefile_DepsHead.mk

# put created library in bin subdirectory
ifeq (,$(TARGET))
$(error Invoking Makefile must set TARGET)
endif
TARGET_FULL_PATH:=$(BIN_CONFIG_PATH)/lib$(TARGET).a

.PHONY: debug release clean

debug: $(TARGET_FULL_PATH)
release: $(TARGET_FULL_PATH)

clean:
	rm -rf $(BIN_PATH)

$(TARGET_FULL_PATH): $(OBJECTS_FULL_PATH)
	$(info )
	$(info [ Library ] =============================================)
	$(info TARGET   : $(TARGET))
	$(info CONFIG   : $(CONFIG))
	$(info ARCHIVER : $(ARCHIVER))
	$(info SOURCES  : $(SOURCES))
	$(info )
	$(ARCHIVER) rcs $@ $^ 

$(BIN_CONFIG_PATH)/obj/$(BIN_CONFIG_PAD)%.o: %.cc 
	mkdir -p $(@D)
	mkdir -p $(dir $(DEPDIR)/$*.d)
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) -c -o $@ $<


# must be last
include $(BUILD_ROOT)Makefile_DepsTail.mk
