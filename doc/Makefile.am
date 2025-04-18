## This file is part of the YAZ toolkit.
## Copyright (C) Index Data

SUBDIRS = common

XMLFILES = book.xml \
 gfs-options.xml gfs-virtual.xml gfs-synopsis.xml \
 std-oid-table.xml  bib1-diag-table.xml srw-diag-table.xml \
 manref.xml local.ent

HTMLFILES = index.html

MANFILES=yaz-client.1 yaz-ztest.8 \
	yaz-config.1 yaz.7 zoomsh.1 yaz-asncomp.1 \
	yaz-marcdump.1 yaz-iconv.1 yaz-log.7 \
	yaz-illclient.1 yaz-icu.1 yaz-url.1 bib1-attr.7 \
	yaz-json-parse.1 yaz-record-conv.1
REFFILES=yaz-client-man.xml yaz-ztest-man.xml yaz-config-man.xml \
	yaz-man.xml zoomsh-man.xml yaz-asncomp-man.xml \
	yaz-marcdump-man.xml yaz-iconv-man.xml yaz-log-man.xml \
	yaz-illclient-man.xml yaz-icu-man.xml yaz-url-man.xml \
	bib1-attr-man.xml yaz-json-parse-man.xml yaz-record-conv-man.xml

SUPPORTFILES=entities.ent apilayer.obj

doc_DATA = $(HTMLFILES) apilayer.png 
man_MANS = $(MANFILES)

EXTRA_DIST = $(XMLFILES) $(SUPPORTFILES) $(man_MANS) $(REFFILES) \
	$(doc_DATA)

std-oid-table.xml: $(srcdir)/../src/oid.csv
	$(TCLSH) $(top_srcdir)/src/oidtoc.tcl  $(top_srcdir)/src/oid.csv std-oid-table.xml

bib1-diag-table.xml: $(srcdir)/../src/bib1.csv
	$(TCLSH) $(srcdir)/../src/csvtodiag.tcl $(srcdir)/../src/bib1.csv bib1-diag-table.xml bib1-diag-table

srw-diag-table.xml: $(srcdir)/../src/srw.csv
	$(TCLSH) $(srcdir)/../src/csvtodiag.tcl $(srcdir)/../src/srw.csv srw-diag-table.xml srw-diag-table

yaz-client.1: $(srcdir)/yaz-client-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-client-man.xml

yaz-ztest.8: yaz-ztest-man.xml gfs-options.xml gfs-synopsis.xml gfs-virtual.xml
	$(MAN_COMPILE) $(srcdir)/yaz-ztest-man.xml

yaz-config.1: yaz-config-man.xml
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

yaz-url.1: yaz-url-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-url-man.xml

yaz-json-parse.1: yaz-json-parse-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-json-parse-man.xml

yaz-record-conv.1: yaz-record-conv-man.xml
	$(MAN_COMPILE) $(srcdir)/yaz-record-conv-man.xml

$(HTMLFILES): $(XMLFILES)
	rm -f *.html
	$(HTML_COMPILE) $(srcdir)/book.xml

$(MANFILES): local.ent

yaz.pdf: $(XMLFILES)
	$(PDF_COMPILE) $(srcdir)/book.xml && mv book.pdf yaz.pdf


yazj.pdf:
	jade -E14 -D $(srcdir) -d common/print.dsl -t tex $(srcdir)/common/xml.dcl $(srcdir)/book.xml
	rm -f yazj.pdf
	cp book.tex yazj.tex
	pdfjadetex yazj.tex
	pdfjadetex yazj.tex >/dev/null
	pdfjadetex yazj.tex >/dev/null

manref.xml: $(REFFILES) $(srcdir)/common/stripref.xsl local.ent
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
	rm -f manref.xml *.html *.[0-9] *.pdf toc.hhc htmlhelp.hhp

install-data-hook:
	if test -f index.html; then d=.; else d="$(srcdir)"; fi; \
	for p in $$d/*.html; do \
		$(INSTALL_DATA) $$p $(DESTDIR)$(docdir); \
	done

uninstall-hook:
	rm -r $(DESTDIR)$(docdir)

distclean-local: doc-clean
