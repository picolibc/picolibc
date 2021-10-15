FROM debian:testing

COPY .github/packages.txt /

RUN apt-get update && \
    apt-get install -y wget gnupg && \
    wget -O - https://maps.altusmetrum.org/archive/archive-key | apt-key add - && \
    echo "deb http://maps.altusmetrum.org/archive unstable/" > /etc/apt/sources.list.d/keithp.list && \
    (apt-get update || apt-get update || apt-get update) && \
    (apt-get upgrade -y || apt-get upgrade -y || apt-get upgrade -y) && \
    (xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y || \
     xargs -a packages.txt apt-get install -y)

RUN apt-get clean
