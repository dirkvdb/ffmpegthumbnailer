#!/bin/sh -x
libtoolize --force
aclocal
autoconf
automake --add-missing
