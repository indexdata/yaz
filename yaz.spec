Name: yaz
Version: 1.8.3
Release: 1
Copyright: YAZ License
Group: Development/Libraries
Vendor: Index Data ApS <info@indexdata.dk>
Source: yaz-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}-root
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.dk/yaz/
Summary: Z39.50 Library

%description
YAZ is a C library for developing client - and server applications
using the ANSI/NISO Z39.50 protocol for Information Retrieval. 

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=/usr --enable-shared --enable-tcpd
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr install
cd doc; make prefix=$RPM_BUILD_ROOT/usr install

%files
%defattr(-,root,root)
%doc README LICENSE CHANGELOG TODO
%config /usr/share/yaz/tab
/usr/bin/yaz-client
/usr/bin/yaz-ztest
/usr/bin/yaz-config
/usr/bin/yaz-comp
/usr/bin/zoomsh
/usr/lib/libyaz.a
/usr/lib/libyaz.so
/usr/lib/libyaz.so.1
/usr/lib/libyaz.so.1.0.0
/usr/lib/libyazthread.a
/usr/lib/libyazthread.so
/usr/lib/libyazthread.so.1
/usr/lib/libyazthread.so.1.0.0
/usr/lib/libyazmalloc.a
/usr/lib/libyazmalloc.so
/usr/lib/libyazmalloc.so.1
/usr/lib/libyazmalloc.so.1.0.0
/usr/include/yaz
/usr/share/yaz/z39.50
/usr/share/yaz/ill
/usr/share/yaz/doc
/usr/share/aclocal/yaz.m4
