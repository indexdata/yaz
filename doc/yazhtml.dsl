<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY docbook.dsl SYSTEM "/home/adam/proj/docbook-dsssl-1.70/html/docbook.dsl" CDATA DSSSL>
<!-- ENTITY docbook.dsl SYSTEM "/usr/lib/sgml/stylesheet/dsssl/docbook/nwalsh/html/docbook.dsl" CDATA DSSSL -->
<!ENTITY html-common.dsl SYSTEM "./html-common.dsl">
]>
<!--
  $Header: /home/cvsroot/yaz/doc/Attic/yazhtml.dsl,v 1.2 2001-07-19 12:46:57 adam Exp $
-->
<style-sheet>
<style-specification use="docbook">
<style-specification-body>

(define %html-ext% ".html")
(define %shade-verbatim% #t)

</style-specification-body>
</style-specification>
<external-specification id="docbook" document="docbook.dsl">
</style-sheet>
  
