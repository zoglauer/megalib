#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.


# Part 1:
# Helper functions

TIMESTART=$(date +%s)

confhelp() {
  echo ""
  echo "Setup script for MEGAlib"
  echo " "
  echo "This script downloads MEGAlib, downloads and builds ROOT and Geant4, compiles MEGAlib, "
  echo "and finally creates an environment script for MEGAlib"
  echo "This script can be used for the initial installation of MEGAlib as well as for its update!"
  echo " "
  echo "Usage: ./setup.sh [options - all are optional!]";
  echo " "
  echo " "
  echo "Options for all paths and programs:"
  echo "--megalib-path=[path to MEGAlib - default: \"MEGAlib\"]"
  echo "    This is the path to where MEGAlib is installed. If the path exists, we will try to update MEGAlib."
  echo "    MEGAlib must have been checked out from one of the Repositories or we cannot use it!"
  echo " "
  echo "--repository=[git or svn - default: git]"
  echo "    The repository from where we are retrieving MEGAlib. Always use git unless you know the svn password :)"
  echo " "
  echo "--release=[rel or dev]"
  echo "    Choose between release or development version."
  echo " "
  echo "--keep-megalib-as-is"
  echo "    Do not update the MEGAlib from a repository, just recompile it."
  echo "    You need to set \"--megalib-path\"!"
  echo " "
  echo "--external-path=[path - default: \"directory given by --megalib-path\"\external]"
  echo "    Directory where to install the required version of ROOT and Geant4."
  echo " "
  echo "--root=[path to existing ROOT installation]"
  echo "    Instead of installing a new version of ROOT use this one. If it is not compatible with MEGAlib, the script will stop with an error."
  echo " "
  echo "--geant=[path to existing GEANT4 installation]"
  echo "    Instead of installing a new version of GEANT4 use this one. If it is not compatible with MEGAlib, the script will stop with an error."
  echo " "
  echo "--maxthreads=[integer >=1]"
  echo "    The maximum number of threads to be used for compilation. Default is the number of cores in your system."
  echo " "
  echo "--patch=[yes or no (default no)]"
  echo "    Apply MEGAlib internal (!) ROOT or Geant4 patches, if there are any."
  echo " "
  echo "--debug=[off/no, on/yes]"
  echo "    Debugging options for ROOT, Geant4 & MEGAlib (Default is on for MEGAlib, but off for ROOT and Geant4)"
  echo " "
  echo "--cleanup=[off/no, on/yes (default: off)]"
  echo "    Remove intermediate build files"
  echo " "
  echo "--optimization=[off/no, normal/on/yes, strong/hard (requires gcc 4.2 or higher)]"
  echo "    Compilation optimization for MEGAlib ONLY (Default is normal)"
  echo " "
  echo "--updates=[off/no, on/yes]"
  echo "    Check periodically for updates. Default is off."
  echo "    Even if set to on, update checks will only be performed, if the user has write access to the MEGAlib installation."
  echo " "
  echo "General options:"
  echo "--help or -h"
  echo "    Show this help."
  echo " "
  echo " "
}

issuereport() {
  echo " "
  echo "       Please take a look if you find the issue here (look at open and closed issues):"
  echo "       https://github.com/zoglauer/megalib/issues"
  echo "       If not add your problem and write me an email!"
  echo " "
}

absolutefilename() {
  echo "$(cd "$(dirname "$1")" && pwd)/$(basename "$1")"
}

# Part 2:
# A first round of sanity checks

if [ "$SHELL" != "/bin/bash" ]; then
  echo " "
  echo "Info: This is a bash script. Thus you need to use bash for it and not: ${SHELL}"
  echo " "
fi

if [[ $EUID -eq 0 ]]; then
  echo " " 1>&2
  echo "Error: For security reasons, this script is not intended to be run as superuser/root." 1>&2
  echo "       It is intended for a user installation of MEGAlib." 1>&2
  echo " " 1>&2
  exit 1
fi



# Part 3:
# Upgrade the input options:

# Store command line as array
CMD=( "$@" )

# Check for help
for C in "${CMD[@]}"; do
  if [[ ${C} == *-h* ]]; then
    echo ""
    confhelp
    exit 0
  fi
done


echo ""
echo ""
echo ""
echo "Setting up MEGAlib on your system"
echo "=================================================="
echo ""
echo " "
echo "(1) Verifying chosen configuration options:"
echo " "


# Default options:
MEGALIBPATH=""
KEEPMEGALIBASIS="off"
REPOSITORY="git"
RELEASE="rel"
EXTERNALPATH=""
ROOTPATH=""
GEANT4PATH=""
OS=`uname -s`
OPT="normal"
DEBUG="off"
UPDATES="off"
PATCH="on"
CLEANUP="off"
BRANCH=""

MAXTHREADS=1;
if ( `test -f /usr/sbin/sysctl` ); then
  MAXTHREADS=`sysctl -n hw.logicalcpu_max`
elif ( `test -f /proc/cpuinfo` ); then
  MAXTHREADS=`grep processor /proc/cpuinfo | wc -l`
fi
if [ "$?" != "0" ]; then
  MAXTHREADS=1
fi



# Prelude - Find an old configuration
for C in "${CMD[@]}"; do
  if [[ ${C} == *-me*=* ]]; then
    MEGALIBPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  fi
done
# If we have a path make sure it contains a MEGAlib installation
if [ "${MEGALIBPATH}" != "" ]; then
  if [ ! -f ${MEGALIBPATH}/src/global/misc/src/MGlobal.cxx ]; then
    echo ""
    echo "ERROR: You have given a MEGAlib path (\"${MEGALIBPATH}\") to a non-empty directory which doesn't contain MEGAlib!"
    echo "       Either give a not existing directory as MEGAlib path, or the path to an existing MEGAlib installation"
    exit 1
  fi
fi
# If we don't have one check if we are in a MEGAlib directory, or use the default "MEGAlib" path
if [ "${MEGALIBPATH}" == "" ]; then
  if [ -f src/global/misc/src/MGlobal.cxx ]; then
    MEGALIBPATH="."
  else
    MEGALIBPATH="MEGAlib"
  fi
fi

if [ -f "${MEGALIBPATH}/config/SetupOptions.txt" ]; then
  echo " * Loading old options as default --- they will be overwritten by your command line options!"
  OLDCMD=`cat ${MEGALIBPATH}/config/SetupOptions.txt`
  CMD=( ${OLDCMD} ${CMD[@]}  )
fi


# Overwrite default options with user options:
for C in "${CMD[@]}"; do
  if [[ ${C} == *-me*=* ]]; then
    MEGALIBPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-rep*=* ]]; then
    REPOSITORY=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-rel*=* ]]; then
    RELEASE=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-e*=* ]]; then
    EXTERNALPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-ro*=* ]]; then
    ROOTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-g*=* ]]; then
    GEANT4PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-o*=* ]]; then
    OPT=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-d*=* ]]; then
    DEBUG=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-u*=* ]]; then
    UPDATES=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-co*=* ]]; then
    COMP=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-cl*=* ]]; then
    CLEANUP=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-p*=* ]]; then
    PATCH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-ma*=* ]]; then
    MAXTHREADS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-k* ]]; then
    KEEPMEGALIBASIS="on"
  elif [[ ${C} == *-b* ]]; then
    BRANCH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-h* ]]; then
    echo ""
    confhelp
    exit 0
  else
    echo ""
    echo "ERROR: Unknown command line option: ${C}"
    echo "       See \"./configure --help\" for a list of options"
    exit 1
  fi
done


# Everything to lower case:
REPOSITORY=`echo ${REPOSITORY} | tr '[:upper:]' '[:lower:]'`
RELEASE=`echo ${RELEASE} | tr '[:upper:]' '[:lower:]'`
OS=`echo ${OS} | tr '[:upper:]' '[:lower:]'`
OPT=`echo ${OPT} | tr '[:upper:]' '[:lower:]'`
DEBUG=`echo ${DEBUG} | tr '[:upper:]' '[:lower:]'`
COMP=`echo ${COMP} | tr '[:upper:]' '[:lower:]'`
UPDATES=`echo ${UPDATES} | tr '[:upper:]' '[:lower:]'`
PATCH=`echo ${PATCH} | tr '[:upper:]' '[:lower:]'`
CLEANUP=`echo ${CLEANUP} | tr '[:upper:]' '[:lower:]'`


# Provide feed back and perform error checks:

HERE=$(pwd)
if [[ "${HERE}" != "${HERE% *}" ]]; then
  echo "ERROR: The installation directory needs to be a path without spaces,"
  echo "       but you chose: \"${HERE}\""
  exit 1
fi

if [ "${MEGALIBPATH}" != "" ]; then
  MEGALIBPATH=`absolutefilename ${MEGALIBPATH}`
fi
if [[ "${MEGALIBPATH}" != "${MEGALIBPATH% *}" ]]; then
  echo "ERROR: MEGAlib needs to be installed in a path without spaces,"
  echo "       but you chose: \"${MEGALIBPATH}\""
  exit 1
fi
echo " * Using this path to MEGAlib: ${MEGALIBPATH}"

if [ "${KEEPMEGALIBASIS}" == "on" ]; then
  echo " * Keeping the existing MEGAlib installation, just recompile."
else
  echo " * Updating MEGAlib to the latest version"
fi

if [ "${EXTERNALPATH}" != "" ]; then
  EXTERNALPATH=`absolutefilename ${EXTERNALPATH}`
fi
if [[ "${EXTERNALPATH}" != "${EXTERNALPATH% *}" ]]; then
  echo "ERROR: The external software needs to be installed in a path without spaces,"
  echo "       but you chose: \"${EXTERNALPATH}\""
  exit 1
fi
echo " * Using this path to install external software (ROOT, Geant4): ${EXTERNALPATH}"

if [ "${ROOTPATH}" != "" ]; then
  ROOTPATH=`absolutefilename ${ROOTPATH}`
fi
if [[ "${ROOTPATH}" != "${ROOTPATH% *}" ]]; then
  echo "ERROR: ROOT needs to be installed in a path without spaces,"
  echo "       but you chose: \"${ROOTPATH}\""
  exit 1
fi
if [ "${ROOTPATH}" == "" ]; then
  echo " * Download latest compatible version of ROOT"
else
  echo " * Using the installation of ROOT: ${ROOTPATH}"
fi

if [ "${GEANT4PATH}" != "" ]; then
  GEANT4PATH=`absolutefilename ${GEANT4PATH}`
fi
if [[ "${GEANT4PATH}" != "${GEANT4PATH% *}" ]]; then
  echo "ERROR: Geant4 needs to be installed in a path without spaces,"
  echo "       but you chose: \"${GEANT4PATH}\""
  exit 1
fi
if [ "${GEANT4PATH}" == "" ]; then
  echo " * Download latest compatible version of Geant4"
else
  echo " * Using the installation of Geant4: ${GEANT4PATH}"
fi

if [[ ${REPOSITORY} == s* ]]; then
  REPOSITORY="svn"
  echo " * Using the svn repository"
  type svn >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo " "
    echo "ERROR: svn needs to be installed if you want to use the svn repository"
    exit 1
  fi
elif [[ ${REPOSITORY} == g* ]]; then
  REPOSITORY="git"
  echo " * Using the git repository"
  type git >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo " "
    echo "ERROR: git needs to be installed if you want to use the git repository"
    exit 1
  fi
elif [[ ${REPOSITORY} == c* ]]; then
  REPOSITORY="cvs"
  echo " * Using the cvs repository"
  type cvs >/dev/null 2>&1
  if [ $? -ne 0 ]; then
    echo " "
    echo "ERROR: cvs needs to be installed if you want to use the cvs repository"
    exit 1
  fi
else
  echo " "
  echo "ERROR: Unsupported repository: ${REPOSITORY}"
  confhelp
  exit 1
fi

if [[ ${RELEASE} == r* ]]; then
  RELEASE="rel"
  echo " * Using latest release version"
  if [[ ${BRANCH} != "" ]]; then
    echo "   Ignoring request for branch ${BRANCH} since you requested the release version."
    echo "   Please use --rel=dev if you want to use a specific MEGAlib development branch"
    BRANCH=""
  fi
elif [[ ${RELEASE} == d* ]]; then
  RELEASE="dev"
  echo " * Using latest development version"
else
  echo " "
  echo "ERROR: Unknown MEGAlib version: ${RELEASE}"
  confhelp
  exit 1
fi

if [[ ${BRANCH} != "" ]]; then
  if [[ ${REPOSITORY} == git ]]; then
    echo " * Using MEGAlib git branch ${BRANCH}"
  else
    echo "ERROR: You can only select a branch when using the git repository."
    exit 1
  fi
fi

if [[ ${OS} == l* ]]; then
  OS="linux"
  echo " * Using operating system architecture Linux"
elif ( [[ ${OS} == d* ]] || [[ ${OS} == m* ]] ); then
  OS="darwin"
  echo " * Using operating system architecture Darwin (Mac OS X)"
else
  echo " "
  echo "ERROR: Unknown operating system architecture: ${OS}"
  confhelp
  exit 1
fi

if ( [[ ${OPT} == of* ]] || [[ ${OPT} == no ]] ); then
  OPT="off"
  echo " * Using no code optimization"
elif ( [[ ${OPT} == nor* ]] || [[ ${OPT} == on ]] || [[ ${OPT} == y* ]] ); then
  OPT="normal"
  echo " * Using normal code optimization"
elif ( [[ ${OPT} == s* ]] || [[ ${OPT} == h* ]] ); then
  OPT="strong"
  echo " * Using strong code optimization"
else
  echo " "
  echo "ERROR: Unknown code optimization: ${OPT}"
  confhelp
  exit 1
fi


if ( [[ ${DEBUG} == of* ]] || [[ ${DEBUG} == no ]] ); then
  DEBUG="off"
  echo " * Using no debugging code"
elif ( [[ ${DEBUG} == on ]] || [[ ${DEBUG} == y* ]] || [[ ${DEBUG} == nor* ]] ); then
  DEBUG="normal"
  echo " * Using debugging code"
elif ( [[ ${DEBUG} == st* ]] || [[ ${DEBUG} == h* ]] ); then
  DEBUG="strong"
  echo " * Using more debug flags"
else
  echo " "
  echo "ERROR: Unknown debugging code selection: ${DEBUG}"
  confhelp
  exit 1
fi


if [ ! -z "${MAXTHREADS##[0-9]*}" ] 2>/dev/null; then
  echo "ERROR: The maximum number of threads must be a number and not ${MAXTHREADS}!"
  exit 1
fi
if [ "${MAXTHREADS}" -le "0" ]; then
  echo "ERROR: The maximum number of threads must be at least 1 and not ${MAXTHREADS}!"
  exit 1
else
  echo " * Using this maximum number of threads: ${MAXTHREADS}"
fi


if ( [[ ${UPDATES} == of* ]] || [[ ${UPDATES} == n* ]] ); then
  UPDATES="off"
  echo " * Don't check for updates"
elif ( [[ ${UPDATES} == on ]] || [[ ${UPDATES} == y* ]] ); then
  UPDATES="on"
  echo " * Check for updates if the user has write access to the MEGAlib installation"
else
  echo " "
  echo "ERROR: Unknown option for updates: ${UPDATES}"
  confhelp
  exit 1
fi


if ( [[ ${PATCH} == of* ]] || [[ ${PATCH} == n* ]] ); then
  PATCH="off"
  echo " * Don't apply MEGAlib internal ROOT and Geant4 patches"
elif ( [[ ${PATCH} == on ]] || [[ ${PATCH} == y* ]] ); then
  PATCH="on"
  echo " * Apply MEGAlib internal ROOT and Geant4 patches"
else
  echo " "
  echo "ERROR: Unknown option for patch: ${PATCH}"
  confhelp
  exit 1
fi


if ( [[ ${CLEANUP} == of* ]] || [[ ${CLEANUP} == n* ]] ); then
  CLEANUP="off"
  echo " * Don't clean up intermediate build files"
elif ( [[ ${CLEANUP} == on ]] || [[ ${CLEANUP} == y* ]] ); then
  CLEANUP="on"
  echo " * Clean up intermediate build files"
else
  echo " "
  echo "ERROR: Unknown option for clean up: ${CLEANUP}"
  confhelp
  exit 1
fi




echo " "
echo "(2) Preparing the MEGAlib source code:"
echo " "

STARTPATH=`pwd`
UPDATEMODE="NO"

if [ -d $MEGALIBPATH ]; then
  # MEGAlib does exist - update it

  cd ${MEGALIBPATH}
  MEGALIBPATH=`pwd` # For later - in case this was relative

  if [ ! -f src/global/misc/src/MGlobal.cxx ]; then
    echo " "
    echo "ERROR: The directory given in your MEGAlib path (\"${MEGALIBPATH}\") is not a MEGAlib directory."
    exit 1
  fi

  if [ "${KEEPMEGALIBASIS}" == "off" ]; then
    OLDREPOSITORY="none"
    if [ -d CVS ]; then
      OLDREPOSITORY="cvs"
    elif [ -d .svn ]; then
      OLDREPOSITORY="svn"
    elif [ -d .git ]; then
      OLDREPOSITORY="git"
    else
      echo " "
      echo "ERROR: We have a MEGAlib directory at \"${MEGALIBPATH}\" but it has not been checked out from any repository."
      exit 1
    fi

    if  [ "${OLDREPOSITORY}" != "${REPOSITORY}" ]; then
      echo " "
      echo "ERROR: You want to update an existing version of MEGAlib checked out from ${OLDREPOSITORY} with ${REPOSITORY}... an impossible task..."
      exit 1
    fi

    echo "Making a backup of your old source code - just in case..."
    BACKUP=SourceCodeBackupDuringMEGAlibSetup.`date +'%d%m%y.%H%M%S'`.tgz
    tar cfz ${BACKUP} src
    if [ "$?" != "0" ]; then
      echo " "
      echo "ERROR: Something went wrong during backing up your source code."
      exit 1
    fi
    if [ ! -d "backup" ]; then
      mkdir backup
    fi
    mv ${BACKUP} backup


    if [ "${REPOSITORY}" == "svn" ]; then
      if [ "${RELEASE}" == "dev" ]; then
        echo "Switching to latest development version of MEGAlib in the svn repository..."
        svn switch svn://thetis/MEGAlibRepository/MEGAlib/trunk
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to switch to the latest development version in svn"
          exit 1
        fi
        svn update
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to update the svn repository"
          exit 1
        fi
      else
        echo "Switching to latest release version of MEGAlib in the svn repository......"
        # Find the branch with the highest version, switch and to it
        Branch=`svn list svn://thetis/MEGAlibRepository/MEGAlib/branches | grep MEGAlib_v | sort -n | tail -n 1`
        # and switch to this branch:
        svn switch svn://thetis/MEGAlibRepository/MEGAlib/branches/${Branch}
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to switch to the latest release branch in svn"
          exit 1
        fi
        svn update
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to the svn repository"
          exit 1
        fi
      fi
    elif [ "${REPOSITORY}" == "cvs" ]; then
      if [ "${RELEASE}" == "dev" ]; then
        echo "Switching to latest development version of MEGAlib in the cvs repository..."
        cvs -d :pserver:anonymous@cvs.mpe.mpg.de:/home/zoglauer/Repository update -A -d
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to switch to the latest development version in cvs"
          exit 1
        fi
      else
        echo "Switching to latest release version of MEGAlib in the cvs repository......"
        cd ${MEGALIBPATH}
        Branch=`cvs status -v config/Version.txt | grep MEGAlib_  | awk -F" " '{ print $1 }' | sort -n | tail -n 1`
        cvs update -r ${Branch} -d
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to update the cvs repository to the latest release branch"
          exit 1
        fi
        cd ${STARTPATH}
      fi
    elif [ "${REPOSITORY}" == "git" ]; then
      cd ${MEGALIBPATH}

      echo "Getting all the latest changes from the repository..."
      git fetch origin
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to fetch the latest versions from the repository"
        exit 1
      fi

      CurrentBranch=`git rev-parse --abbrev-ref HEAD`
      echo "Current branch: ${CurrentBranch}"

      if [ "${BRANCH}" == "" ]; then
        if [ "${RELEASE}" == "dev" ]; then
          if [ "${CurrentBranch}" != "master" ]; then
            echo "Switching to the latest development version of MEGAlib in the git repository..."
            git checkout master
            if [ "$?" != "0" ]; then
              echo " "
              echo "ERROR: Unable to switch to the latest development version in git"
              exit 1
            fi
          fi
        else
          Branch=`git ls-remote --heads git://github.com/zoglauer/megalib.git | grep MEGAlib_v | awk -F"refs/heads/" '{ print $2 }' | sort -n | tail -n 1`
          if [ "${CurrentBranch}" != "${Branch}" ]; then
            echo "Switching to latest release version of MEGAlib from the git repository..."
            git checkout ${Branch}
            if [ "$?" != "0" ]; then
              echo " "
              echo "ERROR: Unable to update the git repository to the latest release branch"
              exit 1
            fi
          fi
        fi
      else
        echo "Switching to user selected branch ${BRANCH}..."
        git checkout ${BRANCH}
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to switch to user selected branch ${BRANCH}..."
          exit 1
        fi
      fi

      echo "Fast forwarding to the head"
      git pull origin
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to fast forward to the head"
        exit 1
      fi

      cd ${STARTPATH}
    fi
    if [ "$?" != "0" ]; then
      echo " "
      echo "ERROR: Something went wrong during updating the repository."
      exit 1
    fi

    echo " "
    echo "SUCCESS: Updated MEGAlib!"
  else
    echo " "
    echo "SUCCESS: Keeping MEGAlib as is!"
  fi
else
  # MEGAlib does not exist - download it

  if [ "${KEEPMEGALIBASIS}" == "on" ]; then
    echo " "
    echo "ERROR: You wanted to keep the MEGAlib installation as is, but you do need one, i.e. you do need to give its path"
    exit 1
  fi

  if [ "${REPOSITORY}" == "svn" ]; then
    echo "Using svn to checkout MEGAlib..."
    if [ "${RELEASE}" == "dev" ]; then
      echo "Checking out latest development version of MEGAlib from the svn repository..."
      svn co svn://thetis/MEGAlibRepository/MEGAlib/trunk ${MEGALIBPATH}
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to checkout the latest development version from svn"
        exit 1
      fi
    else
      echo "Checking out latest release version of MEGAlib from the svn repository..."
      # Find the branch with the highest version, switch and to it
      Branch=`svn list svn://thetis/MEGAlibRepository/MEGAlib/branches | grep MEGAlib_v | sort -n | tail -n 1`
      # and switch to this branch:
      svn co svn://thetis/MEGAlibRepository/MEGAlib/branches/${Branch} ${MEGALIBPATH}
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to checkout the latest release branch from svn"
        exit 1
      fi
    fi
  elif [ "${REPOSITORY}" == "cvs" ]; then
    echo "Using cvs to checkout MEGAlib..."
    if [ "${RELEASE}" == "dev" ]; then
      echo "Checking out latest development version of MEGAlib from the cvs repository..."
      cvs -d :pserver:anonymous@cvs.mpe.mpg.de:/home/zoglauer/Repository -z3 co -P -d ${MEGALIBPATH} MEGAlib
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to checkout the latest development version from csv"
        exit 1
      fi
    else
      echo "Checking out latest release version of MEGAlib from the svn repository..."
      cvs -d :pserver:anonymous@cvs.mpe.mpg.de:/home/zoglauer/Repository -z3 co -P -d ${MEGALIBPATH} MEGAlib
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to checkout the latest development version from csv"
        exit 1
      fi
      cd ${MEGALIBPATH}
      Branch=`cvs status -v config/Version.txt | grep MEGAlib_  | awk -F" " '{ print $1 }' | sort -n | tail -n 1`
      cvs update -r ${Branch} -d
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to update the cvs repository to the latest release branch"
        exit 1
      fi
      cd ${STARTPATH}
    fi
  elif [ "${REPOSITORY}" == "git" ]; then
    echo "Using git to clone the MEGAlib directory..."
    git clone https://github.com/zoglauer/megalib.git ${MEGALIBPATH}
    if [ "$?" != "0" ]; then
      echo " "
      echo "ERROR: Unable to checkout the latest development version from git"
      exit 1
    fi
    if [[ ${BRANCH} == "" ]]; then
      if [ "${RELEASE}" == "rel" ]; then
        echo "Switching to latest release version of MEGAlib from the git repository..."
        Branch=`git ls-remote --heads git://github.com/zoglauer/megalib.git | grep MEGAlib_v | awk -F"refs/heads/" '{ print $2 }' | sort -n | tail -n 1`
        if ( [ "$?" != "0" ] || [ "${Branch}" == "" ] ); then
          echo " "
          echo "ERROR: Unable to find the latest release branch"
          exit 1
        fi
        cd ${MEGALIBPATH}
        git checkout ${Branch}
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to update the git repository to the latest release branch"
          exit 1
        fi

        cd ${STARTPATH}
      fi
    else
      echo "Switching to branch ${BRANCH}..."
      cd ${MEGALIBPATH}
      git checkout ${BRANCH}
      if [ "$?" != "0" ]; then
        echo " "
        echo "ERROR: Unable to update the git repository to branch ${BRANCH}"
        exit 1
      fi

      cd ${STARTPATH}
    fi
  else
    echo " "
    echo "ERROR: Unknown repository: ${REPOSITORY}"
    exit 1;
  fi
  if [ "$?" != "0" ]; then
    echo " "
    echo "ERROR: Something went wrong during MEGAlib checkout."
    exit 1
  fi

  cd ${MEGALIBPATH}
  MEGALIBPATH=`pwd` # For later - in case this was relative

  if [ ! -f src/global/misc/src/MGlobal.cxx ]; then
    echo " "
    echo "ERROR: The directory given in your MEGAlib path (\"${MEGALIBPATH}\") is not a MEGAlib directory."
    exit 1
  fi
  cd ${HERE}

  echo " "
  echo "SUCCESS: Checked out MEGAlib from its repository!"
fi


# Prepare the environment script
ENVFILE="${MEGALIBPATH}/new-source-megalib.sh"
rm -f ${ENVFILE}
echo "#/bin/bash" >> ${ENVFILE}
echo " " >> ${ENVFILE}
echo "# You can call this file everytime you want to work with MEGAlib via" >> ${ENVFILE}
echo "# source source-megalib.sh" >> ${ENVFILE}
echo "# or put the following line into your .bashrc file" >> ${ENVFILE}
echo "# . ${MEGALIBPATH}/bin/source-megalib.sh" >> ${ENVFILE}
echo " " >> ${ENVFILE}
echo "MEGALIBDIR=${MEGALIBPATH}" >> ${ENVFILE}


# Do a compiler test
COMPILER="g++"
if [[ ${OS} == linux ]]; then
  COMPILER="g++"
elif [[ ${OS} == macosx ]]; then
  COMPILER="c++"
fi

bash ${MEGALIBPATH}/config/configure_compilertest ${COMPILER}
if [ "$?" != "0" ]; then
  exit 1
fi




echo " "
echo "(3) Downloading and building ROOT:"
echo " "



MEGALIBDIR=${MEGALIBPATH}
export MEGALIB=${MEGALIBDIR}

#echo "ATTENTION: HARD CODED MEGALIB PATH SINCE THE LATEST VERSION IS NOT YET CHECKED IN!!!!"
#MEGALIBDIR="/home/andreas/Home/Science/Software/MEGAlib"

if [ "${EXTERNALPATH}" == "" ]; then EXTERNALPATH=${MEGALIBPATH}/external; fi
EXTERNALPATH=$(cd $(dirname ${EXTERNALPATH}); pwd)/$(basename ${EXTERNALPATH})

cd ${STARTPATH}
if [ "${ROOTPATH}" != "" ]; then
  # Check if we can use the given ROOT version

  bash ${MEGALIBDIR}/config/check-rootversion.sh --check=${ROOTPATH}
  RESULT=$?
  if [ "${RESULT}" != "0" ]; then
    if [ "${RESULT}" == "127" ]; then
      echo " "
      echo "ERROR: Cannot find check-rootversion.sh. Either your MEGAlib version is too old or corrupt..."
      exit 1
    else
      echo " "
      echo "ERROR: The directory ${ROOTPATH} cannot be used as your ROOT version for MEGAlib."
      exit 1
    fi
  fi
  echo "ROOTDIR=$(cd $(dirname ${ROOTPATH}); pwd)/$(basename ${ROOTPATH})" >> ${ENVFILE}
else
  # Download and build a new ROOT version
  if [ ! -d ${EXTERNALPATH} ]; then
    mkdir ${EXTERNALPATH}
  fi
  cd ${EXTERNALPATH}
  echo "Switching to build-root.sh script..."
  bash ${MEGALIBDIR}/config/build-root.sh -e=${ENVFILE} -p=${PATCH} --debug=${DEBUG} --maxthreads=${MAXTHREADS} --cleanup=${CLEANUP} | tee RootBuildLog.txt
  RESULT=${PIPESTATUS[0]}

  # If we have a new ROOT dir, copy the build log there
  NEWROOT4DIR=`grep ROOTDIR\= ${ENVFILE} | awk -F= '{ print $2 }'`
  if [[ -d ${NEWROOT4DIR} ]]; then
    mv RootBuildLog.txt ${NEWROOT4DIR}
  fi

  # Now handle build errors
  if [ "${RESULT}" != "0" ]; then
    if [ "${RESULT}" == "127" ]; then
      echo " "
      echo "ERROR: Cannot find build-root.sh. Either your MEGAlib version is too old or corrupt..."
      exit 1
    else
      echo " "
      echo "ERROR: Something went wrong during the ROOT setup."
      # The check currently happens in configure_compilertest
      #if [[ ${OS} == darwin ]]; then
      #  echo " "
      #  echo "       Since you are on a Mac, the most common problem is that you have not installed/updated the "
      #  echo "       Xcode command-line tools correctly."
      #  echo "       Please issue the following command to install them, and rerun the MEGAlib setup script:"
      #  echo " "
      #  echo "       xcode-select --install"
      #  echo " "
      #  echo "       If this does not help:"
      #fi
      issuereport
      exit 1
    fi
  fi
fi

echo " "
echo "SUCCESS: We have a usable ROOT version!"



echo " "
echo "(4) Downloading and building Geant4:"
echo " "

cd ${STARTPATH}
if [ "${GEANT4PATH}" != "" ]; then
  # Check if we can use the given Geant4 version

  bash ${MEGALIBDIR}/config/check-geant4version.sh --check=${GEANT4PATH}
  RESULT=$?
  if [ "${RESULT}" != "0" ]; then
    if [ "${RESULT}" == "127" ]; then
      echo " "
      echo "ERROR: Cannot find check-geant4version.sh. Either your MEGAlib version is too old or corrupt..."
      exit 1
    else
      echo " "
      echo "ERROR: The directory ${GEANT4PATH} cannot be used as your Geant4 version for MEGAlib."
      exit 1
    fi
  fi
  echo "GEANT4DIR=$(cd $(dirname ${GEANT4PATH}); pwd)/$(basename ${GEANT4PATH})" >> ${ENVFILE}
else
  # Download and build a new Geant4 version
  if [ ! -d ${EXTERNALPATH} ]; then
    mkdir ${EXTERNALPATH}
  fi
  cd ${EXTERNALPATH}
  echo "Switching to build-geant4.sh script..."
  bash ${MEGALIBDIR}/config/build-geant4.sh -e=${ENVFILE} -p=${PATCH} --debug=${DEBUG} --maxthreads=${MAXTHREADS} --cleanup=${CLEANUP} | tee Geant4BuildLog.txt
  RESULT=${PIPESTATUS[0]}


  # If we have a new Geant4 dir, copy the build log there
  NEWGEANT4DIR=`grep GEANT4DIR\= ${ENVFILE} | awk -F= '{ print $2 }'`
  if [[ -d ${NEWGEANT4DIR} ]]; then
    mv Geant4BuildLog.txt ${NEWGEANT4DIR}
  fi

  # Now handle build errors
  if [ "${RESULT}" != "0" ]; then
    if [ "${RESULT}" == "127" ]; then
      echo " "
      echo "ERROR: Cannot find build-geant4.sh. Either your MEGAlib version is too old or corrupt..."
      exit 1
    else
      echo " "
      echo "ERROR: Something went wrong during the Geant4 setup."
      issuereport
      exit 1
    fi
  fi
fi

echo " "
echo "SUCCESS: We have a usable Geant4 version!"



echo " "
echo "(5) Setup MEGAlib"
echo " "

# Finalize the source script:
echo " " >> ${ENVFILE}
echo "source ${MEGALIBPATH}/config/env.sh --root=\${ROOTDIR} --geant4=\${GEANT4DIR} --megalib=\${MEGALIBDIR}" >> ${ENVFILE}
echo " " >> ${ENVFILE}

source ${ENVFILE}



cd ${MEGALIB}

echo "Configuring MEGAlib..."
bash configure --os=${OS} --debug=${DEBUG} --opt=${OPT} --updates=${UPDATES}
if [ "$?" != "0" ]; then
  echo " "
  echo "ERROR: Something went wrong during MEGAlib configuration"
  issuereport
  exit 1
fi



echo "Compiling MEGAlib..."
make -j${MAXTHREADS}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while compiling MEGAlib!"
  issuereport
  exit 1
fi

echo "Renaming and moving the environment script"
mv ${ENVFILE} bin/source-megalib.sh

echo "Storing last good options"
rm -f ${MEGALIBDIR}/config/SetupOptions.txt
SETUP="--external-path=${EXTERNALPATH} --root=${ROOTPATH} --geant4=${GEANT4PATH} --release=${RELEASE} --repository=${REPOSITORY} --optimization=${OPT} --debug=${DEBUG} --updates=${UPDATES} --patch=${PATCH} --cleanup=${CLEANUP}"
if [[ ${BRANCH} != "" ]]; then
  SETUP+=" --branch=${BRANCH}"
fi

echo "${SETUP}" >> ${MEGALIBDIR}/config/SetupOptions.txt

TIMEEND=$(date +%s)
TIMEDIFF=$(( ${TIMEEND} - ${TIMESTART} ))
echo " "
echo "Finished! Execution duration: ${TIMEDIFF} seconds"

echo " "
echo " "
echo "SUCCESS: MEGAlib should be installed now"
echo " "
echo "In order to run any MEGAlib programs, you have to source the following file first:"
echo "source ${MEGALIBDIR}/bin/source-megalib.sh"
echo "You can do this in your .bashrc file, or everytime you want to use MEGAlib."
echo " "


exit 0
