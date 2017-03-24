ACLOCAL_AMFLAGS=-I m4
AUTOMAKE_OPTIONS = foreign
SUBDIRS = src

bin_PROGRAMS = compass
compass_SOURCES = src/compass.c
LDADD = src/libcompass.la -lm -lgsl -lgslcblas -lcrypto
AM_CPPFLAGS = \
-Isrc \
-Isrc/env \
-Isrc/util
