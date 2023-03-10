FROM debian:testing

COPY .github/packages.txt /

COPY .github/extra-files.txt /

RUN apt-get update && \
    apt-get install -y wget gnupg && \
    wget -O /etc/apt/trusted.gpg.d/altusmetrum.gpg https://maps.altusmetrum.org/archive/archive-key.gpg && \
    echo "deb http://maps.altusmetrum.org/archive unstable/" > /etc/apt/sources.list.d/keithp.list && \
    (apt-get update || apt-get update || apt-get update) && \
    (apt-get upgrade -y || apt-get upgrade -y || apt-get upgrade -y) && \
    (xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y) && \
    mkdir -p /opt && \
    (cd /opt && while read file; do wget $file || exit 1; tar xf `basename $file`; done) < extra-files.txt

RUN apt-get clean
