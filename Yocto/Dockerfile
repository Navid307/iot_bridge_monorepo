FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

# Install required packages
RUN apt-get update && apt-get install -y \
    gawk \
    wget \
    git-core \
    diffstat \
    unzip \
    texinfo \
    gcc-multilib \
    build-essential \
    chrpath \
    socat \
    cpio \
    python3 \
    python3-pip \
    python3-pexpect \
    xz-utils \
    debianutils \
    iputils-ping \
    python3-git \
    python3-jinja2 \
    libegl1-mesa \
    libsdl1.2-dev \
    xterm \
    curl \
    file \
    vim \
    sudo \
    zstd \
    liblz4-tool \
    locales && \
    locale-gen en_US.UTF-8 && update-locale && \
    rm -rf /var/lib/apt/lists/*

# Add a user for building
RUN useradd -m -s /bin/bash yocto
RUN echo "yocto ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# Switch to the yocto user
USER yocto
WORKDIR /home/yocto

# Clone Poky and other layers (replace with actual dependencies)
RUN git clone -b kirkstone git://git.yoctoproject.org/poky.git && \
    git clone -b kirkstone git://git.openembedded.org/meta-openembedded && \
    git clone -b kirkstone https://git.yoctoproject.org/meta-raspberrypi


# Set the default shell to bash
SHELL ["/bin/bash", "-c"]

# Set default user to root for package installations
USER yocto

# Set the working directory to yocto home directory
WORKDIR /home/yocto