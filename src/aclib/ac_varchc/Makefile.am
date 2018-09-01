## Process this file with automake to produce Makefile.in

## Includes
AM_CPPFLAGS = -std=c++11 -I. -I$(top_srcdir)/src/aclib/ac_decoder 


## The ArchC library
noinst_LTLIBRARIES = libacvarchc.la

## ArchC library includes
include_HEADERS = vac_control.H vac_models.H vac_storage.H vac_reg.H vac_memport.H vac_utils.H vac_regbank.H vac_stats.H
noinst_HEADERS = elfio/*.hpp

libacvarchc_la_SOURCES = vac_control.cpp vac_utils.cpp vac_stats.cpp
