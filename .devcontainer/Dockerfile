FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ENV SHELL /bin/bash

# install common dependencies
RUN apt update && apt install -y \
    build-essential git curl zip unzip tar gdb \
    cmake openssl libssl-dev libz-dev \
    pkg-config

# Install vcpkg
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg && \
    /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics && \
    ln -s /opt/vcpkg/vcpkg /usr/bin/vcpkg

# Setup default user
ARG USERNAME=admin
ARG USER_UID=1000
ARG USER_GID=$USER_UID

RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && apt-get update \
    && apt-get install -y sudo \
    && echo $USERNAME ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

RUN chown -R $USERNAME:$USERNAME /opt/vcpkg

USER $USERNAME
