## $Id: Makefile.am,v 1.33 2002-09-16 18:45:13 adam Exp $

docdir=$(datadir)/doc/@PACKAGE@

XMLFILES=$(srcdir)/yaz.xml.in $(srcdir)/introduction.xml \
 $(srcdir)/installation.xml $(srcdir)/indexdata.xml $(srcdir)/asn.xml \
 $(srcdir)/tools.xml $(srcdir)/odr.xml $(srcdir)/comstack.xml \
 $(srcdir)/frontend.xml $(srcdir)/license.xml $(srcdir)/future.xml  \
 $(srcdir)/client.xml $(srcdir)/zoom.xml $(srcdir)/credits.xml \
 $(srcdir)/ztest-options.xml $(srcdir)/yaz-client-commands.xml

HTMLFILES = asn.external.html asn.html asn.oid.html asn.pdu.html \
 asn.preparing.html client.commands.html client.html client.invoking.html \
 client.searching.html comstack.addresses.html comstack.client.html \
 comstack.common.html comstack.diagnostics.html comstack.html \
 comstack.introduction.html comstack.server.html comstack.summary.html \
 credits.html future.html indexdata.html installation.html \
 installation.unix.html \
 installation.win32.html introduction.html \
 license.html license.other.html odr.debugging.html \
 odr.html odr.programming.html odr.use.html server.backend.html \
 server.backendfunctions.html server.frontend.html server.html \
 server.invocation.html server.main.html tools.html tools.nmem.html \
 tools.oid.html yaz.html zoom.events.html zoom.html \
 zoom.options.html zoom.records.html zoom.resultsets.html zoom.query.html \
 zoom.scan.html

DOCFILES=$(HTMLFILES) yaz.pdf
MANFILES=yaz-client.1 yaz-client-ssl.1 yaz-ztest.8 \
	yaz-ztest-ssl.8 yaz-config.1 yaz.7 zoomsh.1
REFFILES=yaz-client.sgml yaz-ztest.sgml yaz-config.sgml \
	yaz-man.sgml zoomsh.sgml

SUPPORTFILES=yazhtml.dsl yazphp.dsl yazprint.dsl xml.dcl id.eps id.png

EXTRA_DIST = $(XMLFILES) $(SUPPORTFILES) $(DOCFILES) $(MANFILES) $(REFFILES)

doc_DATA = $(DOCFILES)
man_MANS = $(MANFILES)

yaz-client.1: yaz-client.sgml
	cd $(srcdir); docbook2man yaz-client.sgml

yaz-ztest.8: yaz-ztest.sgml
	cd $(srcdir); docbook2man yaz-ztest.sgml

yaz-config.1: yaz-config.sgml
	cd $(srcdir); docbook2man yaz-config.sgml

yaz.7: yaz-man.sgml
	cd $(srcdir); docbook2man yaz-man.sgml

zoomsh.1: zoomsh.sgml
	cd $(srcdir); docbook2man zoomsh.sgml

yaz-ztest-ssl.8: yaz-ztest.8
	ln -s yaz-ztest.8 yaz-ztest-ssl.8

yaz-client-ssl.1: yaz-client.1
	ln -s yaz-client.1 yaz-client-ssl.1

$(HTMLFILES): $(XMLFILES) 
	cd $(srcdir); jade -E14 -d yazhtml.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/yaz.php: $(XMLFILES)
	cd $(srcdir); jade -E14 -d yazphp.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/yaz.pdf: $(XMLFILES)
	cd $(srcdir); jade -d yazprint.dsl -t tex xml.dcl yaz.xml
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex
