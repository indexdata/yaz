## $Id: Makefile.am,v 1.12 2001-07-19 12:46:57 adam Exp $

docdir=$(pkgdatadir)/doc

doc_DATA = $(srcdir)/yaz.sgml $(srcdir)/profiles.sgml \
 yaz.txt yaz.ps profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

EXTRA_DIST = $(srcdir)/yaz.sgml $(srcdir)/profiles.sgml \
 yaz.txt yaz.ps profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

yaz.txt: $(srcdir)/yaz.sgml
	sgml2txt -f $(srcdir)/yaz.sgml

yaz.dvi: $(srcdir)/yaz.sgml
	sgml2latex -p a4 -o dvi $(srcdir)/yaz.sgml

yaz.ps: $(srcdir)/yaz.sgml
	sgml2latex -p a4 -o ps $(srcdir)/yaz.sgml

profiles.txt: $(srcdir)/profiles.sgml
	sgml2txt -f $(srcdir)/profiles.sgml

profiles.dvi: $(srcdir)/profiles.sgml
	sgml2latex -p a4 -o dvi $(srcdir)/profiles.sgml

profiles.ps: $(srcdir)/profiles.sgml
	sgml2latex -p a4 -o ps $(srcdir)/profiles.sgml

yaz.html: $(srcdir)/yaz.sgml
	sgml2html $(srcdir)/yaz.sgml

XMLFILES=$(srcdir)/yaz.xml $(srcdir)/introduction.xml \
 $(srcdir)/installation.xml $(srcdir)/indexdata.xml $(srcdir)/asn.xml \
 $(srcdir)/tools.xml $(srcdir)/odr.xml $(srcdir)/comstack.xml \
 $(srcdir)/frontend.xml $(srcdir)/license.xml $(srcdir)/future.xml 

$(srcdir)/book1.html: $(XMLFILES) $(srcdir)/yazhtml.dsl
	cd $(srcdir); jade -E14 -d yazhtml.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/book1.php: $(XMLFILES) $(srcdir)/yazphp.dsl
	cd $(srcdir); jade -E14 -d yazphp.dsl -t sgml xml.dcl yaz.xml

$(srcdir)/yaz.tex: $(XMLFILES) $(srcdir)/yazprint.dsl
	cd $(srcdir); jade -d yazprint.dsl -t tex xml.dcl yaz.xml
