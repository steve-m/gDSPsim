%define ver 0.30
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
%{prefix}/share/gnome/gdspsim/C/gdspsim.html


%changelog
* Mon Jun 24 2002 Kerry Keal <kerry@industrialmusic.com>
- Lots of simulator bug fixes
- Added File IO
- Added Plot capabilities

* Mon Jan 7 2002 Kerry Keal <kerry@industrialmusic.com>
- Process all opcodes

