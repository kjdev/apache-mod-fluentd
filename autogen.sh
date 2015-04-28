#!/bin/sh

set -e

# Constants
ACLOCAL="aclocal"
AUTOHEADER="autoheader"
AUTOMAKE="automake"
AUTOCONF="autoconf"
LIBTOOLIZE="libtoolize"

# Cleanup
echo "Cleanup"
rm -rf .libs autom4te*.cache scripts aclocal.m4 configure config.log config.status config.guess config.sub .deps stamp-h1 depcomp install-sh ltmain.sh missing libtool config.h config.h.in m4 ar-lib
rm -f *.o *.la *.lo *.slo Makefile.in Makefile

if [ "$1" = "clean" ]; then
    exit
fi

echo "Running aclocal"
${ACLOCAL}

echo "Running libtoolize"
${LIBTOOLIZE} -c

echo "Running autoheader"
${AUTOHEADER}

echo "Running automake"
${AUTOMAKE} -a

echo "Running autoconf"
${AUTOCONF}
