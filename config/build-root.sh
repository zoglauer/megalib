#!/bin/bash

# This file is part of MEGAlib.
# Copyright (C) by Andreas Zoglauer.
#
# Please see the MEGAlib software license and documentation for more informations.

CONFIGUREOPTIONS=" "
# Install path relative to the build path --- simply one up in this script
CONFIGUREOPTIONS+=" -DCMAKE_INSTALL_PREFIX=.."
# Make sure we ignore some default paths of macport.
type port >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  PORTPATH=$(which port)
  PORTPATH=${PORTPATH%/bin/port}
  CONFIGUREOPTIONS+=" -DCMAKE_IGNORE_PATH=${PORTPATH};${PORTPATH}/bin;${PORTPATH}/include;${PORTPATH}/include/libxml2;${PORTPATH}/include/unicode"
fi
# Until ROOT 6.24: C++ 11
CONFIGUREOPTIONS+=" -DCMAKE_CXX_STANDARD=14"
# We want a minimal system and enable what we really need:
#CONFIGUREOPTIONS+=" -Dgminimal=ON"
# Open GL -- needed by geomega
CONFIGUREOPTIONS+=" -Dopengl=ON"
# Mathmore -- needed for fitting, e.g. ARMs"
CONFIGUREOPTIONS+=" -Dmathmore=ON"
# Minuit2 -- needed for parallel fitting with melinator
# Minuit 2 is default startin g ROOT 6.30: CONFIGUREOPTIONS+=" -Dminuit2=ON"
# XFT -- needed for smoothed fonts
CONFIGUREOPTIONS+=" -Dxft=ON"
# Afterimage -- support to draw images in pads and save as png, etc.
CONFIGUREOPTIONS+=" -Dasimage=ON"
# Stuff for linking, paths in so files, versioning etc
CONFIGUREOPTIONS+=" -Dexplicitlink=ON -Drpath=ON -Dsoversion=ON"
# enable builtin glew
CONFIGUREOPTIONS+=" -Dbuiltin_glew=ON"

# In case you have trouble with anything related to freetype, try to comment in this option
# CONFIGUREOPTIONS+=" -Dbuiltin-freetype=ON"

# In case you get strange error messages concerning jpeg, png, tiff
# CONFIGUREOPTIONS+=" -Dasimage=OFF -Dastiff=OFF -Dbuiltin_afterimage=OFF"

# In case you have trouble with zlib (gz... something error messages)
# CONFIGUREOPTIONS+=" -Dbuiltin_zlib=ON -Dbuiltin_lzma=ON"

# By default we build with python 3:
type python3 >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  PPATH=$(which python3)
  if [[ -f ${PPATH} ]]; then
    if [[ ${PPATH} == *conda* ]]; then
      echo "ERROR: You cannot use a python version installed via (ana)conda with ROOT."
      exit 1
    else
      CONFIGUREOPTIONS+=" -DPYTHON_EXECUTABLE:FILEPATH=${PPATH} -Dpython3=ON"
    fi
  fi
fi

# Enable cuda if available
type nvcc >/dev/null 2>&1
if [[ $? -eq 0 ]]; then
  CONFIGUREOPTIONS+=" -Dcuda=ON"
fi

# In case ROOT complains about your python version
# CONFIGUREOPTIONS+=" -Dpython=OFF"
# CONFIGUREOPTIONS+=" -Dpython3=OFF"

# Switching off things we do not need right now but which are on by default
CONFIGUREOPTIONS+=" -Dalien=OFF -Dbonjour=OFF -Dcastor=OFF -Ddavix=OFF -Dfortran=OFF -Dfitsio=OFF -Dchirp=OFF -Ddcache=OFF -Dgfal=OFF -Dglite=off -Dhdfs=OFF -Dkerb5=OFF -Dldap=OFF -Dmonalisa=OFF -Dodbc=OFF -Doracle=OFF -Dpch=OFF -Dpgsql=OFF -Dpythia6=OFF -Dpythia8=OFF -Drfio=OFF -Dsapdb=OFF -Dshadowpw=OFF -Dsqlite=OFF -Dsrp=OFF -Dssl=OFF -Dxrootd=OFF"

# Explictly add gcc -- cmake seems to sometimes digg up other compilers on the system, not the default one...
# CONFIGUREOPTIONS+=" -DCMAKE_C_COMPILER=$(which gcc) -DCMAKE_CXX_COMPILER=$(which g++)"

# The compiler
COMPILEROPTIONS=`gcc --version | head -n 1`


# Check if some of the frequently used software is installed:
type cmake >/dev/null 2>&1
if [ $? -ne 0 ]; then
  echo "ERROR: cmake must be installed"
  exit 1
else
  VER=`cmake --version | grep ^cmake`
  VER=${VER#cmake version };
  OLDIFS=${IFS}; IFS='.'; Tokens=( ${VER} ); IFS=${OLDIFS};
  VERSION=$(( 10000*${Tokens[0]} + 100*${Tokens[1]} + ${Tokens[2]} ));
  if (( ${VERSION} < 30403 )); then
    echo "ERROR: the version of cmake needs to be at least 3.4.3 and not ${VER}"
    exit 1
  fi
fi
type curl >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: curl must be installed"
    exit 1
fi
type openssl >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: openssl must be installed"
    exit 1
fi


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
  echo "--rootversion=[e.g. 5.34, 6.10]"
  echo "    Use the given ROOT version instead of the one required by MEGAlib."
  echo " "
  echo "--sourcescript=[file name of new environment script]"
  echo "    File in which the MEGAlib environment is/will be stored. This is used by the MEGAlib setup script"
  echo " "
  echo "--debug=[off/no, on/yes - default: off]"
  echo "    Compile with degugging options."
  echo " "
  echo "--keepenvironmentasis=[off/no, on/yes - default: off]"
  echo "    By default all relevant environment paths (such as LD_LIBRRAY_PATH, CPATH) are reset to empty to avoid most libray conflicts."
  echo "    This flag toggles this behaviour and lets you decide to keep your environment or not."
  echo " "
  echo "--maxthreads=[integer >=1 - default: 1]"
  echo "    The maximum number of threads to be used for compilation. Default is the number of cores in your system."
  echo " "
  echo "--patch=[yes or no - default: no]"
  echo "    Apply MEGAlib internal (!) ROOT patches, if there are any for this version."
  echo " "
  echo "--cleanup=[off/no, on/yes - default: off]"
  echo "    Remove intermediate build files"
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
MAXTHREADS=1024
DEBUG="off"
DEBUGSTRING=""
DEBUGOPTIONS=""
PATCH="off"
CLEANUP="off"
KEEPENVASIS="off"
WANTEDVERSION=""

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-t*=* ]]; then
    TARBALL=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-s*=* ]] || [[ ${C} == *-e*=* ]]; then
    ENVFILE=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-m*=* ]]; then
    MAXTHREADS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-d*=* ]]; then
    DEBUG=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-p*=* ]]; then
    PATCH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-cl*=* ]]; then
    CLEANUP=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-r*=* ]]; then
    WANTEDVERSION=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-k*=* ]]; then
    KEEPENVASIS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-h* ]]; then
    echo ""
    confhelp
    exit 0
  else
    echo ""
    echo "ERROR: Unknown command line option: ${C}"
    echo "       See \"$0 --help\" for a list of options"
    echo " "
    exit 1
  fi
done



echo ""
echo ""
echo ""
echo "Setting up ROOT..."
echo ""
echo "Verifying chosen configuration options:"
echo ""

if [ "${TARBALL}" != "" ]; then
  if [[ ! -f "${TARBALL}" ]]; then
    echo "ERROR: The chosen tarball cannot be found: ${TARBALL}"
    exit 1
  else
    echo " * Using this tarball: ${TARBALL}"
  fi
fi


if [ "${ENVFILE}" != "" ]; then
  if [[ ! -f "${ENVFILE}" ]]; then
    echo "ERROR: The chosen environment file cannot be found: ${ENVFILE}"
    exit 1
  else
    echo " * Using this environment file: ${ENVFILE}"
  fi
fi


if [ ! -z "${MAXTHREADS##[0-9]*}" ] 2>/dev/null; then
  echo "ERROR: The maximum number of threads must be number and not ${MAXTHREADS}!"
  exit 1
fi
if [ "${MAXTHREADS}" -le "0" ]; then
  echo "ERROR: The maximum number of threads must be at least 1 and not ${MAXTHREADS}!"
  exit 1
else
  echo " * Using this maximum number of threads: ${MAXTHREADS}"
fi


DEBUG=`echo ${DEBUG} | tr '[:upper:]' '[:lower:]'`
if ( [[ ${DEBUG} == of* ]] || [[ ${DEBUG} == no ]] ); then
  DEBUG="off"
  DEBUGSTRING=""
  DEBUGOPTIONS=""
  echo " * Using no debugging code"
elif ( [[ ${DEBUG} == on ]] || [[ ${DEBUG} == y* ]] || [[ ${DEBUG} == nor* ]] ); then
  DEBUG="normal"
  DEBUGSTRING="_debug"
  DEBUGOPTIONS="-DCMAKE_BUILD_TYPE=Debug"
  echo " * Using debugging code"
else
  echo "ERROR: Unknown debugging code selection: ${DEBUG}"
  confhelp
  exit 0
fi


PATCH=`echo ${PATCH} | tr '[:upper:]' '[:lower:]'`
if ( [[ ${PATCH} == of* ]] || [[ ${PATCH} == n* ]] ); then
  PATCH="off"
  echo " * Don't apply MEGAlib internal ROOT and Geant4 patches"
elif ( [[ ${PATCH} == on ]] || [[ ${PATCH} == y* ]] ); then
  PATCH="on"
  echo " * Apply MEGAlib internal ROOT and Geant4 patches"
else
  echo " "
  echo "ERROR: Unknown option for updates: ${PATCH}"
  confhelp
  exit 1
fi


CLEANUP=`echo ${CLEANUP} | tr '[:upper:]' '[:lower:]'`
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


KEEPENVASIS=`echo ${KEEPENVASIS} | tr '[:upper:]' '[:lower:]'`
if ( [[ ${KEEPENVASIS} == of* ]] || [[ ${KEEPENVASIS} == n* ]] ); then
  KEEPENVASIS="off"
  echo " * Clearing the environment paths LD_LIBRARY_PATH, CPATH"
  # We cannot clean PATH, otherwise no programs can be found anymore
  export LD_LIBRARY_PATH=""
  export SHLIB_PATH=""
  export CPATH=""
  export CMAKE_PREFIX_PATH=""
  export DYLD_LIBRARY_PATH=""
  export JUPYTER_PATH=""
  export LIBPATH=""
  export MANPATH=""
elif ( [[ ${KEEPENVASIS} == on ]] || [[ ${KEEPENVASIS} == y* ]] ); then
  KEEPENVASIS="on"
  echo " * Keeping the existing environment paths as is."
else
  echo " "
  echo "ERROR: Unknown option for keeping MEGAlib or not: ${KEEPENVASIS}"
  confhelp
  exit 1
fi


echo " "
echo " "
echo "Getting ROOT..."
VER=""
ROOTTOPDIR=""
if [ "${TARBALL}" != "" ]; then
  # Use given ROOT tarball
  echo "The given ROOT tarball is ${TARBALL}"

  # Determine the name of the top level directory in the tar ball
  ROOTTOPDIR=`tar tzf ${TARBALL} | sed -e 's@/.*@@' | uniq`
  RESULT=$?
  if [ "${RESULT}" != "0" ]; then
    echo "ERROR: Cannot find top level directory in the tar ball!"
    exit 1
  fi

  # Check if it has the correct version:
  VER=`tar xfzO ${TARBALL} ${ROOTTOPDIR}/build/version_number | sed 's|/|.|g'`
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

  if [[ ${WANTEDVERSION} != "" ]]; then
    if [[ ${VER} != ${WANTEDVERSION}.* ]]; then
      echo "ERROR: The ROOT tarball has not the same version ${VER} you wanted on the command line ${WANTEDVERSION}!"
      exit 1
    fi
  else
    bash ${MEGALIB}/config/check-rootversion.sh --good-version=${VER}
    if [ "$?" != "0" ]; then
      echo "ERROR: The ROOT tarball you supplied does not contain an acceptable ROOT version!"
      exit 1
    fi
  fi
else
  # Download it

  # Get desired version:
  if [[ ${WANTEDVERSION} == "" ]]; then
    WANTEDVERSION=`bash ${MEGALIB}/config/check-rootversion.sh --get-max`
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to determine required ROOT version!"
      exit 1
    fi
  fi
  echo "Looking for ROOT version ${WANTEDVERSION} with latest patch on the ROOT website --- sometimes this takes a few minutes..."

  # Now check root repository for the selected version:
  TARBALL=""
  MAX_TRIALS=3
  for s in `seq -w 00 2 98`; do
    TESTTARBALL="root_v${WANTEDVERSION}.${s}.source.tar.gz"
    echo "Trying to find ${TESTTARBALL}..."
    EXISTS=`curl -s --head https://root.cern.ch/download/${TESTTARBALL} | grep gzip`
    if [[ ${EXISTS} == "" ]]; then # sometimes version 00 does not exist...
      MAX_TRIALS=$(( MAX_TRIALS - 1 ))
      if [[ ${MAX_TRIALS} -eq 0 ]]; then
        break
      fi
    else 
      TARBALL=${TESTTARBALL}
    fi
  done
  if [[ -z ${TARBALL} ]]; then
    echo "ERROR: Unable to find a suitable ROOT tar ball"
    exit 1
  fi
  echo "Using ROOT tar ball ${TARBALL}"

  # Check if it already exists locally
  REQUIREDOWNLOAD="true"
  if [ -f ${TARBALL} ]; then
    # ... and has the same size
    LOCALSIZE=`wc -c < ${TARBALL} | tr -d ' '`
    SAMESIZE=`curl -s --head https://root.cern.ch/download/${TARBALL}`
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
    echo "Starting the download."
    echo "If the download fails, you can continue it via the following command and then call this script again - it will use the downloaded file."
    echo " "
    echo "curl -O -C - https://root.cern.ch/download/${TARBALL}"
    echo " "
    curl -O https://root.cern.ch/download/${TARBALL}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to download the tarball from the ROOT website!"
      exit 1
    fi
  fi

  # Determine the name of the top level directory in the tar ball
  ROOTTOPDIR=`tar tzf ${TARBALL} | sed -e 's@/.*@@' | uniq`
  RESULT=$?
  if [ "${RESULT}" != "0" ]; then
    echo "ERROR: Cannot find top level directory in the tar ball!"
    exit 1
  fi

  VER=`tar xfzO ${TARBALL} ${ROOTTOPDIR}/build/version_number | sed 's|/|.|g'`
  if [ "$?" != "0" ]; then
    echo "ERROR: Something went wrong unpacking the ROOT tarball!"
    exit 1
  fi
  if echo ${VER} | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Something terrible is wrong with your version string..."
    exit 1
  fi
  echo "Version of ROOT is: ${VER}"
fi


ROOTCORE=root_v${VER}
ROOTDIR=root_v${VER}${DEBUGSTRING}
ROOTSOURCEDIR=root_v${VER}-source   # Attention: the cleanup checks this name pattern before removing it
ROOTBUILDDIR=root_v${VER}-build     # Attention: the cleanup checks this name pattern before removing it


# Hardcoding default patch conditions
# Needs to be done after the ROOT version is known and before we check the exiting installation
if [[ ${ROOTCORE} == "root_v6.24.08" ]] || [[ ${ROOTCORE} == "root_v6.24.10" ]]; then
  echo "This version of ROOT requires a mandatory patch"
  PATCH="on"
fi

echo "Checking for old installation..."
if [ -d ${ROOTDIR} ]; then
  cd ${ROOTDIR}
  if [ -f COMPILE_SUCCESSFUL ]; then

    SAMEOPTIONS=`cat COMPILE_SUCCESSFUL | grep -F -x -- "${CONFIGUREOPTIONS}"`
    if [ "${SAMEOPTIONS}" == "" ]; then
      echo "The old installation used different compilation options..."
    fi

    SAMECOMPILER=`cat COMPILE_SUCCESSFUL | grep -F -x -- "${COMPILEROPTIONS}"`
    if [ "${SAMECOMPILER}" == "" ]; then
      echo "The old installation used a different compiler..."
    fi

    SAMEPATCH=""
    PATCHPRESENT="no"
    if [ -f "${MEGALIB}/resource/patches/${ROOTCORE}.patch" ]; then
      PATCHPRESENT="yes"
      PATCHPRESENTMD5=`openssl md5 "${MEGALIB}/resource/patches/${ROOTCORE}.patch" | awk -F" " '{ print $2 }'`
    fi
    PATCHSTATUS=`cat COMPILE_SUCCESSFUL | grep -- "^Patch"`
    if [[ ${PATCHSTATUS} == Patch\ applied* ]]; then
      PATCHMD5=`echo ${PATCHSTATUS} | awk -F" " '{ print $3 }'`
    fi

    if [[ ${PATCH} == on ]]; then
      if [[ ${PATCHPRESENT} == yes ]] && [[ ${PATCHSTATUS} == Patch\ applied* ]] && [[ ${PATCHPRESENTMD5} == ${PATCHMD5} ]]; then
        SAMEPATCH="YES";
      elif [[ ${PATCHPRESENT} == no ]] && [[ ${PATCHSTATUS} == Patch\ not\ applied* ]]; then
        SAMEPATCH="YES";
      else
        echo "The old installation didn't use the same patch..."
        SAMEPATCH=""
      fi
    elif [[ ${PATCH} == off ]]; then
      if [[ ${PATCHSTATUS} == Patch\ not\ applied* ]] || [[ -z ${PATCHSTATUS}  ]]; then    # last one means empty
        SAMEPATCH="YES";
      else
        echo "The old installation used a patch, but now we don't want any..."
        SAMEPATCH=""
      fi
    fi


    if ( [ "${SAMEOPTIONS}" != "" ] && [ "${SAMECOMPILER}" != "" ] && [ "${SAMEPATCH}" != "" ] ); then
      echo "Your already have a usable ROOT version installed!"
      if [ "${ENVFILE}" != "" ]; then
        echo "Storing the ROOT directory in the MEGAlib source script..."
        echo "ROOTDIR=`pwd`" >> ${ENVFILE}
      fi
      exit 0
    fi
  fi

  echo "Old installation is either incompatible or incomplete. Removing ${ROOTDIR}..."

  cd ..
  if echo "${ROOTDIR}" | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Feeding my paranoia of having a \"rm -r\" in a script:"
    echo "       There should not be any spaces in the ROOT version..."
    exit 1
  fi
  rm -rf "${ROOTDIR}"
fi



echo "Unpacking..."
mkdir ${ROOTDIR}
cd ${ROOTDIR}
tar xfz ../${TARBALL} > /dev/null
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong unpacking the ROOT tarball!"
  exit 1
fi
mv ${ROOTTOPDIR} ${ROOTSOURCEDIR}
mkdir ${ROOTBUILDDIR}



PATCHAPPLIED="Patch not applied"
if [[ ${PATCH} == on ]]; then
  echo "Patching..."
  if [ -f "${MEGALIB}/resource/patches/${ROOTCORE}.patch" ]; then
    patch -p1 < ${MEGALIB}/resource/patches/${ROOTCORE}.patch
    PATCHMD5=`openssl md5 "${MEGALIB}/resource/patches/${ROOTCORE}.patch" | awk -F" " '{ print $2 }'`
    PATCHAPPLIED="Patch applied ${PATCHMD5}"
    echo "Applied patch: ${MEGALIB}/resource/patches/${ROOTCORE}.patch"
  fi
fi



echo "Configuring..."
cd ${ROOTBUILDDIR}
export ROOTSYS=${ROOTDIR}
#if [[ ${OSTYPE} == darwin* ]]; then
#  export CPLUS_INCLUDE_PATH=`xcrun --show-sdk-path`/usr/include
#  export LIBRARY_PATH=$LIBRARY_PATH:`xcrun --show-sdk-path`/usr/lib
#fi
echo "Configure command: cmake ${CONFIGUREOPTIONS} ${DEBUGOPTIONS} ../${ROOTSOURCEDIR}"
cmake ${CONFIGUREOPTIONS} ${DEBUGOPTIONS} ../${ROOTSOURCEDIR}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong configuring (cmake'ing) ROOT!"
  exit 1
fi



CORES=1;
if [[ ${OSTYPE} == darwin* ]]; then
  CORES=`sysctl -n hw.logicalcpu_max`
elif [[ ${OSTYPE} == linux* ]]; then
  CORES=`grep processor /proc/cpuinfo | wc -l`
fi
if [ "$?" != "0" ]; then
  CORES=1
fi
if [ "${CORES}" -gt "${MAXTHREADS}" ]; then
  CORES=${MAXTHREADS}
fi
echo "Using this number of cores for compilation: ${CORES}"



echo "Compiling..."
make -j${CORES}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while compiling ROOT!"
  exit 1
fi



echo "Installing..."
make install
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while installing ROOT!"
  exit 1
fi

# Done. Switch to main ROOT directory
cd ..

if [[ ${CLEANUP} == on ]]; then
  echo "Cleaning up ..."
  # Just a sanity check before our remove...
  if [[ ${ROOTBUILDDIR} == root_v*-build ]]; then
    rm -rf ${ROOTBUILDDIR}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to remove build directory!"
      exit 1
    fi
  else
    echo "INFO: Not cleaning up the build directory, because it is not named as expected: ${ROOTBUILDDIR}"
  fi
  if [[ ${ROOTSOURCEDIR} == root_v*-source ]]; then
    rm -rf ${ROOTSOURCEDIR}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to remove source directory!"
      exit 1
    fi
  else
    echo "INFO: Not cleaning up the source directory, because it is not named as expected: ${ROOTSOURCEDIR}"
  fi
fi


echo "Store our success story..."
rm -f COMPILE_SUCCESSFUL
echo "ROOT compilation & installation successful" >> COMPILE_SUCCESSFUL
echo " " >> COMPILE_SUCCESSFUL
echo "* Configure options:" >> COMPILE_SUCCESSFUL
echo "${CONFIGUREOPTIONS}" >> COMPILE_SUCCESSFUL
echo " " >> COMPILE_SUCCESSFUL
echo "* Compile options:" >> COMPILE_SUCCESSFUL
echo "${COMPILEROPTIONS}" >> COMPILE_SUCCESSFUL
echo " " >> COMPILE_SUCCESSFUL
echo "* Patch application status:" >> COMPILE_SUCCESSFUL
echo "${PATCHAPPLIED}" >> COMPILE_SUCCESSFUL
echo " " >> COMPILE_SUCCESSFUL



echo "Setting permissions..."
cd ..
chown -R ${USER}:${GROUP} ${ROOTDIR}
chmod -R go+rX ${ROOTDIR}


if [ "${ENVFILE}" != "" ]; then
  echo "Storing the ROOT directory in the MEGAlib source script..."
  echo "ROOTDIR=`pwd`/${ROOTDIR}" >> ${ENVFILE}
fi


echo "Done!"
exit 0
