all: yaz.ps yaz.txt profiles.txt profiles.ps

yaz.txt: yaz.sgml
	sgml2txt -f yaz.sgml

profiles.txt: profiles.sgml
	sgml2txt -f profiles.sgml

yaz.ps: yaz.sgml
	sgml2ps yaz.sgml

profiles.ps: profiles.sgml
	sgml2ps	profiles.sgml

yaz.html: yaz.sgml
	sgml2html yaz.sgml
