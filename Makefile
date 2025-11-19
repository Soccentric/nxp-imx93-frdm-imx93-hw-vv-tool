# Configuration
IMX93_IP := 192.168.1.100
USER := root

# Build directories
X64_BUILD_DIR := build/x86
AARCH64_BUILD_DIR := build/aarch64
REMOTE_BASE_DIR := /tmp/frdm-imx93-test

# Docker configuration
DOCKER_IMAGE := imx93-builder
# Ensure we run with the correct platform and mount the current directory
DOCKER_RUN := docker run --platform linux/arm64 --rm -v $(PWD):/workspace -w /workspace $(DOCKER_IMAGE)

.PHONY: help all clean build native shell deploy run run-arm test test-arm

# Default target
all: build native

help:
	@echo "Available targets:"
	@echo "  all                    - Build both aarch64 (cross/docker) and x64 (native) in build/ folder"
	@echo "  build                  - Build for aarch64 (i.MX93 Cortex-A55) using Docker"
	@echo "  native                 - Build natively for x64"
	@echo "  clean                  - Remove all build directories"
	@echo "  shell                  - Open interactive Docker shell (aarch64)"
	@echo "  run                    - Run the x86 executable"
	@echo "  run-arm                - Run the aarch64 executable in Docker"
	@echo "  test                   - Run tests for x86 build"
	@echo "  test-arm               - Run tests for aarch64 build in Docker"
	@echo "  deploy                 - Deploy binaries to FRDM-IMX93 board"

shell:
	$(DOCKER_RUN) bash

clean:
	rm -rf build

# Build for FRDM-IMX93 (aarch64) inside Docker
build:
	# Build the docker image first if it doesn't exist
	$(MAKE) -C docker build
	$(DOCKER_RUN) bash -c "mkdir -p $(AARCH64_BUILD_DIR) && cd $(AARCH64_BUILD_DIR) && cmake ../.. && make -j$$(nproc)"

# Build natively on host
native:
	mkdir -p $(X64_BUILD_DIR) && cd $(X64_BUILD_DIR) && cmake ../.. && make -j$$(nproc)

# Deployment helper function
define deploy_binaries
	ssh $(2)@$(1) "mkdir -p $(REMOTE_BASE_DIR)/bin $(REMOTE_BASE_DIR)/lib && rm -rf $(REMOTE_BASE_DIR)/bin/* $(REMOTE_BASE_DIR)/lib/*"
	scp -r $(3)/bin/* $(2)@$(1):$(REMOTE_BASE_DIR)/bin/
	scp -r $(3)/lib/* $(2)@$(1):$(REMOTE_BASE_DIR)/lib/
endef

# Deploy binaries to FRDM-IMX93 board
deploy:
	$(call deploy_binaries,$(IMX93_IP),$(USER),$(AARCH64_BUILD_DIR))

# Run the x86 executable
run: native
	$(X64_BUILD_DIR)/bin/imx93_peripheral_test_app

# Run the aarch64 executable in Docker
run-arm: build
	$(DOCKER_RUN) $(AARCH64_BUILD_DIR)/bin/imx93_peripheral_test_app

# Run tests for x86 build
test: native
	cd $(X64_BUILD_DIR) && ctest

# Run tests for aarch64 build in Docker
test-arm: build
	$(DOCKER_RUN) bash -c "cd $(AARCH64_BUILD_DIR) && ctest"

