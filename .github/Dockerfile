FROM debian:unstable AS download-tarballs

RUN apt-get update && \
    apt-get install -y wget gnupg && \
    wget -O - https://keithp.com/archive/archive-key | apt-key add - && \
    echo "deb http://keithp.com/archive unstable/" > /etc/apt/sources.list.d/keithp.list && \
    apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    meson \
    ninja-build \
    gcc-arm-none-eabi \
    gcc-riscv64-unknown-elf \
    gcc-aarch64-linux-gnu \
    qemu-system-arm=1:5.0-4.1 \
    qemu-system-misc=1:5.0-4.1

# Copy picolibc into container
RUN mkdir picolibc
COPY ./ picolibc/

WORKDIR ./picolibc
