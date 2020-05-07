FROM debian:unstable AS download-tarballs

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    meson \
    ninja-build \
    gcc-arm-none-eabi \
    gcc-riscv64-unknown-elf

# Copy freedom-e-sdk into container
RUN mkdir picolibc
COPY ./ picolibc/

WORKDIR ./picolibc
