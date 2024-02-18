# JackTrip container image using Redhat Universal Base Image ubi-init
#
# Copyright (c) 2023-2024 JackTrip Labs, Inc.
# MIT License.
#
# To build this from repository root: "podman build -t jacktrip/jacktrip ."

# use a temporary container to build jacktrip
FROM registry.fedoraproject.org/fedora:34 AS builder

# install tools require to build jacktrip
RUN dnf install -y --nodocs gcc gcc-c++ meson python3-pyyaml python3-jinja2 qt5-qtbase-devel jack-audio-connection-kit-devel

# copy files from repository to build container
COPY . /root

# configure and run the build
RUN cd /root \
	&& PKG_CONFIG_PATH=/usr/local/lib/pkgconfig meson setup -Ddefault_library=static -Dnogui=true --buildtype release builddir \
	&& meson compile -C builddir \
	&& strip builddir/jacktrip

# use the jack ubi-init container
FROM jacktrip/jack:1.9.22-20240218

# install libraries that we need for things to run
RUN dnf install -y --nodocs libicu pcre libstdc++ compat-openssl11 pcre2-utf16

# add jacktrip user, enable service and allow access to jackd
RUN useradd -r -m -N -G audio -s /usr/sbin/nologin jacktrip \
	&& chown -R jacktrip.audio /home/jacktrip \
	&& chmod g+rwx /home/jacktrip \
	&& ln -s /etc/systemd/system/jacktrip.service /etc/systemd/system/multi-user.target.wants \
	&& sed -i 's,PassEnvironment=.*,PassEnvironment=SAMPLE_RATE BUFFER_SIZE JACK_OPTS JACKTRIP_OPTS,' /etc/systemd/system/defaults.service \
	&& sed -i 's,WantedBy=.*,WantedBy=multi-user.target jack.service jacktrip.service,' /etc/systemd/system/defaults.service \
	&& echo 'if [[ -z "$JACKTRIP_OPTS" ]]; then JACKTRIP_OPTS="-S -t -z --hubpatch 4 --bufstrategy 4 -q auto"; fi' >> /usr/sbin/defaults.sh \
	&& echo 'echo "JACKTRIP_OPTS=\"$JACKTRIP_OPTS\"" > /etc/default/jacktrip' >> /usr/sbin/defaults.sh

# install jacktrip service
COPY linux/container/jacktrip.service /etc/systemd/system/

# copy the artifacts we built into the final container image
COPY --from=builder /lib64/libQt5Core.so.5 /lib64/libQt5Network.so.5 /lib64/
COPY --from=builder /root/builddir/jacktrip /usr/local/bin/

# jacktrip hub server listens on 4464
EXPOSE 4464/tcp
