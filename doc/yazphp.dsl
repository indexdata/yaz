<!DOCTYPE style-sheet PUBLIC "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY docbook.dsl SYSTEM "/usr/share/sgml/docbook/stylesheet/dsssl/modular/html/docbook.dsl"
  CDATA DSSSL>
]>
<!--
  $Id: yazphp.dsl,v 1.4 2001-10-22 13:57:24 adam Exp $
-->
<style-sheet>
<style-specification use="docbook">
<style-specification-body>

(define %html-ext% ".php")
(define %shade-verbatim% #t)

(define newline "\U-000D")

(define (html-document title-sosofo body-sosofo)
  (let* (;; Let's look these up once, so that we can avoid calculating
         ;; them over and over again.
         (prev         (prev-chunk-element))
         (next         (next-chunk-element))
         (prevm        (prev-major-component-chunk-element))
         (nextm        (next-major-component-chunk-element))
         (navlist      (list prev next prevm nextm))
	 
         ;; Let's make it possible to control the output even in the
         ;; nochunks case. Note: in the nochunks case, (chunk?) will
         ;; return #t for only the root element.
         (make-entity? (and (or (not nochunks) rootchunk)
                            (chunk?)))
	 
         (make-head?   (or make-entity?
                           (and nochunks
                                (node-list=? (current-node)
                                             (sgml-root-element)))))
         (doc-sosofo 
          (if make-head?
	      (make sequence
		(make formatting-instruction data:
		      (string-append "<" "?php "
				     newline
				     "require \"../../id_common.inc\";"
				     newline
				     "id_header(\""
				     )
		      )
		title-sosofo
		(make formatting-instruction data:
		      (string-append "\");"
				     newline
				     "?" ">"
				     )
		      )
		(header-navigation (current-node) navlist)
		body-sosofo
		(footer-navigation (current-node) navlist)
		(make formatting-instruction data:
		      (string-append "<" "?php id_footer() ?>")
		      )
		)
	      body-sosofo
	      )
	  )
	 )
    (if make-entity?
	(make entity
	  system-id: (html-entity-file (html-file))
	  (if %html-pubid%
	      (make document-type
		name: "HTML"
		public-id: %html-pubid%)
	      (empty-sosofo))
	  doc-sosofo)
	(if (node-list=? (current-node) (sgml-root-element))
	    (make sequence
	      (if %html-pubid%
		  (make document-type
		    name: "HTML"
		    public-id: %html-pubid%)
		  (empty-sosofo))
	      doc-sosofo)
	    doc-sosofo)
	)
    )
  )

</style-specification-body>
</style-specification>
<external-specification id="docbook" document="docbook.dsl">
</style-sheet>

<!--
Local Variables:
mode: scheme
End:
-->
