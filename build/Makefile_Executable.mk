
ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif

include $(BUILD_ROOT)Makefile_Config.mk
include $(BUILD_ROOT)Makefile_GatherSources.mk
include $(BUILD_ROOT)Makefile_CppPlayUtils.mk
include $(BUILD_ROOT)Makefile_DepsHead.mk

ifeq (,$(TARGET))
$(error Invoking Makefile must set TARGET)
endif
TARGET_FULL_PATH:=$(BIN_CONFIG_PATH)/$(TARGET)

$(TARGET_FULL_PATH): $(CPPPLAY_UTIL_LIBS_PATHS) $(OBJECTS_FULL_PATH) $(EXTRA_OBJECTS_FULL_PATH)
	$(info )
	$(info [ $(EXECUTABLE_VARIANT) Executable ] =============================================)
	$(info TARGET   : $(TARGET))
	$(info CONFIG   : $(CONFIG))
	$(info LINKER   : $(LINKER))
	$(info SOURCES  : $(SOURCES))
	$(info SYSLIBS  : $(SYSLIBS))
	$(info )
	$(LINKER) $(addprefix -L,$(CPPPLAY_UTIL_LIBS_DIR_PATHS)) -o $@ $(OBJECTS_FULL_PATH) $(EXTRA_OBJECTS_FULL_PATH) $(addprefix -l,$(CPPPLAY_UTIL_LIBS)) $(addprefix -l,$(SYSLIBS)) $(LFLAGS)

$(BIN_CONFIG_PATH)/obj/$(BIN_CONFIG_PAD)%.o : %.cc
	mkdir -p $(@D)
	mkdir -p $(dir $(DEPDIR)/$*.d)
	$(CPP) $(CFLAGS) $(CPPFLAGS) $(DEPFLAGS) $(addprefix -I,$(INCLUDE_PATHS)) $(addprefix -I,$(EXTRA_SOURCE_INCLUDE_PATHS_RESOLVED)) $(addprefix -I,$(CPPPLAY_UTIL_INCLUDE_PATHS)) -c -o $@ $<

$(CPPPLAY_UTIL_LIBS_PATHS):
	make $(CONFIG) -C $(subst $(BIN_CONFIG_PATH),,$(@D))


# must be last
include $(BUILD_ROOT)Makefile_DepsTail.mk
