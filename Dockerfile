FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG X32DK_RELEASE=20220418
ARG X32DK_ARCHIVE=chillys-sega-devkit-20220418-opt.tar.zst

RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        ca-certificates \
        curl \
        libgmp10 \
        libisl22 \
        libmpc3 \
        libmpfr6 \
        make \
        zstd \
    && rm -rf /var/lib/apt/lists/*

RUN curl --fail --location --retry 3 \
        "https://github.com/viciious/32XDK/releases/download/${X32DK_RELEASE}/${X32DK_ARCHIVE}" \
        --output "/tmp/${X32DK_ARCHIVE}" \
    && tar --extract --use-compress-program=unzstd --file "/tmp/${X32DK_ARCHIVE}" --directory / \
    && rm "/tmp/${X32DK_ARCHIVE}" \
    && /opt/toolchains/sega/sh-elf/bin/sh-elf-gcc --version \
    && /opt/toolchains/sega/m68k-elf/bin/m68k-elf-gcc --version

ENV MARSDEV=/opt/toolchains/sega
ENV PATH="/opt/toolchains/sega/bin:/opt/toolchains/sega/sh-elf/bin:/opt/toolchains/sega/m68k-elf/bin:${PATH}"

WORKDIR /workspace

CMD ["make", "release"]
