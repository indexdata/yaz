SGMLF=sgml-format
SGMLR=sgml-roff
SGMLT=sgml-tex

all: yaz.ps yaz.txt

yaz.txt: yaz.sgml
	$(SGMLF) -Tnroff yaz.sgml | $(SGMLR) | sed -e 's/	/        /g' > yaz.txt

yaz.ps: yaz.sgml
	$(SGMLF) yaz.sgml | $(SGMLT) -x
	mv out.ps yaz.ps
