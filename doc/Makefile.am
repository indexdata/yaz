## This file is part of the YAZ toolkit.
## Copyright (C) 1995-2009 Index Data

SUBDIRS = common

XMLFILES=bookinfo.xml introduction.xml installation.xml \
 indexdata.xml \
 asn.xml tools.xml odr.xml comstack.xml server.xml license.xml \
 future.xml zoom.xml credits.xml gfs-options.xml \
 yaz.xml yaz-client-commands.xml soap.xml gfs-virtual.xml gfs-synopsis.xml \
 std-oid-table.xml manref.xml

HTMLFILES = index.html

MANFILES=yaz-client.1 yaz-ztest.8 \
	yaz-config.8 yaz.7 zoomsh.1 yaz-asncomp.1 \
	yaz-marcdump.1 yaz-iconv.1 yaz-log.7 \
	yaz-illclient.1 yaz-icu.1 bib1-attr.7
REFFILES=yaz-client-man.xml yaz-ztest-man.xml yaz-config-man.xml \
	yaz-man.xml zoomsh-man.xml yaz-asncomp-man.xml \
	yaz-marcdump-man.xml yaz-iconv-man.xml yaz-log-man.xml \
	yaz-illclient-man.xml yaz-icu-man.xml bib1-attr-man.xml

SUPPORTFILES=entities.ent apilayer.obj local.ent.in

doc_DATA = $(HTMLFILES) apilayer.png 
man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(SUPPORTFILES) $(man_MANS) $(REFFILES) \
	$(doc_DATA)

std-oid-table.xml: $(srcdir)/../src/oid.csv
	$(TCLSH) $(srcdir)/../src/oidtoc.tcl $(srcdir) $(srcdir)/../src/oid.csv std-oid-table.xml

yaz-client.1: $(srcdir)/yaz-client-man.xml $(srcdir)/yaz-client-commands.xml
	$(MAN_COMPILE) $(srcdir)/yaz-client-man.xml

yaz-ztest.8: yaz-ztest-man.xml gfs-options.xml gfs-synopsis.xml gfs-virtual.xml
	$(MAN_COMPILE) $(srcdir)/yaz-ztest-man.xml

yaz-config.8: yaz-config-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-config-man.xml

yaz.7: yaz-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-man.xml

bib1-attr.7: bib1-attr-man.xml
	$(MAN_COMPILE) $(srcdir)/bib1-attr-man.xml

zoomsh.1: zoomsh-man.xml
	$(MAN_COMPILE) $(srcdir)/zoomsh-man.xml

yaz-asncomp.1: yaz-asncomp-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-asncomp-man.xml

yaz-marcdump.1: yaz-marcdump-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-marcdump-man.xml

yaz-iconv.1: yaz-iconv-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-iconv-man.xml

yaz-illclient.1: yaz-illclient-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-illclient-man.xml

yaz-log.7: yaz-log-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-log-man.xml

yaz-icu.1: yaz-icu-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-icu-man.xml

$(HTMLFILES): $(XMLFILES)
	rm -f *.html
	$(HTML_COMPILE) $(srcdir)/yaz.xml

yaz.pdf: $(XMLFILES)
	$(PDF_COMPILE) $(srcdir)/yaz.xml


yazj.pdf:
	jade -E14 -D $(srcdir) -d common/print.dsl -t tex $(srcdir)/common/xml.dcl $(srcdir)/yaz.xml
	rm -f yazj.pdf
	cp yaz.tex yazj.tex
	pdfjadetex yazj.tex
	pdfjadetex yazj.tex >/dev/null
	pdfjadetex yazj.tex >/dev/null

manref.xml: $(REFFILES) $(srcdir)/common/stripref.xsl
	rm -f manref.xml
	for i in $(REFFILES); do \
		xsltproc $(srcdir)/common/stripref.xsl $(srcdir)/$$i | sed 1d >>manref.xml; \
	done

apilayer.png: 
	tgif -print -xbm apilayer.obj
	xbmtopbm <apilayer.xbm|pnmtopng >apilayer.png

dist-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		cp $$p $(distdir); \
	done

doc-clean:
	rm -f *.html *.[0-9] *.pdf

install-data-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		$(INSTALL_DATA) $$p $(DESTDIR)$(docdir); \
	done

uninstall-hook:
	rm -r $(DESTDIR)$(docdir)
