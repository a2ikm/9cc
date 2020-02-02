FROM ubuntu:bionic

RUN set -ex; \
      apt-get update; \
      apt-get install -y --no-install-recommends \
        build-essential \
      ; \
      rm -r /var/lib/apt/lists/*
