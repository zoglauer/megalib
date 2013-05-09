#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.


CONFIGUREOPTIONS="--gminimal --enable-asimage --enable-xft --enable-opengl --enable-mathmore --enable-explicitlink --enable-rpath --enable-soversion"
COMPILEROPTIONS=`gcc --version | head -n 1`


confhelp() {
  echo ""
  echo "Building ROOT"
  echo " " 
  echo "Usage: ./build-root.sh [options]";
  echo " "
  echo " "
  echo "Options:"
  echo "--tarball=[file name of ROOT tar ball]"
  echo "    Use this tarball instead of downloading it from the ROOT website" 
  echo " "
  echo "--environment-script=[file name of new environment script]"
  echo "    File in which the ROOT path is stored. This is used by the MEGAlib setup script" 
  echo " "
  echo "--help or -h"
  echo "    Show this help."
  echo " "
  echo " "
}

# Store command line
CMD=""
while [[ $# -gt 0 ]] ; do
    CMD="${CMD} $1"
    shift
done

# Check for help
for C in ${CMD}; do
  if [[ ${C} == *-h* ]]; then
    echo ""
    confhelp
    exit 0
  fi
done

TARBALL=""
ENVFILE=""

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-t*=* ]]; then
    TARBALL=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo "Using this tarball: ${TARBALL}"
  elif [[ ${C} == *-e* ]]; then
    ENVFILE=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo "Using this MEGALIB environment file: ${ENVFILE}"
  elif [[ ${C} == *-h* ]]; then
    echo ""
    confhelp
    exit 0
  else
    echo ""
    echo "ERROR: Unknown command line option: ${C}"
    echo "       See \"$0 --help\" for a list of options"
    exit 1
  fi
done


echo "Getting ROOT..."
VER=""
if [ "${TARBALL}" != "" ]; then
  # Use given ROOT tarball
  echo "The given ROOT tarball is ${TARBALL}"
  
  # Check if it has the correct version:
  VER=`tar xfzO ${TARBALL} root/build/version_number | sed 's|/|.|g'`
  RESULT=$?
  if [ "${RESULT}" != "0" ]; then
    echo "ERROR: Something went wrong unpacking the ROOT tarball!"
    exit 1
  fi
  if echo ${VER} | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Something terrible is wrong with your version string..."
    exit 1
  fi
  echo "Version of ROOT is: ${VER}"
  
  bash ${MEGALIB}/config/check-rootversion.sh --good-version=${VER}
  if [ "$?" != "0" ]; then
    echo "ERROR: The ROOT tarball you supplied does not contain an acceptable ROOT version!"
    exit 1
  fi
else
  # Download it
  
  # Get desired version:
  WANTEDVERSION=`bash ${MEGALIB}/config/check-rootversion.sh --get-max`
  if [ "$?" != "0" ]; then
    echo "ERROR: Unable to determine required ROOT version!"
    exit 1
  fi
  echo "Looking for ROOT version ${WANTEDVERSION} with latest patch on ROOT website"
  
  # Now check root repository for the given version:
  TARBALL=`curl ftp://root.cern.ch/root/ -sl | grep "root_v${WANTEDVERSION}" | grep "source.tar.gz" | sort | tail -n 1`
  if [ "${TARBALL}" == "" ]; then
    echo "ERROR: Unable to find suitable ROOT tar ball at the ROOT website"
    exit 1
  fi
  echo "Using ROOT tar ball ${TARBALL}"
  
  # Check if it already exists locally
  REQUIREDOWNLOAD="true"
  if [ -f ${TARBALL} ]; then
    # ... and has the same size
    LOCALSIZE=`wc -c < ${TARBALL} | tr -d ' '`
    SAMESIZE=`curl --head ftp://root.cern.ch/root/${TARBALL}`
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to determine remote tarball size"
      exit 1
    fi
    SAMESIZE=`echo ${SAMESIZE} | grep ${LOCALSIZE}`
    if [ "${SAMESIZE}" != "" ]; then
      REQUIREDOWNLOAD="false"
      echo "File is already present and has same size, thus no download required!"
    fi
  fi
  
  if [ "${REQUIREDOWNLOAD}" == "true" ]; then
    curl -O ftp://root.cern.ch/root/${TARBALL}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to download the tarball from the ROOT website!"
      exit 1
    fi
  fi
  
  VER=`tar xfzO ${TARBALL} root/build/version_number | sed 's|/|.|g'`
  if [ "$?" != "0" ]; then
    echo "ERROR: Something went wrong unpacking the ROOT tarball!"
    exit 1
  fi
  if echo ${VER} | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Something terrible is wrong with your version string..."
    exit 1
  fi
  echo "Version of root is: ${VER}"
fi



echo "Checking for old installation..."
if [ -d root_v${VER} ]; then
  cd root_v${VER}
  if [ -f COMPILE_SUCCESSFUL ]; then
    SAMEOPTIONS=`cat COMPILE_SUCCESSFUL | grep -- "${CONFIGUREOPTIONS}"`
    if [ "${SAMEOPTIONS}" == "" ]; then
      echo "The old installation used different compilation options..."
    fi
    SAMECOMPILER=`cat COMPILE_SUCCESSFUL | grep -- "${COMPILEROPTIONS}"`
    if [ "${SAMECOMPILER}" == "" ]; then
      echo "The old installation used a different compiler..."
    fi
    if ( [ "${SAMEOPTIONS}" != "" ] && [ "${SAMECOMPILER}" != "" ] ); then
      echo "Your already have a usable ROOT version installed!"
      if [ "${ENVFILE}" != "" ]; then
        echo "Storing the ROOT directory in the MEGAlib source script..."
        echo "ROOTDIR=`pwd`" >> ${ENVFILE}
      fi
      exit 0
    fi
  fi
    
  echo "Old installation is most likely incompatible. Removing root_v${VER}"
  cd ..
  if echo "root_v${VER}" | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Feeding my paranoia of having a \"rm -r\" in a script:"
    echo "       There should not be any spaces in the ROOT version..."
    exit 1
  fi
  rm -r "root_v${VER}"
fi



echo "Unpacking..."
mkdir root_v${VER}
cd root_v${VER}
tar xvfz ../${TARBALL}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong unpacking the ROOT tarball!"
  exit 1
fi
mv root/* .
rmdir root



echo "Configuring..."
sh configure ${CONFIGUREOPTIONS}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong configuring ROOT!"
  exit 1
fi



CORES=1;
if ( `test -f /usr/sbin/sysctl` ); then
  CORES=`sysctl -n hw.logicalcpu_max`
elif ( `test -f /proc/cpuinfo` ); then 
  CORES=`grep processor /proc/cpuinfo | wc -l`
fi
if [ "$?" != "0" ]; then
  CORES=1
fi
echo "Using this number of cores for compilation: ${CORES}"



echo "Compiling..."
make -j${CORES}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while compiling ROOT!"
  exit 1
fi



echo "Store our success story..."
rm -f COMPILE_SUCCESSFUL
echo "${CONFIGUREOPTIONS}" >> COMPILE_SUCCESSFUL
echo "${COMPILEROPTIONS}" >> COMPILE_SUCCESSFUL



echo "Setting permissions..."
cd ..
chown -R ${USER}:${GROUP} root_v${VER}
chmod -R go+rX root_v${VER}


if [ "${ENVFILE}" != "" ]; then
  echo "Storing the ROOT directory in the MEGAlib source script..."
  echo "ROOTDIR=`pwd`/root_v${VER}" >> ${ENVFILE}
fi


echo "Done!"
exit 0
