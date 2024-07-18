
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

