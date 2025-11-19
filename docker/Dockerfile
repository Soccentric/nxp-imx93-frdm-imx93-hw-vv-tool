FROM debian:bookworm

ARG UID
ARG GID

# Install build dependencies and utilities for i.MX93 development
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    sudo \
    bash-completion \
    vim \
    nano \
    file \
    gcc-aarch64-linux-gnu \
    g++-aarch64-linux-gnu \
    crossbuild-essential-arm64 \
    && rm -rf /var/lib/apt/lists/*

# Create user with sudo privileges
# Use a generic name 'imx93-user' and map to host UID/GID
RUN groupadd -g $GID imx93-user || groupadd -g $GID imx93-user-group
RUN useradd -m -u $UID -g $GID -s /bin/bash -G sudo imx93-user
RUN echo "imx93-user ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Enable tab completion
RUN echo "if [ -f /etc/bash_completion ]; then . /etc/bash_completion; fi" >> /home/imx93-user/.bashrc

USER imx93-user

# Set working directory
WORKDIR /workspace