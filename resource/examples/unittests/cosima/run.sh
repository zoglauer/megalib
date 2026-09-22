#!/bin/bash

# run.sh
#
# Copyright (C) by the MEGAlib contributors.
#
# This file is part of MEGAlib.
#
# MEGAlib is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# MEGAlib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License (License.md) for more details.
#
# SPDX-License-Identifier: LGPL-3.0-or-later


# The minimum MEGAlib version when starting comparison
MINMEGALIBVERSION=29903

# First run all sims for ~2 minutes

# Get all the source files
HERE=$(pwd)
cd ${MEGALIB}/resource/examples/cosima/source
FILES=$(ls *.source)
cd ${HERE}

# Exclude some
FILES=${FILES//ActivationStep2.source}
FILES=${FILES//ActivationStep3.source}
FILES=${FILES//BlackAbsorber.source}
FILES=${FILES//EnergyResolutionTester.source}
FILES=${FILES//Run.source}
FILES=${FILES//UseCase1.source}


# Megalib version ID
MegalibVersion=`cat ${MEGALIB}/config/Version.txt  | sed 's|\.||g'`

# ROOT version ID
if [ -f ${ROOTSYS}/bin/root-config ]; then
  rv=`${ROOTSYS}/bin/root-config --version`
  version=`echo $rv | awk -F. '{ print $1 }'`;
  release=`echo $rv | awk -F/ '{ print $1 }' | awk -F. '{ print $2 }'| sed 's/0*//'`;
  patch=`echo $rv | awk -F/ '{ print $2 }'| sed 's/0*//'`;
  RootVersion=$((10000*${version} + 100*${release} + ${patch}))
else
  echo " "
  echo "ERROR: No correct ROOT installation found"
  exit 1;
fi

# Geant4 version ID
if [ -f ${G4LIB}/../../bin/geant4-config ]; then
  Geant4Version=`${G4LIB}/../../bin/geant4-config --version | sed 's|\.||g'`
else
  echo " "
  echo "ERROR: No correct Geant4 installation found"
  exit 1;
fi

# The version tag
TAG=m${MegalibVersion}.r${RootVersion}.g${Geant4Version} 


# Simulate
for F in ${FILES}; do
  mwait --i=cores --p=cosima
  echo "Starting simulation of ${F}"
  timeout 180s cosima -s 1 ${MEGALIB}/resource/examples/cosima/source/${F} > /dev/null &
done
wait


for F in $(ls *.sim); do 
  echo "Analyzing ${F}..."
  OUT=${F//.inc1.id1.sim}.${TAG}.sha256sum
  EXISTING=$(ls ${F//.inc1.id1.sim}.m*.sha256sum)
  FINISHED=$(tail -n 10 ${F} | grep "^TS")
  if [[ ${FINISHED} == "" ]]; then
    head -n 100000 ${F} | grep -v -E "^Geometry|^MEGAlib|^Date|^#" | sha256sum > ${OUT}
  else 
    cat ${F} | grep -v -E "^Geometry|^MEGAlib|^Date|^#" | sha256sum > ${OUT}
  fi
  for E in ${EXISTING}; do
    if [[ ${E} == ${OUT} ]]; then continue; fi;
    VERSION=$(echo ${E} | awk -F"." '{print $(NF-3)}') # we cannot count rom the beginnig but must count from the end
    VERSION=${VERSION#m}
    if [ "${VERSION}" -ge "${MINMEGALIBVERSION}" ]; then
      DIFF=$(diff ${OUT} ${E})
      if [[ ${DIFF} == "" ]]; then
        echo "Comparing ${OUT} with ${E} --> identical"
      else
        echo "Comparing ${OUT} with ${E} --> NOT identical"
      fi
    fi
  done
done

exit;


