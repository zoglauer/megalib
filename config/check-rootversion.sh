#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.


# Allowed versions

confhelp() {
  echo ""
  echo "Check for a correct version of ROOT"
  echo " " 
  echo "Usage: ./check-root.sh [options]";
  echo " "
  echo " "
  echo "Options:"
  echo "--get-max"
  echo "    Return the allowed maximal ROOT version" 
  echo "--get-min"
  echo "    Return the allowed minimum ROOT version" 
  echo " "
  echo "--check=[path to ROOT]"
  echo "    Check the given path if it contains a good ROOT version." 
  echo " "
  echo "--good-version=[version string]"
  echo "    Check the given version string contains a good ROOT version."   
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

CHECK="false"
GET="false"
GOOD="false"
ROOTPATH=""
TESTVERSION=""

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-c*=* ]]; then
    ROOTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
    CHECK="true"
    GET="false"
    GOOD="false"
  elif [[ ${C} == *-get-ma* ]]; then
    ROOTPATH=""
    CHECK="false"
    GET="true"
    MAX="true"
    GOOD="false"
  elif [[ ${C} == *-get-mi* ]]; then
    ROOTPATH=""
    CHECK="false"
    GET="true"
    MAX="false"
    GOOD="false"
  elif [[ ${C} == *-go* ]]; then
    ROOTPATH=""
    CHECK="false"
    GET="false"
    MAX="false"
    GOOD="true"
    TESTVERSION=`echo ${C} | awk -F"=" '{ print $2 }'`
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

VERSIONS=`cat ${MEGALIB}/config/AllowedROOTVersions.txt` 
RootVersionMin=`echo ${VERSIONS} | awk -F" " '{ print $1 }'`
RootVersionMax=`echo ${VERSIONS} | awk -F" " '{ print $2 }'`

RootVersionMinString="${RootVersionMin:(-3):1}.${RootVersionMin:(-2):2}"
RootVersionMaxString="${RootVersionMax:(-3):1}.${RootVersionMax:(-2):2}"

if [ "${GET}" == "true" ]; then
  if [ "${MAX}" == "true" ]; then
    echo "${RootVersionMaxString}"
  else 
    echo "${RootVersionMinString}"
  fi
  exit 0;
fi

if [ "${GOOD}" == "true" ]; then
  version=`echo ${TESTVERSION} | awk -F. '{ print $1 }'`;
  release=`echo ${TESTVERSION} | awk -F/ '{ print $1 }' | awk -F. '{ print $2 }'| sed 's/0*//'`;
  RootVersion=$((100*${version} + ${release}))
  
  if ([ ${RootVersion} -ge ${RootVersionMin} ] && [ ${RootVersion} -le ${RootVersionMax} ]); then
    echo "Found a good ROOT version: ${TESTVERSION}"
    exit 0
  else
    echo ""
    echo "ERROR: ROOT version (${TESTVERSION}) is not acceptable"
    echo "       You require a version between ${RootVersionMinString} and ${RootVersionMaxString}"
    exit 1
  fi
fi  

if [ "${CHECK}" == "true" ]; then
  if [ ! -f ${ROOTPATH}/bin/root-config ]; then
    echo " "
    echo "ERROR: The given directory ${ROOTPATH} does no contain a correct ROOT installation"
    exit 1;
  fi

  rv=`${ROOTPATH}/bin/root-config --version`
  version=`echo $rv | awk -F. '{ print $1 }'`;
  release=`echo $rv | awk -F/ '{ print $1 }' | awk -F. '{ print $2 }'| sed 's/0*//'`;
  patch=`echo $rv | awk -F/ '{ print $2 }'| sed 's/0*//'`;
  RootVersion=$((100*${version} + ${release}))

  if ([ ${RootVersion} -ge ${RootVersionMin} ] && [ ${RootVersion} -le ${RootVersionMax} ]); then
    echo "SUCCESS"
    exit 0
  else
    echo ""
    echo "ERROR: No acceptable ROOT version found: ${RootVersion}"
    exit 1
  fi
fi

exit 1
