#  This is free and unencumbered software released into the public domain.
#
#  Anyone is free to copy, modify, publish, use, compile, sell, or
#  distribute this software, either in source code form or as a compiled
#  binary, for any purpose, commercial or non-commercial, and by any
#  means.
#
#  In jurisdictions that recognize copyright laws, the author or authors
#  of this software dedicate any and all copyright interest in the
#  software to the public domain. We make this dedication for the benefit
#  of the public at large and to the detriment of our heirs and
#  successors. We intend this dedication to be an overt act of
#  relinquishment in perpetuity of all present and future rights to this
#  software under copyright law.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
#  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
#  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
#  IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
#  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
#  ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
#  OTHER DEALINGS IN THE SOFTWARE.
#
#  For more information, please refer to <https://unlicense.org>

SHELL=/bin/bash

ALL_GOALS=cbuild cbuildnocache crun cstop cexec cshell cclean ccreate
.PHONY: $(ALL_GOALS)

ifeq (,$(filter cbuild cbuildnocache,$(MAKECMDGOALS)))
ifeq (,$(STACK_PATH))
$(error STACK_PATH must be provided)
endif
endif

ifeq (,$(filter cbuild cbuildnocache,$(MAKECMDGOALS)))
ifeq (,$(IMAGE_NAME))
$(error IMAGE_NAME must be provided)
endif
endif

ifeq (,$(filter cbuild cbuildnocache,$(MAKECMDGOALS)))
ifeq (,$(CONTAINER_NAME))
$(error CONTAINER_NAME must be provided)
endif
endif

ifneq (,$(filter $(ALL_GOALS),$(MAKECMDGOALS)))
$(info )
$(info [Makefile_DevContainer.mk] =============================================)
$(info $(MAKECMDGOALS))
$(info STACK_PATH=$(STACK_PATH))
$(info )
endif

cbuild:
	docker build \
		--build-arg GTEST_VERSION=$(GTEST_VERSION) \
		--build-arg GBENCH_VERSION=$(GBENCH_VERSION) \
		--tag=$(IMAGE_NAME) .

cbuildnocache:
	docker build --no-cache \
		--build-arg GTEST_VERSION=$(GTEST_VERSION) \
		--build-arg GBENCH_VERSION=$(GBENCH_VERSION) \
		--tag=$(IMAGE_NAME) .

ccreate:
	docker run \
		-w /home/stack \
		-ti \
		--cap-add=SYS_PTRACE --name=$(CONTAINER_NAME) \
		--env STACK_PATH=$(STACK_PATH) \
		--mount type=bind,source=$(STACK_PATH),target=/home/stack \
		-d $(IMAGE_NAME):latest \
		/bin/bash

cstop:
	- docker stop $(CONTAINER_NAME)

cstart:
	- docker start $(CONTAINER_NAME)

cexec:
	docker exec -it $(CONTAINER_NAME) bash

cshell:cexec

cclean:
	- docker stop $(CONTAINER_NAME)
	- docker rm $(CONTAINER_NAME)

