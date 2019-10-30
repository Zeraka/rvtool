dot -> pdf:
"""""""""""

1. dot -> eps
dot -Teps <dot file> > <new eps file>

2. eps -> pdf
epstopdf <eps file>

Example:
dot -Teps acmodel.dot > acmodel.eps
epstopdf acmodel.eps

3. pdf rotation:
pdf90 acmodel.pdf
