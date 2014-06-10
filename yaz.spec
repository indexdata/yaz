# This file is part of the YAZ toolkit
# Copyright (C) Index Data
# See the file LICENSE for details.
#
# spec file for YAZ

%define idmetaversion %(. ./IDMETA; echo $VERSION|tr -d '\n')
Name: yaz
Summary: Z39.50 Programs
Version: %{idmetaversion}
Release: 1.indexdata

# determine system
%define is_mandrake %(test -e /etc/mandrake-release && echo 1 || echo 0)
%define is_suse %(test -e /etc/SuSE-release >/dev/null && echo 1 || echo 0)
%define is_suse11 %(grep 'VERSION = 11' /etc/SuSE-release >/dev/null 2>&1 && echo 1 || echo 0)
%define is_fedora %(test -e /etc/fedora-release && echo 1 || echo 0)
Requires: libxslt, gnutls, readline, libyaz5 = %{version}
License: BSD
Group: Applications/Internet
Vendor: Index Data ApS <info@indexdata.dk>
Source: yaz-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root
Prefix: %{_prefix}

%define TCPWRAPPER tcp_wrappers

%if %is_fedora
%define TCPWRAPPER tcp_wrappers-devel
%endif

%if %is_suse
%define TCPWRAPPER tcpd-devel
%endif

BuildRequires: %{TCPWRAPPER}

%if %is_suse11
BuildRequires: libgnutls-devel
%else
BuildRequires: gnutls-devel
%endif

BuildRequires: pkgconfig
BuildRequires: libxml2-devel
BuildRequires: libxslt-devel
BuildRequires: readline-devel
BuildRequires: libicu-devel
BuildRequires: wget
BuildRequires: cyrus-sasl-devel
BuildRequires: libgcrypt-devel
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.com/yaz

%description
This package contains both a test-server and clients (normal & ssl)
for the ANSI/NISO Z39.50 protocol for Information Retrieval.

%package -n libyaz5
Summary: Z39.50 Library
Group: Libraries
Requires: libxslt, gnutls, libicu, libmemcached, libgcrypt

%description -n libyaz5
YAZ is a library for the ANSI/NISO Z39.50 protocol for Information
Retrieval.

%post -n libyaz5 -p /sbin/ldconfig 
%postun -n libyaz5 -p /sbin/ldconfig 

%package -n libyaz5-devel
Summary: Z39.50 Library - development package
Group: Development/Libraries
Requires: libyaz5 = %{version}, libxml2-devel, libxslt-devel, libicu-devel
Conflicts: libyaz-devel, libyaz4-devel

%description -n libyaz5-devel
Development libraries and includes for the libyaz package.

%package -n yaz-illclient
Summary: ILL client
Group: Applications/Communication
Requires: readline, libyaz5 = %{version}

%description -n yaz-illclient
yaz-illclient: an ISO ILL client.

%package -n yaz-icu
Summary: Command line utility for ICU utilities of YAZ
Group: Applications/Communication
Requires: libyaz5 = %{version}

%description -n yaz-icu
The yaz-icu program is a command-line based client which exposes the ICU
chain facility of YAZ.

%prep
%setup

wget -q http://ftp.indexdata.dk/pub/support/libmemcached-1.0.18.tar.gz
tar zxf libmemcached-1.0.18.tar.gz

%build

YD=`pwd`
cd libmemcached-1.0.18
./configure --disable-shared --prefix=${YD}/libmemcached
make install
cd ..
PKG_CONFIG_PATH=${YD}/libmemcached/lib/pkgconfig CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=%{_prefix} --libdir=%{_libdir} --mandir=%{_mandir} \
	--enable-shared --enable-tcpd --with-xslt --with-gnutls --with-icu
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr ${RPM_BUILD_ROOT}
make install DESTDIR=${RPM_BUILD_ROOT}
rm ${RPM_BUILD_ROOT}/%{_libdir}/*.la

%clean
rm -fr ${RPM_BUILD_ROOT}

%files
%defattr(-,root,root)
%doc README LICENSE NEWS
%{_bindir}/yaz-client
%{_bindir}/yaz-ztest
%{_bindir}/zoomsh
%{_bindir}/yaz-marcdump
%{_bindir}/yaz-iconv
%{_bindir}/yaz-json-parse
%{_bindir}/yaz-url
%{_mandir}/man1/yaz-client.*
%{_mandir}/man1/yaz-json-parse.*
%{_mandir}/man1/yaz-url.*
%{_mandir}/man8/yaz-ztest.*
%{_mandir}/man1/zoomsh.*
%{_mandir}/man1/yaz-marcdump.*
%{_mandir}/man1/yaz-iconv.*
%{_mandir}/man7/yaz-log.*
%{_mandir}/man7/bib1-attr.*

%files -n libyaz5
%defattr(-,root,root)
%{_libdir}/*.so.*

%files -n libyaz5-devel
%defattr(-,root,root)
%{_bindir}/yaz-config
%{_bindir}/yaz-asncomp
%{_includedir}/yaz
%{_libdir}/pkgconfig/yaz.pc
%{_libdir}/*.so
%{_libdir}/*.a
%{_datadir}/aclocal/yaz.m4
%{_mandir}/man1/yaz-asncomp.*
%{_mandir}/man7/yaz.*
%{_mandir}/man?/yaz-config.*
%{_datadir}/doc/yaz
%{_datadir}/yaz

%files -n yaz-illclient
%defattr(-,root,root)
%{_bindir}/yaz-illclient
%{_mandir}/man1/yaz-illclient.*

%files -n yaz-icu
%defattr(-,root,root)
%{_bindir}/yaz-icu
%{_mandir}/man1/yaz-icu.*
