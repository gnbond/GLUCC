#! /bin/sh

# nocompile.sh filename
#
# 1. parse the file to see if any "// expect: " lines, these are expected compile errors
# 2. build the target with CMake and capture the output
# 3. return 1 if the compile succeeds!
# 4. return 0 if stderr contains any of the strings
# 5. else return 1

filename="$1"
targetname=`basename $filename .cpp`-build

patfile=/tmp/nocompile-patfile.$$
out=/tmp/nocompile-out.$$
#trap 'rm $patfile $out' 0 1 2

sed -n -e "s=^// expect: *==p" < $filename > $patfile
if [ `wc -l < $patfile` -lt 1 ]; then
    echo "No expected failures found in $filename" 1>&2
    exit 1;
fi
if cmake --build . --target $targetname >& $out ; then
    echo "Build of $targetname unexpectedly succeeded" 1>&2
    exit 1
fi

errors=`fgrep -f $patfile $out`
if  [ -z "$errors" ]; then
    echo "Build failed but not with expected error" 1>&2
    exit 1
fi

echo "Failed with expected error $errors"
exit 0
