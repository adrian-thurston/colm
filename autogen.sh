#!/bin/bash

set -x

case `uname` in
	Darwin)
		${LIBTOOLIZE:-glibtoolize} --quiet --copy --force
		;;
	*)
		${LIBTOOLIZE:-libtoolize} --quiet --copy --force
	;;
esac

aclocal
autoheader
automake --foreign --add-missing
autoconf
