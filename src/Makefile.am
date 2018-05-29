.NOTPARALLEL:

bin_PROGRAMS = ss

ss_SOURCES  = ss.c
ss_SOURCES += util.h util.c
ss_SOURCES += andl-lexer.l andl-parser.y
ss_SOURCES += ctl.h ctl.c
ss_SOURCES += smc.h smc.c
ss_SOURCES += state_space.h state_space.c

ss_YFLAGS = -d
ss_CFLAGS = $(SYLVAN_CFLAGS) $(XML_CFLAGS)
ss_LDADD = $(SYLVAN_LIBS) $(XML_LIBS)

EXTRA_DIST  = andl-lexer.c andl-lexer.h
EXTRA_DIST += ss-andl-parser.c ss-andl-parser.h

BUILT_SOURCES  = andl-lexer.c andl-lexer.h
BUILT_SOURCES += ss-andl-parser.c ss-andl-parser.h

CLEANFILES  = andl-lexer.c andl-lexer.h
CLEANFILES += ss-andl-parser.c ss-andl-parser.h

