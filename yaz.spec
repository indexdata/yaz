Name: yaz
Version: 1.7
Release: 1
Copyright: BSD
Group: Development/Libraries
Vendor: Index Data ApS <info@indexdata.dk>
Source: yaz-%{version}.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}-root
Packager: Adam Dickmeiss <adam@indexdata.dk>
URL: http://www.indexdata.dk/yaz/
Summary: Z39.50 Library

%description
The YAZ package is a C library for developing client - and
server applications using the ANSI/NISO Z39.50 protocol for Information
Retrieval. 

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --prefix=/usr --enable-shared
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
/usr/lib/libyaz.a
/usr/lib/libyaz.so
/usr/lib/libyaz.so.0
/usr/lib/libyaz.so.0.0.0
/usr/include/yaz
/usr/share/yaz/doc
