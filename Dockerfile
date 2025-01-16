# JackTrip container image using Redhat Universal Base Image ubi-init
#
# Copyright (c) 2023-2024 JackTrip Labs, Inc.
# MIT License.
#
# To build this from repository root: "podman build -t jacktrip/jacktrip ."

# container image versions
ARG FEDORA_VERSION=34
ARG JACK_VERSION=latest


###############
# STAGE BUILDER
###############
# use a temporary container to build jacktrip
FROM registry.fedoraproject.org/fedora:${FEDORA_VERSION} AS builder

# install tools require to build jacktrip
RUN dnf install -y --nodocs cmake gcc gcc-c++ meson git python3-pyyaml python3-jinja2 glib2-devel jack-audio-connection-kit-devel dbus-devel

ENV QT_VERSION=6.5.3
RUN if [ "$(uname -m)" = "x86_64" ]; then export ARCH=amd64; else export ARCH=arm64; fi \
	&& curl -L -s -o /root/qt.tar.gz "https://files.jacktrip.org/contrib/qt/qt-${QT_VERSION}-static-linux-${ARCH}.tar.gz" \
	&& tar -C /opt -xzf /root/qt.tar.gz \
	&& rm -f /root/qt.tar.gz

# copy files from repository to build container
COPY . /root

# configure and run the build
RUN cd /root \
	&& export QT_PATH=/opt/qt-${QT_VERSION}-static \
	&& export PATH=${QT_PATH}/bin:${PATH} \
	&& export LDFLAGS="-L${QT_PATH}/lib -L${QT_PATH}/plugins/tls" \
	&& meson setup -Ddefault_library=static -Dnogui=true --buildtype release builddir \
	&& meson compile -C builddir

# stage files in INSTALLDIR
ENV INSTALLDIR=/artifacts
RUN mkdir -p ${INSTALLDIR}/usr/local/bin/ ${INSTALLDIR}/etc/systemd/system/ \
	&& cp /root/builddir/jacktrip ${INSTALLDIR}/usr/local/bin/ \
	&& strip ${INSTALLDIR}/usr/local/bin/jacktrip
COPY linux/container/jacktrip.service ${INSTALLDIR}/etc/systemd/system/


########################
# STAGE JACKTRIP (FINAL)
########################
# use the jack ubi-init container
FROM jacktrip/jack:${JACK_VERSION}

# add jacktrip user, enable service and allow access to jackd
RUN useradd -r -m -N -G audio -s /usr/sbin/nologin jacktrip \
	&& chown -R jacktrip.audio /home/jacktrip \
	&& chmod g+rwx /home/jacktrip \
	&& ln -s /etc/systemd/system/jacktrip.service /etc/systemd/system/multi-user.target.wants \
	&& sed -i 's,PassEnvironment=.*,PassEnvironment=SAMPLE_RATE BUFFER_SIZE JACK_OPTS JACKTRIP_OPTS,' /etc/systemd/system/defaults.service \
	&& sed -i 's,WantedBy=.*,WantedBy=multi-user.target jack.service jacktrip.service,' /etc/systemd/system/defaults.service \
	&& echo 'if [[ -z "$JACKTRIP_OPTS" ]]; then JACKTRIP_OPTS="-S -t -z --hubpatch 4 --bufstrategy 4 -q auto"; fi' >> /usr/sbin/defaults.sh \
	&& echo 'echo "JACKTRIP_OPTS=\"$JACKTRIP_OPTS\"" > /etc/default/jacktrip' >> /usr/sbin/defaults.sh

# copy the artifacts we built into the final container image
COPY --from=builder /artifacts /

# jacktrip hub server listens on 4464 and uses 61000+ for clients
EXPOSE 4464/tcp
EXPOSE 61000-61100/udp
