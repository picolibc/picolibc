FROM debian:unstable AS download-tarballs

RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    build-essential \
    meson \
    ninja-build \
    qemu-system-arm \
    gcc-arm-none-eabi

# Copy freedom-e-sdk into container
RUN mkdir picolibc
COPY ./ picolibc/

WORKDIR ./picolibc
