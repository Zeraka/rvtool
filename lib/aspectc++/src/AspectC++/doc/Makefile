LYXFILES := ac-compilerman.lyx ac-languageref.lyx
DIRS := CompilerManual LanguageReference
PDFFILES := $(patsubst %.lyx,gen/%.pdf,$(LYXFILES))
XHTMLFILES := $(patsubst %.lyx,gen/%.xhtml,$(LYXFILES))

VPATH = CompilerManual:LanguageReference

all: $(PDFFILES) $(XHTMLFILES)

gen/%.pdf: %.lyx
	@echo Exporting $@
	@lyx -E pdf2 $@ $< > gen/logpdf.txt

gen/%.xhtml: %.lyx
	@echo Exporting $@
	@lyx -E xhtml $@ $< > gen/logxhtml.txt
	@echo Transforming export file
	@mv $@ $@.trans
	@xsltproc gen/fixhtml.xsl $@.trans > $@
	@rm $@.trans
	@echo Renaming PNGs
	@for f in gen/*_doc_*.png; do mv $$f gen/$${f#*_doc_}; done

clean:
	@rm -rf $(PDFFILES) $(XHTMLFILES) gen/log*.txt gen/*~ $(patsubst %, gen/%_*.png, $(DIRS))
