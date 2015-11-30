bin_PROGRAMS = nct
nct_SOURCES = nct.c nct.h score.c score.h
nct_LDADD = ../lib/libgnu.a

confpaths.h: Makefile
	echo "#define SCOREDIR \"$(localstatedir)/games\"" > $@

score.o: confpaths.h

DISTCLEANFILES = confpaths.h
