## $Id: Makefile.am,v 1.9 2001-01-04 13:36:24 adam Exp $

docdir=$(pkgdatadir)/doc

doc_DATA = yaz.sgml profiles.sgml \
 yaz.txt yaz.ps profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

EXTRA_DIST = yaz.sgml profiles.sgml \
 yaz.txt yaz.ps profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

yaz.txt: yaz.sgml
	sgml2txt -f yaz.sgml

yaz.ps: yaz.sgml
	sgml2latex -p a4 -o ps yaz.sgml

profiles.txt: profiles.sgml
	sgml2txt -f profiles.sgml

profiles.ps: profiles.sgml
	sgml2latex -p a4 -o ps profiles.sgml

yaz.html: yaz.sgml
	sgml2html yaz.sgml

XMLFILES=yaz.xml introduction.xml installation.xml indexdata.xml \
  asn.xml tools.xml odr.xml comstack.xml frontend.xml license.xml future.xml

book1.htm: $(XMLFILES)
	jade -E14 -d ../../docbook/docbook/html/docbook.dsl -t sgml xml.dcl yaz.xml

yaz.tex: $(XMLFILES)
	jade -d ../../docbook/docbook/print/docbook.dsl -t tex xml.dcl yaz.xml
