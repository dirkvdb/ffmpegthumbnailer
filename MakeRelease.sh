#!/bin/bash

version=""
project="ffmpegthumbnailer"
password=$1

if [ "$password" == "" ]
then
	echo "No password for Google Code supplied"
	exit 1
fi

status=`svn status -usvn status -u`
if [ "$status" != "" ]
then
	echo "svn status is not clean, commit changes first"
	exit 1
fi

#get version number
version=`cat configure.ac | grep AC_INIT | cut -d[ -f3 | cut -d] -f1`

#build the code
builddir="out-$version"
rm -rf $builddir
mkdir -p $builddir
cd $builddir
../configure
if [ $? != 0 ]
then
	echo "Configure failed"
	exit 1
fi

make check
if [ $? != 0 ]
then
	echo "Make check failed"
	exit 1
fi

./testrunner
if [ $? != 0 ]
then
	echo "Unittests did not succeed"
	exit 1
fi

make distcheck
if [ $? != 0 ]
then
	echo "Make failed"
	exit 1
fi

cd ..

#upload archive to google
python googlecode_upload.py -s "Release $version" -p $project -u dirk.vdb -w $password -l "Featured,Type-Source,OpSys-Linux"

#create a tag
#svn copy https://ffmpegthumbnailer.googlecode.com/svn/trunk https://ffmpegthumbnailer.googlecode.com/svn/tags/$project-$version -m "Tag of release $version"
