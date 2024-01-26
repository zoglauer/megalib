#!/bin/bash


# Step 0: Setup

# Some sanity checks:
if [[ ! -f SimulationParameters.dat ]]; then
  echo "ERROR: Configuration file \"SimulationParameters.dat\" not found"
  exit 1
fi

# Source the steering file
source SimulationParameters.dat

# Some sanity checks:
if [[ ${PointSourceDir} == "" ]]; then
  echo "ERROR: No point source directory given where we will store the data"
  exit 1
fi

if [[ ${Geometry} == "" ]] || [[ ! -f ${Geometry} ]]; then
  echo "ERROR: No geometry file found: \"${Geometry}\""
  exit 1
fi


# The templates should also be the same:
PointSourceMonoTemplate=${MEGALIB}/resource/examples/advanced/Pipeline/PointSourceMonoTemplate.source
PointSourceGaussTemplate=${MEGALIB}/resource/examples/advanced/Pipeline/PointSourceGaussTemplate.source
PointSourceContinuumTemplate=${MEGALIB}/resource/examples/advanced/Pipeline/PointSourceContinuumTemplate.source
PointSourceContinuumPolarizedTemplate=${MEGALIB}/resource/examples/advanced/Pipeline/PointSourceContinuumPolarizedTemplate.source


THREADS=1
NCPUS=`grep ^cpu\\scores /proc/cpuinfo | uniq |  awk '{print $4}'`

# Create the directory where to store the simulation file
if [ ! -d "${PointSourceDir}" ]; then
  mkdir ${PointSourceDir}
fi

cd ${PointSourceDir}

# Step 1: Create sopurce files

AllSourceFiles=""

# Create the continuum source files first - those take longest to simulate
for e in ${PointSourceContinuumStartEnergies}; do
  for t in ${PointSourceThetas}; do
    SourceFile="PS_Continuum_${e}keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%START%%|${e}|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceContinuumTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"

    SourceFile="PS_ContinuumPolarized_${e}keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%START%%|${e}|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceContinuumPolarizedTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"
  done
done


# Create the mono energetic sim files
for e in ${PointSourceMonoEnergies}; do
  for t in ${PointSourceThetas}; do
    SourceFile="PS_${e}keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%ENERGY%%|${e}|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceMonoTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"
  done
done

# Create the broad line sim files
if [[ ${SimulateBroadLines} == true ]]; then
  for t in ${PointSourceThetas}; do
    SourceFile="PS_1157keV_4keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%ENERGY%%|1157|g" -e "s|%%SIGMA%%|4|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceGaussTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"
    
    SourceFile="PS_847keV_14keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%ENERGY%%|847|g" -e "s|%%SIGMA%%|14|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceGaussTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"
    
    SourceFile="PS_511keV_1keV_${t}deg.source"
    rm -f ${SourceFile}
    sed -e "s|%%THETA%%|${t}|g" -e "s|%%ENERGY%%|511|g" -e "s|%%SIGMA%%|1|g" -e "s|%%EVENTSORTRIGGERS%%|${EventsOrTriggers}|g" -e "s|%%EVENTS%%|${Events}|g" -e "s|%%GEOMETRY%%|${Geometry}|g" < ${PointSourceGaussTemplate} > ${SourceFile}
    AllSourceFiles+=" ${SourceFile}"
  done
fi


# Step 2: Perform simulations

for a in ${AllSourceFiles}; do

  mwait --instances=threads --load=threads -p=cosima
  echo "Starting to simulate ${a}"
  cosima -z ${a} > /dev/null &
  sleep 1
  
done

# Wait for ALL simulations to finish
echo "Waiting till all simulations have finished"
wait



# Step 3: Reconstruct all data

if [[ -f ${RevanConfiguration} ]]; then
  echo "Starting parallel event reconstruction"
  mrevan -c ${RevanConfiguration} -g ${Geometry} -f *.sim.gz
fi

echo "DONE!"

