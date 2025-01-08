#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.

# Part 1:
# Store command line as array
CMD=( "$@" )

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
  echo "Options:"
  echo " "
  echo "Remark: This script stores the last used options and will read it the next time it is started."
  echo "        Thus if you restart it, make sure to overwrite any options you want to change!"
  echo " "
  echo "--megalibpath=[path to MEGAlib - first launch default: \"MEGAlib\"]"
  echo "    This is the path to where MEGAlib is installed. If the path exists, we will try to update MEGAlib."
  echo "    MEGAlib must have been checked out from one of the Repositories or we cannot use it!"
  echo " "
  echo "--branch=[name of a git branch - first launch default is the lastest release version]"
  echo "    Choose a specific branch of the MEGAlib git repository. If the option is not given the latest release will be used."
  echo " "
  echo "--keepmegalibasis=[off/no, on/yes - first launch default: off]"
  echo "    Choose if you want to keep the current MEGAlib installation and just recompile, or if you want to update from the repository and then recompile."
  echo "    If set to off, you need to set \"--megalib-path\", and --release, --repository, and --branch are ignored!"
  echo " "
  echo "--keepenvironmentasis=[off/no, on/yes - first launch default: off]"
  echo "    By default all relevant environment paths (such as LD_LIBRRAY_PATH, CPATH) are reset to empty"
  echo "    to avoid most libray conflicts. This flag toggles this behaviour and lets you decide to keep your environment or not."
  echo " "
  echo "--externalpath=[path - first launch default: \"directory given by --megalib-path\"\external]"
  echo "    Directory where to install the required version of ROOT and Geant4."
  echo " "
  echo "--root=[options: empty (default), path to existing ROOT installation]"
  echo "    If empty (or the option has not been given at all), download and install the latest compatible version"
  echo "    If a path to an existing ROOT installation is given, then use this one. If it is not compatible with MEGAlib, the script will stop with an error."
  echo " "
  echo "--geant=[options: empty (default), path to existing GEANT4 installation]"
  echo "    If empty (or the option has not been given at all), download and install the latest compatible version"
  echo "    If a path to an existing GEANT4 installation is given, then use this one. If it is not compatible with MEGAlib, the script will stop with an error."
  echo " "
  echo "--heasoft=[options: off (default) empty, path to existing HEASoft installation]"
  echo "    If empty (or the option has not been given at all), download and install the latest compatible version"
  echo "    If the string \"off\" is given, do not install HEASoft. This will affect some tertiary tools of MEGAlib, such as storing the data in fits files."
  echo "    If a path to an existing HEASoft installation is given, then use this one. If it is not compatible with MEGAlib, the script will stop with an error."
  echo " "
  echo "--maxthreads=[integer >=1]"
  echo "    The maximum number of threads to be used for compilation. Default is the number of cores in your system."
  echo " "
  echo "--patch=[yes or no - first launch default: no]"
  echo "    Apply MEGAlib internal (!) ROOT or Geant4 patches, if there are any."
  echo " "
  echo "--debug=[off/no, on/yes - first launch default: off]"
  echo "    Debugging options for ROOT, Geant4 & MEGAlib."
  echo " "
  echo "--cleanup=[off/no, on/yes - first launch default: off]"
  echo "    Remove intermediate build files"
  echo " "
  echo "--optimization=[off/no, normal/on/yes, strong/hard (requires gcc 4.2 or higher) - first launch default: on]"
  echo "    Compilation optimization for MEGAlib ONLY (Default is normal)"
  echo " "
  echo "--updates=[off/no, on/yes - first launch default: off]"
  echo "    Check periodically for updates. Default is off."
  echo "    Even if set to on, update checks will only be performed, if the user has write access to the MEGAlib installation."
  echo " "
  #echo "--allowroot"
  #echo "    By default this script does not allow to be installed as ROOT. This option allows it."
  #echo " " 
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

# Check if the command line contains "--allowroot"
ALLOWROOT="off"
for C in "${CMD[@]}"; do
  if [[ ${C} == *--allowroot* ]]; then
    ALLOWROOT="on"
  fi
done

if [[ $EUID -eq 0 ]] && [[ ${ALLOWROOT} == off ]]; then
  echo " " 1>&2
  echo "Error: For security reasons, this script is not intended to be run as superuser/root." 1>&2
  echo "       It is intended for a user installation of MEGAlib." 1>&2
  echo " " 1>&2
  exit 1
fi

type cmake >/dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "Error: cmake must be installed"
  exit 1
else
  VER=`cmake --version | grep ^cmake`
  VER=${VER#cmake version };
  OLDIFS=${IFS}; IFS='.'; Tokens=( ${VER} ); IFS=${OLDIFS};
  VERSION=$(( 10000*${Tokens[0]} + 100*${Tokens[1]} + ${Tokens[2]} ));
  if (( ${VERSION} < 30600 )); then
    echo "ERROR: the version of cmake needs to be at least 3.6 and not ${VER}"
    exit 1
  fi
fi

# Part 3:
# Upgrade the input options:

# Check for help
for C in "${CMD[@]}"; do
  if [[ ${C} == *-h ]] || [[ ${C} == *-hel* ]]; then
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
KEEPENVASIS="off"
REPOSITORY="git"
RELEASE="rel"
EXTERNALPATH=""
ROOTPATH=""
GEANT4PATH=""
HEASOFTPATH="off"
OSTYPE=$(uname -s)
OPT="normal"
DEBUG="off"
UPDATES="off"
PATCH="on"
CLEANUP="off"
BRANCH=""
# ALLOWROOT="off" - initialized in the beginning not here

MAXTHREADS=1;
if [[ ${OSTYPE} == *arwin* ]]; then
  MAXTHREADS=`sysctl -n hw.logicalcpu_max`
elif [[ ${OSTYPE} == *inux* ]]; then
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
    #REPOSITORY=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo " * Ignoring no longer supported \"repository\" option"
  elif [[ ${C} == *-rel*=* ]]; then
    #RELEASE=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo " * Ignoring no longer supported \"release\" option"
  elif [[ ${C} == *-e*=* ]]; then
    EXTERNALPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-ro*=* ]]; then
    ROOTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-g*=* ]]; then
    GEANT4PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-hea*=* ]]; then
    HEASOFTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
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
  elif [[ ${C} == *-keepm* ]]; then
    KEEPMEGALIBASIS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-keepe* ]]; then
    KEEPENVASIS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-b* ]]; then
    BRANCH=`echo ${C} | awk -F"=" '{ print $2 }'`
    RELEASE="dev"
  elif [[ ${C} == *--allowroot* ]]; then
    # Handled above too
    ALLOWROOT="on"
  elif [[ ${C} == *-h ]] || [[ ${C} == *-hel* ]]; then
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
OSTYPE=`echo ${OSTYPE} | tr '[:upper:]' '[:lower:]'`
OPT=`echo ${OPT} | tr '[:upper:]' '[:lower:]'`
DEBUG=`echo ${DEBUG} | tr '[:upper:]' '[:lower:]'`
COMP=`echo ${COMP} | tr '[:upper:]' '[:lower:]'`
UPDATES=`echo ${UPDATES} | tr '[:upper:]' '[:lower:]'`
PATCH=`echo ${PATCH} | tr '[:upper:]' '[:lower:]'`
CLEANUP=`echo ${CLEANUP} | tr '[:upper:]' '[:lower:]'`
KEEPMEGALIBASIS=`echo ${KEEPMEGALIBASIS} | tr '[:upper:]' '[:lower:]'`
KEEPENVASIS=`echo ${KEEPENVASIS} | tr '[:upper:]' '[:lower:]'`



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


if ( [[ ${KEEPMEGALIBASIS} == of* ]] || [[ ${KEEPMEGALIBASIS} == n* ]] ); then
  KEEPMEGALIBASIS="off"
  echo " * Updating MEGAlib to the latest version"
elif ( [[ ${KEEPMEGALIBASIS} == on ]] || [[ ${KEEPMEGALIBASIS} == y* ]] ); then
  KEEPMEGALIBASIS="on"
  echo " * Keeping the existing MEGAlib installation, just recompiling MEGAlib."
else
  echo " "
  echo "ERROR: Unknown option for keeping MEGAlib or not: ${KEEPMEGALIBASIS}"
  confhelp
  exit 1
fi


if [[ ${BRANCH} != "" ]]; then
  echo " * Using MEGAlib branch ${BRANCH}"
else
  echo " * Using the latest MEGAlib release"
fi


if ( [[ ${KEEPENVASIS} == of* ]] || [[ ${KEEPENVASIS} == n* ]] ); then
  KEEPENVASIS="off"
  echo " * Clearing the environment paths PATH, LD_LIBRARY_PATH, CPATH"
  # We cannot clean PATH, otherwise no programs can be found anymore
  export LD_LIBRARY_PATH=""
  export CPATH=""
elif ( [[ ${KEEPENVASIS} == on ]] || [[ ${KEEPENVASIS} == y* ]] ); then
  KEEPENVASIS="on"
  echo " * Keeping the existing environment paths as is."
else
  echo " "
  echo "ERROR: Unknown option for keeping MEGAlib or not: ${KEEPENVASIS}"
  confhelp
  exit 1
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

if [[ "${HEASOFTPATH}" != "off" ]]; then
  if [[ "${HEASOFTPATH}" != "" ]]; then
    HEASOFTPATH=`absolutefilename ${HEASOFTPATH}`
  fi
  if [[ "${HEASOFTPATH}" != "${HEASOFTPATH% *}" ]]; then
    echo "ERROR: HEASoft needs to be installed in a path without spaces,"
    echo "       but you chose: \"${HEASOFTPATH}\""
    exit 1
  fi
  if [ "${HEASOFTPATH}" == "" ]; then
    echo " * Download latest compatible version of HEASoft"
  else
    echo " * Using the installation of HEASoft ${HEASOFTPATH}"
  fi
else 
  echo " * Not using HEASoft"
fi


if [[ ${OSTYPE} == *inux* ]]; then
  OSTYPE="linux"
  echo " * Using operating system architecture Linux"
elif ( [[ ${OSTYPE} == d* ]] || [[ ${OSTYPE} == m* ]] ); then
  OSTYPE="darwin"
  echo " * Using operating system architecture Darwin (Mac OS X)"
else
  echo " "
  echo "ERROR: Unknown operating system architecture: \"${OSTYPE}\""
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
    if [ ! -d .git ]; then
      echo " "
      echo "ERROR: We have a MEGAlib directory at \"${MEGALIBPATH}\" but it has not been checked out from any repository."
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
    
    
    # Starting git pull
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

      BRANCH=`git ls-remote --heads https://github.com/zoglauer/megalib.git | grep -v alpha | grep -v beta | grep MEGAlib_v | awk -F"refs/heads/" '{ print $2 }' | sort -n | tail -n 1`
      if [ "${CurrentBranch}" != "${Branch}" ]; then
        echo "Switching to latest release version of MEGAlib from the git repository..."
        git checkout ${BRANCH}
        if [ "$?" != "0" ]; then
          echo " "
          echo "ERROR: Unable to update the git repository to the latest release branch"
          exit 1
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

    echo "Fast forwarding to the head of branch ${BRANCH}..."
    git pull origin ${BRANCH}
    if [ "$?" != "0" ]; then
      echo " "
      echo "ERROR: Unable to fast forward to the head of this branch"
      exit 1
    fi

    cd ${STARTPATH}
    
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
      Branch=`git ls-remote --heads https://github.com/zoglauer/megalib.git | grep -v alpha | grep -v beta | grep MEGAlib_v | awk -F"refs/heads/" '{ print $2 }' | sort -n | tail -n 1`
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
if [[ ${OSTYPE} == linux ]]; then
  COMPILER="g++"
elif [[ ${OSTYPE} == macosx ]]; then
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
      echo "ERROR: The directory ${ROOTPATH} cannot be used as your ROOT install for MEGAlib."
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
  bash ${MEGALIBDIR}/config/build-root.sh -source=${ENVFILE} -patch=${PATCH} --debug=${DEBUG} --maxthreads=${MAXTHREADS} --cleanup=${CLEANUP} --keepenvironmentasis=${KEEPENVASIS} 2>&1 | tee RootBuildLog.txt
  RESULT=${PIPESTATUS[0]}

  # If we have a new ROOT dir, copy the build log there
  NEWROOT4DIR=`grep ROOTDIR\= ${ENVFILE} | awk -F= '{ print $2 }'`
  if [[ -d ${NEWROOT4DIR} ]]; then
    if [[ -f ${NEWROOT4DIR}/RootBuildLog.txt ]]; then
      mv ${NEWROOT4DIR}/RootBuildLog.txt ${NEWROOT4DIR}/RootBuildLog_before$(date +'%y%m%d%H%M%S').txt
    fi
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
      if [[ -d ${NEWROOT4DIR} ]]; then
        echo "       Please check the *whole* file ${NEWROOT4DIR}/RootBuildLog.txt for errors."
      else 
        echo "       Please check the *whole* file $(pwd)/RootBuildLog.txt for errors."
      fi
      echo " "
      echo "       Since this is an issue with ROOT and not MEGAlib, please try to google the error message, "
      echo "       because other ROOT users might face the same issue."
      echo " "
      echo "       If that fails, please take a look if you find the issue here (look at open and closed issues):"
      echo "       https://github.com/zoglauer/megalib/issues"
      echo " "
      echo "       If not, please add your problem there and attach your RootBuildLog.txt."
      echo " "
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
      echo "ERROR: The directory ${GEANT4PATH} cannot be used as your Geant4 install for MEGAlib."
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
  bash ${MEGALIBDIR}/config/build-geant4.sh -source=${ENVFILE} -patch=${PATCH} --debug=${DEBUG} --maxthreads=${MAXTHREADS} --cleanup=${CLEANUP} --keepenvironmentasis=${KEEPENVASIS} 2>&1 | tee Geant4BuildLog.txt
  RESULT=${PIPESTATUS[0]}


  # If we have a new Geant4 dir, copy the build log there
  NEWGEANT4DIR=`grep GEANT4DIR\= ${ENVFILE} | awk -F= '{ print $2 }'`
  if [[ -d ${NEWGEANT4DIR} ]]; then
    if [[ -f ${NEWGEANT4DIR}/Geant4BuildLog.txt ]]; then
      mv ${NEWGEANT4DIR}/Geant4BuildLog.txt ${NEWGEANT4DIR}/Geant4BuildLog_before$(date +'%y%m%d%H%M%S').txt
    fi
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
echo "(5) Downloading and building HEASoft"
echo " "

if [[ ${HEASOFTPATH} != off ]]; then
  cd ${STARTPATH}
  if [ "${HEASOFTPATH}" != "" ]; then
    # Check if we can use the given Geant4 version
  
    bash ${MEGALIBDIR}/config/check-heasoftversion.sh --check=${HEASOFTPATH}
    RESULT=$?
    if [ "${RESULT}" != "0" ]; then
      if [ "${RESULT}" == "127" ]; then
        echo " "
        echo "ERROR: Cannot find check-heasoftversion.sh. Either your MEGAlib version is too old or corrupt..."
        exit 1
      else
        echo " "
        echo "ERROR: The directory ${HEASOFTPATH} cannot be used as your HEASoft install for MEGAlib."
        exit 1
      fi
    fi
    echo "HEASOFTDIR=$(cd $(dirname ${HEASOFTPATH}); pwd)/$(basename ${HEASOFTPATH})" >> ${ENVFILE}
  else
    # Download and build a new HEASoft version
    if [ ! -d ${EXTERNALPATH} ]; then
      mkdir ${EXTERNALPATH}
    fi
    cd ${EXTERNALPATH}
    echo "Switching to build-heasoft.sh script..."
    bash ${MEGALIBDIR}/config/build-heasoft.sh -source=${ENVFILE}   2>&1 | tee HEASoftBuildLog.txt
    RESULT=${PIPESTATUS[0]}
  
  
    # If we have a new HEASoft dir, copy the build log there
    NEWHEASOFTDIR=`grep HEASOFTDIR\= ${ENVFILE} | awk -F= '{ print $2 }'`
    if [[ -d ${NEWHEASOFTDIR} ]]; then
      if [[ -f ${NEWHEASOFTDIR}/HEASoftBuildLog.txt ]]; then
        mv ${NEWHEASOFTDIR}/HEASoftBuildLog.txt ${NEWHEASOFTDIR}/HEASoftBuildLog_before$(date +'%y%m%d%H%M%S').txt
      fi
      mv HEASoftBuildLog.txt ${NEWHEASOFTDIR}
    fi
  
    # Now handle build errors
    if [ "${RESULT}" != "0" ]; then
      if [ "${RESULT}" == "127" ]; then
        echo " "
        echo "ERROR: Cannot find build-heosoft.sh. Either your MEGAlib version is too old or corrupt..."
        exit 1
      else
        echo " "
        echo "ERROR: Something went wrong during the HEASoft setup."
        issuereport
        exit 1
      fi
    fi
  fi
  fi

echo " "
echo "SUCCESS: We have a usable HEASoft version!"



echo " "
echo "(6) Setup MEGAlib"
echo " "

# Finalize the source script:
echo " " >> ${ENVFILE}
ENVSTRING="source ${MEGALIBPATH}/config/env.sh --root=\${ROOTDIR}"
if (( $(cat ${ENVFILE} | grep "^GEANT4DIR" | wc -l) == 1 )); then
  ENVSTRING+=" --geant4=\${GEANT4DIR}"
fi
if (( $(cat ${ENVFILE} | grep "^HEASOFTDIR" | wc -l) == 1 )); then
  ENVSTRING+=" --heasoft=\${HEASOFTDIR}"
fi
ENVSTRING+=" --megalib=\${MEGALIBDIR}"
echo "${ENVSTRING}" >> ${ENVFILE}
echo " " >> ${ENVFILE}

source ${ENVFILE}



cd ${MEGALIB}

echo "Configuring MEGAlib..."
bash configure --os=${OSTYPE} --debug=${DEBUG} --opt=${OPT} --updates=${UPDATES}
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


echo "Storing last good options..."
rm -f ${MEGALIBDIR}/config/SetupOptions.txt
SETUP="--external-path=${EXTERNALPATH} --root=${ROOTPATH} --geant4=${GEANT4PATH} --release=${RELEASE} --repository=${REPOSITORY} --optimization=${OPT} --debug=${DEBUG} --updates=${UPDATES} --patch=${PATCH} --cleanup=${CLEANUP} --keepmegalibasis=${KEEPMEGALIBASIS} --keepenvironmentasis=${KEEPENVASIS} --maxthreads=${MAXTHREADS}"
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
echo " "
echo "ATTENTION:"
echo " "
echo "In order to run the MEGAlib programs, you have to source the following file first:"
echo " "
echo "source ${MEGALIBDIR}/bin/source-megalib.sh"
echo " "
if [[ ${SHELL} == *zsh* ]]; then
  echo "You can add this line to your ~/.zprofile file,"
else
  echo "You can add this line to your ~/.bashrc file (or ~/.bash_profile on macOS),"
fi
echo "or execute this line everytime you want to use MEGAlib."
echo " "


exit 0
