Summary: YAZ - A Z39.50 Library
Name: yaz
Version: 1.6
Release: 1
Copyright: BSD
Group: Development/Libraries
Vendor: Index Data ApS <info@indexdata.dk>
Url: http://www.indexdata.dk/yaz/
Source: yaz-1.6.tar.gz
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
 ./configure --prefix=/usr --enable-yc --enable-tcpd
make CFLAGS="$RPM_OPT_FLAGS"

%install
rm -fr $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT/usr install

%files
%defattr(-,root,root)
%doc README LICENSE CHANGELOG TODO
%config /usr/share/yaz/tab
/usr/bin/yaz-client
/usr/bin/yaz-ztest
/usr/bin/yaz-config
/usr/bin/yaz-comp
/usr/lib/libyaz.a
/usr/include/yaz
/usr/share/yaz/doc
