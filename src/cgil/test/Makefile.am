bin_PROGRAMS = \
	cgil-c cgil-crack cgil-csharp cgil-d \
	cgil-go cgil-java cgil-js cgil-julia \
	cgil-ocaml cgil-ruby cgil-rust

check_SCRIPTS = subject.mk subject.sh

pkgdata_SCRIPTS = runtests

EXTRA_DIST = subject.mk.in subject.sh.in runtests

subject.mk: subject.mk.in Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

subject.sh: subject.sh.in Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

runtests: runtests.in Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

CLEANFILES = subject.mk subject.sh

cgil-c$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-c.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-crack$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-crack.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-csharp$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-csharp.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-d$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-d.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-go$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-go.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-java$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-java.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-js$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-js.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-julia$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-julia.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-ocaml$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-ocaml.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-ruby$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-ruby.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<

cgil-rust$(EXEEXT): $(top_srcdir)/src/cgil/rlhc-rust.lm $(top_srcdir)/src/cgil/ril.lm
	$(SUBJ_COLM_BIN) -I ../ -o $@ $<
