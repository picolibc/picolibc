FROM debian:sid

COPY .github/linux-packages.txt /

COPY .github/linux-files.txt /

RUN dpkg --add-architecture i386 && \
    (apt-get update || apt-get update || apt-get update) && \
    (apt-get upgrade -y || apt-get upgrade -y || apt-get upgrade -y) && \
    (xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends || \
     xargs -a linux-packages.txt apt-get install -y --no-install-recommends) && \
    apt-get clean && \
    mkdir -p /opt && \
    (cd /opt && \
     for file in `cat /linux-files.txt`; do \
	echo "Fetching $file"; \
	wget -nv "$file" || exit 1; \
	echo "Unpacking $file"; \
	tar xf `basename "$file"`; \
	echo "Removing $file"; \
	rm `basename "$file"`; \
     done) && \
    rm -rf /var/lib/apt/lists/*

