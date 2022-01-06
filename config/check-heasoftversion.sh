#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.


# Allowed versions

confhelp() {
  echo ""
  echo "Check for a correct version of HEASoft"
  echo " " 
  echo "Usage: ./check-HEASoft.sh [options]";
  echo " "
  echo " "
  echo "Options:"
  echo "--get-max"
  echo "    Return the allowed maximum HEASoft version" 
  echo "--get-min"
  echo "    Return the allowed minimum HEASoft version" 
  echo " "
  echo "--check=[path to HEASoft]"
  echo "    Check the given path if it contains a good HEASoft version." 
  echo " "
  echo "--good-version=[version string]"
  echo "    Check the given version string contains a good HEASoft version."   
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
HEASoftPATH=""
TESTVERSION=""

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-c*=* ]]; then
    HEASoftPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
    CHECK="true"
    GET="false"
    GOOD="false"
  elif [[ ${C} == *-get-ma* ]]; then
    HEASoftPATH=""
    CHECK="false"
    GET="true"
    MAX="true"
    GOOD="false"
  elif [[ ${C} == *-get-mi* ]]; then
    HEASoftPATH=""
    CHECK="false"
    GET="true"
    MAX="false"
    GOOD="false"
  elif [[ ${C} == *-go* ]]; then
    HEASoftPATH=""
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


VERSIONS=`cat ${MEGALIB}/config/AllowedHEASoftVersions.txt` 
HEASoftVersionMin=`echo ${VERSIONS} | awk -F" " '{ print $1 }'`
HEASoftVersionMax=`echo ${VERSIONS} | awk -F" " '{ print $NF }'`

HEASoftVersionMinString="${HEASoftVersionMin:(-3):1}.${HEASoftVersionMin:(-2):2}"
HEASoftVersionMaxString="${HEASoftVersionMax:(-3):1}.${HEASoftVersionMax:(-2):2}"

if [ "${GET}" == "true" ]; then
  if [ "${MAX}" == "true" ]; then
    echo "${HEASoftVersionMaxString}"
  else 
    echo "${HEASoftVersionMinString}"
  fi
  exit 0;
fi


if [ "${GOOD}" == "true" ]; then
  version=`echo ${TESTVERSION} | awk -F. '{ print $1 }'`;
  release=`echo ${TESTVERSION} | awk -F. '{ print $2 }'`;
  HEASoftVersion=$((100*${version} + ${release}))
  
  if ([ ${HEASoftVersion} -ge ${HEASoftVersionMin} ] && [ ${HEASoftVersion} -le ${HEASoftVersionMax} ]); then
    echo "Found a good HEASoft version: ${TESTVERSION}"
    exit 0
  else
    echo ""
    echo "ERROR: HEASoft version (${TESTVERSION}) is not acceptable"
    echo "       You require a version between ${HEASoftVersionMinString} and ${HEASoftVersionMaxString}"
    exit 1
  fi
fi  

if [ "${CHECK}" == "true" ]; then
  if (`test -f ${HEASoftPATH}/bin/ftversion`); then
    rv=$(${HEASoftPATH}/bin/ftversion | awk -F"V" '{ print $2 }'  | sed 's/[^0-9.]*//g'); 
    version=`echo ${rv} | awk -F. '{ print $1 }'`;
    release=`echo ${rv} | awk -F. '{ print $2 }'`;
    HEASoftVersion=$((100*${version} + ${release}))
  else
    echo " "
    echo "ERROR: The given directory ${HEASoftPATH} does no contain a correct HEASoft installation"
    exit 1;
  fi

  if ([ ${HEASoftVersion} -ge ${HEASoftVersionMin} ] && [ ${HEASoftVersion} -le ${HEASoftVersionMax} ]); then
    echo "The given HEASoft version ${rv} is acceptable"
    exit 0;
  else
    echo ""
    echo "ERROR: No acceptable HEASoft version found: ${HEASoftVersion} (min: ${HEASoftVersionMinString}, max: ${HEASoftVersionMaxString})"
    exit 1
  fi
fi

exit 1
