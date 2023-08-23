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
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

