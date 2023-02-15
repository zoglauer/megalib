# The automatic sensitivity calculator

# Kill this and all child processes on Ctrl-C
trap "pkill -P $$; exit 1;" SIGINT


# Setup parameters

# General
MAIN_DIR="$(pwd)"
MEGALIB_BRANCH="feature/dee2022"

# Mass model
MASSMODEL_DIR=${MAIN_DIR}/massmodel-cosi-smex-v11
MASSMODEL_SIMS=${MASSMODEL_DIR}/COSISMEX.geo.setup
MASSMODEL_ANALYSIS=${MASSMODEL_DIR}/COSISMEX.O64.geo.setup

# Background
BACKGROUND_DIR=${MAIN_DIR}/Background
BACKGROUND_GENERATOR_OPTIONS="-e 10 1000000000 -b 100 -o 575 6 12 -tps AverageTrappedProtonsElectronAP9Average.spenvis.txt -cps CosmicProtons.spenvis.txt -cas CosmicAlphas.spenvis.txt"
BACKGROUND_PHOTON_EVENTS=100000000
BACKGROUND_PHOTON_SIMS=1000
BACKGROUND_HADRON_EVENTS=100000
BACKGROUND_HADRON_SIMS=5000
BACKGROUND_LEPTON_EVENTS=1000000
BACKGROUND_LEPTON_SIMS=1000
BACKGROUND_TRAPPED_EVENTS=15000000
BACKGROUND_TRAPPED_SIMS=300
BACKGROUND_HADRONDECAY_EVENTS=5000000
BACKGROUND_HADRONDECAY_SIMS=400
BACKGROUND_TRAPPEDDECAY_EVENTS=5000000
BACKGROUND_TRAPPEDDECAY_SIMS=400

# Sources
PS_DIR=${MAIN_DIR}/PointSources
PS_ANGLES="0 10 20 30 40 50 60 70 80 90 100 110 120 130 140 150 160 170 180"
PS_ENERGIES="1808.7 1173.2 1332.5 100 130 200 350 500 550 700 1000 1300 2000 3500 5000 7000 10000"
PS_EVENTS=100000000

# Revan
RESULTS_DIR=${MAIN_DIR}/Analysis
REVAN_CFG="${MAIN_DIR}/COSISMEX.O64.Bayes.revan.cfg"

# Optimizer
OPTIMIZER_DIR=${MAIN_DIR}/Analysis
OPTIMIZER_CONTINUUM=""
OPTIMIZER_NARROWLINE=""
OPTIMIZER_CFG=""

# Overall
RUN_SOURCE="TRUE"
RUN_BACKGROUND="TRUE"
RUN_REVAN="TRUE"
RUN_OPTIMIZER="TRUE"
RUN_MIMREC="FALSE"

##################

# Sanity checks

# Are we in screen?
if [[ ${STY} == "" ]]; then
  echo "Sorry. I only let you run this multi-day script in screen"
  exit 1
fi

# Correct MEGAlib branch
if [[ $(cd $MEGALIB; git status | head -n 1 | awk '{ print $3 }') != $MEGALIB_BRANCH ]]; then
  echo "Sorry. You need to use MEGAlib branch $MEGALIB_BRANCH"
  exit 1
fi

# Geometries exist



# Run source sims
if [[ ${RUN_SOURCE} == "TRUE" ]]; then
  cd ${MAIN_DIR}
  if [[ ! -d ${PS_DIR} ]]; then
    mkdir ${PS_DIR}
  fi
  cd ${PS_DIR}
  cp ${MEGALIB}/resource/examples/advanced/Pipeline/SimulateSources.sh .
  cp ${MEGALIB}/resource/examples/advanced/Pipeline/Common.source .
  cp ${MEGALIB}/resource/examples/advanced/Pipeline/*Template* .

  rm -f SimulationParameters.dat
  echo "SourceDir=\"${PS_DIR}\"" >> SimulationParameters.dat 
  echo "PointSourceDir=\"${PS_DIR}\"" >> SimulationParameters.dat
  echo "Geometry=\"${MASSMODEL_SIMS}\"" >> SimulationParameters.dat
  echo "PointSourceThetas=\"${PS_ANGLES}\"" >> SimulationParameters.dat
  echo "PointSourceMonoEnergies=\"${PS_ENERGIES}\"" >> SimulationParameters.dat
  echo "SimulateBroadLines=true" >> SimulationParameters.dat
  echo "PointSourceContinuumStartEnergies=\"80 800\"" >> SimulationParameters.dat
  echo "EventsOrTriggers=\"Events\"" >> SimulationParameters.dat
  echo "Events=\"${PS_EVENTS}\"" >> SimulationParameters.dat

  bash SimulateSources.sh &

  # Sleep a while to make sure everything is started on this machine locally
  sleep 30
fi

# Run background sims
if [[ ${RUN_BACKGROUND} == "TRUE" ]]; then
  cd ${MAIN_DIR}
  if [[ ! -d ${BACKGROUND_DIR} ]]; then
    mkdir ${BACKGROUND_DIR}
  fi

  # Create the background
  cd ${MEGALIB}/resource/examples/advanced/Background 
  git clean -fx .
  make
  BackgroundGenerator ${BACKGROUND_GENERATOR_OPTIONS}
  cp *.source *.dat ${BACKGROUND_DIR}

  # Fix the source files
  cd ${BACKGROUND_DIR}

  sed -i "s|.*Geometry      .*|Geometry             ${MASSMODEL_SIMS}|" Common.partial.source
  sed -i "s|SpaceSim.Events.*|SpaceSim.Events    ${BACKGROUND_PHOTON_EVENTS}|" PhotonicComponents.source
  sed -i "s|SpaceSim.Events.*|SpaceSim.Events    ${BACKGROUND_LEPTON_EVENTS}|" LeptonicComponents.source
  sed -i "s|SpaceSim.Events.*|SpaceSim.Events    ${BACKGROUND_HADRON_EVENTS}|" HadronicComponentsPrompt.source
  sed -i "s|SpaceSim.Events.*|SpaceSim.Events    ${BACKGROUND_TRAPPED_EVENTS}|" TrappedHadronicComponentsPrompt.source
  sed -i "s|ActivationStep3.Events.*|ActivationStep3.Events    ${BACKGROUND_HADRONDECAY_EVENTS}|" HadronicComponentsDecay.source
  sed -i "s|ActivationStep3.Events.*|ActivationStep3.Events    ${BACKGROUND_TRAPPEDDECAY_EVENTS}|" TrappedHadronicComponentsDecay.source 

  dcosima -s=HadronicComponentsPrompt.source -z -n=HadronicComponentsPrompt -i=${BACKGROUND_HADRON_SIMS} &
  dcosima -s=TrappedHadronicComponentsPrompt.source -z -n=TrappedHadronicComponentsPrompt -i=${BACKGROUND_TRAPPED_SIMS} &
  dcosima -s=LeptonicComponents.source -z -n=LeptonicComponents -i=${BACKGROUND_LEPTON_SIMS} &
  dcosima -s=PhotonicComponents.source -z -n=PhotonicComponents -i=${BACKGROUND_PHOTON_SIMS} &

  wait

  cd $(ls -1tr | grep -v Trapped | grep HadronicComponentsPrompt_ID | tail -n1)
  rm -f HadronicBackgroundIsotopes.source
  for i in `ls HadronicBackgroundIsotopes.p1.inc*.dat`; do echo "A.IsotopeProductionFile $i" >> HadronicBackgroundIsotopes.source; done
  cp ../HadronicComponentsBuildUp.source .
  cosima HadronicComponentsBuildUp.source
  cp HadronicBackgroundActivation.dat ..
  cd ${BACKGROUND_DIR}

  cd $(ls -1tr | grep TrappedHadronicComponentsPrompt_ID | tail -n1)
  rm -f TrappedHadronicBackgroundIsotopes.source
  for i in `ls TrappedHadronicBackgroundIsotopes.p1.inc*.dat`; do echo "A.IsotopeProductionFile $i" >> TrappedHadronicBackgroundIsotopes.source; done
  cp ../TrappedHadronicComponentsBuildUp.source .
  cosima TrappedHadronicComponentsBuildUp.source
  cp TrappedHadronicBackgroundActivation.dat ..
  cd ${BACKGROUND_DIR}

  wait

  dcosima -s=HadronicComponentsDecay.source -z -n=HadronicComponentsDecay -i=${BACKGROUND_HADRONDECAY_SIMS} &
  dcosima -s=TrappedHadronicComponentsDecay.source -z -n=TrappedHadronicComponentsDecay -i=${BACKGROUND_TRAPPEDDECAY_SIMS} &

  wait

fi

# The inital source runs might not yet be finished.
wait



if [[ ${RUN_REVAN} == "TRUE" ]]; then
      
  cd ${MAIN_DIR}
  if [[ ! -d ${RESULTS_DIR} ]]; then
    mkdir ${RESULTS_DIR}
  fi

  SIM_DIRS="${PS_DIR}"
  for D in `ls -d ${BACKGROUND_DIR}/*_ID*`; do
    SIM_DIRS+=" ${D}"
  done

  for D in ${SIM_DIRS}; do
    echo "Starting revans in ${D}"
    cd ${D}
    rm *.tra*
    mwait -p=revan -m=threads
    mrevan -a -c ${REVAN_CFG} -g ${MASSMODEL_ANALYSIS} -f *id1.sim* &
    sleep 100
  done

  wait

  for D in ${SIM_DIRS}; do
    echo "Copying tra files from ${D} to ${RESULTS_DIR}"
    cd ${D}
    cp -r *.tra* ${RESULTS_DIR}
  done

fi



if [[ ${RUN_OPTIMIZER} == "TRUE" ]]; then

  cd ${RESULTS_DIR}

  echo "Preparing data for optimizer runs"

  cp ${MEGALIB}/resource/examples/advanced/Pipeline/OptimizerContinuum.sh .
  cp ${MEGALIB}/resource/examples/advanced/Pipeline/OptimizerMimrec.cfg .
  cp ${MEGALIB}/resource/examples/advanced/Pipeline/OptimizerNarrowLine.sh .

  echo "Getting leptonic observation time"
  TIME_LEPTONIC=$(mtime LeptonicBackground.p1.inc* | grep "Observation" | awk -F: '{ print $2 }' | xargs)
  echo "Getting photonic observation time"
  TIME_PHOTONIC=$(mtime PhotonicBackground.p1.inc* | grep "Observation" | awk -F: '{ print $2 }' | xargs)
  echo "Getting hadronic prompt observation time"
  TIME_HADRONIC_PROMPT=$(mtime HadronicBackground.p1.inc* | grep "Observation" | awk -F: '{ print $2 }' | xargs)
  echo "Getting hadronic decay observation time"
  TIME_HADRONIC_DECAY=$(mtime HadronicBackgroundDecay.p1.inc* | grep "Observation" | awk -F: '{ print $2 }' | xargs)
  echo "Getting trapped decay observation time"
  TIME_TRAPPED_DECAY=$(mtime TrappedHadronicBackgroundDecay.p1.inc* | grep "Observation" | awk -F: '{ print $2 }' | xargs)


 
  echo "Getting start area"
  FIRST=$(ls PS_* | head -n 1)
  FIRST=${FIRST/.tra/.sim}
  START_AREA=$(zcat ${PS_DIR}/${FIRST} | head -n 100 | grep "SimulationStartAreaFarField" | awk '{ print $2 }' | xargs )

  OPTI="OptimizerOptions.dat"
  if [ -f ${OPTI} ]; then
    rm ${OPTI}
  fi

  echo "# Optimizer options" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "version=\"1\"" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "sourcedir=\".\"" >> ${OPTI}
  echo "backgrounddir=\".\"" >> ${OPTI}
  echo "cfg=\"-c OptimizerMimrec.cfg\"" >> ${OPTI}
  echo "geo=\"-g ${MASSMODEL_ANALYSIS}\"" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "OnAxis=\"false\"" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "area=\"${START_AREA}\"" >> ${OPTI}
  echo "started=\"${PS_EVENTS}\"" >> ${OPTI}
  echo "gzipped=\".gz\"" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "energies=\"130 200 350 550 700 1000 1300 2000 3500 5000 7000 10000\"" >> ${OPTI}
  echo " " >> ${OPTI}
  echo "backgroundall=\"-b \${backgrounddir}/TrappedHadronicBackgroundDecay.p1.tra.gz ${TIME_TRAPPED_DECAY} -b \${backgrounddir}/HadronicBackgroundDecay.p1.tra.gz ${TIME_HADRONIC_DECAY} -b \${backgrounddir}/PhotonicBackground.p1.tra.gz ${TIME_PHOTONIC} -b \${backgrounddir}/HadronicBackground.p1.tra.gz ${TIME_HADRONIC_PROMPT} -b \${backgrounddir}/LeptonicBackground.p1.tra.gz ${TIME_LEPTONIC}\" " >> ${OPTI}
  echo " " >> ${OPTI}

  echo "Starting optimizer conitinuum"
  bash OptimizerContinuum.sh &
  echo "Getting optimizer narrow lines"
  bash OptimizerNarrowLine.sh &

  wait
fi

echo "Done"

wait

















