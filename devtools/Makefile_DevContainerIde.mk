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

ALL_GOALS=ibuild ibuildnocache irun istop iexec ishell iclean icreate
.PHONY: $(ALL_GOALS)

ifeq (,$(filter ibuild ibuildnocache,$(MAKECMDGOALS)))
ifeq (,$(STACK_PATH))
$(error STACK_PATH must be provided)
endif
endif

ifeq (,$(filter ibuild ibuildnocache,$(MAKECMDGOALS)))
ifeq (,$(IMAGE_NAME))
$(error IMAGE_NAME must be provided as the name of the dev base image)
endif
endif

ifeq (,$(filter ibuild ibuildnocache,$(MAKECMDGOALS)))
ifeq (,$(IMAGE_NAME_IDE))
$(error IMAGE_NAME_IDE must be provided)
endif
endif

ifeq (,$(filter ibuild ibuildnocache,$(MAKECMDGOALS)))
ifeq (,$(CONTAINER_NAME_IDE))
$(error CONTAINER_NAME_IDE must be provided)
endif
endif

ifneq (,$(filter $(ALL_GOALS),$(MAKECMDGOALS)))
$(info )
$(info [Makefile_DevContainerIde.mk] =============================================)
$(info $(MAKECMDGOALS))
$(info STACK_PATH=$(STACK_PATH))
$(info )
endif

ibuild:
	docker build --tag=$(IMAGE_NAME_IDE) --build-arg IMAGE_NAME=$(IMAGE_NAME) ./devtools

ibuildnocache:
	docker build --no-cache --tag=$(IMAGE_NAME_IDE) --build-arg IMAGE_NAME=$(IMAGE_NAME) ./devtools

icreate:
	docker run \
		-w /home/stack \
		-ti \
		--cap-add=SYS_PTRACE --name=$(CONTAINER_NAME_IDE) \
		--env STACK_PATH=$(STACK_PATH) \
		--mount type=bind,source=$(STACK_PATH),target=/home/stack \
		-d $(IMAGE_NAME_IDE):latest \
		/bin/bash

istop:
	- docker stop $(CONTAINER_NAME_IDE)

istart:
	- docker start $(CONTAINER_NAME_IDE)

iexec:
	docker exec -it $(CONTAINER_NAME_IDE) bash

ishell:cexec

iclean:
	- docker stop $(CONTAINER_NAME_IDE)
	- docker rm $(CONTAINER_NAME_IDE)

