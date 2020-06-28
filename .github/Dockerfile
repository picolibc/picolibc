FROM debian:unstable AS download-tarballs

RUN apt-get update && \
    apt-get install -y wget gnupg && \
    wget -O - https://maps.altusmetrum.org/archive/archive-key | apt-key add - && \
    echo "deb http://maps.altusmetrum.org/archive unstable/" > /etc/apt/sources.list.d/keithp.list && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    meson \
    ninja-build \
    gcc-arm-none-eabi \
    gcc-riscv64-unknown-elf \
    gcc-aarch64-linux-gnu \
    gcc-xtensa-lx106 \
    qemu-system-arm=1:5.0-5.1 \
    qemu-system-misc=1:5.0-5.1

# Copy picolibc into container
RUN mkdir picolibc
COPY ./ picolibc/

WORKDIR ./picolibc
