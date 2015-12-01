bin_PROGRAMS = nct
nct_SOURCES = nct.c nct.h score.c score.h

confpaths.h: Makefile
	echo "#define SCOREDIR \"$(localstatedir)/games\"" > $@

score.o: confpaths.h

DISTCLEANFILES = confpaths.h
