#
# resolve CppPlay util dependencies
#
# expects CPPPLAY_UTIL_LIBS to be set as a space-separated list
# of the CppPlay utils libraries that are depedencies for the
# invoking binary.
#
# Note: Link order may matter (such as on Linux).
#
# For example:
# CPPPLAY_UTIL_LIB=darray <another CppPlay dep> <another CppPlay dep>
#

ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included MakeFile_Config.mk)
endif

ifeq (,$(BUILD_ROOT))
$(error Must be included in Makefile heirarchy where an ancestor Makefile has detected the build system root relative path and sets it to BUILD_ROOT)
endif

# create CppPlay util path from root
CPPPLAY_UTIL_PATHS:=$(addprefix $(BUILD_ROOT)../, $(CPPPLAY_UTIL_LIBS))

# create CppPlay util include path from root
CPPPLAY_UTIL_INCLUDE_PATHS:=$(addsuffix /include, $(CPPPLAY_UTIL_PATHS))

# create CppPlay util library directory path from root
CPPPLAY_UTIL_LIBS_DIR_PATHS:=$(addsuffix /$(BIN_CONFIG_PATH)/, $(CPPPLAY_UTIL_PATHS))
# $(info CPPPLAY_UTIL_LIBS_DIR_PATHS=$(CPPPLAY_UTIL_LIBS_DIR_PATHS))

# create CppPlay util library path from root
CPPPLAY_UTIL_LIBS_PATHS:=$(join $(CPPPLAY_UTIL_LIBS_DIR_PATHS),$(addprefix lib,$(addsuffix .a, $(CPPPLAY_UTIL_LIBS))))
# $(info CPPPLAY_UTIL_LIBS_PATHS=$(CPPPLAY_UTIL_LIBS_PATHS))

# create CppPlay util make commands, used to ensure dependeny library is up to date
CPPPLAY_UTIL_MAKE:=$(addprefix make -C , $(addsuffix ;,$(CPPPLAY_UTIL_PATHS)))

# create CppPlay util make clean commands
CPPPLAY_UTIL_MAKE_CLEAN:=$(addprefix make clean -C , $(addsuffix ;,$(CPPPLAY_UTIL_PATHS)))

