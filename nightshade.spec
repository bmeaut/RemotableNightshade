Name: nightshade
Version: 11.12.1
Release: 1
Summary: Astronomical Simulation and Visualization
License: GPLv3+
Group: Applications/Scientific
Source: %{name}-%{version}.tar.gz
URL: http://nightshadesoftware.org
Packager: Rob Spearman/Kipp Cannon
Requires: SDL
Requires: SDL_Pango
Requires: SDL_mixer
BuildRequires: SDL-devel
BuildRequires: SDL_Pango-devel
BuildRequires: SDL_mixer-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root
# Package is not, really, relocatable (uses a compiled-in path to look for
# data files).
#Prefix: %{_prefix}
%description
Nightshade is free open source (GPL) astronomy and Earth science simulation
and visualization software which renders realistic views around our solar 
system in real time with openGL. It is a fork of Stellarium designed 
for planetarium and educational use.

%prep
%setup -q


%build
%configure
%{__make}


%install
%makeinstall


%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -Rf ${RPM_BUILD_ROOT}
rm -Rf ${RPM_BUILD_DIR}/%{name}-%{version}


%files
%defattr(-,root,root)
%doc AUTHORS ChangeLog COPYING HACKING INSTALL NEWS README TODO TRADEMARKS
%{_bindir}/*
%{_datadir}/%{name}
%{_datadir}/locale
%{_datadir}/man/man1/nightshade.1.gz


