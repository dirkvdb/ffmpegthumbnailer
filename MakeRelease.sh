#!/bin/bash

function checkresult {
    "$@"
    local status=$?
    if [ $status -ne 0 ]; then
        echo "error with $@ status=$status" >&2
        exit $status
    fi
    return $status
}

option=$1

project="ffmpegthumbnailer"

if [[ $(git diff --shortstat 2> /dev/null | tail -n1) != "" ]]
then
	echo "git status is not clean, commit changes first"
	#exit 1
fi

#get version number
version_major=`cat CMakeLists.txt | grep set\(PACKAGE_VERSION_MAJOR | cut -d ' ' -f2 | sed 's/)//'`
version_minor=`cat CMakeLists.txt | grep set\(PACKAGE_VERSION_MINOR | cut -d ' ' -f2 | sed 's/)//'`
version_patch=`cat CMakeLists.txt | grep set\(PACKAGE_VERSION_PATCH | cut -d ' ' -f2 | sed 's/)//'`
version="${version_major}.${version_minor}.${version_patch}"

echo $version

#build the code
builddir="out-$version"
rm -rf $builddir
mkdir -p $builddir

cd $builddir
checkresult cmake -DENABLE_TESTS=ON -DENABLE_THUMBNAILER=ON -DCMAKE_BUILD_TYPE=Release -DENABLE_STATIC=ON -DENABLE_SHARED=ON ..
checkresult make -j4
checkresult ctest CTEST_OUTPUT_ON_FAILURE=1
checkresult make package_source
cd ..

if [ "$option" == "noupload" ]
then
    echo "Skipping upload"
	exit 0
fi

#upload archive to google
checkresult github-release release --user dirkvdb --repo ffmpegthumbnailer --tag ${version} --name ffmpegthumbnailer-${version} --description "ffmpegthumbnailer release ${version}"
checkresult github-release upload --user dirkvdb --repo ffmpegthumbnailer --tag ${version} --name ffmpegthumbnailer-${version}.tar.bz2 --file ${builddir}/ffmpegthumbnailer-${version}.tar.bz2
rm -rf $builddir
