## $Id: Makefile.am,v 1.38 2003-02-21 00:24:26 adam Exp $

docdir=$(datadir)/doc/@PACKAGE@

XMLFILES=yaz.xml.in introduction.xml installation.xml indexdata.xml \
 asn.xml tools.xml odr.xml comstack.xml frontend.xml license.xml \
 future.xml client.xml zoom.xml credits.xml ztest-options.xml \
 yaz-client-commands.xml

HTMLFILES = \
 asn.external.html asn.html asn.oid.html asn.pdu.html asn.preparing.html \
 client.commands.html client.html client.invoking.html client.searching.html \
 comstack.addresses.html comstack.client.html comstack.common.html \
 comstack.diagnostics.html comstack.html comstack.introduction.html \
 comstack.server.html comstack.summary.html credits.html future.html \
 indexdata.html installation.html installation.unix.html \
 installation.win32.html introduction.api.html introduction.html \
 license.html license.other.html odr.debugging.html odr.html \
 odr.programming.html odr.use.html server.backendfunctions.html \
 server.backend.html server.frontend.html server.html server.invocation.html \
 server.main.html tools.html tools.nmem.html tools.oid.html yaz.html \
 zoom.events.html zoom.html zoom.options.html zoom.query.html \
 zoom.records.html zoom.resultsets.html zoom.scan.html

MANFILES=yaz-client.1 yaz-client-ssl.1 yaz-ztest.8 \
	yaz-ztest-ssl.8 yaz-config.1 yaz.7 zoomsh.1
REFFILES=yaz-client.sgml yaz-ztest.sgml yaz-config.sgml \
	yaz-man.sgml zoomsh.sgml

SUPPORTFILES=yazhtml.dsl yazphp.dsl yazprint.dsl tkl.xsl xml.dcl id.eps

doc_DATA = $(HTMLFILES) yaz.pdf id.png

EXTRA_DIST = $(XMLFILES) $(SUPPORTFILES) $(MANFILES) $(REFFILES) $(doc_DATA)

man_MANS = $(MANFILES)

yaz-client.1: $(srcdir)/yaz-client.sgml $(srcdir)/yaz-client-commands.xml
	if test ! -f yaz-client-commands.xml ; then ln -s $(srcdir)/yaz-client-commands.xml .; fi
	docbook2man $(srcdir)/yaz-client.sgml

yaz-ztest.8: yaz-ztest.sgml ztest-options.xml
	if test ! -f ztest-options.xml ; then ln -s $(srcdir)/ztest-options .; fi
	docbook2man $(srcdir)/yaz-ztest.sgml

yaz-config.1: yaz-config.sgml
	docbook2man $(srcdir)/yaz-config.sgml

yaz.7: yaz-man.sgml
	docbook2man $(srcdir)/yaz-man.sgml

zoomsh.1: zoomsh.sgml
	docbook2man $(srcdir)/zoomsh.sgml

yaz-ztest-ssl.8: yaz-ztest.8
	ln -s yaz-ztest.8 yaz-ztest-ssl.8

yaz-client-ssl.1: yaz-client.1
	ln -s yaz-client.1 yaz-client-ssl.1

$(HTMLFILES): $(XMLFILES)  
	jade -E14 -D $(srcdir) -d yazhtml.dsl -t sgml $(srcdir)/xml.dcl yaz.xml

yaz.php: $(XMLFILES)
	jade -E14 -D $(srcdir) -d yazphp.dsl -t sgml $(srcdir)/xml.dcl yaz.xml

yaz.pdf: $(XMLFILES) $(srcdir)/id.png
	if test ! -f id.png ; then ln -s $(srcdir)/id.png .; fi
	jade -E14 -D $(srcdir) -d yazprint.dsl -t tex $(srcdir)/xml.dcl yaz.xml
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex

index.tkl: $(XMLFILES) tkl.xsl
	xsltproc tkl.xsl yaz.xml

