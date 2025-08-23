SHELL := /usr/bin/bash

VENVDIR:=$(abspath ../../../../.venv)
VENVACT:=$(VENVDIR)/bin/activate

all:	gen

gen:	venv
	source $(VENVACT) && \
	python3 gen.py

venv:	$(VENVACT)
	source $(VENVACT) && echo "VENV READY"

$(VENVDIR):	
	mkdir -p $@

$(VENVACT):	$(VENVDIR) 
	python3 -m venv $^
	source $@ && pip install -r requirements.txt

realclean:	clean
	rm -rf $(VENVDIR)

clean:	FORCE
	rm -f *~

.PHONY:	FORCE
