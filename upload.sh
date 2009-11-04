#!/bin/sh
P=yaz
set -x
doxygen >out  2>stderr
if test -s stderr; then
	echo "doxygen warnings.. Fix your Doxygen comments!"
	cat stderr
	exit 1
fi
(cd doc && make ${P}.pdf index.html)
cp NEWS doc/
tar cz --exclude=.git -f - dox doc|ssh website "cd software/${P}; tar xzf -"
exit 0
