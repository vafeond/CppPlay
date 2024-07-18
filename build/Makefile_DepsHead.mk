# automatic dependencies management
# include in binary makefile after Makefile_Config.mk
# include Makefile_DepsTail.mk at the end of the makefile
# pass DEPFLAGS into object creation command


ifeq (,$(MAKEFILE_CONFIG_INCLUDED))
$(error Must be included after Makefile_Config.mk)
endif

DEPDIR:=$(BIN_CONFIG_PATH)/deps/$(BIN_CONFIG_PAD)
DEPFILES=$(SOURCES:%.cc=$(DEPDIR)%.d)
DEPFLAGS= -MT $@ -MMD -MP -MF $(DEPDIR)$*.d


# flag to show that Makefile_Config.mk was included in the build heirarchy
MAKEFILE_DEPSHEAD_INCLUDED=true
