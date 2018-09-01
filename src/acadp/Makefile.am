## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -I. -I$(top_srcdir)/src/aclib/ac_decoder -DYYERROR_VERBOSE

## yacc options
AM_YFLAGS = -d -p adf
AM_LFLAGS = -P adf -o adele_lex.c

## The ADeLe Description parser library 
noinst_LTLIBRARIES = libacadp.la
BUILT_SOURCES = adele_grammar.h
libacadp_la_SOURCES = adele_grammar.y adele_lex.l adele_grammar.h acadp.c model.c approx.c group.c op.c defaults.c acadp.h

CLEANFILES = adele_grammar.tab.h adele_lex.c adele_grammar.c adele_grammar.h
