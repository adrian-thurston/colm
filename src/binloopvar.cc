/*
 *  Copyright 2001-2014 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Ragel.
 *
 *  Ragel is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Ragel is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Ragel; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "ragel.h"
#include "binloopvar.h"
#include "redfsm.h"
#include "gendata.h"

BinLoopVar::BinLoopVar( const CodeGenArgs &args )
:
	BinaryVar( args )
{}

/* Determine if we should use indicies or not. */
void BinLoopVar::calcIndexSize()
{
//	long long sizeWithInds =
//		indicies.size() +
//		transCondSpacesWi.size() +
//		transOffsetsWi.size() +
//		transLengthsWi.size();

//	long long sizeWithoutInds =
//		transCondSpaces.size() +
//		transOffsets.size() +
//		transLengths.size();

	//std::cerr << "sizes: " << sizeWithInds << " " << sizeWithoutInds << std::endl;

	///* If using indicies reduces the size, use them. */
	//useIndicies = sizeWithInds < sizeWithoutInds;
	useIndicies = false;
}


void BinLoopVar::tableDataPass()
{
	taActions();
	taKeyOffsets();
	taSingleLens();
	taRangeLens();
	taIndexOffsets();
	taIndicies();

	taTransCondSpacesWi();
	taTransOffsetsWi();
	taTransLengthsWi();

	taTransCondSpaces();
	taTransOffsets();
	taTransLengths();

	taCondTargs();
	taCondActions();

	taToStateActions();
	taFromStateActions();
	taEofActions();

	taEofTransDirect();
	taEofTransIndexed();

	taKeys();
	taCondKeys();
}

void BinLoopVar::genAnalysis()
{
	redFsm->sortByStateId();

	/* Choose default transitions and the single transition. */
	redFsm->chooseDefaultSpan();
		
	/* Choose the singles. */
	redFsm->moveSelectTransToSingle();

	/* If any errors have occured in the input file then don't write anything. */
	if ( gblErrorCount > 0 )
		return;

	/* Anlayze Machine will find the final action reference counts, among other
	 * things. We will use these in reporting the usage of fsm directives in
	 * action code. */
	analyzeMachine();

	setKeyType();

	/* Run the analysis pass over the table data. */
	setTableState( TableArray::AnalyzePass );
	tableDataPass();

	/* Determine if we should use indicies. */
	calcIndexSize();

	/* Switch the tables over to the code gen mode. */
	setTableState( TableArray::GeneratePass );
}


void BinLoopVar::COND_ACTION( RedCondPair *cond )
{
	int act = 0;
	if ( cond->action != 0 )
		act = cond->action->location+1;
	condActions.value( act );
}

void BinLoopVar::TO_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->toStateAction != 0 )
		act = state->toStateAction->location+1;
	toStateActions.value( act );
}

void BinLoopVar::FROM_STATE_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->fromStateAction != 0 )
		act = state->fromStateAction->location+1;
	fromStateActions.value( act );
}

void BinLoopVar::EOF_ACTION( RedStateAp *state )
{
	int act = 0;
	if ( state->eofAction != 0 )
		act = state->eofAction->location+1;
	eofActions.value( act );
}

std::ostream &BinLoopVar::TO_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numToStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << "\t case " << act->actionId << " {\n";
			ACTION( out, act, IlOpts( 0, false, false ) );
			out << "\n\t}\n";
		}
	}

	return out;
}

std::ostream &BinLoopVar::FROM_STATE_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numFromStateRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << "\t case " << act->actionId << " {\n";
			ACTION( out, act, IlOpts( 0, false, false ) );
			out << "\n\t}\n";
		}
	}

	return out;
}

std::ostream &BinLoopVar::EOF_ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numEofRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << "\t case " << act->actionId << " {\n";
			ACTION( out, act, IlOpts( 0, true, false ) );
			out << "\n\t}\n";
		}
	}

	return out;
}


std::ostream &BinLoopVar::ACTION_SWITCH()
{
	/* Walk the list of functions, printing the cases. */
	for ( GenActionList::Iter act = actionList; act.lte(); act++ ) {
		/* Write out referenced actions. */
		if ( act->numTransRefs > 0 ) {
			/* Write the case label, the action and the case break. */
			out << "\t case " << act->actionId << " {\n";
			ACTION( out, act, IlOpts( 0, false, false ) );
			out << "\n\t}\n";
		}
	}

	return out;
}


void BinLoopVar::writeData()
{
	/* If there are any transtion functions then output the array. If there
	 * are none, don't bother emitting an empty array that won't be used. */
	if ( redFsm->anyActions() )
		taActions();

	taKeyOffsets();
	taKeys();
	taSingleLens();
	taRangeLens();
	taIndexOffsets();

	if ( useIndicies ) {
		taIndicies();
		taTransCondSpacesWi();
		taTransOffsetsWi();
		taTransLengthsWi();
	}
	else {
		taTransCondSpaces();
		taTransOffsets();
		taTransLengths();
	}

	taCondKeys();

	taCondTargs();
	taCondActions();

	if ( redFsm->anyToStateActions() )
		taToStateActions();

	if ( redFsm->anyFromStateActions() )
		taFromStateActions();

	if ( redFsm->anyEofActions() )
		taEofActions();

	if ( redFsm->anyEofTrans() ) {
		taEofTransIndexed();
		taEofTransDirect();
	}

	STATE_IDS();
}

void BinLoopVar::NFA_PUSH_ACTION( RedNfaTarg *targ )
{
	int act = 0;
	if ( targ->push != 0 )
		act = targ->push->actListId+1;
	nfaPushActions.value( act );
}

void BinLoopVar::NFA_POP_ACTION( RedNfaTarg *targ )
{
	int act = 0;
	if ( targ->pop != 0 )
		act = targ->pop->actListId+1;
	nfaPopActions.value( act );
}

void BinLoopVar::writeExec()
{
	testEofUsed = false;
	outLabelUsed = false;
	matchCondLabelUsed = false;

	out <<
		"	{\n"
		"	int _klen;\n";

	if ( redFsm->anyRegCurStateRef() )
		out << "	int _ps;\n";

	out <<
		"	" << UINT() << " _trans = 0;\n" <<
		"	" << UINT() << " _cond = 0;\n"
		"	" << UINT() << " _have = 0;\n"
		"	" << UINT() << " _cont = 1;\n";

	if ( redFsm->anyToStateActions() || redFsm->anyRegActions() 
			|| redFsm->anyFromStateActions() )
	{
		out << 
			"	index " << ARR_TYPE( actions ) << " _acts;\n"
			"	" << UINT() << " _nacts;\n";
	}

	out <<
		"	index " << ALPH_TYPE() << " _keys;\n"
		"	index " << ARR_TYPE( condKeys ) << " _ckeys;\n"
		"	int _cpc;\n"
		"	while ( _cont == 1 ) {\n"
		"\n";

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			"	if ( " << vCS() << " == " << redFsm->errState->id << " )\n"
			"		_cont = 0;\n";
	}

	out << 
//		"label _resume {\n"
		"_have = 0;\n";

	if ( !noEnd ) {
		out << 
			"	if ( " << P() << " == " << PE() << " ) {\n";

		if ( redFsm->anyEofTrans() || redFsm->anyEofActions() ) {
			out << 
				"	if ( " << P() << " == " << vEOF() << " )\n"
				"	{\n";

			if ( redFsm->anyEofTrans() ) {
				TableArray &eofTrans = useIndicies ? eofTransIndexed : eofTransDirect;
				out <<
					"	if ( " << ARR_REF( eofTrans ) << "[" << vCS() << "] > 0 ) {\n"
					"		_trans = (" << UINT() << ")" << ARR_REF( eofTrans ) << "[" << vCS() << "] - 1;\n"
					"		_cond = (" << UINT() << ")" << ARR_REF( transOffsets ) << "[_trans];\n"
					"		_have = 1;\n"
					"	}\n";
					matchCondLabelUsed = true;
			}

			out << "if ( _have == 0 ) {\n";

			if ( redFsm->anyEofActions() ) {
				out <<
					"	index " << ARR_TYPE( actions ) << " __acts;\n"
					"	" << UINT() << " __nacts;\n"
					"	__acts = offset( " << ARR_REF( actions ) << ", " <<
							ARR_REF( eofActions ) << "[" << vCS() << "]" << " );\n"
					"	__nacts = (" << UINT() << ") deref( " << ARR_REF( actions ) << ", __acts );\n"
					"	__acts += 1;\n"
					"	while ( __nacts > 0 ) {\n"
					"		switch ( deref( " << ARR_REF( actions ) << ", __acts ) ) {\n";
					EOF_ACTION_SWITCH() <<
					"		}\n"
					"		__nacts -= 1;\n"
					"		__acts += 1;\n"
					"	}\n";
			}

			out << "}\n";
			
			out << 
				"	}\n"
				"\n";
		}

		out << 
			"	if ( _have == 0 )\n"
			"		_cont = 0;\n"
			"	}\n";

	}

	out << 
		"	if ( _cont == 1 ) {\n"
		"	if ( _have == 0 ) {\n";

	if ( redFsm->anyFromStateActions() ) {
		out <<
			"	_acts = offset( " << ARR_REF( actions ) << ", " << ARR_REF( fromStateActions ) <<
					"[" << vCS() << "]" << " );\n"
			"	_nacts = (" << UINT() << ") deref( " << ARR_REF( actions ) << ", _acts );\n"
			"	_acts += 1;\n"
			"	while ( _nacts > 0 ) {\n"
			"		switch ( deref( " << ARR_REF( actions ) << ", _acts ) ) {\n";
			FROM_STATE_ACTION_SWITCH() <<
			"		}\n"
			"		_nacts -= 1;\n"
			"		_acts += 1;\n"
			"	}\n"
			"\n";
	}

	LOCATE_TRANS();

	if ( useIndicies )
		out << "	_trans = " << ARR_REF( indicies ) << "[_trans];\n";

	LOCATE_COND();

	out << "}\n";
	
	out << "if ( _cont == 1 ) {\n";

	if ( redFsm->anyRegCurStateRef() )
		out << "	_ps = " << vCS() << ";\n";

	out <<
		"	" << vCS() << " = (int)" << ARR_REF( condTargs ) << "[_cond];\n"
		"\n";

	if ( redFsm->anyRegActions() ) {
		out <<
			"	if ( " << ARR_REF( condActions ) << "[_cond] != 0 ) {\n"
			"		_acts = offset( " << ARR_REF( actions ) << ", " << ARR_REF( condActions ) << "[_cond]" << " );\n"
			"		_nacts = (" << UINT() << ") deref( " << ARR_REF( actions ) << ", _acts );\n"
			"		_acts += 1;\n"
			"		while ( _nacts > 0 )\n	{\n"
			"			switch ( deref( " << ARR_REF( actions ) << ", _acts ) )\n"
			"			{\n";
			ACTION_SWITCH() <<
			"			}\n"
			"			_nacts -= 1;\n"
			"			_acts += 1;\n"
			"		}\n"
			"	}\n"
			"\n";
	}

//	if ( /*redFsm->anyRegActions() || */ redFsm->anyActionGotos() || 
//			redFsm->anyActionCalls() || redFsm->anyActionRets() )
//	{
//		out << "}\n";
//		out << "label _again {\n";
//	}

	if ( redFsm->anyToStateActions() ) {
		out <<
			"	_acts = offset( " << ARR_REF( actions ) << ", " << ARR_REF( toStateActions ) <<
					"[" << vCS() << "]" << " );\n"
			"	_nacts = (" << UINT() << ") deref( " << ARR_REF( actions ) << ", _acts );\n"
			"	_acts += 1;\n"
			"	while ( _nacts > 0 ) {\n"
			"		switch ( deref( " << ARR_REF( actions ) << ", _acts ) ) {\n";
			TO_STATE_ACTION_SWITCH() <<
			"		}\n"
			"		_nacts -= 1;\n"
			"		_acts += 1;\n"
			"	}\n"
			"\n";
	}

	if ( redFsm->errState != 0 ) {
		outLabelUsed = true;
		out << 
			"	if ( " << vCS() << " == " << redFsm->errState->id << " )\n"
			"		_cont = 0;\n";
	}

	out << 
		"	if ( _cont == 1 )\n"
		"		" << P() << " += 1;\n"
		"\n"
		/* cont if. */
		"}}\n";
//		"	goto _resume;\n";

//	if ( outLabelUsed )
//		out << "} label _out { {}\n";

	/* The loop. */
	out << "}\n";

//	/* The entry loop. */
//	out << "}}\n";

	/* The execute block. */
	out << "	}\n";
}
