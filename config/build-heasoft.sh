#!/bin/bash


CONFIGUREOPTIONS="--enable-readline"
COMPILEROPTIONS=`gcc --version | head -n 1`


confhelp() {
  echo ""
  echo "Building HEASoft"
  echo " " 
  echo "Usage: ./build-heasoft.sh [options]";
  echo " "
  echo " "
  echo "Options:"
  echo "--tarball=[file name of HEASoft tar ball]"
  echo "    Use this tarball instead of downloading it from the HEASoft website" 
  echo " "
  echo "--environment-script=[file name of new environment script]"
  echo "    File in which the HEASoft path is stored. This is used by the MEGAlib setup script" 
  echo " "
  echo "--help or -h"
  echo "    Show this help."
  echo " "
  echo " "
}

setuphelp() {
  if ( [[ ${SHELL} == *ash ]] || [[ ${SHELL} == *csh ]] ); then
    echo " "
    echo "If you are not using the MEGAlib source script,"
    echo "then you can use the following lines to setup HEASoft: "
    echo " "
    
    HEASOFTPATH=`ls -d heasoft_v${VER}/*86*`
    if [[ ${SHELL} == *csh ]]; then
      echo "You seem to use a C shell variant so, in your \$HOME/.cshrc or \$HOME/.tcshrc do:"
      echo " " 
      echo "setenv HEADAS "`pwd`"/${HEASOFTPATH}"
      echo "alias heainit \"source \$HEADAS/headas-init.csh\""
      echo " "
      echo "And then also add \"heainit\" to your setup script or call it each time before you use HEASoft/MEGAlib"
      echo " "
    elif [[ ${SHELL} == *ash ]]; then
      echo "You seem to use a bourne shell variant so, in your \$HOME/.bashrc or \$HOME/.login or \$HOME/.profile do:"
      echo " " 
      echo "export HEADAS="`pwd`"/${HEASOFTPATH}"
      echo "alias heainit=\"source \$HEADAS/headas-init.sh\""
      echo " "
      echo "And then also add \"heainit\" to your setup script or call it each time before you use HEASoft/MEGAlib"
      echo " "
    fi
  fi
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


echo "Getting HEASoft..."
VER=""
if [ "${TARBALL}" != "" ]; then
  # Use given tarball
  echo "The given HEASoft tarball is ${TARBALL}"
  
  # Check if it has the correct version:
  VER=`echo ${TARBALL} | awk -Fheasoft- '{ print $2 }' | awk -Fsrc '{ print $1 }'`;
  echo "Version of HEASoft is: ${VER}"
else
  # Download it
  
  # The desired version is simply the highest version
  echo "Looking for latest HEASoft version on the HEASoft website"
  
  # Now check root repository for the given version:
  TARBALL=`curl ftp://heasarc.gsfc.nasa.gov/software/lheasoft/release/ -sl | grep "^heasoft\-" | grep "[0-9]src.tar.gz$"`
  if [ "${TARBALL}" == "" ]; then
    echo "ERROR: Unable to find suitable HEASoft tar ball at the HEASoft website"
    exit 1
  fi
  echo "Using HEASoft tar ball ${TARBALL}"
  
  # Check if it already exists locally
  REQUIREDOWNLOAD="true"
  if [ -f "${TARBALL}" ]; then
    # ... and has the same size
    LOCALSIZE=`wc -c < ${TARBALL} | tr -d ' '`
    SAMESIZE=`curl --head ftp://heasarc.gsfc.nasa.gov/software/lheasoft/release/${TARBALL}`
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
    curl -O ftp://heasarc.gsfc.nasa.gov/software/lheasoft/release/${TARBALL}
    if [ "$?" != "0" ]; then
      echo "ERROR: Unable to download the tarball from the HEASoft website!"
      exit 1
    fi
  fi
  
  # Check for the version number:
  VER=`echo ${TARBALL} | awk -Fheasoft- '{ print $2 }' | awk -Fsrc '{ print $1 }'`;
  echo "Version of HEASoft is: ${VER}"
fi



echo "Checking for old installation..."
if [ -d heasoft_v${VER} ]; then
  cd heasoft_v${VER}
  if [ -f COMPILE_SUCCESSFUL ]; then
    SAMEOPTIONS=`cat COMPILE_SUCCESSFUL | grep -F -x -- "${CONFIGUREOPTIONS}"`
    if [ "${SAMEOPTIONS}" == "" ]; then
      echo "The old installation used different compilation options..."
    fi
    SAMECOMPILER=`cat COMPILE_SUCCESSFUL | grep -F -x -- "${COMPILEROPTIONS}"`
    if [ "${SAMECOMPILER}" == "" ]; then
      echo "The old installation used a different compiler..."
    fi
    if ( [ "${SAMEOPTIONS}" != "" ] && [ "${SAMECOMPILER}" != "" ] ); then
      echo "Your already have a usable HEASoft version installed!"
      if [ "${ENVFILE}" != "" ]; then
        echo "Storing the HEASoft directory in the MEGAlib source script..."
        echo "HEASoftDIR=`pwd`" >> ${ENVFILE}
      else 
        cd ..
        setuphelp
      fi
      exit 0
    fi
  fi
    
  echo "Old installation is either incompatible or incomplete. Removing heasoft_v${VER}"
  cd ..
  if echo "heasoft_v${VER}" | grep -E '[ "]' >/dev/null; then
    echo "ERROR: Feeding my paranoia of having a \"rm -r\" in a script:"
    echo "       There should not be any spaces in the HEASoft version..."
    exit 1
  fi
  chmod -R u+w "heasoft_v${VER}"
  rm -r "heasoft_v${VER}"
else
   echo "No old installation present"
fi



echo "Unpacking..."
tar xvfz ${TARBALL} > /dev/null
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong unpacking the HEASoft tarball!"
  exit 1
fi
mv heasoft-${VER} heasoft_v${VER}




echo "Configuring..."
# Minimze the LD_LIBRARY_PATH to prevent problems with multiple readline's
cd heasoft_v${VER}/BUILD_DIR
export LD_LIBRARY_PATH=/usr/lib
sh configure ${CONFIGUREOPTIONS} > config.out 2>&1
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong configuring HEASoft!"
  echo "       Check the file "`pwd`"/config.out"
  exit 1
fi



echo "Compiling..."
make -j1 > build.log 2>&1 
if [ "$?" != "0" ]; then
  echo "ERROR: Something went wrong while compiling HEASoft!"
  echo "       Check the file "`pwd`"/build.log"
  exit 1
fi
ERRORS=`grep -v "char \*\*\*" build.log | grep "\ \*\*\*\ "`
if [ "${ERRORS}" == "" ]; then
  echo "Installing ..."
  make -j1 install > install.log 2>&1 
  ERRORS=`grep -v "char \*\*\*" install.log | grep "\ \*\*\*\ "`
  if [ "${ERRORS}" != "" ]; then
    echo "ERROR: Errors occured during the installation. Check your install.log"
    echo "       Check the file "`pwd`"/install.log"
    exit 1;
  fi
else
  echo "ERROR: Errors occured during the compilation. Check your build.log"
  echo "       Check the file "`pwd`"/build.log"
  exit 1;
fi


# Create a libcfitsio.so, etc. link
cd ../*86*/lib
CFITSIO=`find . -name "libcfitsio.[so|a|dylib|dll]"`
LONGCFITSIO=`find . -name "libcfitsio_*[so|a|dylib|dll]"`
if ( [ "${CFITSIO}" == "" ] && [ "${LONGCFITSIO}" != "" ] ); then 
    NEWCFITSIO=`echo ${LONGCFITSIO} | awk -F'[/]|[.]|[_]' '{ print $3"."$6 }'`
    ln -s ${LONGCFITSIO} ${NEWCFITSIO}
fi



echo "Store our success story..."
cd ../..
rm -f COMPILE_SUCCESSFUL
echo "${CONFIGUREOPTIONS}" >> COMPILE_SUCCESSFUL
echo "${COMPILEROPTIONS}" >> COMPILE_SUCCESSFUL



echo "Setting permissions..."
cd ..
chown -R ${USER}:${GROUP} heasoft_v${VER}
chmod -R go+rX heasoft_v${VER}


if [ "${ENVFILE}" != "" ]; then
  echo "Storing the HEASoft directory in the MEGAlib source script..."
  echo "HEASoftDIR=`pwd`/heasoft_v${VER}" >> ${ENVFILE}
else
  setuphelp
fi


echo "Done!"
exit 0


