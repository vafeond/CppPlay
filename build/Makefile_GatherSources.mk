
ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included MakeFile_Config.mk)
endif

ifeq (,$(SOURCE_PATHS))
SOURCE_PATHS=.
endif


# gather sources
SOURCES:=$(wildcard $(addsuffix /*.cc, $(SOURCE_PATHS)))
# $(info SOURCES=$(SOURCES))
OBJECTS:=$(patsubst %.cc,%.o,$(SOURCES))
# $(info OBJECTS=$(OBJECTS))
OBJECTS_FULL_PATH:=$(addprefix $(BIN_CONFIG_PATH)/obj/$(BIN_CONFIG_PAD),$(OBJECTS))
# $(info OBJECTS_FULL_PATH=$(OBJECTS_FULL_PATH))

# gather extra sources - allow extra sources default root to be overriden
ifeq (,$(EXTRA_SOURCES_ROOT))
EXTRA_SOURCES_ROOT=$(BUILD_ROOT)../extra_sources/
endif
EXTRA_SOURCES=$(realpath $(wildcard $(addsuffix /*.cc, $(addprefix $(EXTRA_SOURCES_ROOT)/,$(EXTRA_SOURCE_PATHS)))))
# $(info EXTRA_SOURCES=$(EXTRA_SOURCES))
EXTRA_OBJECTS:=$(patsubst %.cc,%.o,$(filter-out %/gtest-all.cc,$(EXTRA_SOURCES)))
# $(info EXTRA_OBJECTS=$(EXTRA_OBJECTS))
EXTRA_OBJECTS_FULL_PATH:=$(addprefix $(BIN_CONFIG_PATH)/obj/$(BIN_CONFIG_PAD),$(EXTRA_OBJECTS))
# $(info EXTRA_OBJECTS_FULL_PATH=$(EXTRA_OBJECTS_FULL_PATH))
EXTRA_SOURCE_INCLUDE_PATHS_RESOLVED=$(addprefix $(EXTRA_SOURCES_ROOT)/,$(EXTRA_SOURCE_INCLUDE_PATHS))
# $(info EXTRA_SOURCE_INCLUDE_PATHS_RESOLVED=$(EXTRA_SOURCE_INCLUDE_PATHS_RESOLVED))

