#! /bin/bash

pdflatex fracdesign.tex
bibtex fracdesign
pdflatex fracdesign.tex
