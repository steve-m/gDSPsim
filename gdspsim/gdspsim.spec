%define ver 0.20
%define rel 1
%define prefix /usr

Summary:   A C54 DSP simulator
Name:	   gdspsim
Version:   %ver
Release:   %rel
Copyright: GPL
Group:     Development
BuildRoot: /tmp
Source0:    gdspsim-%{PACKAGE_VERSION}.tgz
URL:       http://gdspsim.sourceforge.net
#Distribution:
#Vender:
Packager:  kerry@industrialmusic.com
#Docdir:
#Requires:

%description
A DSP Simulator for Linux. The first chip supported is the TMS320C54x. 
It allows viewing each stage of the pipeline when stepping through code 
in the disassembly window.

%prep
%setup

%build
make 

%install
make install -e PREFIX=$RPM_BUILD_ROOT/%{prefix}

%clean
make clean

%files
%{prefix}/bin/gdspsim
%{prefix}/share/gdspsim/stop.xpm

%changelog
* Mon Jan 7 2002 Kerry Keal <kerry@industrialmusic.com>
- Process all opcodes

