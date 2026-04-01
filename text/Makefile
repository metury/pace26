TEX = latexmk -output-directory=OUT
all: main.pdf

%.pdf: force
	$(TEX) $*.tex

view:
	xdg-open OUT/main.pdf

clean:
	rm -rf OUT

.PHONY: force all clean
