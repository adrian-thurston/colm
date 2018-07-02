/*
 * Copyright 2007-2018 Adrian Thurston <thurston@colm.net>
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <iostream>

#include "compiler.h"
#include "pool.h"
//#include "debug.h"

using std::cerr;
using std::endl;

DEF_STREAM_FUNCS( stream_funcs_ct, stream_impl_ct );

extern stream_funcs_ct patternFuncs;
extern stream_funcs_ct replFuncs;

struct stream_impl_ct
{
	struct stream_funcs *funcs;

	char *name;
	long line;
	long column;
	long byte;

	struct Pattern *pattern;
	struct PatternItem *pat_item;
	struct Constructor *constructor;
	struct ConsItem *cons_item;

	char eof_sent;
	char eof;

	int offset;
};

char inputStreamGetEofSent( struct colm_program *prg, struct stream_impl_ct *si )
{
	return si->eof_sent;
}

void inputStreamSetEofSent( struct colm_program *prg, struct stream_impl_ct *si, char eof_sent )
{
	si->eof_sent = eof_sent;
}

void inputStreamTransferLoc( location_t *loc, struct stream_impl_ct *si )
{
	loc->name = si->name;
	loc->line = si->line;
	loc->column = si->column;
	loc->byte = si->byte;
}


/*
 * Pattern
 */

struct stream_impl *colm_impl_new_pat( char *name, Pattern *pattern )
{
	struct stream_impl_ct *ss = (struct stream_impl_ct*)malloc(sizeof(struct stream_impl_ct));
	memset( ss, 0, sizeof(struct stream_impl_ct) );
	ss->pattern = pattern;
	ss->pat_item = pattern->list->head;
	ss->funcs = (struct stream_funcs*)&patternFuncs;
	return (struct stream_impl*) ss;
}

LangEl *inputStreamPatternGetLangEl( struct colm_program *prg, struct stream_impl_ct *ss, long *bindId,
		char **data, long *length )
{ 
	LangEl *klangEl = ss->pat_item->prodEl->langEl;
	*bindId = ss->pat_item->bindId;
	*data = 0;
	*length = 0;

	ss->pat_item = ss->pat_item->next;
	ss->offset = 0;
	return klangEl;
}

void inputStreamPatternDestructor( program_t *prg, tree_t **sp, struct stream_impl_ct *ss )
{
}

int inputStreamPatternGetParseBlock( struct colm_program *prg, struct stream_impl_ct *ss, int skip,
		char **pdp, int *copied )
{ 
	*copied = 0;

	PatternItem *buf = ss->pat_item;
	int offset = ss->offset;

	while ( true ) {
		if ( buf == 0 )
			return INPUT_EOF;

		if ( buf->form == PatternItem::TypeRefForm )
			return INPUT_LANG_EL;

		assert ( buf->form == PatternItem::InputTextForm );
		int avail = buf->data.length() - offset;

		if ( avail > 0 ) {
			/* The source data from the current buffer. */
			char *src = &buf->data[offset];
			int slen = avail;

			/* Need to skip? */
			if ( skip > 0 && slen <= skip ) {
				/* Skipping the the whole source. */
				skip -= slen;
			}
			else {
				/* Either skip is zero, or less than slen. Skip goes to zero.
				 * Some data left over, copy it. */
				src += skip;
				slen -= skip;
				skip = 0;

				*pdp = src;
				*copied += slen;
				break;
			}
		}

		buf = buf->next;
		offset = 0;
	}

	return INPUT_DATA;
}

int inputStreamPatternGetData( struct colm_program *prg, struct stream_impl_ct *ss, char *dest, int length )
{ 
	int copied = 0;

	PatternItem *buf = ss->pat_item;
	int offset = ss->offset;

	while ( true ) {
		if ( buf == 0 )
			break;

		if ( buf->form == PatternItem::TypeRefForm )
			break;

		assert ( buf->form == PatternItem::InputTextForm );
		int avail = buf->data.length() - offset;

		if ( avail > 0 ) {
			/* The source data from the current buffer. */
			char *src = &buf->data[offset];
			int slen = avail <= length ? avail : length;

			memcpy( dest+copied, src, slen ) ;
			copied += slen;
			length -= slen;
		}

		if ( length == 0 )
			break;

		buf = buf->next;
		offset = 0;
	}

	return copied;
}

void inputStreamPatternBackup( struct stream_impl_ct *ss )
{
	if ( ss->pat_item == 0 )
		ss->pat_item = ss->pattern->list->tail;
	else
		ss->pat_item = ss->pat_item->prev;
}

extern "C" void inputStreamPatternUndoConsumeLangEl( struct colm_program *prg, struct stream_impl_ct *ss )
{
	inputStreamPatternBackup( ss );
	ss->offset = ss->pat_item->data.length();
}

int inputStreamPatternConsumeData( struct colm_program *prg, struct stream_impl_ct *ss, int length, location_t *loc )
{
	//debug( REALM_INPUT, "consuming %ld bytes\n", length );

	int consumed = 0;

	while ( true ) {
		if ( ss->pat_item == 0 )
			break;

		int avail = ss->pat_item->data.length() - ss->offset;

		if ( length >= avail ) {
			/* Read up to the end of the data. Advance the
			 * pattern item. */
			ss->pat_item = ss->pat_item->next;
			ss->offset = 0;

			length -= avail;
			consumed += avail;

			if ( length == 0 )
				break;
		}
		else {
			ss->offset += length;
			consumed += length;
			break;
		}
	}

	return consumed;
}

int inputStreamPatternUndoConsumeData( struct colm_program *prg, struct stream_impl_ct *ss, const char *data, int length )
{
	ss->offset -= length;
	return length;
}

void pat_stream_set_eof( struct colm_program *prg, struct stream_impl_ct *si )
{
	si->eof = true;
}

void pat_stream_unset_eof( struct colm_program *prg, struct stream_impl_ct *si )
{
//	if ( is_source_stream( si ) ) {
//		struct stream_impl_data *sid = (struct stream_impl_data*)si->queue->si;
//		sid->eof = false;
//	}
//	else {
		si->eof = false;
//	}
}

void repl_stream_set_eof( struct colm_program *prg, struct stream_impl_ct *si )
{
	si->eof = true;
}

void repl_stream_unset_eof( struct colm_program *prg, struct stream_impl_ct *si )
{
//	if ( is_source_stream( si ) ) {
//		struct stream_impl_data *sid = (struct stream_impl_data*)si->queue->si;
//		sid->eof = false;
//	}
//	else {
		si->eof = false;
//	}
}

void pat_transfer_loc_seq( struct colm_program *prg, location_t *loc, struct stream_impl_ct *ss )
{
	loc->name = ss->name;
	loc->line = ss->line;
	loc->column = ss->column;
	loc->byte = ss->byte;
}

stream_funcs_ct patternFuncs = 
{
	&inputStreamPatternGetParseBlock,
	&inputStreamPatternGetData,

	&inputStreamPatternConsumeData,
	/* unused */ 0,
	&inputStreamPatternGetLangEl,

	&inputStreamPatternUndoConsumeData,
	/* unused */ 0,
	&inputStreamPatternUndoConsumeLangEl,

	0,
	
	&pat_stream_set_eof, &pat_stream_unset_eof,
	
	0, 0, 0, 0, 0, 0, /* prepend. */
	0, 0, 0, 0, 0, 0, /* append. */

	&inputStreamPatternDestructor,

	0, 0, 0, 0,

	&inputStreamGetEofSent,
	&inputStreamSetEofSent,

	&pat_transfer_loc_seq,
};


/*
 * Constructor
 */

struct stream_impl *colm_impl_new_cons( char *name, Constructor *constructor )
{
	struct stream_impl_ct *ss = (struct stream_impl_ct*)malloc(sizeof(struct stream_impl_ct));
	memset( ss, 0, sizeof(struct stream_impl_ct) );
	ss->constructor = constructor;
	ss->cons_item = constructor->list->head;
	ss->funcs = (struct stream_funcs*)&replFuncs;
	return (struct stream_impl*)ss;
}

LangEl *inputStreamConsGetLangEl( struct colm_program *prg, struct stream_impl_ct *ss, long *bindId, char **data, long *length )
{ 
	LangEl *klangEl = ss->cons_item->type == ConsItem::ExprType ? 
			ss->cons_item->langEl : ss->cons_item->prodEl->langEl;
	*bindId = ss->cons_item->bindId;

	*data = 0;
	*length = 0;

	if ( ss->cons_item->type == ConsItem::LiteralType ) {
		if ( ss->cons_item->prodEl->typeRef->pdaLiteral != 0 ) {
			bool unusedCI;
			prepareLitString( ss->cons_item->data, unusedCI, 
					ss->cons_item->prodEl->typeRef->pdaLiteral->data,
					ss->cons_item->prodEl->typeRef->pdaLiteral->loc );

			*data = ss->cons_item->data;
			*length = ss->cons_item->data.length();
		}
	}

	ss->cons_item = ss->cons_item->next;
	ss->offset = 0;
	return klangEl;
}

void inputStreamConsDestructor( program_t *prg, tree_t **sp, struct stream_impl_ct *ss )
{
}

int inputStreamConsGetParseBlock( struct colm_program *prg, struct stream_impl_ct *ss,
		int skip, char **pdp, int *copied )
{ 
	*copied = 0;

	ConsItem *buf = ss->cons_item;
	int offset = ss->offset;

	while ( true ) {
		if ( buf == 0 )
			return INPUT_EOF;

		if ( buf->type == ConsItem::ExprType || buf->type == ConsItem::LiteralType )
			return INPUT_LANG_EL;

		assert ( buf->type == ConsItem::InputText );
		int avail = buf->data.length() - offset;

		if ( avail > 0 ) {
			/* The source data from the current buffer. */
			char *src = &buf->data[offset];
			int slen = avail;

			/* Need to skip? */
			if ( skip > 0 && slen <= skip ) {
				/* Skipping the the whole source. */
				skip -= slen;
			}
			else {
				/* Either skip is zero, or less than slen. Skip goes to zero.
				 * Some data left over, copy it. */
				src += skip;
				slen -= skip;
				skip = 0;

				*pdp = src;
				*copied += slen;
				break;
			}
		}

		buf = buf->next;
		offset = 0;
	}

	return INPUT_DATA;
}

int inputStreamConsGetData( struct colm_program *prg, struct stream_impl_ct *ss, char *dest, int length )
{ 
	int copied = 0;

	ConsItem *buf = ss->cons_item;
	int offset = ss->offset;

	while ( true ) {
		if ( buf == 0 )
			break;

		if ( buf->type == ConsItem::ExprType || buf->type == ConsItem::LiteralType )
			break;

		assert ( buf->type == ConsItem::InputText );
		int avail = buf->data.length() - offset;

		if ( avail > 0 ) {
			/* The source data from the current buffer. */
			char *src = &buf->data[offset];
			int slen = avail <= length ? avail : length;

			memcpy( dest+copied, src, slen ) ;
			copied += slen;
			length -= slen;
		}

		if ( length == 0 )
			break;

		buf = buf->next;
		offset = 0;
	}

	return copied;
}

void inputStreamConsBackup( struct stream_impl_ct *ss )
{
	if ( ss->cons_item == 0 )
		ss->cons_item = ss->constructor->list->tail;
	else
		ss->cons_item = ss->cons_item->prev;
}

void inputStreamConsUndoConsumeLangEl( struct colm_program *prg, struct stream_impl_ct *ss )
{
	inputStreamConsBackup( ss );
	ss->offset = ss->cons_item->data.length();
}


int inputStreamConsConsumeData( struct colm_program *prg, struct stream_impl_ct *ss, int length, location_t *loc )
{
	int consumed = 0;

	while ( true ) {
		if ( ss->cons_item == 0 )
			break;

		int avail = ss->cons_item->data.length() - ss->offset;

		if ( length >= avail ) {
			/* Read up to the end of the data. Advance the
			 * pattern item. */
			ss->cons_item = ss->cons_item->next;
			ss->offset = 0;

			length -= avail;
			consumed += avail;

			if ( length == 0 )
				break;
		}
		else {
			ss->offset += length;
			consumed += length;
			break;
		}
	}

	return consumed;
}

int inputStreamConsUndoConsumeData( struct colm_program *prg, struct stream_impl_ct *ss, const char *data, int length )
{
	int origLen = length;
	while ( true ) {
		int avail = ss->offset;

		/* Okay to go up to the front of the buffer. */
		if ( length > avail ) {
			ss->cons_item= ss->cons_item->prev;
			ss->offset = ss->cons_item->data.length();
			length -= avail;
		}
		else {
			ss->offset -= length;
			break;
		}
	}

	return origLen;
}

stream_funcs_ct replFuncs =
{
	&inputStreamConsGetParseBlock,
	&inputStreamConsGetData,

	&inputStreamConsConsumeData,
	/* unused */ 0,
	&inputStreamConsGetLangEl,

	&inputStreamConsUndoConsumeData,
	/* unused. */ 0,
	&inputStreamConsUndoConsumeLangEl,

	0,

	&repl_stream_set_eof, &repl_stream_unset_eof,

	0, 0, 0, 0, 0, 0, /* prepend. */
	0, 0, 0, 0, 0, 0, /* append. */

	&inputStreamConsDestructor,

	0, 0, 0, 0,

	&inputStreamGetEofSent,
	&inputStreamSetEofSent,

	&pat_transfer_loc_seq,
};

void pushBinding( pda_run *pdaRun, parse_tree_t *parseTree )
{
	/* If the item is bound then store it in the bindings array. */
	pdaRun->bindings->push( parseTree );
}

extern "C" void internalSendNamedLangEl( program_t *prg, tree_t **sp,
		struct pda_run *pdaRun, struct stream_impl *is )
{
	/* All three set by consumeLangEl. */
	long bindId;
	char *data;
	long length;

	LangEl *klangEl = is->funcs->consume_lang_el( prg, is, &bindId, &data, &length );
	
	//cerr << "named langEl: " << prg->rtd->lelInfo[klangEl->id].name << endl;

	/* Copy the token data. */
	head_t *tokdata = 0;
	if ( data != 0 )
		tokdata = string_alloc_full( prg, data, length );

	kid_t *input = make_token_with_data( prg, pdaRun, is, klangEl->id, tokdata );

	colm_increment_steps( pdaRun );

	parse_tree_t *parseTree = parse_tree_allocate( pdaRun );
	parseTree->id = input->tree->id;
	parseTree->flags |= PF_NAMED;
	parseTree->shadow = input;

	if ( bindId > 0 )
		pushBinding( pdaRun, parseTree );
	
	pdaRun->parse_input = parseTree;
}

extern "C" void internalInitBindings( pda_run *pdaRun )
{
	/* Bindings are indexed at 1. Need a no-binding. */
	pdaRun->bindings = new bindings;
	pdaRun->bindings->push(0);
}

extern "C" void internalPopBinding( pda_run *pdaRun, parse_tree_t *parseTree )
{
	parse_tree_t *lastBound = pdaRun->bindings->top();
	if ( lastBound == parseTree )
		pdaRun->bindings->pop();
}
