check_SCRIPTS = gentests

EXTRA_DIST = \
	gentests.sh \
	accum1.lm \
	accum2.lm \
	accum3.lm \
	accumbt1.lm \
	accumbt2.lm \
	accumbt3.lm \
	argv1.lm \
	argv2.lm \
	backtrack1.lm \
	backtrack2.lm \
	backtrack3.lm \
	binary1.lm \
	broken/travs2.lm \
	btscan1.lm \
	btscan2.lm \
	call1.lm \
	collect.lm \
	commitbt.lm \
	concat1.lm \
	concat2.lm \
	construct1.lm \
	construct2.lm \
	construct3.lm \
	construct4.lm \
	constructex.lm \
	context1.lm \
	context2.lm \
	context3.lm \
	counting1.lm \
	counting2.lm \
	counting3.lm \
	counting4.lm \
	decl1.lm \
	decl2.lm \
	decl3.lm \
	define1.lm \
	div.lm \
	exit1.lm \
	exit2.lm \
	exit3.lm \
	exit4.lm \
	exit5.lm \
	exit6.lm \
	export1.lm \
	ext1.lm \
	factor1.lm \
	factor2.lm \
	factor3.lm \
	factor4.lm \
	factor5.lm \
	factor6.lm \
	forloop1.lm \
	forloop2.lm \
	forloop3.lm \
	func1.lm \
	func2.lm \
	func3.lm \
	func4.lm \
	generate1.lm \
	generate2.lm \
	heredoc.lm \
	ifblock1.lm \
	ignore1.lm \
	ignore2.lm \
	ignore3.lm \
	ignore4.lm \
	ignore5.lm \
	include1.lm \
	indent.lm \
	inpush1.lm \
	island.lm \
	lhs1.lm \
	liftattrs.lm \
	list1.lm \
	list2.lm \
	list3.lm \
	list4.lm \
	literal1.lm \
	load1.lm \
	logical1.lm \
	lookup1.lm \
	mailbox.lm \
	maketree1.lm \
	map1.lm \
	map2.lm \
	map3.lm \
	map4.lm \
	map5.lm \
	map6.lm \
	matchex.lm \
	maxlen.lm \
	mediawiki/garticle.rl \
	mediawiki/Makefile \
	mediawiki/pdump.rl \
	multiregion1.lm \
	multiregion2.lm \
	mutualrec.lm \
	namespace1.lm \
	nestedcomm.lm \
	new1.lm \
	open1.lm \
	open2.lm \
	order1.lm \
	order2.lm \
	parse1.lm \
	parsetree1.lm \
	pointer1.lm \
	postfix.lm \
	print1.lm \
	prints.lm \
	pull1.lm \
	pull2.lm \
	pushloc.lm \
	ragelambig1.lm \
	ragelambig2.lm \
	ragelambig3.lm \
	ragelambig4.lm \
	rediv.lm \
	ref1.lm \
	ref2.lm \
	ref3.lm \
	reor1.lm \
	reor2.lm \
	reparse.lm \
	repeat1.lm \
	repeat2.lm \
	rhsref1.lm \
	rhsref2.lm \
	rubyhere.lm \
	scan1.lm \
	scope1.lm \
	send1.lm \
	sendstream.lm \
	sprintf.lm \
	stds1.lm \
	streamseq1.lm \
	streamseq2.lm \
	string.lm \
	struct1.lm \
	superid.lm \
	switch1.lm \
	switch2.lm \
	tags1.lm \
	tags2.lm \
	tags3.lm \
	tags4.lm \
	tcontext1.lm \
	til.lm \
	translate1.lm \
	translate2.lm \
	travs1.lm \
	treecmp1.lm \
	typeref1.lm \
	typeref2.lm \
	typeref3.lm \
	undofrag1.lm \
	undofrag2.lm \
	undofrag3.lm \
	undolist1.lm \
	undomap1.lm \
	undomap2.lm \
	utf8.lm \
	void1.lm \
	while1.lm \
	xmlac.lm \
	binary1.in \
	inpush1a.in \
	inpush1b.in \
	inpush1c.in \
	streamseq1a.in \
	streamseq1b.in \
	streamseq1c.in \
	travs2.in \
	include1a.lmi \
	redef1.lm \
	redef2.lm \
	redef3.lm \
	redef4.lm \
	redef5.lm

gentests: gentests.sh Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

CLEANFILES = gentests

clean-local:
	rm -Rf working
	if test '$(builddir)' != '$(srcdir)'; then \
		rm -f *.lm *.lmi *.in; \
	fi

check-local:
	if test '$(builddir)' != '$(srcdir)'; then \
		$(LN_S) -f '$(srcdir)/'*.lm '$(srcdir)/'*.lmi '$(srcdir)/'*.in '$(builddir)'; \
	fi
	'$(top_srcdir)/test/runtests'
