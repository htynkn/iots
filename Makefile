export DOCKER_BUILDKIT=1
export COMPOSE_DOCKER_CLI_BUILD=1

export USER_UID := $(shell id -u)
export USER_GID := $(shell id -g)
export USERNAME := $(shell whoami)

# 变量定义
DEVCONTAINER_CLI = npx @devcontainers/cli
WORKSPACE_DIR = $(shell pwd)

.PHONY: help up shell stop clean

help:
	@echo "可用命令:"
	@echo "  make shell  - 启动并进入远程开发环境 (主用命令)"
	@echo "  make up     - 仅构建并启动容器"
	@echo "  make stop   - 停止并移除容器"
	@echo "  make clean  - 清理未使用的 Docker 镜像和卷"

# 核心命令：启动容器并进入交互式 Shell
shell: up
	@echo "--- 正在进入开发环境 ---"
	@$(DEVCONTAINER_CLI) exec --workspace-folder $(WORKSPACE_DIR) /bin/bash -c "source /opt/esp/idf/export.sh > /dev/null 2>&1 && bash"

# 启动容器
up:
	@echo "--- 正在启动/构建容器 ---"
	@$(DEVCONTAINER_CLI) up --workspace-folder $(WORKSPACE_DIR)

# 停止容器
stop:
	@echo "--- 正在停止容器 ---"
	@docker ps -q --filter "label=devcontainer.local_folder=$(WORKSPACE_DIR)" | xargs -r docker stop

# 清理
clean: stop
	@docker system prune -f
