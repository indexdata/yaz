<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY docbook.dsl SYSTEM "../../docbook-dsssl/html/docbook.dsl"
  CDATA DSSSL>
]>
<!--
  $Id: yazhtml.dsl,v 1.4 2001-08-14 11:50:07 adam Exp $
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
