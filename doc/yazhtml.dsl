<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY docbook.dsl SYSTEM "/usr/share/sgml/docbook/stylesheet/dsssl/modular/html/docbook.dsl"
  CDATA DSSSL>
]>
<!--
  $Id: yazhtml.dsl,v 1.6 2001-10-24 09:27:59 adam Exp $
-->
<style-sheet>
<style-specification use="docbook">
<style-specification-body>

(define %use-id-as-filename% #t)
(define %output-dir% "html")
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
