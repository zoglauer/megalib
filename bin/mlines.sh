#! /bin/bash

help() {
    echo ""
    echo "mlines - line counter for MEGAlib";
    echo "(C) 2003-2008 by Andreas Zoglauer"
    echo "";
    echo "Usage: mlines <dir>";
    echo "";
    echo "Remark: if <dir> is omitted the MEGAlib is used";
    echo "";
}

Files() {
    for file in `find . -name '*.cxx'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.cc'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.f'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.hh'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.h'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.C'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name '*.cpp'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name 'Makefile'`; do
        CMD=$CMD' '$dir/$file;
    done
    for file in `find . -name 'GNUmakefile'`; do
        CMD=$CMD' '$dir/$file;
    done
}


dir=""
CMD=" "
echo "Gathering file information..."

if [ $# -ne 1 ] ; then
    dir=$MEGALIB/src
    echo "Changing to $dir..."
    cd $dir
    Files
    dir=$COSIMA/src
    echo "Changing to $dir..."
    cd $dir
    Files
    dir=$COSIMA/include
    echo "Changing to $dir..."
    cd $dir
    Files
    dir=$GMEGA/src
    echo "Changing to $dir..."
    cd $dir
    Files
else
    dir=$1
    echo "Changing to $dir..."
    cd $dir
    Files
fi;



echo "Calculating number of lines..."
wc -l $CMD

exit
