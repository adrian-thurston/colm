/*
 * Copyright 2001-2018 Adrian Thurston <thurston@colm.net>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _PARSEDATA_H
#define _PARSEDATA_H

#include <iostream>
#include <limits.h>

#include "avlmap.h"
#include "bstmap.h"
#include "dlist.h"
#include "fsmgraph.h"
#include "compare.h"
#include "common.h"

#include "ragel.h"
#include "avlmap.h"
#include "bstmap.h"
#include "vector.h"
#include "dlist.h"
#include "fsmgraph.h"

struct NameInst;
struct Join;
struct ParseData;

/* Tree nodes. */

struct NfaUnion;
struct MachineDef;
struct LongestMatch;
struct LongestMatchPart;
struct LmPartList;
struct Range;
struct LengthDef;
struct Action;
struct InlineList;

/* Reference to a named state. */
struct NameRef : public Vector<std::string> {};
typedef Vector<NameRef*> NameRefList;
typedef Vector<NameInst*> NameTargList;

/*
 * LongestMatch
 *
 * Wherever possible the item match will execute on the character. If not
 * possible the item match will execute on a lookahead character and either
 * hold the current char (if one away) or backup.
 *
 * How to handle the problem of backing up over a buffer break?
 * 
 * Don't want to use pending out transitions for embedding item match because
 * the role of item match action is different: it may sometimes match on the
 * final transition, or may match on a lookahead character.
 *
 * Don't want to invent a new operator just for this. So just trail action
 * after machine, this means we can only use literal actions.
 *
 * The item action may 
 *
 * What states of the machine will be final. The item actions that wrap around
 * on the last character will go straight to the start state.
 *
 * Some transitions will be lookahead transitions, they will hold the current
 * character. Crossing them with regular transitions must be restricted
 * because it does not make sense. The transition cannot simultaneously hold
 * and consume the current character.
 */
struct LongestMatchPart
{
	LongestMatchPart( Join *join, Action *action, 
			const InputLoc &semiLoc, int longestMatchId )
	: 
		join(join), action(action), semiLoc(semiLoc), 
		longestMatchId(longestMatchId), inLmSelect(false) { }

	InputLoc getLoc();
	
	Join *join;
	Action *action;
	InputLoc semiLoc;

	Action *setActId;
	Action *actOnLast;
	Action *actOnNext;
	Action *actLagBehind;
	Action *actNfaOnLast;
	Action *actNfaOnNext;
	Action *actNfaOnEof;
	int longestMatchId;
	bool inLmSelect;
	LongestMatch *longestMatch;

	LongestMatchPart *prev, *next;
};

/* Declare a new type so that ptreetypes.h need not include dlist.h. */
struct LmPartList
	: DList<LongestMatchPart> {};

struct LongestMatch
{
	/* Construct with a list of joins */
	LongestMatch( const InputLoc &loc, LmPartList *longestMatchList )
	: 
		loc(loc),
		longestMatchList(longestMatchList),
		lmSwitchHandlesError(false),
		nfaConstruction(false)
	{ }

	InputLoc loc;
	LmPartList *longestMatchList;
	std::string name;
	Action *lmActSelect;
	bool lmSwitchHandlesError;
	bool nfaConstruction;

	LongestMatch *next, *prev;

	/* Tree traversal. */
	FsmRes walkClassic( ParseData *pd );
	FsmRes walk( ParseData *pd );

	FsmRes mergeNfaStates( ParseData *pd, FsmAp *fsm );
	bool onlyOneNfa( ParseData *pd, FsmAp *fsm, StateAp *st, NfaTrans *in );
	bool matchCanFail( ParseData *pd, FsmAp *fsm, StateAp *st );
	void eliminateNfaActions( ParseData *pd, FsmAp *fsm );
	void advanceNfaActions( ParseData *pd, FsmAp *fsm );
	FsmRes buildBaseNfa( ParseData *pd );
	FsmRes walkNfa( ParseData *pd );

	void makeNameTree( ParseData *pd );
	void resolveNameRefs( ParseData *pd );
	void transferScannerLeavingActions( FsmAp *graph );
	void runLongestMatch( ParseData *pd, FsmAp *graph );
	Action *newLmAction( ParseData *pd, const InputLoc &loc, const char *name, 
			InlineList *inlineList );
	void makeActions( ParseData *pd );
	void findName( ParseData *pd );
	void restart( FsmAp *graph, TransAp *trans );
	void restart( FsmAp *graph, CondAp *cond );
};


/* Priority name dictionary. */
typedef AvlMapEl<std::string, int> PriorDictEl;
typedef AvlMap<std::string, int, CmpString> PriorDict;

struct NameMapVal
{
	Vector<NameInst*> vals;
};

/* Tree of instantiated names. */
typedef AvlMapEl<std::string, NameMapVal*> NameMapEl;
typedef AvlMap<std::string, NameMapVal*, CmpString> NameMap;
typedef Vector<NameInst*> NameVect;

/* Node in the tree of instantiated names. */
struct NameInst
{
	NameInst( const InputLoc &loc, NameInst *parent, std::string name, int id, bool isLabel ) : 
		loc(loc), parent(parent), name(name), id(id), isLabel(isLabel),
		isLongestMatch(false), numRefs(0), numUses(0), start(0), final(0) {}

	~NameInst();

	InputLoc loc;

	/* Keep parent pointers in the name tree to retrieve 
	 * fully qulified names. */
	NameInst *parent;

	std::string name;
	int id;
	bool isLabel;
	bool isLongestMatch;

	int numRefs;
	int numUses;

	/* Names underneath us, excludes anonymous names. */
	NameMap children;

	/* All names underneath us in order of appearance. */
	NameVect childVect;

	/* Join scopes need an implicit "final" target. */
	NameInst *start, *final;

	/* During a fsm generation walk, lists the names that are referenced by
	 * epsilon operations in the current scope. After the link is made by the
	 * epsilon reference and the join operation is complete, the label can
	 * have its refcount decremented. Once there are no more references the
	 * entry point can be removed from the fsm returned. */
	NameVect referencedNames;

	/* Pointers for the name search queue. */
	NameInst *prev, *next;

	/* Check if this name inst or any name inst below is referenced. */
	bool anyRefsRec();
};

typedef DList<NameInst> NameInstList;

/* Stack frame used in walking the name tree. */
struct NameFrame 
{
	NameInst *prevNameInst;
	int prevNameChild;
	NameInst *prevLocalScope;
};

extern const int ORD_PUSH;
extern const int ORD_RESTORE;
extern const int ORD_COND;
extern const int ORD_COND2;
extern const int ORD_TEST;

#endif
