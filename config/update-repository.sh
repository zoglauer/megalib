#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.


confhelp() {
  echo ""
  echo "Update a repository with new files from another repository"
  echo " " 
  echo "Usage: ./update-repository.sh -r='repository' -n='repository with newer files'";
  echo " "
  echo " "
  echo "Options:"
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

GITREPO=""
NEWREPO=""

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-r*=* ]]; then
    GITREPO=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo "Using this repository for upgrade: ${GITREPO}"
  elif [[ ${C} == *-n* ]]; then
    NEWREPO=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo "Using this repository with newer files: ${NEWREPO}"
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

if [ "${GITREPO}" == "" ]; then
    echo ""
    echo "ERROR: You need to give a git repository."
    exit 1
fi

if [ ! -d ${GITREPO} ]; then
    echo ""
    echo "ERROR: The given git repository does not exist: \"${GITREPO}\""
    exit 1
fi

if [ "${NEWREPO}" == "" ]; then
    echo ""
    echo "ERROR: You need to givea repository containing the latest files."
    exit 1
fi

if [ ! -d ${NEWREPO} ]; then
    echo ""
    echo "ERROR: The repository with the newer files does not exist: \"${NEWREPO}\""
    exit 1
fi

EXCLUDELIST=".svn .git CVS include lib *~ *.sim *.tra *gz"

HERE=`pwd`
cd ${GITREPO}

FIND="find ."
for e in ${EXCLUDELIST}; do
  FIND="${FIND} -name ${e} -prune -o "
done
FILES=`${FIND} -type f -name "*" -printf "%P "`

for f in ${FILES}; do
  if [ ! -f ${NEWREPO}/${f} ]; then
    #echo "File no longer exists: ${NEWREPO}/${f}"
    continue;
  fi 
  DIFF=`diff -q ${GITREPO}/${f} ${NEWREPO}/${f}`
  if [ "${DIFF}" != "" ]; then
    echo "Not identical and copying: ${GITREPO}/${f}"  
    cp ${NEWREPO}/${f} ${GITREPO}/${f}
  #else
  #  echo "Identical: ${GITREPO}/${f}"
  fi
done






echo "Done!"
exit 0
