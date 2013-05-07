#!/bin/bash

if [ "$#" != "1" ] ; then
    echo " "
    echo "Error: You need to give the packed heasoft source code, e.g."
    echo "       bash build-heasoft.sh heasoft-6.12src.tar.gz"
    echo " "
    exit 0;
fi;

VER=`echo ${1} | awk -Fheasoft- '{ print $2 }'| awk -Fsrc.tar.gz '{ print $1 }'`;
echo "Version of heasoft is: ${VER}"

if ( test -d heasoft_v${VER} ); then
    echo "Removing old install..."
    rm -rf heasoft_v${VER}
fi
echo "Unpacking..."
tar xvfz $1 > /dev/null
mv heasoft-${VER} heasoft_v${VER}

CORES=1;
if ( `test -f /usr/sbin/sysctl` ); then
    CORES=`sysctl -n hw.logicalcpu_max`
elif ( `test -f /proc/cpuinfo` ); then 
    CORES=`grep processor /proc/cpuinfo | wc -l`
fi

echo "Using this number of cores: ${CORES}"

cd heasoft_v${VER}/BUILD_DIR


echo "Configuring ..."
# Minimze the LD_LIBRARY_PATH to prevent problems with multiple readline's
export LD_LIBRARY_PATH=/usr/lib
./configure --enable-readline > config.out 2>&1 
echo "Compiling ..."
make -j1 > build.log 2>&1 
ERRORS=`grep -v "char \*\*\*" build.log | grep "\*\*\*"`
if [ "${ERRORS}" == "" ]; then
    echo "Installing ..."
    make -j1 install > install.log 2>&1 
    ERRORS=`grep -v "char \*\*\*" install.log | grep "\*\*\*"`
    if [ "${ERRORS}" != "" ]; then
        echo "Errors occured during the installation. Check your install.log"
        exit 1;
    fi
else
    echo "Errors occured during the compilation. Check your build.log"
    exit 1;
fi

# Create a libcfitsio.so, etc. link
cd ../*86*/lib
CFITSIO=`find . -name "libcfitsio??[so|a|dylib|dll]"`
LONGCFITSIO=`find . -name "libcfitsio_*[so|a|dylib|dll]" -printf "%f"`
if ( [ "${CFITSIO}" == "" ] && [ "${LONGCFITSIO}" != "" ] ); then 
    NEWCFITSIO=`echo ${LONGCFITSIO} | awk -F'[_]|[.]' '{ print $1"."$4 }'`
    ln -s ${LONGCFITSIO} ${NEWCFITSIO}
fi

# Set all permissions, etc.
cd ../../..
chown -R ${USER}:${GROUP} heasoft_v${VER}
chmod -R go+rX heasoft_v${VER}

echo "Finished ..."


