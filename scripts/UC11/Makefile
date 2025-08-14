VENVDIR:=../../.venv
PYTHON:=$(VENVDIR)/bin/python

CODEFILE:=/data/ackley/PART4/code/D/MFMx10/src/spikes/riscv10/code/riscv10.dtoml
#CODEFILE:=/data/ackley/PART4/code/D/MFMx10/src/spikes/riscv10/code/riscv11.dtoml

all:	demo


demo:	FORCE
	$(PYTHON) Main.py $(CODEFILE)


.PHONY:	FORCE
