URL: http://saturntools.sf.net/
Summary: System and process monitoring utilities
Name: ops
%define major_version 13
%define minor_version 0
%define revision 0
%define version %{major_version}.%{minor_version}.%{revision}
Version: %{version}
Release: 1
License: GPL
Group: Applications/Video
Source: http://saturntools.sf.net/ops-%{version}.tar.gz
BuildRoot: %{_tmppath}/ops-root
Packager: <saturntools-ops-linux@lists.sf.net>

%description
The ops package contains a graphical to to control a video camera.
This camera was created by Puredigital and marketed in 2005 by the
CVS pharmacy as a "disposable" camcorder.

%prep
%setup

%build
make CFLAGS="$RPM_OPT_FLAGS" DESTDIR=$RPM_BUILD_ROOT install="install -D"

%install
rm -rf $RPM_BUILD_ROOT
make CFLAGS="$RPM_OPT_FLAGS" DESTDIR=$RPM_BUILD_ROOT install="install -D" install

%clean
rm -rf $RPM_BUILD_ROOT

%post

%files
%defattr(0644,root,root,755)
%doc COPYING README AUTHORS
%attr(555,root,root) /lib*/lib*.so*
%attr(555,root,root) /bin/*
%attr(555,root,root) /sbin/*
%attr(555,root,root) /usr/bin/*

%attr(0644,root,root) /usr/share/man/man1/*
%attr(0644,root,root) /usr/share/man/man5/*
%attr(0644,root,root) /usr/share/man/man8/*
