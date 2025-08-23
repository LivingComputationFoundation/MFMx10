# SUBDIRS here are expected to be independent of each other
#ifeq ($(PLATFORM),tile)
#SUBDIRS= mfmt2 mfzrun stub
#else
SUBDIRS= riscv10
#endif

.PHONY:	$(SUBDIRS) all clean realclean run

all:	$(SUBDIRS)

run:	$(SUBDIRS)

clean:	$(SUBDIRS)
	@rm -f *~

realclean:	clean $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@ $(MAKECMDGOALS)
