Summary: YAZ - A Z39.50 Library
Name: yaz
Version: 1.5
Release: 1
Copyright: GPL
Group: Development/Libraries
Vendor: Index Data ApS <info@indexdata.dk>
Url: http://www.indexdata.dk/yaz/
Source: yaz-1.5.tar.gz
BuildRoot: /var/tmp/%{name}-%{version}-root
Packager: Adam Dickmeiss <adam@indexdata.dk>

%description
The YAZ package is a developers' library for developing client - and
server application using the ANSI/NISO Z39.50 protocol for Information
Retrieval.

%prep
%setup

%build

CFLAGS="$RPM_OPT_FLAGS" \
 ./configure --with-build-root=$RPM_BUILD_ROOT --prefix=/usr --enable-yc --enable-tcpd
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr $RPM_BUILD_ROOT
make install

%files
%defattr(-,root,root)
%doc README LICENSE CHANGELOG TODO
%config /usr/lib/yaz/tab
/usr/bin/yaz-client
/usr/bin/yaz-ztest
/usr/bin/yaz-config
/usr/lib/libyaz.a
/usr/include/yaz
%dir /usr/lib/yaz
