# Colm = COmputer Language Machinery

Colm is a programming language designed for the analysis and [transformation of computer languages](https://www.program-transformation.org/Transform/TransformationSystems).<br>
Colm is influenced primarily by [TXL](https://www.txl.ca/).


## What is a transformation language?

A transformation language has a type system based on formal languages.<br>
Rather than defining classes or data structures, one defines grammars.

A parser is constructed automatically from the grammar, and the parser is used for two purposes:

- to parse the input language,
- and to parse the structural patterns in the program that performs the analysis.

In this setting, grammar-based parsing is critical because it guarantees that both the input and the structural patterns are parsed into trees from the same set of types, allowing comparison.


## Colm's features

Colm is not-your-typical-scripting-language™:

- Colm's main contribution lies in the parsing method.<br>Colm's parsing engine is generalized, but it also allows for the construction of arbitrary global data structures that can be queried during parsing. In other generalized methods, construction of global data requires some very careful consideration because of inherent concurrency in the parsing method. It is such a tricky task that it is often avoided altogether and the problem is deferred to a post-parse disambiguation of the parse forest.
- By default Colm will create an elf executable that can be used standalone for that actual transformations.
- Colm is a static and strong typed scripting language.
- Colm is very tiny and fast and can easily be embedded/linked with c/cpp programs.
- Colm's runtime is a stackbased VM that starts with the bare minimum of the language and bootstraps itself.


## Examples

This is how Colm is greeting the world ([`hello_world.lm`](doc/colm/code/hello_world.lm)):
```colm
print "hello world\n"
```

Here's a Colm program implementing a little assignment language ([`assign.lm`](doc/colm/code/assign.lm)) and its parse tree synthesis afterwards.
```colm
lex
	token id / ('a' .. 'z' | 'A' .. 'Z' ) + /
	token number / ( '0' .. '9' )+ /
	literal `= `;
	ignore / [ \t\n]+ /
end

def value
	[id] | [number]

def assignment
	[id `= value `;]

def assignment_list
	[assignment assignment_list]
|	[assignment]
|	[]

parse Simple: assignment_list[ stdin ]

if ( ! Simple ) {
	print( "[error]\n" )
	exit( 1 )
}
else {
	for I:assignment in Simple {
		print( $I.id, "->", $I.value, "\n" )
	}
}
```

More real-world programs parsing several languages implemented in Colm can be found in the [`grammar/`-folder](https://github.com/adrian-thurston/colm/tree/master/grammar).


## Usage

To immediatelly compile and run e.g. the `hello_world.lm` program from above, call

```
$ colm -r hello_world.lm
hello world
```

Run `colm --help` for help on further options.

```
$ colm --help
usage: colm [options] file
general:
   -h, -H, -?, --help   print this usage and exit
   -v --version         print version information and exit
   -b <ident>           use <ident> as name of C object encapulaing the program
   -o <file>            if -c given, write C parse object to <file>,
                        otherwise write binary to <file>
   -p <file>            write C parse object to <file>
   -e <file>            write C++ export header to <file>
   -x <file>            write C++ export code to <file>
   -m <file>            write C++ commit code to <file>
   -a <file>            additional code file to include in output program
   -E N=V               set a string value available in the program
   -I <path>            additional include path for the compiler
   -i                   activate branchpoint information
   -L <path>            additional library path for the linker
   -l                   activate logging
   -r                   run output program and replace process
   -c                   compile only (don't produce binary)
   -V                   print dot format (graphiz)
   -d                   print verbose debug information

```


## Building

To build Colm on your own, see the following dependencies and build instructions.

### Dependencies

This package has no external dependencies, other than usual autotools and C/C++ compiler programs.

For the program:
- make
- libtool
- gcc
- g++
- autoconf
- automake

For the documentation, install [`asciidoc`](https://asciidoctor.org/) and [`fig2dev`](https://github.com/getlarky/fig2dev) as well.

### Building

Colm is built in the usual autotool way:

```
$ ./autogen.sh
$ ./configure
$ make
$ make install
```

### Run-time dependencies

The colm program depends on GCC at runtime. It produces a C program as output,
then compiles and links it with a runtime library. The compiled program depends
on the colm library.

To find the includes and the runtime library to pass to GCC, colm looks at
`argv[0]` to decide if it is running out of the source tree. If it is, then the
compile and link flags are derived from `argv[0]`. Otherwise, it uses the install
location (prefix) to construct the flags.


## Syntax highlighting

There is a vim syntax definition file [colm.vim](/colm.vim).


## License

Colm is free software under the MIT license.<br>
Please see the COPYING file for more details.
