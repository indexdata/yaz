<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY docbook.dsl SYSTEM "/usr/share/sgml/docbook/stylesheet/dsssl/modular/html/docbook.dsl"
  CDATA DSSSL>
]>
<!--
  $Id: yazhtml.dsl,v 1.5 2001-10-22 13:57:24 adam Exp $
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
  
<!--
Local Variables:
mode: scheme
End:
-->
