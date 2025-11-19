IMAGE_NAME ?= imx93-builder

.PHONY: build run clean delete-images

build:
	@echo "Checking for qemu-user-static (required for ARM64 emulation on x64 host)..."
	@which qemu-aarch64 >/dev/null 2>&1 || (echo "qemu-user-static not found. Install with: sudo apt-get install qemu-user-static binfmt-support && sudo update-binfmts --enable qemu-aarch64" && exit 1)
	@echo "Building Docker image $(IMAGE_NAME) for FRDM-IMX93..."
	docker build --platform linux/arm64 --build-arg UID=$(shell id -u) --build-arg GID=$(shell id -g) -t $(IMAGE_NAME) .

run:
	docker run --platform linux/arm64 -it --rm --hostname frdm-imx93 $(IMAGE_NAME)

clean:
	docker rmi $(IMAGE_NAME) || true

delete-images:
	docker rmi $(docker images -q) || true
