/*
 *  Copyright 2006-2012 Adrian Thurston <thurston@complang.org>
 */

/*  This file is part of Colm.
 *
 *  Colm is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 * 
 *  Colm is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with Colm; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include <iostream>
#include <string>
#include <errno.h>

#include "parser.h"
#include "config.h"
#include "avltree.h"
#include "parsedata.h"
#include "parser.h"
#include "global.h"
#include "input.h"
#include "conscolm.h"
#include "exports1.h"
#include "colm/colm.h"

using std::string;

extern RuntimeData main_runtimeData;

void LoadColm::walkProdElList( ProdElList *list, prod_el_list &prodElList )
{
	if ( prodElList.ProdElList() != 0 ) {
		prod_el_list RightProdElList = prodElList.ProdElList();
		walkProdElList( list, RightProdElList );
	}
	
	if ( prodElList.ProdEl() != 0 ) {
		prod_el El = prodElList.ProdEl();
		String typeName = El.Id().text().c_str();

		ObjectField *captureField = 0;
		if ( El.OptName().Name() != 0 ) {
			String fieldName = El.OptName().Name().text().c_str();
			captureField = ObjectField::cons( internal, 0, fieldName );
		}

		RepeatType repeatType = RepeatNone;
		if ( El.OptRepeat().Star() != 0 )
			repeatType = RepeatRepeat;

		ProdEl *prodEl = prodElName( internal, typeName,
				NamespaceQual::cons(namespaceStack.top()),
				captureField, repeatType, false );

		appendProdEl( list, prodEl );
	}
}

void LoadColm::walkProdList( LelDefList *outProdList, prod_list &prodList )
{
	if ( prodList.ProdList() != 0 ) {
		prod_list RightProdList = prodList.ProdList();
		walkProdList( outProdList, RightProdList );
	}

	ProdElList *outElList = new ProdElList;
	prod_el_list prodElList = prodList.Prod().ProdElList();
	walkProdElList( outElList, prodElList );

	Production *prod = BaseParser::production( internal, outElList, false, 0, 0 );
	prodAppend( outProdList, prod );
}

LexFactor *LoadColm::walkLexFactor( lex_factor &lexFactor )
{
	LexFactor *factor = 0;
	if ( lexFactor.Literal() != 0 ) {
		String litString = lexFactor.Literal().text().c_str();
		Literal *literal = Literal::cons( internal, litString, Literal::LitString );
		factor = LexFactor::cons( literal );
	}
	if ( lexFactor.Id() != 0 ) {
		String id = lexFactor.Id().text().c_str();
		factor = lexRlFactorName( id, internal );
	}
	else if ( lexFactor.Expr() != 0 ) {
		lex_expr LexExpr = lexFactor.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );
		factor = LexFactor::cons( join );
	}
	else if ( lexFactor.Low() != 0 ) {
		String low = lexFactor.Low().text().c_str();
		Literal *lowLit = Literal::cons( internal, low, Literal::LitString );

		String high = lexFactor.High().text().c_str();
		Literal *highLit = Literal::cons( internal, high, Literal::LitString );

		Range *range = Range::cons( lowLit, highLit );
		factor = LexFactor::cons( range );
	}
	return factor;
}

LexFactorNeg *LoadColm::walkLexFactorNeg( lex_factor_neg &lexFactorNeg )
{
	if ( lexFactorNeg.FactorNeg() != 0 ) {
		lex_factor_neg Rec = lexFactorNeg.FactorNeg();
		LexFactorNeg *recNeg = walkLexFactorNeg( Rec );
		LexFactorNeg *factorNeg = LexFactorNeg::cons( internal, recNeg, LexFactorNeg::CharNegateType );
		return factorNeg;
	}
	else {
		lex_factor LexFactorTree = lexFactorNeg.Factor();
		LexFactor *factor = walkLexFactor( LexFactorTree );
		LexFactorNeg *factorNeg = LexFactorNeg::cons( internal, factor );
		return factorNeg;
	}
}

LexFactorRep *LoadColm::walkLexFactorRep( lex_factor_rep &lexFactorRep )
{
	LexFactorRep *factorRep = 0;
	if ( lexFactorRep.Star() != 0 ) {
		lex_factor_rep Rec = lexFactorRep.FactorRep();
		LexFactorRep *recRep = walkLexFactorRep( Rec );
		factorRep = LexFactorRep::cons( internal, recRep, 0, 0, LexFactorRep::StarType );
	}
	else if ( lexFactorRep.Plus() != 0 ) {
		lex_factor_rep Rec = lexFactorRep.FactorRep();
		LexFactorRep *recRep = walkLexFactorRep( Rec );
		factorRep = LexFactorRep::cons( internal, recRep, 0, 0, LexFactorRep::PlusType );
	}
	else {
		lex_factor_neg LexFactorNegTree = lexFactorRep.FactorNeg();
		LexFactorNeg *factorNeg = walkLexFactorNeg( LexFactorNegTree );
		factorRep = LexFactorRep::cons( internal, factorNeg );
	}
	return factorRep;
}

LexFactorAug *LoadColm::walkLexFactorAug( lex_factor_rep &lexFactorRep )
{
	LexFactorRep *factorRep = walkLexFactorRep( lexFactorRep );
	return LexFactorAug::cons( factorRep );
}

LexTerm *LoadColm::walkLexTerm( lex_term &lexTerm )
{
	if ( lexTerm.Term() != 0 ) {
		lex_term Rec = lexTerm.Term();
		LexTerm *leftTerm = walkLexTerm( Rec );

		lex_factor_rep LexFactorRepTree = lexTerm.FactorRep();
		LexFactorAug *factorAug = walkLexFactorAug( LexFactorRepTree );
		LexTerm *term = LexTerm::cons( leftTerm, factorAug, LexTerm::ConcatType );
		return term;
	}
	else {
		lex_factor_rep LexFactorRepTree = lexTerm.FactorRep();
		LexFactorAug *factorAug = walkLexFactorAug( LexFactorRepTree );
		LexTerm *term = LexTerm::cons( factorAug );
		return term;
	}
}

LexExpression *LoadColm::walkLexExpr( lex_expr &LexExprTree )
{
	if ( LexExprTree.Expr() != 0 ) {
		lex_expr Rec = LexExprTree.Expr();
		LexExpression *leftExpr = walkLexExpr( Rec );

		lex_term lexTerm = LexExprTree.Term();
		LexTerm *term = walkLexTerm( lexTerm );
		LexExpression *expr = LexExpression::cons( leftExpr, term, LexExpression::OrType );

		return expr;
	}
	else {
		lex_term lexTerm = LexExprTree.Term();
		LexTerm *term = walkLexTerm( lexTerm );
		LexExpression *expr = LexExpression::cons( term );
		return expr;
	}
}

void LoadColm::walkTokenList( token_list &tokenList )
{
	if ( tokenList.TokenList() != 0 ) {
		token_list RightTokenList = tokenList.TokenList();
		walkTokenList( RightTokenList );
	}
	
	if ( tokenList.TokenDef() != 0 ) {
		token_def tokenDef = tokenList.TokenDef();
		String name = tokenDef.Id().text().c_str();

		ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, name, pd->nextObjectId++ ); 

		lex_expr LexExpr = tokenDef.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );

		defineToken( internal, name, join, objectDef, 0, false, false, false );
	}

	if ( tokenList.IgnoreDef() != 0 ) {
		ignore_def IgnoreDef = tokenList.IgnoreDef();

		ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, 0, pd->nextObjectId++ ); 

		lex_expr LexExpr = IgnoreDef.Expr();
		LexExpression *expr = walkLexExpr( LexExpr );
		LexJoin *join = LexJoin::cons( expr );

		defineToken( internal, 0, join, objectDef, 0, true, false, false );
	}
}

void LoadColm::walkLexRegion( item &LexRegion )
{
	pushRegionSet( internal );

	token_list tokenList = LexRegion.TokenList();
	walkTokenList( tokenList );

	popRegionSet();
}

void LoadColm::walkDefinition( item &define )
{
	prod_list ProdList = define.ProdList();

	LelDefList *defList = new LelDefList;
	walkProdList( defList, ProdList );

	String name = define.DefId().text().c_str();
	NtDef *ntDef = NtDef::cons( name, namespaceStack.top(), contextStack.top(), false );
	ObjectDef *objectDef = ObjectDef::cons( ObjectDef::UserType, name, pd->nextObjectId++ ); 
	cflDef( ntDef, objectDef, defList );
}

void LoadColm::consParseStmt( StmtList *stmtList )
{
	/* Parse the "start" def. */
	NamespaceQual *nspaceQual = NamespaceQual::cons( namespaceStack.top() );
	TypeRef *typeRef = TypeRef::cons( internal, nspaceQual, String("start"), RepeatNone );

	/* Pop argv, this yields the file name . */
	ExprVect *popArgs = new ExprVect;
	QualItemVect *popQual = new QualItemVect;
	popQual->append( QualItem( internal, String( "argv" ), QualItem::Dot ) );

	LangVarRef *popRef = LangVarRef::cons( internal, popQual, String("pop") );
	LangExpr *pop = LangExpr::cons( LangTerm::cons( InputLoc(), popRef, popArgs ) );

	/* Construct a literal string 'r', for second arg to open. */
	ConsItem *modeConsItem = ConsItem::cons( internal, ConsItem::InputText, String("r") );
	ConsItemList *modeCons = new ConsItemList;
	modeCons->append( modeConsItem );
	LangExpr *modeExpr = LangExpr::cons( LangTerm::cons( internal, modeCons ) );
	
	/* Call open. */
	QualItemVect *openQual = new QualItemVect;
	LangVarRef *openRef = LangVarRef::cons( internal, openQual, String("open") );
	ExprVect *openArgs = new ExprVect;
	openArgs->append( pop );
	openArgs->append( modeExpr );
	LangExpr *open = LangExpr::cons( LangTerm::cons( InputLoc(), openRef, openArgs ) );

	/* Construct a list containing the open stream. */
	ConsItem *consItem = ConsItem::cons( internal, ConsItem::ExprType, open );
	ConsItemList *list = ConsItemList::cons( consItem );

	/* Will capture the parser to "P" */
	ObjectField *objField = ObjectField::cons( internal, 0, String("P") );

	/* Parse the above list. */
	LangExpr *parseExpr = parseCmd( internal, false, objField, typeRef, 0, list );
	LangStmt *parseStmt = LangStmt::cons( internal, LangStmt::ExprType, parseExpr );
	stmtList->append( parseStmt );
}

void LoadColm::consExportStmt( StmtList *stmtList )
{
	QualItemVect *qual = new QualItemVect;
	qual->append( QualItem( internal, String( "P" ), QualItem::Dot ) );
	LangVarRef *varRef = LangVarRef::cons( internal, qual, String("tree") );
	LangExpr *expr = LangExpr::cons( LangTerm::cons( internal, LangTerm::VarRefType, varRef ) );

	NamespaceQual *nspaceQual = NamespaceQual::cons( namespaceStack.top() );
	TypeRef *typeRef = TypeRef::cons( internal, nspaceQual, String("start"), RepeatNone );
	ObjectField *program = ObjectField::cons( internal, typeRef, String("ColmTree") );
	LangStmt *programExport = exportStmt( program, LangStmt::AssignType, expr );
	stmtList->append( programExport );
}

void LoadColm::go()
{
	StmtList *stmtList = new StmtList;

	const char *argv[2];
	argv[0] = inputFileName;
	argv[1] = 0;

	colmInit( 0 );
	ColmProgram *program = colmNewProgram( &main_runtimeData );
	colmRunProgram( program, 1, argv );

	/* Extract the parse tree. */
	start Start = ColmTree( program );

	if ( Start == 0 ) {
		gblErrorCount += 1;
		std::cerr << inputFileName << ": parse error" << std::endl;
		return;
	}

	/* Walk the list of items. */
	_repeat_item ItemList = Start.ItemList();
	while ( !ItemList.end() ) {

		item Item = ItemList.value();
		if ( Item.DefId() != 0 )
			walkDefinition( Item );
		else if ( Item.TokenList() != 0 )
			walkLexRegion( Item );
		ItemList = ItemList.next();
	}

	colmDeleteProgram( program );

	consParseStmt( stmtList );
	consExportStmt( stmtList );

	pd->rootCodeBlock = CodeBlock::cons( stmtList, 0 );
}