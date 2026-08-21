FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive
ARG X32DK_RELEASE=20220418
ARG X32DK_ARCHIVE=chillys-sega-devkit-20220418-opt.tar.zst
ARG SGDK_COMMIT=ea1e49d156ec071c8759517a56d781cce8543d6c

RUN apt-get update \
    && apt-get install --no-install-recommends -y \
        ca-certificates \
        curl \
        git \
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

RUN curl --fail --location --retry 3 \
        "https://github.com/ArtemioUrbina/SGDK_1_11_Suite/archive/${SGDK_COMMIT}.tar.gz" \
        --output /tmp/sgdk.tar.gz \
    && mkdir --parents /opt/sgdk \
    && tar --extract --gzip --file /tmp/sgdk.tar.gz \
        --directory /opt/sgdk --strip-components=1 \
        "SGDK_1_11_Suite-${SGDK_COMMIT}/inc" \
        "SGDK_1_11_Suite-${SGDK_COMMIT}/lib" \
        "SGDK_1_11_Suite-${SGDK_COMMIT}/res" \
    && rm /tmp/sgdk.tar.gz \
    && test -f /opt/sgdk/lib/libmd.a

ENV MARSDEV=/opt/toolchains/sega
ENV SGDK=/opt/sgdk
ENV PATH="/opt/toolchains/sega/bin:/opt/toolchains/sega/sh-elf/bin:/opt/toolchains/sega/m68k-elf/bin:${PATH}"

WORKDIR /workspace

CMD ["make", "release"]
