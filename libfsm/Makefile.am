# libfsm contains only the FSM construction code and the backend code
# generators for C, asm and cgil (Code Gen Intermediate Language) . It is
# useful for building state machine code generators in programs not connected
# to the ragel language.
lib_LTLIBRARIES = libfsm.la

pkginclude_HEADERS = \
	action.h fsmgraph.h common.h \
	gendata.h redfsm.h dot.h

# nodist_pkginclude_HEADERS = config.h

#
# libfsm: state machine construction and direct code generation.
#
libfsm_la_CPPFLAGS = -I$(top_srcdir)/aapl

dist_libfsm_la_SOURCES = \
	buffer.h codegen.h \
	actloop.h actexp.h \
	tables.h \
	binary.h bingoto.h binbreak.h binvar.h \
	flat.h flatgoto.h flatbreak.h flatvar.h \
	switch.h switchgoto.h switchbreak.h switchvar.h \
	goto.h gotoloop.h gotoexp.h \
	ipgoto.h asm.h \
	idbase.cc fsmstate.cc fsmbase.cc fsmattach.cc fsmmin.cc fsmgraph.cc \
	fsmap.cc fsmcond.cc fsmnfa.cc common.cc redfsm.cc gendata.cc \
	allocgen.cc codegen.cc \
	actexp.cc binvar.cc \
	tables.cc tabgoto.cc tabbreak.cc tabvar.cc \
	binary.cc bingoto.cc binbreak.cc actloop.cc \
	flat.cc flatgoto.cc flatbreak.cc flatvar.cc \
	switch.cc switchgoto.cc switchbreak.cc switchvar.cc \
	goto.cc gotoloop.cc gotoexp.cc ipgoto.cc \
	dot.cc asm.cc

nodist_libfsm_la_SOURCES = \
	version.h

libfsm_la_LDFLAGS = -no-undefined

if LINKER_NO_UNDEFINED
libfsm_la_LDFLAGS += -Wl,--no-undefined
endif

