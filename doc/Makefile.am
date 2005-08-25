## $Id: Makefile.am,v 1.60 2005-08-25 12:44:24 adam Exp $

docdir=$(datadir)/doc/@PACKAGE@

XMLFILES=yaz.xml.in introduction.xml installation.xml indexdata.xml \
 asn.xml tools.xml odr.xml comstack.xml frontend.xml license.xml \
 future.xml client.xml zoom.xml credits.xml gfs-options.xml \
 yaz-client-commands.xml soap.xml gfs-virtual.xml gfs-synopsis.xml

HTMLFILES = \
 asn.external.html asn.html asn.oid.html asn.pdu.html asn.preparing.html \
 client.commands.html client.html client.invoking.html client.searching.html \
 comstack.addresses.html comstack.client.html comstack.common.html \
 comstack.diagnostics.html comstack.html comstack.introduction.html \
 comstack.server.html comstack.ssl.html comstack.summary.html \
 credits.html future.html \
 indexdata.html installation.html installation.unix.html \
 installation.win32.html introduction.api.html introduction.html \
 license.html license.other.html odr.debugging.html odr.html \
 odr.programming.html odr.use.html server.backendfunctions.html \
 server.backend.html server.frontend.html server.html server.invocation.html \
 server.main.html server.vhosts.html \
 soap.html soap.http.html soap.xml.html soap.srw.html \
 tools.html tools.marc.html tools.nmem.html tools.log.html tools.oid.html \
 yaz.html zoom.events.html zoom.html zoom.options.html zoom.query.html \
 zoom.records.html zoom.resultsets.html zoom.scan.html

MANFILES=yaz-client.1 yaz-ztest.8 \
	yaz-config.8 yaz.7 zoomsh.1 yaz-asncomp.1 \
	yaz-marcdump.1 yaz-iconv.1 yaz-log.7
REFFILES=yaz-client-man.xml yaz-ztest-man.xml yaz-config-man.xml \
	yaz-man.xml zoomsh-man.xml yaz-asncomp-man.xml \
	yaz-marcdump-man.xml yaz-iconv-man.xml yaz-log-man.xml

SUPPORTFILES=yazhtml.dsl yazprint.dsl tkl.xsl xml.dcl id.eps \
	apilayer.obj yaz.css

doc_DATA = $(HTMLFILES) yaz.pdf id.png apilayer.png yaz.css
man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(SUPPORTFILES) $(man_MANS) $(REFFILES) \
	$(doc_DATA) Doxyfile

yaz-client.1: $(srcdir)/yaz-client-man.xml $(srcdir)/yaz-client-commands.xml
	docbook2man $(srcdir)/yaz-client-man.xml

yaz-ztest.8: yaz-ztest-man.xml gfs-options.xml gfs-synopsis.xml gfs-virtual.xml
	docbook2man $(srcdir)/yaz-ztest-man.xml

yaz-config.8: yaz-config-man.xml
	docbook2man $(srcdir)/yaz-config-man.xml

yaz.7: yaz-man.xml
	docbook2man $(srcdir)/yaz-man.xml

zoomsh.1: zoomsh-man.xml
	docbook2man $(srcdir)/zoomsh-man.xml

yaz-asncomp.1: yaz-asncomp-man.xml
	docbook2man $(srcdir)/yaz-asncomp-man.xml

yaz-marcdump.1: yaz-marcdump-man.xml
	docbook2man $(srcdir)/yaz-marcdump-man.xml

yaz-iconv.1: yaz-iconv-man.xml
	docbook2man $(srcdir)/yaz-iconv-man.xml

yaz-log.7: yaz-log-man.xml
	docbook2man $(srcdir)/yaz-log-man.xml

$(HTMLFILES): $(XMLFILES)  
	jade -E14 -D $(srcdir) -d yazhtml.dsl -t sgml $(srcdir)/xml.dcl yaz.xml

yaz.pdf: $(XMLFILES) $(srcdir)/id.png
	if test ! -f id.png ; then ln -s $(srcdir)/id.png .; fi
	if test ! -f apilayer.png ; then ln -s $(srcdir)/apilayer.png .; fi
	jade -E14 -D $(srcdir) -d yazprint.dsl -t tex $(srcdir)/xml.dcl yaz.xml
	pdfjadetex yaz.tex >/dev/null
	pdfjadetex yaz.tex >/dev/null
	pdfjadetex yaz.tex >/dev/null

index.tkl: $(XMLFILES) tkl.xsl
	xsltproc tkl.xsl yaz.xml

apilayer.png: 
	tgif -print -xbm apilayer.obj
	xbmtopbm <apilayer.xbm|pnmtopng >apilayer.png

dist-hook:
	for f in $(srcdir)/*.html; do \
		found=0; \
		b=`basename $$f`; \
		for h in $(HTMLFILES); do \
			if test "$$h" = "$$b"; then \
				found=1; \
			fi \
		done; \
		if test "$$found" = "0"; then \
			echo "$$b not found in HTMLFILES"; \
			exit 1; \
		fi \
	done


