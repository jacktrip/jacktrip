Summary: Linux-based system is used for multi-machine jam sessions over Internet2.
Name: jackTrip
Version: 0.25
Release: 1
Source0: %{name}-%{version}.tar.gz
License: GPL
Group: SoundWIRE at CCRMA

BuildRequires: qt-devel, qt-designer, qwt-devel, stk

%description
This linux-based system is used for multi-machine jam sessions over Internet2. It is still being developed at CCRMA (SoundWIRE group).At a miniumum, two machines are needed and it's easiest if they've got static ip addresses. The path between needs >3Mbps and low jitter. Set them up so ssh is easy (set up keys, allow X display to export) and run the session from two windows on one machine. Watch out for dhcp, watch out for incomplete /etc/hosts entries, watch out for firewalls and NAT boxes. If iperf udp looks ok on port 4464, in both directions, you've got it. You'll need root for lots of the installation steps (where indicated), but not to run.

%prep
%setup -q -n jackTrip-0.25

%build
cd src/
make

%install
cd src/
make install

%files
%defattr(-,root,root)
/usr/bin/jtrip


%changelog
* Sun Feb 18 2007 Juan-Pablo Caceres <jcaceres@ccrma.Stanford.EDU>
- Initial build.
