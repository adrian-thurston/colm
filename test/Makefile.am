SUBDIRS = . aapl.d colm.d rlhc.d rlparse.d trans.d

check_SCRIPTS = subject.mk subject.sh

pkgdata_DATA = runtests

EXTRA_DIST = subject.mk.in subject.sh.in runtests

subject.mk: subject.mk.in Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

subject.sh: subject.sh.in Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

dist: dist.sh
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

CLEANFILES = subject.mk subject.sh
