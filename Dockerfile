# Dockerfile for PMDoT Engine Builds
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install dependencies for Godot Engine & Aseprite compilation
RUN apt-get update && apt-get install -y \
    build-essential \
    pkg-config \
    git \
    python3 \
    python3-pip \
    cmake \
    ninja-build \
    libx11-dev \
    libxcursor-dev \
    libxinerama-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libasound2-dev \
    libpulse-dev \
    libudev-dev \
    libxi-dev \
    libxrandr-dev \
    yasm \
    nasm \
    && rm -rf /var/lib/apt/lists/*

# Install SCons
RUN python3 -m pip install --no-cache-dir scons

WORKDIR /pmdot

COPY . /pmdot

# Run pmdot CLI install
RUN python3 pmdot.py install

CMD ["python3", "pmdot.py", "build"]
