## $Id: Makefile.am,v 1.21 2001-10-28 23:44:59 adam Exp $

docdir=$(pkgdatadir)/doc

XMLFILES=$(srcdir)/yaz.xml $(srcdir)/introduction.xml \
 $(srcdir)/installation.xml $(srcdir)/indexdata.xml $(srcdir)/asn.xml \
 $(srcdir)/tools.xml $(srcdir)/odr.xml $(srcdir)/comstack.xml \
 $(srcdir)/frontend.xml $(srcdir)/license.xml $(srcdir)/future.xml  \
 $(srcdir)/client.xml $(srcdir)/zoom.xml

HTMLFILES = asn.external.html asn.html asn.oid.html asn.pdu.html \
 asn.preparing.html client.commands.html client.html client.invoking.html \
 client.searching.html comstack.addresses.html comstack.client.html \
 comstack.common.html comstack.diagnostics.html comstack.html \
 comstack.introduction.html comstack.server.html comstack.summary.html \
 future.html indexdata.html installation.html installation.unix.html \
 installation.win32.html introduction.html \
 license.html license.other.html odr.debugging.html \
 odr.html odr.programming.html odr.use.html server.backend.html \
 server.backendfunctions.html server.frontend.html server.html \
 server.invocation.html server.main.html tools.html tools.nmem.html \
 tools.oid.html yaz.html zoom.events.html zoom.html \
 zoom.options.html zoom.records.html zoom.resultsets.html zoom.search.html

DOCFILES=$(XMLFILES) $(HTMLFILES) yaz.pdf

EXTRA_DIST = $(DOCFILES)

doc_DATA = $(DOCFILES)

$(HTMLFILES): $(XMLFILES) $(srcdir)/yazhtml.dsl
	cd $(srcdir); jade -E14 -d yazhtml.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/yaz.php: $(XMLFILES) $(srcdir)/yazphp.dsl
	cd $(srcdir); jade -E14 -d yazphp.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/yaz.pdf: $(srcdir)/yaz.tex
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex
	pdfjadetex yaz.tex

$(srcdir)/yaz.tex: $(XMLFILES) $(srcdir)/yazprint.dsl
	cd $(srcdir); jade -d yazprint.dsl -t tex xml.dcl yaz.xml
