Summary: Multimachine jam sessions over internet
Name: jacktrip
Version: 0.25
Release: 1%{?dist}
License: STK
Group: Applications/Multimedia
URL: http://ccrma.stanford.edu/groups/soundwire/donwloads/jacktrip/jacktrip-0.25.tar.gz
Source0: http://ccrma.stanford.edu/soundwire/donwloads/jacktrip/jacktrip-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Packager: Fernando Lopez-Lezcano & Juan-Pablo Caceres
Vendor: Planet CCRMA
Distribution: Planet CCRMA

BuildRequires: qt-devel qwt-devel stk
BuildRequires: jack-audio-connection-kit-devel alsa-lib-devel

%description
This linux-based system is used for multi-machine jam sessions over
Internet2. It is still being developed at CCRMA. At a miniumum, two
machines are needed and it's easiest if they've got static ip
addresses. The path between needs >3Mbps and low jitter.

%prep
%setup -q -n jacktrip-%{version}

%build
cd src
./m
make

%install
%{__rm} -rf %{buildroot}
%{__mkdir} -p %{buildroot}%{_bindir}
%{__install} -m 0755 src/jacktrip %{buildroot}%{_bindir}/jacktrip

%clean
%{__rm} -rf %{buildroot}


%files
%defattr(-,root,root,-)
%doc README
%{_bindir}/jacktrip


%changelog
* Mon Feb 19 2007 Juan-Pablo Caceres <jcaceres@ccrma.Stanford.EDU> - 0.25-1
- modified names and version
* Thu Jul 20 2006 Fernando Lopez-Lezcano <nando@ccrma.stanford.edu> - 25-1
- initial build.
