
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
	docker build --no-cache --tag=$(IMAGE_NAME_IDE) ./devtools

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

