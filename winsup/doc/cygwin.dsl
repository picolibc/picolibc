<!DOCTYPE style-sheet PUBLIC
          "-//James Clark//DTD DSSSL Style Sheet//EN" [
<!ENTITY % html "IGNORE">
<![%html;[
<!ENTITY % print "IGNORE">
<!ENTITY docbook.dsl PUBLIC
         "-//Norman Walsh//DOCUMENT DocBook HTML Stylesheet//EN"
	 CDATA dsssl>
]]>
<!ENTITY % print "INCLUDE">
<![%print;[
<!ENTITY docbook.dsl PUBLIC
         "-//Norman Walsh//DOCUMENT DocBook Print Stylesheet//EN"
	 CDATA dsssl>
]]>
]>

<style-sheet>

<!--      PRINT       -->
<style-specification id="print" use="docbook">
<style-specification-body> 

;; The following are 
;; <!-- Cygnus customizations by Mark Galassi -->
;; ====================
;; customize the print stylesheet
;; ====================

;; make funcsynopsis look pretty
(define %funcsynopsis-decoration%
  ;; Decorate elements of a FuncSynopsis?
  #t)

;; use graphics in admonitions, and have their path be "."
;; NO: we are not yet ready to use gifs in TeX and so forth
(define %admon-graphics-path%
  "./")
(define %admon-graphics%
  #f)

;; this is necessary because right now jadetex does not understand
;; symbolic entities, whereas things work well with numeric entities.
(declare-characteristic preserve-sdata?
          "UNREGISTERED::James Clark//Characteristic::preserve-sdata?"
          #f)
(define %two-side% #t)

(define %section-autolabel% 
  ;; Are sections enumerated?
  #t)
;; (define %title-font-family% 
;;   ;; The font family used in titles
;;   "Ariel")
(define %visual-acuity%
  ;; General measure of document text size
  ;; "presbyopic"
  ;; "large-type"
  "presbyopic")

(define %generate-part-toc% #t)


;;; The following customizations are from Tim Waugh's selfdocbook
;;; http://cyberelk.net/tim/docbook/
;;; 
;;; TeX backend can go to PS (where EPS is needed)
;;; or to PDF (where PNG is needed).  So, just
;;; omit the file extension altogether and let
;;; tex/pdfjadetex sort it out on its own.
(define (graphic-file filename)
 (let ((ext (file-extension filename)))
  (if (or (equal? 'backend 'tex) ;; Leave off the extension for TeX
          (not filename)
          (not %graphic-default-extension%)
          (member ext %graphic-extensions%))
      filename
      (string-append filename "." %graphic-default-extension%))))

;;; Full justification.
(define %default-quadding%
 'justify)

;;; To make URLs line wrap we use the TeX 'url' package.
;;; See also: jadetex.cfg
;; First we need to declare the 'formatting-instruction' flow class.
(declare-flow-object-class formatting-instruction
"UNREGISTERED::James Clark//Flow Object Class::formatting-instruction")
;; Then redefine ulink to use it.
(element ulink
  (make sequence
    (if (node-list-empty? (children (current-node)))
	; ulink url="...", /ulink
	(make formatting-instruction
	  data: (string-append "\\url{"
			       (attribute-string (normalize "url"))
			       "}"))
	(if (equal? (attribute-string (normalize "url"))
		    (data-of (current-node)))
	; ulink url="http://...", http://..., /ulink
	    (make formatting-instruction data:
		  (string-append "\\url{"
				 (attribute-string (normalize "url"))
				 "}"))
	; ulink url="http://...", some text, /ulink
	    (make sequence
	      ($charseq$)
	      (literal " (")
	      (make formatting-instruction data:
		    (string-append "\\url{"
				   (attribute-string (normalize "url"))
				   "}"))
	      (literal ")"))))))
;;; And redefine filename to use it too.
(element filename
  (make formatting-instruction
    data: (string-append "\\path{" (data-of (current-node)) "}")))

</style-specification-body>
</style-specification>

<!--      HTML       -->
<style-specification id="html" use="docbook">
<style-specification-body> 

;; If true (non-zero), elements of the FuncSynopsis will be decorated 
;; (e.g. bold or italic).
(define %funcsynopsis-decoration% #t)

;; If true, a Table of Contents will be generated for each 'Article'.
(define %generate-article-toc% #t)

;; If true, a Table of Contents will be generated for each Part.
(define %generate-part-toc% #t)

;; The name of the stylesheet to place in the HTML LINK TAG, 
;; or #f to suppress the stylesheet LINK.
(define %stylesheet% "docbook.css")

(define %use-id-as-filename% #t)

(define %html-ext% ".html")

</style-specification-body>
</style-specification>

<external-specification id="docbook" document="docbook.dsl">

</style-sheet>
