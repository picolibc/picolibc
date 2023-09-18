# syntax=docker/dockerfile:1
FROM debian:testing

COPY .github/linux-packages.txt /

COPY .github/linux-files.txt /

RUN apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates wget gnupg && \
    wget -O /etc/apt/trusted.gpg.d/altusmetrum.gpg https://maps.altusmetrum.org/archive/archive-key.gpg && \
    echo "deb http://maps.altusmetrum.org/archive unstable/" > /etc/apt/sources.list.d/keithp.list && \
    (apt-get update || apt-get update || apt-get update) && \
    (apt-get upgrade -y || apt-get upgrade -y || apt-get upgrade -y) && \
    (xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends) && \
    mkdir -p /opt && \
    (cd /opt && while read file; do wget $file || exit 1; tar xf `basename $file`; rm `basename $file`; done) < linux-files.txt && \
    if [ -f /opt/zephyr-sdk-*-hosttools*.sh ]; then \
	echo "Unpacking host tools"; \
	/opt/zephyr-sdk-*-hosttools*.sh -y -d /opt/zephyr-sdk-hosttools; \
	rm /opt/zephyr-sdk-*-hosttools*.sh; \
    else \
	echo "No host tools found"; \
    fi && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

ADD --checksum=sha256:0b33f39766fe9db67f40418aed6a5b3d7b2f4f7fab025a8213264b77a2d0e1b1 https://github.com/ccache/ccache/releases/download/v4.8.2/ccache-4.8.2-linux-x86_64.tar.xz /
RUN tar xf ccache-4.8.2-linux-x86_64.tar.xz -C /usr/bin --strip-components=1 --no-same-owner ccache-4.8.2-linux-x86_64/ccache && \
    rm -f ccache-*-linux-x86_64.tar.xz
