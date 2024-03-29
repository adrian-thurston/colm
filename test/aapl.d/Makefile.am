#
#   Copyright 2001, 2002 Adrian Thurston <thurston@cs.queensu.ca>
#

CPPFLAGS = @SUBJ_AAPL_CPPFLAGS@

check_PROGRAMS = \
	stress_avltree stress_avliter stress_avlmel \
	stress_avlmelkey stress_avlitree stress_avlmap \
	stress_avlset stress_avlimel stress_avlimelkey \
	stress_avlimap stress_avliset stress_svector \
	stress_allsort stress_stblsort \
	test_vector test_bsttable test_bstmap \
	test_bstset test_doublelist test_dlistval \
	test_mergesort test_allavl test_avliter \
	test_quicksort test_insertsort test_bubblesort \
	test_allsort test_svector test_sbsttable \
	test_sbstmap test_sbstset test_avlkeyless \
	test_avlikeyless test_compare test_string \
	test_rope

check_SCRIPTS = gentests

stress_avltree_SOURCES = stress_avltree.cpp util.cpp
stress_avliter_SOURCES = stress_avliter.cpp util.cpp
stress_avlmel_SOURCES = stress_avlmel.cpp util.cpp
stress_avlmelkey_SOURCES = stress_avlmelkey.cpp util.cpp
stress_avlitree_SOURCES = stress_avlitree.cpp util.cpp
stress_avlmap_SOURCES = stress_avlmap.cpp util.cpp
stress_avlset_SOURCES = stress_avlset.cpp util.cpp
stress_avlimel_SOURCES = stress_avlimel.cpp util.cpp
stress_avlimelkey_SOURCES = stress_avlimelkey.cpp util.cpp
stress_avlimap_SOURCES = stress_avlimap.cpp util.cpp
stress_avliset_SOURCES = stress_avliset.cpp util.cpp
stress_svector_SOURCES = stress_svector.cpp util.cpp
stress_allsort_SOURCES = stress_allsort.cpp util.cpp
stress_stblsort_SOURCES = stress_stblsort.cpp util.cpp
test_vector_SOURCES = test_vector.cpp util.cpp
test_bsttable_SOURCES = test_bsttable.cpp util.cpp
test_bstmap_SOURCES = test_bstmap.cpp util.cpp
test_bstset_SOURCES = test_bstset.cpp util.cpp
test_doublelist_SOURCES = test_doublelist.cpp util.cpp
test_dlistval_SOURCES = test_dlistval.cpp util.cpp
test_mergesort_SOURCES = test_mergesort.cpp util.cpp
test_allavl_SOURCES = test_allavl.cpp util.cpp
test_avliter_SOURCES = test_avliter.cpp util.cpp
test_quicksort_SOURCES = test_quicksort.cpp util.cpp
test_insertsort_SOURCES = test_insertsort.cpp util.cpp
test_bubblesort_SOURCES = test_bubblesort.cpp util.cpp
test_allsort_SOURCES = test_allsort.cpp util.cpp
test_svector_SOURCES = test_svector.cpp util.cpp
test_sbsttable_SOURCES = test_sbsttable.cpp util.cpp
test_sbstmap_SOURCES = test_sbstmap.cpp util.cpp
test_sbstset_SOURCES = test_sbstset.cpp util.cpp
test_avlkeyless_SOURCES = test_avlkeyless.cpp util.cpp
test_avlikeyless_SOURCES = test_avlikeyless.cpp util.cpp
test_compare_SOURCES = test_compare.cpp util.cpp
test_string_SOURCES = test_string.cpp util.cpp
test_rope_SOURCES = test_rope.cpp util.cpp

EXTRA_DIST = \
	avlverify.h util.h gentests.sh \
	test_allavl.exp test_allsort.exp test_avlikeyless.exp test_avliter.exp \
	test_avlkeyless.exp test_bstmap.exp test_bstset.exp test_bsttable.exp \
	test_bubblesort.exp test_compare.exp test_dlistval.exp test_doublelist.exp \
	test_insertsort.exp test_mergesort.exp test_quicksort.exp test_rope.exp \
	test_sbstmap.exp test_sbstset.exp test_sbsttable.exp test_string.exp \
	test_svector.exp test_vector.exp 

gentests: gentests.sh Makefile
	@$(top_srcdir)/sedsubst $< $@ -w,+x $(SED_SUBST)

#*************************************

# Programs.
#CXX		= g++

#
# Make targets from sources. 
#STRESS_TARGS = $(STRESS:%.cpp=%.tst)
#TEST_TARGS = $(TEST:%.cpp=%.tst)
#TARGS = $(STRESS_TARGS) $(TEST_TARGS)
#DEPS = $(STRESS:%.cpp=.deps/%.d) $(TEST:%.cpp=.deps/%.d) .deps/util.d
#
## Rules.
#main: $(STRESS_TARGS) $(TEST_TARGS)
#
#.deps:
#	mkdir .deps
#
#$(TARGS): %.tst: %.cpp util.o .deps
#	@$(CXX) -M $(CFLAGS) $< \
#		| sed 's/^\([a-zA-Z0-9_-]*\)\.o:/\1.tst:/;' > .deps/$*.d
#	$(CXX) $(CFLAGS) -o $@ $< util.o $(LIBS)
#
#distclean: clean
#	rm -Rf .deps Makefile
#
#clean:
#	rm -Rf .deps *.tst *.o
#
#-include $(DEPS)

CLEANFILES = gentests

clean-local:
	rm -Rf working
	if test '$(builddir)' != '$(srcdir)'; then \
		rm -f *.exp; \
	fi

check-local:
	if test '$(builddir)' != '$(srcdir)'; then \
		$(LN_S) -f '$(srcdir)/'*.exp '$(builddir)'; \
	fi
	'$(top_srcdir)/test/runtests'
