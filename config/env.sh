# Set all MEGAlib-related environment variables

# Store command line
CMD=""
while [[ $# -gt 0 ]] ; do
    CMD="${CMD} $1"
    shift
done


# Default options:
NODEFAULT="!@#$%"
PREFIX=`pwd`
ROOTPATH=${NODEFAULT}
GEANT3PATH=${NODEFAULT}
CLHEPPATH=${NODEFAULT}
GEANT4PATH=${NODEFAULT}
HEADASPATH=${NODEFAULT}
MEGALIBPATH=${NODEFAULT}

# Overwrite default options with user options:
for C in ${CMD}; do
  if [[ ${C} == --p*=* ]]; then
    PREFIX=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --r*=* ]]; then
    ROOTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --g*3=* ]]; then
    GEANT3PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --g*4=* ]]; then
    GEANT4PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --h*=* ]]; then
    HEADASPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --c*=* ]]; then
    CLHEPPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == --m*=* ]]; then
    MEGALIBPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-h* ]]; then
    echo ""
    echo "Usage:"
    echo "  --prefix=<path>  prefix: path prefix to all non absolute paths"
    echo "  --root=<path>    path to ROOT"
    echo "  --geant4=<path>  path to Geant4"
    echo "  --clhep=<path>   path to CLHEP"
    echo "  --geant3=<path>  path to Geant3"
    echo "  --headas=<path>  path to HEADAS"
    echo "  --megalib=<path> path to MEGAlib"
    echo ""
  else
    echo ""
    echo "ERROR: Unknown command line option: ${C}"
    echo ""
    return
  fi
done

# Verify path:
if [[ "${PREFIX}" != */ ]]; then
  PREFIX="${PREFIX}/"
fi


# Set HEADAS:
if [ "${HEADASPATH}" != "${NODEFAULT}" ]; then
  if [[ "${HEADASPATH}" != /* ]]; then
    HEADASPATH="${PREFIX}${HEADASPATH}"
  fi

  if (test ! -d ${HEADASPATH}); then
    echo ""
    echo "ERROR: HEADAS directory not found: ${HEADASPATH}"
    echo ""
    return
  fi
  
  HEADASFOUND=false
  for i in `ls -d ${HEADASPATH}/*86*`; do
    if ( test -f ${i}/headas-init.sh ); then 
      export HEADAS=${i}
      alias heainit=". ${HEADAS}/headas-init.sh"
      source ${HEADAS}/headas-init.sh
      HEADASFOUND=true
    fi
  done

  if [ ${HEADASFOUND} == false ]; then
    echo ""
    echo "ERROR: HEADAS software not found in HEADAS directory"
    echo ""
    return
  fi
fi


# Set ROOT:
if [ "${ROOTPATH}" != "${NODEFAULT}" ]; then
  if [[ "${ROOTPATH}" != /* ]]; then
    ROOTPATH="${PREFIX}${ROOTPATH}"
  fi

  if (test ! -d ${ROOTPATH}); then
    echo ""
    echo "ERROR: Root directory not found: ${ROOTPATH}"
    echo ""
    return
  fi
   
  # Has to come before HEADAS since both have a libMinuit.so
  export ROOTSYS=${ROOTPATH}   
  export PATH=${ROOTSYS}/bin:${PATH}    
  export LD_LIBRARY_PATH=${ROOTSYS}/lib:${LD_LIBRARY_PATH}
  if [[ `uname -a` == *Darwin* ]]; then
    export DYLD_LIBRARY_PATH=${ROOTSYS}/lib:${LD_LIBRARY_PATH}
  fi
fi


# Set CLHEP:
if [ "${CLHEPPATH}" != "${NODEFAULT}" ]; then
  if [[ "${CLHEPPATH}" != /* ]]; then
    CLHEPPATH="${PREFIX}${CLHEPPATH}"
  fi

  if (test ! -d ${CLHEPPATH}); then
    echo ""
    echo "ERROR: CLHEP directory not found: ${CLHEPPATH}"
    echo ""
    return
  fi
   
  export CLHEP_BASE_DIR=${CLHEPPATH}
  export G4CLHEP_BASE_DIR=${CLHEPPATH}
  export CLHEP_INCLUDE_DIR=${CLHEPPATH}/include
  export CLHEP_LIB_DIR=${CLHEPPATH}/lib
  export LD_LIBRARY_PATH=${CLHEP_LIB_DIR}:${LD_LIBRARY_PATH}
  if [[ `uname -a` == *Darwin* ]]; then
    export DYLD_LIBRARY_PATH=${CLHEP_LIB_DIR}/lib:${LD_LIBRARY_PATH}
  fi
fi


# Set GEANT4:
if [ "${GEANT4PATH}" != "${NODEFAULT}" ]; then
  if [[ "${GEANT4PATH}" != /* ]]; then
    GEANT4PATH="${PREFIX}${GEANT4PATH}"
  fi

  if (test ! -d ${GEANT4PATH}); then
    echo ""
    echo "ERROR: GEANT4 directory not found: ${GEANT4PATH}"
    echo ""
    return
  fi
  
  if (test -f ${GEANT4PATH}/bin/geant4.sh); then
    source ${GEANT4PATH}/bin/geant4.sh > /dev/null  
    source ${GEANT4PATH}/share/Geant4-`${GEANT4PATH}/bin/geant4-config --version`/geant4make/geant4make.sh

    export LD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}:${LD_LIBRARY_PATH}
    if [[ `uname -a` == *Darwin* ]]; then
      export DYLD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}/lib:${LD_LIBRARY_PATH}
    fi

    export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1
  elif (test -f ${GEANT4PATH}/env.sh); then
    source ${GEANT4PATH}/env.sh > /dev/null  

    export LD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}:${LD_LIBRARY_PATH}
    if [[ `uname -a` == *Darwin* ]]; then
      export DYLD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}/lib:${LD_LIBRARY_PATH}
    fi

    export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1
  else
    echo " " 
    echo "WARNING: geant4.sh or env.sh not found in Geant4 directory: ${GEANT4PATH}/bin"
    echo "         Assuming we are currently installing Geant4 otherwise make sure to run \"./Configure\" (Geant4 < 9.5) or \"cmake install\" (Geant4 >= 9.5)!"
    echo ""
    G4INSTALL=${GEANT4PATH}
  fi 

fi


# Set GEANT3:
if [ "${GEANT3PATH}" != "${NODEFAULT}" ]; then
  if [[ "${GEANT3PATH}" != /* ]]; then
    GEANT3PATH="${PREFIX}${GEANT3PATH}"
  fi

  if (test ! -d ${GEANT3PATH}); then
    echo ""
    echo "ERROR: GEANT3 directory not found: ${GEANT3PATH}"
    echo ""
    return
  fi
   
  export CERN=${GEANT3PATH}
  export CERN_LEVEL=
  export CERN_ROOT=${GEANT3PATH}
  export LD_LIBRARY_PATH=${CERN}/lib:${LD_LIBRARY_PATH}
  export PATH=$CERN_ROOT/bin:$PATH   
fi


# Set MEGAlib
if [ "${MEGALIBPATH}" != "${NODEFAULT}" ]; then
  if [[ "${MEGALIBPATH}" != /* ]]; then
    MEGALIBPATH="${PREFIX}${MEGALIBPATH}"
  fi

  if (test ! -d ${MEGALIBPATH}); then
    echo ""
    echo "ERROR: MEGAlib directory not found: ${MEGALIBPATH}"
    echo ""
    return
  fi
   
  # Has to come before HEADAS since both have a libMinuit.so
  export MEGALIB=${MEGALIBPATH}   
  export PATH=${MEGALIB}/bin:${PATH}    
  export LD_LIBRARY_PATH=${MEGALIB}/lib:${LD_LIBRARY_PATH}
  if [[ `uname -a` == *Darwin* ]]; then
    export DYLD_LIBRARY_PATH=${MEGALIB}/lib:${LD_LIBRARY_PATH}
  fi

  alias m='cd ${MEGALIB}'
fi

