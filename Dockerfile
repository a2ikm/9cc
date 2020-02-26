FROM ubuntu:bionic

RUN set -ex; \
      apt-get update; \
      apt-get install -y --no-install-recommends \
        build-essential \
        gdb \
      ; \
      rm -r /var/lib/apt/lists/*
