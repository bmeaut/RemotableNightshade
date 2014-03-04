Name: libnscontrol
Version: 0.1.0
Release: 3
Summary: Control library for Nightshade Astronomical Simulator
License: LGPLv3+
Group: Applications/Scientific
Source: %{name}-%{version}.tar.gz
URL: http://nightshadesoftware.org
Packager: Rob Spearman

BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Control library for controlling Nightshade simulator using shared memory interface.

%prep
%setup -q

%build
%configure
%{__make}

%install
%makeinstall

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%clean
[ ${RPM_BUILD_ROOT} != "/" ] && rm -Rf ${RPM_BUILD_ROOT}
rm -Rf ${RPM_BUILD_DIR}/%{name}-%{version}

%files
%defattr(-,root,root)
%{_libdir}/libnscontrol.a
%{_libdir}/libnscontrol.la
%{_libdir}/libnscontrol.so
%{_libdir}/libnscontrol.so.0
%{_libdir}/libnscontrol.so.0.0.1
%{_includedir}/nshade_shared_memory_connection.h  
%{_includedir}/nshade_shared_memory.h

%changelog
* Mon Mar 15 2010 Rob <rob@digitaliseducation.com>
- Initial version

