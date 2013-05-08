# Set all MEGAlib-related environment variables

# Store command line
MTMP_CMD=""
while [[ $# -gt 0 ]] ; do
    MTMP_CMD="${MTMP_CMD} $1"
    shift
done


# Default options:
MTMP_NODEFAULT="!@#$%"
MTMP_PREFIX=`pwd`
MTMP_ROOTPATH=${MTMP_NODEFAULT}
MTMP_GEANT3PATH=${MTMP_NODEFAULT}
MTMP_CLHEPPATH=${MTMP_NODEFAULT}
MTMP_GEANT4PATH=${MTMP_NODEFAULT}
MTMP_HEADASPATH=${MTMP_NODEFAULT}
MTMP_MEGALIBPATH=${MTMP_NODEFAULT}

# Overwrite default options with user options:
for C in ${MTMP_CMD}; do
  if [[ ${C} == *-p*=* ]]; then
    MTMP_PREFIX=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-r*=* ]]; then
    MTMP_ROOTPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-geant3=* ]]; then
    MTMP_GEANT3PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-g*=* ]]; then
    MTMP_GEANT4PATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-h*=* ]]; then
    MTMP_HEADASPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-c*=* ]]; then
    MTMP_CLHEPPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
  elif [[ ${C} == *-m*=* ]]; then
    MTMP_MEGALIBPATH=`echo ${C} | awk -F"=" '{ print $2 }'`
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
if [[ "${MTMP_PREFIX}" != */ ]]; then
  MTMP_PREFIX="${MTMP_PREFIX}/"
fi


# Set ROOT:
if [ "${MTMP_ROOTPATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_ROOTPATH}" != /* ]]; then
    MTMP_ROOTPATH="${MTMP_PREFIX}${MTMP_ROOTPATH}"
  fi

  if (test ! -d ${MTMP_ROOTPATH}); then
    echo ""
    echo "ERROR: Root directory not found: ${MTMP_ROOTPATH}"
    echo ""
    return
  fi

  if (test ! -f ${MTMP_ROOTPATH}/bin/thisroot.sh); then
    echo ""
    echo "ERROR: Root environment script not found: ${MTMP_ROOTPATH}/bin/thisroot.sh"
    echo ""
    return
  fi
   
  # Has to come before HEADAS since both have a libMinuit.so
  source ${MTMP_ROOTPATH}/bin/thisroot.sh
fi


# Set CLHEP:
if [ "${MTMP_CLHEPPATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_CLHEPPATH}" != /* ]]; then
    MTMP_CLHEPPATH="${MTMP_PREFIX}${MTMP_CLHEPPATH}"
  fi

  if (test ! -d ${MTMP_CLHEPPATH}); then
    echo ""
    echo "ERROR: CLHEP directory not found: ${MTMP_CLHEPPATH}"
    echo ""
    return
  fi
   
  export CLHEP_BASE_DIR=${MTMP_CLHEPPATH}
  export G4CLHEP_BASE_DIR=${MTMP_CLHEPPATH}
  export CLHEP_INCLUDE_DIR=${MTMP_CLHEPPATH}/include
  export CLHEP_LIB_DIR=${MTMP_CLHEPPATH}/lib
  export LD_LIBRARY_PATH=${CLHEP_LIB_DIR}:${LD_LIBRARY_PATH}
  if [[ `uname -a` == *Darwin* ]]; then
    export DYLD_LIBRARY_PATH=${CLHEP_LIB_DIR}/lib:${LD_LIBRARY_PATH}
  fi
fi


# Set GEANT4:
if [ "${MTMP_GEANT4PATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_GEANT4PATH}" != /* ]]; then
    MTMP_GEANT4PATH="${MTMP_PREFIX}${MTMP_GEANT4PATH}"
  fi

  if (test ! -d ${MTMP_GEANT4PATH}); then
    echo ""
    echo "ERROR: GEANT4 directory not found: ${MTMP_GEANT4PATH}"
    echo ""
    return
  fi

  if (test -f ${MTMP_GEANT4PATH}/bin/geant4.sh); then
    PATHTOGEANT4MAKE=${MTMP_GEANT4PATH}/share/Geant4-`${MTMP_GEANT4PATH}/bin/geant4-config --version`/geant4make
    MTMP_HERE=`pwd`
    cd ${PATHTOGEANT4MAKE}
    source geant4make.sh
    cd ${MTMP_HERE}

    if [[ `uname -a` == *Darwin* ]]; then
      export LD_LIBRARY_PATH=${G4LIB}/..:${LD_LIBRARY_PATH}
      #export DYLD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}/lib:${DYLD_LIBRARY_PATH}
    fi

    export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1
  elif (test -f ${MTMP_GEANT4PATH}/env.sh); then
    source ${MTMP_GEANT4PATH}/env.sh > /dev/null  

    export LD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}:${LD_LIBRARY_PATH}
    if [[ `uname -a` == *Darwin* ]]; then
      export DYLD_LIBRARY_PATH=${G4INSTALL}/lib/${G4SYSTEM}/lib:${LD_LIBRARY_PATH}
    fi

    export G4NEUTRONHP_USE_ONLY_PHOTONEVAPORATION=1
  else
    echo " " 
    echo "WARNING: geant4.sh or env.sh not found in Geant4 directory: ${MTMP_GEANT4PATH}/bin"
    echo "         Assuming we are currently installing Geant4 otherwise make sure to run \"./Configure\" (Geant4 < 9.5) or \"cmake install\" (Geant4 >= 9.5)!"
    echo ""
    G4INSTALL=${MTMP_GEANT4PATH}
  fi 

fi


# Set GEANT3:
if [ "${MTMP_GEANT3PATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_GEANT3PATH}" != /* ]]; then
    MTMP_GEANT3PATH="${MTMP_PREFIX}${MTMP_GEANT3PATH}"
  fi

  if (test ! -d ${MTMP_GEANT3PATH}); then
    echo ""
    echo "ERROR: GEANT3 directory not found: ${MTMP_GEANT3PATH}"
    echo ""
    return
  fi
   
  export CERN=${MTMP_GEANT3PATH}
  export CERN_LEVEL=
  export CERN_ROOT=${MTMP_GEANT3PATH}
  export LD_LIBRARY_PATH=${CERN}/lib:${LD_LIBRARY_PATH}
  export PATH=$CERN_ROOT/bin:$PATH   
fi


# Set MEGAlib
if [ "${MTMP_MEGALIBPATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_MEGALIBPATH}" != /* ]]; then
    MTMP_MEGALIBPATH="${MTMP_PREFIX}${MTMP_MEGALIBPATH}"
  fi

  if (test ! -d ${MTMP_MEGALIBPATH}); then
    echo ""
    echo "ERROR: MEGAlib directory not found: ${MTMP_MEGALIBPATH}"
    echo ""
    return
  fi
   
  # Has to come before HEADAS since both have a libMinuit.so
  export MEGALIB=${MTMP_MEGALIBPATH}   
  export PATH=${MEGALIB}/bin:${PATH}    
  export LD_LIBRARY_PATH=${MEGALIB}/lib:${LD_LIBRARY_PATH}
  if [[ `uname -a` == *Darwin* ]]; then
    export DYLD_LIBRARY_PATH=${MEGALIB}/lib:${LD_LIBRARY_PATH}
  fi

  alias m='cd ${MEGALIB}'
fi


# Set HEADAS:
if [ "${MTMP_HEADASPATH}" != "${MTMP_NODEFAULT}" ]; then
  if [[ "${MTMP_HEADASPATH}" != /* ]]; then
    MTMP_HEADASPATH="${MTMP_PREFIX}${MTMP_HEADASPATH}"
  fi

  if (test ! -d ${MTMP_HEADASPATH}); then
    echo ""
    echo "ERROR: HEADAS directory not found: ${MTMP_HEADASPATH}"
    echo ""
    return
  fi
  
  MTMP_HEADASFOUND=false
  MTMP_CFITSIOFOUND=false
  for i in `ls -d ${MTMP_HEADASPATH}/*86*`; do
    if ( test -f ${i}/headas-init.sh ); then 
      export HEADAS=${i}
      alias heainit=". ${HEADAS}/headas-init.sh"
      source ${HEADAS}/headas-init.sh
      MTMP_HEADASFOUND=true
    fi
    if [[ `uname -a` == *Linux* ]]; then
      if ( test -f ${i}/lib/libcfitsio.so ); then 
        MTMP_CFITSIOFOUND=true
      fi
    else
      # Too many installation options here - don't do the check...
      MTMP_CFITSIOFOUND=true
    fi
  done

  if [ ${MTMP_HEADASFOUND} == false ]; then
    echo ""
    echo "ERROR: HEADAS software not found in HEADAS directory"
    echo ""
    return
  fi
  if [ ${MTMP_CFITSIOFOUND} == false ]; then
    echo ""
    echo "WARNING: libcfitsio not found in the HEADAS library directory"
    echo "         You should make a link such as libcfitsio_3.XY.so -> libcfitsio.so"
    echo ""
  fi
fi



