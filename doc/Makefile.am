## $Id: Makefile.am,v 1.6 2000-04-05 07:39:54 adam Exp $

docdir=$(pkgdatadir)/doc

doc_DATA = yaz.sgml 
x = \
 yaz.txt yaz.ps profiles.sgml profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

EXTRA_DIST = yaz.sgml 
y = \
 yaz.txt yaz.ps profiles.sgml profiles.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.html yaz-9.html

yaz.txt: yaz.sgml
	sgml2txt -f yaz.sgml

yaz.ps: yaz.sgml
	sgml2latex -o ps yaz.sgml

profiles.txt: profiles.sgml
	sgml2txt -f profiles.sgml

profiles.ps: profiles.sgml
	sgml2latex -o ps profiles.sgml

yaz.html: yaz.sgml
	sgml2html yaz.sgml
