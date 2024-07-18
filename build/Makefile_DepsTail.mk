# automatic dependency management targets
# see Makefile_DepsHead.mk

ifeq (,$(MAKEFILE_DEPSHEAD_INCLUDED))
$(error Must be used with Makefile_DepsHead.mk)
endif

$(DEPDIR):
	mkdir -p $(DEPDIR)

$(DEPFILES):

include $(wildcard $(DEPFILES))
