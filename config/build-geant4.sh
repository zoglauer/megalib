#!/bin/bash

# Start with the configure options, since we want to compare them to what was done previously
CONFIGUREOPTIONS=""
type g++ >/dev/null 2>&1
if [ $? -eq 0 ]; then
    # echo "g++ compiler found - using it as default!";
    CONFIGUREOPTIONS="-DCMAKE_C_COMPILER=gcc -DCMAKE_CXX_COMPILER=g++"
fi
CONFIGUREOPTIONS="${CONFIGUREOPTIONS} -DCMAKE_INSTALL_PREFIX=.. -DGEANT4_INSTALL_DATA=ON -DGEANT4_USE_OPENGL_X11=ON -DGEANT4_INSTALL_DATA_TIMEOUT=14400 -DGEANT4_USE_SYSTEM_EXPAT=OFF -DGEANT4_BUILD_CXXSTD=c++11"
COMPILEROPTIONS=`gcc --version | head -n 1`


# Check if some of the frequently used software is installed:
type cmake >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: cmake must be installed"
    exit 1
fi 
type curl >/dev/null 2>&1
if [ $? -ne 0 ]; then
    echo "ERROR: curl must be installed"
    exit 1
fi 


confhelp() {
  echo ""
  echo "Building GEANT4"
  echo " " 
  echo "Usage: ./build-geant4.sh [options]";
  echo " "
  echo " "
  echo "Options:"
  echo "--tarball=[file name of Geant4 tarball]"
  echo "    Use this tarball instead of downloading it from the Geant4 website" 
  echo " "
  echo "--environment-script=[file name of new environment script]"
  echo "    File in which the Geant4 path is stored. This is used by the MEGAlib setup script" 
  echo " "
  echo "--debug=[off/no, on/yes, strong/hard]"
  echo "    Default is on."
  echo " "
  echo "--maxthreads=[integer >=1]"
  echo "    The maximum number of threads to be used for compilation. Default is the number of cores in your system."
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

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == *-t*=* ]]; then
    TARBALL=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-e*=* ]]; then
    ENVFILE=`echo ${C} | awk -F"=" '{ print $2 }'`
    echo "Using this MEGALIB environment file: ${ENVFILE}"
  elif [[ ${C} == *-m*=* ]]; then
    MAXTHREADS=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-d*=* ]]; then
    DEBUG=`echo ${C} | awk -F"=" '{ print $2 }'`
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

echo ""
echo ""
echo ""
echo "Setting up Geant4..."
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



echo "Getting Geant4..."
VER=""

if [ "${TARBALL}" != "" ]; then
  # Use given tarball
  echo "The given Geant4 tarball is ${TARBALL}"
  
  # Check if it has the correct version:
  VER=`echo ${TARBALL} | awk -Fgeant4. '{ print $2 }' | awk -F.t '{ print $1 }'`;
  echo "Version of Geant4 is: ${VER}"
  
  bash ${MEGALIB}/config/check-geant4version.sh --good-version=${VER}
  if [ "$?" != "0" ]; then
    echo "ERROR: The Geant4 tarball you supplied does not contain an acceptable Geant4 version!"
    exit 1
  fi
else
  # Download it
  
  # Get desired version:
  WANTEDVERSION=`bash ${MEGALIB}/config/check-geant4version.sh --get-max`
  if [ "$?" != "0" ]; then
    echo "ERROR: Unable to determine required Geant4 version!"
    exit 1
  fi
  echo "Looking for Geant4 version ${WANTEDVERSION} with latest patch on the Geant4 website --- sometimes this takes a few minutes..."
  
  # Now check Geant4 repository for the given version:
  TESTTARBALL="geant4.${WANTEDVERSION}.tar.gz"
  echo "Trying to find ${TESTTARBALL}..."
  EXISTS=`curl -s --head http://geant4.web.cern.ch/geant4/support/source/${TESTTARBALL} | grep gzip`
  if [ "${EXISTS}" == "" ]; then
    echo "ERROR: Unable to find suitable Geant4 tar ball at the Geant4 website"
    exit 1
  fi
  TARBALL=${TESTTARBALL}
  for s in `seq -w 01 10`; do
    TESTTARBALL="geant4.${WANTEDVERSION}.p${s}.tar.gz"
    echo "Trying to find ${TESTTARBALL}..."
    EXISTS=`curl -s --head http://geant4.web.cern.ch/geant4/support/source/${TESTTARBALL} | grep gzip`
    if [ "${EXISTS}" == "" ]; then
      break
    fi
    TARBALL=${TESTTARBALL}
  done
  echo "Using Geant4 tar ball ${TARBALL}"
  
  # Check if it already exists locally
  REQUIREDOWNLOAD="true"
  if [ -f ${TARBALL} ]; then
    # ... and has the same size
    LOCALSIZE=`wc -c < ${TARBALL} | tr -d ' '`
    SAMESIZE=`curl -s --head http://geant4.web.cern.ch/geant4/support/source/${TARBALL}`
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
    curl -O http://geant4.web.cern.ch/geant4/support/source/${TARBALL}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to download the tarball from the Geant4 website!"
      exit 1
    fi
  fi
  
  VER=`echo ${TARBALL} | awk -Fgeant4. '{ print $2 }' | awk -F.t '{ print $1 }'`;
  if [ "$?" != "0" ]; then
    echo "ERROR: Something went wrong during determining the Geant4 version"
    exit 1
  fi
  echo "Version of Geant4 is: ${VER}"
fi



echo "Checking for old installation..."
GEANT4DIR=geant4_v${VER}${DEBUGSTRING}
if [ -d ${GEANT4DIR} ]; then
  cd ${GEANT4DIR}
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
      echo "Your already have a usable Geant4 version installed!"
      if [ "${ENVFILE}" != "" ]; then
        echo "Storing the Geant4 directory in the MEGAlib source script..."
        echo "GEANT4DIR=`pwd`" >> ${ENVFILE}
      fi
      exit 0
    fi
  fi
    
  echo "Old installation is either incompatible or incomplete. Removing ${GEANT4DIR}"
  cd ..
  if echo "${GEANT4DIR}" | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Feeding my paranoia of having a \"rm -r\" in a script:"
    echo "       There should not be any spaces in the Geant4 version..."
    exit 1
  fi
  rm -rf "${GEANT4DIR}"
fi



echo "Unpacking..."
if ( [[ ${TARBALL} == *.tgz ]] || [[ ${TARBALL} == *.tar.gz ]] ); then
    tar xvfz ${TARBALL} > /dev/null
elif [[ $1 == *.tar ]] ; then
    tar xvf ${TARBALL} > /dev/null
else
    echo "Error: File has unknown suffix: $1 (known: tgz, tar.gz, tar)"
    exit 1
fi
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong unpacking the Geant4 tarball!"
  exit 1
fi
mv geant4.${VER} geant4.${VER}-source
mkdir ${GEANT4DIR}
cd ${GEANT4DIR}
mv ../geant4.${VER}-source .
mkdir geant4.${VER}-build
cd geant4.${VER}-build



echo "Configuring ..."
export LD_LIBRARY_PATH=""
cmake ${CONFIGUREOPTIONS} ${DEBUGOPTIONS} ../geant4.${VER}-source
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong configuring (cmake'ing) Geant4!"
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
if [ "${CORES}" -gt "${MAXTHREADS}" ]; then
  CORES=${MAXTHREADS}
fi
echo "Using this number of cores: ${CORES}"



echo "Compiling..."
make -j${CORES}
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while compiling Geant4!"
  exit 1
fi



echo "Installing..."
make install
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while installing Geant4!"
  exit 1
fi


echo "Store our success story..."
cd ..
rm -f COMPILE_SUCCESSFUL
echo "${CONFIGUREOPTIONS}" >> COMPILE_SUCCESSFUL
echo "${COMPILEROPTIONS}" >> COMPILE_SUCCESSFUL



echo "Setting permissions..."
cd ..
chown -R ${USER}:${GROUP} ${GEANT4DIR}
chmod -R go+rX ${GEANT4DIR}



if [ "${ENVFILE}" != "" ]; then
  echo "Storing the Geant4 directory in the MEGAlib source script..."
  echo "GEANT4DIR=`pwd`/${GEANT4DIR}" >> ${ENVFILE}
fi



echo "SUCCESS!"
exit 0

