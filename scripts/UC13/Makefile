VENVDIR:=../../.venv
PYTHON:=$(VENVDIR)/bin/python

CODEFILE:=/data/ackley/PART4/code/D/MFMx10/src/spikes/riscv10/code/riscv10.dtoml
#CODEFILE:=/data/ackley/PART4/code/D/MFMx10/src/spikes/riscv10/code/riscv11.dtoml

GENDIR:=/data/ackley/PART4/code/D/MFMx10/gen
GENFILE:=$(GENDIR)/UCLGENERATEDCODE.inc

all run:	demo


demo:	$(GENDIR)
	$(PYTHON) Main.py $(CODEFILE) >$(GENFILE)

debug:	$(GENDIR)
	$(PYTHON) -m pdb Main.py $(CODEFILE)

$(GENDIR):	FORCE
	mkdir -p $@

.PHONY:	FORCE
