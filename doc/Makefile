SGMLF=sgml-format
SGMLR=sgml-roff
SGMLT=sgml-tex

all: yaz.ps yaz.txt profiles.txt profiles.ps

yaz.txt: yaz.sgml
	$(SGMLF) -Tnroff yaz.sgml | $(SGMLR) | sed -e 's/	/        /g' > yaz.txt

profiles.txt: profiles.sgml
	$(SGMLF) -Tnroff profiles.sgml | $(SGMLR) | sed -e 's/	/        /g' > profiles.txt

yaz.ps: yaz.sgml
	$(SGMLF) yaz.sgml | $(SGMLT) -x
	mv out.ps yaz.ps

profiles.ps: profiles.sgml
	$(SGMLF) profiles.sgml | $(SGMLT) -x
	mv out.ps profiles.ps
