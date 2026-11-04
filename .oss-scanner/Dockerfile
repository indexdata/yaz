# Dockerfile for oss-scanner
FROM debian:bookworm
ENV DEBIAN_FRONTEND=noninteractive \
    CC=clang \
    CXX=clang++
RUN apt-get update && apt-get install -y --no-install-recommends \
      ca-certificates build-essential clang git pkg-config \
      tcl bison libxslt1-dev libgnutls28-dev libreadline-dev \
      libwrap0-dev libicu-dev libstemmer-dev automake libtool xsltproc docbook-xml docbook-xsl \
    && rm -rf /var/lib/apt/lists/*

COPY . /src
WORKDIR /src

RUN ./buildconf.sh && CFLAGS="-g -Wall -fno-omit-frame-pointer" \
    ./configure --disable-shared --with-stemmer && make -j"$(nproc)"
RUN make check || echo "WARNING: make check reported failures"
