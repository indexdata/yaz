## $Id: Makefile.am,v 1.2 2000-02-28 11:44:39 adam Exp $

docdir=$(pkgdatadir)/doc

doc_DATA = \
 yaz.sgml yaz.txt yaz.ps profile.txt profiles.ps yaz.html \
 yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
 yaz-6.html yaz-7.html yaz-8.htlm yaz-9.html

#EXTRA_DIST = \
# yaz.sgml yaz.txt yaz.ps profile.txt profiles.ps yaz.html \
# yaz-1.html yaz-2.html yaz-3.html yaz-4.html yaz-5.html \
# yaz-6.html yaz-7.html yaz-8.htlm yaz-9.html

yaz.txt: yaz.sgml
	sgml2txt -f yaz.sgml

yaz.ps: yaz.sgml
	sgml2ps yaz.sgml

profiles.txt: profiles.sgml
	sgml2txt -f profiles.sgml

profiles.ps: profiles.sgml
	sgml2ps	profiles.sgml

yaz.html: yaz.sgml
	sgml2html yaz.sgml
