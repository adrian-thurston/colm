/*
 * Copyright 2006-2018 Adrian Thurston <thurston@colm.net>
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>

#include "debug.h"
#include "pcheck.h"
#include "version.h"
#include "compiler.h"

#if defined(CONS_INIT)
#include "consinit.h"
#elif defined(LOAD_INIT)
#include "loadinit.h"
#else
#include "loadfinal.h"
#endif

using std::istream;
using std::ifstream;
using std::ostream;
using std::ios;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

/* Graphviz dot file generation. */
bool genGraphviz = false;

using std::ostream;
using std::istream;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::cout;
using std::cerr;
using std::cin;
using std::endl;

InputLoc internal;

/* Io globals. */
istream *inStream = 0;
ostream *outStream = 0;
const char *inputFn = 0;
const char *outputFn = 0;
const char *intermedFn = 0;
const char *binaryFn = 0;
const char *exportHeaderFn = 0;
const char *exportCodeFn = 0;
const char *commitCodeFn = 0;
const char *buildDir = 0;
const char *objectName = "colm_object";
bool exportCode = false;
bool hostAdapters = true;

bool generateGraphviz = false;
bool verbose = false;
bool logging = false;
bool branchPointInfo = false;
bool run = false;
bool addUniqueEmptyProductions = false;
bool gblLibrary = false;
long gblActiveRealm = 0;
bool outputSpecifiedWithDashP = false;

ArgsVector includePaths;
ArgsVector libraryPaths;
DefineVector defineArgs;
ArgsVector additionalCodeFiles;

/* Print version information. */
void version();

/* Total error count. */
int gblErrorCount = 0;

/*
 * Alphabet Type for the parsing machinery. The trees/strings of parsed data
 * all use char type. Currently we can support signed char, unsigned char or
 * char. If changing this, the colm_alph_t type needs to change as well.
 * Currently, this is a compile time change only. A colm binary currently
 * connot be made to work with multiple alphabet types.
 */

HostType hostTypesC[] =
{
	{ "unsigned", "char", false, 0, UCHAR_MAX, sizeof(unsigned char) },
};


HostLang hostLangC = { hostTypesC, 1, 0 };
HostLang *hostLang = &hostLangC;

/* Print the opening to an error in the input, then return the error ostream. */
ostream &error( const InputLoc &loc )
{
	/* Keep the error count. */
	gblErrorCount += 1;

	if ( loc.fileName != 0 )
		cerr << loc.fileName << ":";
	else
		cerr << "<input>:";

	if ( loc.line == -1 ) {
		cerr << "INT: ";
	}
	else {
		cerr << loc.line << ":" << loc.col << ": ";
	}
	return cerr;
}

/* Print the opening to a program error, then return the error stream. */
ostream &error()
{
	gblErrorCount += 1;
	cerr << "error: " PROGNAME ": ";
	return cerr;
}


/* Print the opening to a warning, then return the error ostream. */
ostream &warning( )
{
	cerr << "warning: " << inputFn << ": ";
	return cerr;
}

/* Print the opening to a warning in the input, then return the error ostream. */
ostream &warning( const InputLoc &loc )
{
	assert( inputFn != 0 );
	cerr << "warning: " << inputFn << ":" <<
			loc.line << ":" << loc.col << ": ";
	return cerr;
}

void escapeLineDirectivePath( std::ostream &out, char *path )
{
	for ( char *pc = path; *pc != 0; pc++ ) {
		if ( *pc == '\\' )
			out << "\\\\";
		else
			out << *pc;
	}
}

void escapeLineDirectivePath( std::ostream &out, char *path );
void scan( char *fileName, istream &input );

bool printStatistics = false;

/* Print a summary of the options. */
void usage()
{
	cout <<
"usage: colm [options] file\n"
"general:\n"
"   -h, -H, -?, --help   print this usage and exit\n"
"   -v --version         print version information and exit\n"
"   -b <ident>           use <ident> as name of C object encapulaing the program\n"
"   -o <file>            if -c given, write C parse object to <file>,\n"
"                        otherwise write binary to <file>\n"
"   -p <file>            write C parse object to <file>\n"
"   -e <file>            write C++ export header to <file>\n"
"   -x <file>            write C++ export code to <file>\n"
"   -m <file>            write C++ commit code to <file>\n"
"   -a <file>            additional code file to include in output program\n"
"   -E N=V               set a string value available in the program\n"
"   -I <path>            additional include path for the compiler\n"
"   -i                   activate branchpoint information\n"
"   -L <path>            additional library path for the linker\n"
"   -l                   activate logging\n"
"   -r                   run output program and replace process\n"
"   -c                   compile only (don't produce binary)\n"
"   -V                   print dot format (graphiz)\n"
"   -d                   print verbose debug information\n"
#if DEBUG
"   -D <tag>             print more information about <tag>\n"
"                        (BYTECODE|PARSE|MATCH|COMPILE|POOL|PRINT|INPUT|SCAN\n"
#endif
	;
}

/* Print version information. */
void version()
{
	cout << "Colm version " VERSION << " " PUBDATE << endl <<
			"Copyright (c) 2007-2020 by Dr. Adrian D. Thurston" << endl;
}

/* Scans a string looking for the file extension. If there is a file
 * extension then pointer returned points to inside the string
 * passed in. Otherwise returns null. */
const char *findFileExtension( const char *stemFile )
{
	const char *ppos = stemFile + strlen(stemFile) - 1;

	/* Scan backwards from the end looking for the first dot.
	 * If we encounter a '/' before the first dot, then stop the scan. */
	while ( 1 ) {
		/* If we found a dot or got to the beginning of the string then
		 * we are done. */
		if ( ppos == stemFile || *ppos == '.' )
			break;

		/* If we hit a / then there is no extension. Done. */
		if ( *ppos == '/' ) {
			ppos = stemFile;
			break;
		}
		ppos--;
	}

	/* If we got to the front of the string then bail we
	 * did not find an extension  */
	if ( ppos == stemFile )
		ppos = 0;

	return ppos;
}

/* Make a file name from a stem. Removes the old filename suffix and
 * replaces it with a new one. Returns a newed up string. */
char *fileNameFromStem( const char *stemFile, const char *suffix )
{
	int len = strlen( stemFile );
	assert( len > 0 );

	/* Get the extension. */
	const char *ppos = findFileExtension( stemFile );

	/* If an extension was found, then shorten what we think the len is. */
	if ( ppos != 0 )
		len = ppos - stemFile;

	int slen = suffix != 0 ? strlen( suffix ) : 0;
	char *retVal = new char[ len + slen + 1 ];
	strncpy( retVal, stemFile, len );
	if ( suffix != 0 )
		strcpy( retVal + len, suffix );
	retVal[len+slen] = 0;

	return retVal;
}

void openOutputCompiled()
{
	/* Start with the fn given by -o option. */
	binaryFn = outputFn;

	if ( binaryFn == 0 )
		binaryFn = fileNameFromStem( inputFn, 0 );

	if ( intermedFn == 0 )
		intermedFn = fileNameFromStem( binaryFn, ".c" );

	if ( binaryFn != 0 && inputFn != 0 &&
			strcmp( inputFn, binaryFn  ) == 0 )
	{
		error() << "output file \"" << binaryFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( intermedFn != 0 && inputFn != 0 &&
			strcmp( inputFn, intermedFn  ) == 0 )
	{
		error() << "intermediate file \"" << intermedFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( intermedFn != 0 ) {
		/* Open the output stream, attaching it to the filter. */
		ofstream *outFStream = new ofstream( intermedFn );

		if ( !outFStream->is_open() ) {
			error() << "error opening " << intermedFn << " for writing" << endl;
			exit(1);
		}

		outStream = outFStream;
	}
	else {
		/* Writing out ot std out. */
		outStream = &cout;
	}
}

void openOutputLibrary()
{
	if ( outputFn == 0 )
		outputFn = fileNameFromStem( inputFn, ".c" );

	/* Make sure we are not writing to the same file as the input file. */
	if ( outputFn != 0 && inputFn != 0 &&
			strcmp( inputFn, outputFn  ) == 0 )
	{
		error() << "output file \"" << outputFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( outputFn != 0 ) {
		/* Open the output stream, attaching it to the filter. */
		ofstream *outFStream = new ofstream( outputFn );

		if ( !outFStream->is_open() ) {
			error() << "error opening " << outputFn << " for writing" << endl;
			exit(1);
		}

		outStream = outFStream;
	}
	else {
		/* Writing out ot std out. */
		outStream = &cout;
	}
}

void openExports( )
{
	/* Make sure we are not writing to the same file as the input file. */
	if ( inputFn != 0 && exportHeaderFn != 0 && strcmp( inputFn, exportHeaderFn  ) == 0 ) {
		error() << "output file \"" << exportHeaderFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( exportHeaderFn != 0 ) {
		/* Open the output stream, attaching it to the filter. */
		ofstream *outFStream = new ofstream( exportHeaderFn );

		if ( !outFStream->is_open() ) {
			error() << "error opening " << exportHeaderFn << " for writing" << endl;
			exit(1);
		}

		outStream = outFStream;
	}
	else {
		/* Writing out ot std out. */
		outStream = &cout;
	}
}

void openExportsImpl( )
{
	/* Make sure we are not writing to the same file as the input file. */
	if ( inputFn != 0 && exportCodeFn != 0 && strcmp( inputFn, exportCodeFn  ) == 0 ) {
		error() << "output file \"" << exportCodeFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( exportCodeFn != 0 ) {
		/* Open the output stream, attaching it to the filter. */
		ofstream *outFStream = new ofstream( exportCodeFn );

		if ( !outFStream->is_open() ) {
			error() << "error opening " << exportCodeFn << " for writing" << endl;
			exit(1);
		}

		outStream = outFStream;
	}
	else {
		/* Writing out ot std out. */
		outStream = &cout;
	}
}

void openCommit( )
{
	/* Make sure we are not writing to the same file as the input file. */
	if ( inputFn != 0 && commitCodeFn != 0 && strcmp( inputFn, commitCodeFn  ) == 0 ) {
		error() << "output file \"" << commitCodeFn  <<
				"\" is the same as the input file" << endl;
	}

	if ( commitCodeFn != 0 ) {
		/* Open the output stream, attaching it to the filter. */
		ofstream *outFStream = new ofstream( commitCodeFn );

		if ( !outFStream->is_open() ) {
			error() << "error opening " << commitCodeFn << " for writing" << endl;
			exit(1);
		}

		outStream = outFStream;
	}
	else {
		/* Writing out ot std out. */
		outStream = &cout;
	}
}

int compileOutputCommand( const char *command )
{
	if ( verbose )
		cout << "compiling with: '" << command << "'" << endl;
	int res = system( command );
	if ( res != 0 )
		error() << "there was a problem compiling the output" << endl;

	return res;
}

void runOutputProgram()
{
	if ( verbose )
		cout << "running output: '" << binaryFn << "'" << endl;

	execl( binaryFn, binaryFn, NULL );
	/* We shall never return here! */
}

void compileOutput()
{
	const char *compiler = getenv( "CC" );
	if ( compiler == 0 )
		compiler = "gcc";

	const char *cflags = getenv( "CFLAGS" );
	if ( cflags == 0 )
		cflags = "";

	int length = 1024 + strlen( compiler ) + strlen( cflags ) + 
			strlen( intermedFn ) + strlen( binaryFn );
	for ( ArgsVector::Iter af = additionalCodeFiles; af.lte(); af++ )
		length += strlen( *af ) + 2;
	for ( ArgsVector::Iter ip = includePaths; ip.lte(); ip++ )
		length += strlen( *ip ) + 3;
	for ( ArgsVector::Iter lp = libraryPaths; lp.lte(); lp++ )
		length += strlen( *lp ) + 3;
	if ( buildDir != 0 )
		length += strlen( buildDir ) * 3;
#define COMPILE_COMMAND_STRING "%s -Wall -Wwrite-strings" \
		" -g %s" \
		" -o %s" \
		" %s"
	char *command = new char[length];
	if ( buildDir != 0 ) {
		sprintf( command,
				"%s/libtool --tag=CC --mode=link "
				COMPILE_COMMAND_STRING
				" -I%s/src/include"
				" -static"
				" %s/src/libcolm.la",
				buildDir, compiler, cflags,
				binaryFn, intermedFn,
				buildDir, buildDir );
	}
	else {
		sprintf( command,
				COMPILE_COMMAND_STRING
				" -I" INCLUDEDIR
				" -L" LIBDIR
				" -Wl,-rpath," LIBDIR,
				compiler, cflags,
				binaryFn, intermedFn );
	}
#undef COMPILE_COMMAND_STRING
	for ( ArgsVector::Iter af = additionalCodeFiles; af.lte(); af++ ) {
		strcat( command, " " );
		strcat( command, *af );
	}
	for ( ArgsVector::Iter ip = includePaths; ip.lte(); ip++ ) {
		strcat( command, " -I" );
		strcat( command, *ip );
	}
	for ( ArgsVector::Iter lp = libraryPaths; lp.lte(); lp++ ) {
		strcat( command, " -L" );
		strcat( command, *lp );
	}

	if ( buildDir == 0 )
		strcat( command, " -lcolm" );

	if( !compileOutputCommand( command ) && run )
		runOutputProgram();

	delete[] command;
}

void processArgs( int argc, const char **argv )
{
	ParamCheck pc( "p:cD:e:x:I:L:vdliro:S:M:vHh?-:sVa:m:b:E:B:", argc, argv );

	while ( pc.check() ) {
		switch ( pc.state ) {
		case ParamCheck::match:
			switch ( pc.parameter ) {
			case 'I':
				includePaths.append( pc.parameterArg );
				break;
			case 'v':
				version();
				exit(0);
				break;
			case 'd':
				verbose = true;
				break;
			case 'l':
				logging = true;
				break;
			case 'L':
				libraryPaths.append( pc.parameterArg );
				break;
			case 'i':
				branchPointInfo = true;
				break;
			case 'r':
				run = true;
				break;
			case 'p':
				outputSpecifiedWithDashP = true;
				/* fallthrough */
			case 'o':
				/* Output. */
				if ( *pc.parameterArg == 0 )
					error() << "a zero length output file name was given" << endl;
				else if ( outputFn != 0 )
					error() << "more than one output file name was given" << endl;
				else {
					/* Ok, remember the output file name. */
					outputFn = pc.parameterArg;
				}
				break;

			case 'b':
				/* object name. */
				if ( *pc.parameterArg == 0 )
					error() << "a zero length object name was given" << endl;
				else {
					/* Ok, remember the output file name. */
					objectName = pc.parameterArg;
					hostAdapters = false;
				}
				break;

			case 'H': case 'h': case '?':
				usage();
				exit(0);
			case 's':
				printStatistics = true;
				break;
			case 'V':
				generateGraphviz = true;
				break;
			case '-':
				if ( strcasecmp(pc.parameterArg, "help") == 0 ) {
					usage();
					exit(0);
				}
				else if ( strcasecmp(pc.parameterArg, "version") == 0 ) {
					version();
					exit(0);
				}
				else {
					error() << "--" << pc.parameterArg <<
							" is an invalid argument" << endl;
				}
				break;
			case 'c':
				gblLibrary = true;
				break;
			case 'e':
				exportHeaderFn = pc.parameterArg;
				break;
			case 'x':
				exportCodeFn = pc.parameterArg;
				break;
			case 'a':
				additionalCodeFiles.append( pc.parameterArg );
				break;
			case 'm':
				commitCodeFn = pc.parameterArg;
				break;
			case 'B':
				buildDir = pc.parameterArg;
				break;

			case 'E': {
				const char *eq = strchr( pc.parameterArg, '=' );
				if ( eq == 0 )
					fatal( "-E option argument must contain =" );
				if ( eq == pc.parameterArg )
					fatal( "-E variable name is of zero length" );

				defineArgs.append( DefineArg(
						String( pc.parameterArg, eq-pc.parameterArg ),
						String( eq + 1 ) ) );

				break;
			}

			case 'D':
#if DEBUG
				// @NOTE: keep this in sync with 'debug.c': 'colm_realm_names'
				if ( strcmp( pc.parameterArg, colm_realm_names[0] ) == 0 )
					gblActiveRealm |= REALM_BYTECODE;
				else if ( strcmp( pc.parameterArg, colm_realm_names[1] ) == 0 )
					gblActiveRealm |= REALM_PARSE;
				else if ( strcmp( pc.parameterArg, colm_realm_names[2] ) == 0 )
					gblActiveRealm |= REALM_MATCH;
				else if ( strcmp( pc.parameterArg, colm_realm_names[3] ) == 0 )
					gblActiveRealm |= REALM_COMPILE;
				else if ( strcmp( pc.parameterArg, colm_realm_names[4] ) == 0 )
					gblActiveRealm |= REALM_POOL;
				else if ( strcmp( pc.parameterArg, colm_realm_names[5] ) == 0 )
					gblActiveRealm |= REALM_PRINT;
				else if ( strcmp( pc.parameterArg, colm_realm_names[6] ) == 0 )
					gblActiveRealm |= REALM_INPUT;
				else if ( strcmp( pc.parameterArg, colm_realm_names[7] ) == 0 )
					gblActiveRealm |= REALM_SCAN;
				else
					fatal( "unknown argument to -D %s\n", pc.parameterArg );
#else
				fatal( "-D option specified but debugging messages not compiled in\n" );
#endif
				break;

			}
			break;

		case ParamCheck::invalid:
			error() << "-" << pc.parameter << " is an invalid argument" << endl;
			break;

		case ParamCheck::noparam:
			/* It is interpreted as an input file. */
			if ( *pc.curArg == 0 )
				error() << "a zero length input file name was given" << endl;
			else if ( inputFn != 0 )
				error() << "more than one input file name was given" << endl;
			else {
				/* OK, Remember the filename. */
				inputFn = pc.curArg;
			}
			break;
		}
	}
}

bool readCheck( const char *fn )
{
	int result = true;

	/* Check if we can open the input file for reading. */
	ifstream *inFile = new ifstream( fn );
	if ( ! inFile->is_open() )
		result = false;

	delete inFile;
	return result;
}

/* Main, process args and call yyparse to start scanning input. */
int main(int argc, const char **argv)
{
	processArgs( argc, argv );

	if ( verbose )
		gblActiveRealm = 0xffffffff;
	
	/* Bail on above errors. */
	if ( gblErrorCount > 0 )
		exit(1);

	/* Make sure we are not writing to the same file as the input file. */
	if ( inputFn != 0 && outputFn != 0 &&
			strcmp( inputFn, outputFn  ) == 0 )
	{
		error() << "output file \"" << outputFn  <<
				"\" is the same as the input file" << endl;
	}

#if defined(LOAD_INIT) || defined(LOAD_COLM)
	/* Open the input file for reading. */
	if ( inputFn == 0 ) {
		error() << "colm: no input file given" << endl;
	}
	else {
		/* Check if we can open the input file for reading. */
		if ( ! readCheck( inputFn ) )
			error() << "could not open " << inputFn << " for reading" << endl;
	}
#endif

	if ( !gblLibrary && outputSpecifiedWithDashP ) {
		error() << "-p option must be used with -c" << endl;
	}


	/* Bail on above errors. */
	if ( gblErrorCount > 0 )
		exit(1);

	Compiler *pd = new Compiler;

#if defined(CONS_INIT)
	BaseParser *parser = new ConsInit( pd );
#elif defined(LOAD_INIT)
	BaseParser *parser = new LoadInit( pd, inputFn );
#else
	BaseParser *parser = consLoadColm( pd, inputFn );
#endif

	parser->go( gblActiveRealm );

	/* Parsing complete, check for errors.. */
	if ( gblErrorCount > 0 )
		return 1;

	/* Initiate a compile following a parse. */
	pd->compile();

	/*
	 * Write output.
	 */
	if ( generateGraphviz ) {
		outStream = &cout;
		pd->writeDotFile();
	}
	else {
		if ( gblLibrary )
			openOutputLibrary();
		else
			openOutputCompiled();

		pd->generateOutput( gblActiveRealm, ( commitCodeFn == 0 ) );
		if ( outStream != 0 )
			delete outStream;

		if ( !gblLibrary )
			compileOutput();

		if ( exportHeaderFn != 0 ) {
			openExports();
			pd->generateExports();
			delete outStream;
		}
		if ( exportCodeFn != 0 ) {
			openExportsImpl();
			pd->generateExportsImpl();
			delete outStream;
		}
		if ( commitCodeFn != 0 ) {
			openCommit();
			pd->writeCommit();
			delete outStream;
		}
	}

	delete parser;
	delete pd;

	/* Bail on above errors. */
	if ( gblErrorCount > 0 )
		exit(1);

	return 0;
}
