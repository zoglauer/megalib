#/bin/bash

checkload() {
  Load1Limit="0.9"
  while true; do
    Load1=$( uptime | awk -F: '{ print $NF }' | awk -F" " -F"," '{ print $1 }')
    if (( $(echo "${Load1} > ${Load1Limit}" | bc -l) )); then
      echo "Load 1 (${Load1}) above limit for benchmark (${Load1Limit})."
      echo "Please close down all programs using lots of load, such as your browser, mail client, virtual box, etc."
      echo "Waiting for 5 seconds."
      echo " "
      sleep 5
    else
      break
    fi
  done
}


# For compatibility with macOS Catalina, compile a little C++ program to get the time in milliseconds

TempEXE=$(mktemp)
TimerC="${TempEXE}.cpp"

cat > ${TimerC} <<'_EOF'
#include <chrono>
#include <iostream>
#include <iomanip>

int main() {
  using namespace std::chrono;
  milliseconds ms = duration_cast< milliseconds >(system_clock::now().time_since_epoch());
  std::cout<<std::fixed<<std::setprecision(3)<<(double) ms.count() / 1000.0<<std::endl;
  return 0;
}
_EOF

g++ ${TimerC} -o ${TempEXE}


# Collect information
if [[ $(uname -s) == Linux ]]; then
  OSFlavour=$( cat /etc/os-release | grep ^ID= | awk -F= '{print $2}' )
  OSVersion=$( cat /etc/os-release | grep ^VERSION_ID= | awk -F= '{print $2}' | tr -d '"' )
  Kernel=$( uname -r )

  CPUModel=$( lscpu | grep "Model name" | awk -F: '{print $2 }' | sed -e 's/^[[:space:]]*//' )
  CPUSockets=$( lscpu | grep "Socket" | awk -F: '{print $2 }' | sed -e 's/^[[:space:]]*//' )
  CPUCoresPerSocket=$( lscpu | grep "per socket" | awk -F: '{print $2 }' | sed -e 's/^[[:space:]]*//' )
  CPUThreadsPerCore=$( lscpu | grep "per core" | awk -F: '{print $2 }' | sed -e 's/^[[:space:]]*//' )
elif [[ $(uname -s) == Darwin ]]; then
  OSFlavour="macOS"
  OSVersion=$(sw_vers | grep ProductVersion | awk -F":" '{ print $2 }' | sed -e 's/^[[:space:]]*//')
  Kernel=$( uname -r )

  CPUModel=$( sysctl -n machdep.cpu.brand_string )
  CPUSockets=1
  CPUCoresPerSocket=$( sysctl -n hw.physicalcpu_max )
  CPUThreadsPerCore=$(( $(sysctl -n hw.ncpu) / $(sysctl -n hw.physicalcpu_max) ))
fi

Cores=$(( ${CPUSockets} * ${CPUCoresPerSocket} ))
Threads=$(( ${CPUSockets} * ${CPUCoresPerSocket} * ${CPUThreadsPerCore} ))

Geant4Version=$( geant4-config --version )
ROOTVersion=$( root-config --version )

MEGAlibVersion=$( cat ${MEGALIB}/config/Version.txt )
MEGAlibGitHash=$( git log -1 --pretty=format:"%H %cd" --date=short )
MEGAlibOptimization=$( cat ${MEGALIB}/config/Makefile.options | grep OPT | awk -F"OPT =" '{ print $2 }' | sed -e 's/^[[:space:]]*//' )

# Make nice short CPU name
CPUName=${CPUModel}
if [[ ${CPUName} == *Intel* ]]; then
  CPUName=${CPUName//Intel(R) Xeon(R)}
  CPUName=${CPUName//Intel(R) Core(TM)}
  CPUName=${CPUName//CPU}
  CPUName=$( echo "${CPUName}" | awk -F@ '{ print $1 }' )
elif [[ ${CPUName} == *Ryzen* ]]; then
  CPUName=${CPUName//AMD}
  CPUName=$( echo "${CPUName}" | awk -FCore '{ print $1 }' | awk '{ $NF=""; print}' )
fi
CPUName=$( echo -e "${CPUName}" | sed -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//' )


# Make sure we have enough RAM: We need 1.25 GB / Thread
if [[ $(uname -s) == Linux ]]; then
  RAM=$( cat /proc/meminfo | grep MemTotal | awk '{ print $2 }' )
elif [[ $(uname -s) == Darwin ]]; then
  RAM=$( sysctl -n hw.memsize )
  RAM=$( echo "${RAM} / 1024" | bc ) 
fi

RequiredRAM=$( echo " ${Threads} * 800000" | bc )

if (( ${RAM} < ${RequiredRAM} )); then
  echo "Error: Not enough RAM available on your machine: Required: ${RequiredRAM} vs. Available: ${RAM}"
  exit 1;
fi

checkload

CoolDownTime=20




# 1  COSIMA

# First run a single thread

echo " "
echo "Running single core COSIMA benchmark"
Start=$( ${TempEXE} )
cosima -z -v 0 -s 1 -f 1 Benchmark.source > /dev/null
Stop=$( ${TempEXE} )

DurationCosimaSingle=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondCosimaSingle=$( echo "scale=3; 100000 / (${Stop} - ${Start})" | bc -l )
echo "* Duration single core: ${DurationCosimaSingle} seconds"
echo "* Events per second single core: ${EventsPerSecondCosimaSingle} events/seconds"


# Then run all threads:

echo "Running multi core COSIMA benchmark"
Start=$( ${TempEXE} )
for i in $( seq 1 ${Threads} ); do
  cosima -z -v 0 -s $i -p 1 -f $i Benchmark.source > /dev/null &
done
wait
Stop=$( ${TempEXE} )

DurationCosimaMultiple=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondCosimaMultiple=$( echo "scale=3; 100000 * ${Threads} / (${Stop} - ${Start})" | bc -l )
echo "* Duration multi core: ${DurationCosimaMultiple} seconds"
echo "* Events per second multi core: ${EventsPerSecondCosimaMultiple} events/seconds"


echo " "
echo "Sleeping for ${CoolDownTime} seconds to let the CPU cool down a bit..."
sleep ${CoolDownTime}




# 2 REVAN

# First run a single thread

echo " "
echo "Running single core REVAN benchmark"
Start=$( ${TempEXE} )
revan -g Benchmark.geo.setup -f Benchmark.inc1.id1.sim.gz -c Benchmark.revan.cfg -a -n > /dev/null
Stop=$( ${TempEXE} )

DurationRevanSingle=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondRevanSingle=$( echo "scale=3; 100000 / (${Stop} - ${Start})" | bc -l )
echo "* Duration single core: ${DurationRevanSingle} seconds"
echo "* Events per second single core: ${EventsPerSecondRevanSingle} events/seconds"


# Then run all threads:

echo "Running multi core REVAN benchmark"
Start=$( ${TempEXE} )
for i in $( seq 1 ${Threads} ); do
  revan -g Benchmark.geo.setup -f Benchmark.p1.inc${i}.id1.sim.gz -c Benchmark.revan.cfg -a -n > /dev/null &
done
wait
Stop=$( ${TempEXE} )

DurationRevanMultiple=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondRevanMultiple=$( echo "scale=3; 100000 * ${Threads} / (${Stop} - ${Start})" | bc -l )
echo "* Duration multi core: ${DurationRevanMultiple} seconds"
echo "* Events per second multi core: ${EventsPerSecondRevanMultiple} events/seconds"


echo " "
echo "Sleeping for ${CoolDownTime} seconds to let the CPU cool down a bit..."
sleep ${CoolDownTime}




# 3 RESPONSECREATOR

# First run a single thread

echo " "
echo "Running single core RESPONSECREATOR benchmark"
Start=$( ${TempEXE} )
responsecreator -m s -o emin=10:emax=600:ebins=300:eskybins=100 -f Benchmark.inc1.id1.sim.gz -g Benchmark.geo.setup -c Benchmark.revan.cfg -b Benchmark.mimrec.cfg -r Benchmark.rsp  > /dev/null
Stop=$( ${TempEXE} )

DurationResponseCreatorSingle=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondResponseCreatorSingle=$( echo "scale=3; 100000 / (${Stop} - ${Start})" | bc -l )
echo "* Duration single core: ${DurationResponseCreatorSingle} seconds"
echo "* Events per second single core: ${EventsPerSecondResponseCreatorSingle} events/seconds"


# Then run all threads:

echo "Running multi core RESPONSECREATOR benchmark"
Start=$( ${TempEXE} )
for i in $( seq 1 ${Threads} ); do
  responsecreator -m s -o emin=10:emax=600:ebins=300:eskybins=100 -f Benchmark.p1.inc${i}.id1.sim.gz -g Benchmark.geo.setup -c Benchmark.revan.cfg -b Benchmark.mimrec.cfg -r Benchmark.rsp.${i}  > /dev/null &
done
wait
Stop=$( ${TempEXE} )

DurationResponseCreatorMultiple=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondResponseCreatorMultiple=$( echo "scale=3; 100000 * ${Threads} / (${Stop} - ${Start})" | bc -l )
echo "* Duration multi core: ${DurationResponseCreatorMultiple} seconds"
echo "* Events per second multi core: ${EventsPerSecondResponseCreatorMultiple} events/seconds"


rm Benchmark.rsp*


echo " "
echo "Sleeping for ${CoolDownTime} seconds to let the CPU cool down a bit..."
sleep ${CoolDownTime}




# 4 MIMREC imaging

# First run a single thread

echo " "
echo "Running single core MIMREC imaging benchmark"
Start=$( ${TempEXE} )
mimrec -g Benchmark.geo.setup -f Benchmark.inc1.id1.tra.gz -c Benchmark.mimrec.cfg -o Benchmark.jpg -i -n -C MemoryManagement.NThreads=1 > /dev/null
Stop=$( ${TempEXE} )

DurationMimrecImagingSingle=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondMimrecImagingSingle=$( echo "scale=3; 100000 / (${Stop} - ${Start})" | bc -l )
echo "* Duration single core: ${DurationMimrecImagingSingle} seconds"
echo "* Events per second single core: ${EventsPerSecondMimrecImagingSingle} events/seconds"


# Then run all threads:

echo "Running multi core MIMREC imaging benchmark"
Start=$( ${TempEXE} )
mimrec -g Benchmark.geo.setup -f Benchmark.inc1.id1.tra.gz -c Benchmark.mimrec.cfg -o Benchmark.jpg -i -n -C MemoryManagement.NThreads=${Threads} > /dev/null
Stop=$( ${TempEXE} )

DurationMimrecImagingMultiple=$( echo "scale=3; ${Stop} - ${Start}" | bc -l )
EventsPerSecondMimrecImagingMultiple=$( echo "scale=3; 100000 / (${Stop} - ${Start})" | bc -l )
echo "* Duration multi core: ${DurationMimrecImagingMultiple} seconds"
echo "* Events per second multi core: ${EventsPerSecondMimrecImagingMultiple} events/seconds"

rm Benchmark*.tra.gz
rm Benchmark.jpg




# FINALIZE

# Create the summary file
SummaryFile="${HOSTNAME}_$(date +%Y%m%d_%H%M%S).bm"
echo "# Benchmark summary file" > ${SummaryFile}
echo " " >> ${SummaryFile}
echo "HostName:                                  ${HOSTNAME}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "OSFlavour:                                 ${OSFlavour}" >> ${SummaryFile}
echo "OSVersion:                                 ${OSVersion}" >> ${SummaryFile}
echo "Kernel:                                    ${Kernel}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "CPUModel:                                  ${CPUModel}" >> ${SummaryFile}
echo "CPUName:                                   ${CPUName}" >> ${SummaryFile}
echo "CPUSockets:                                ${CPUSockets}" >> ${SummaryFile}
echo "CPUCoresPerSocket:                         ${CPUCoresPerSocket}" >> ${SummaryFile}
echo "CPUThreadsPerCore:                         ${CPUThreadsPerCore}" >> ${SummaryFile}
echo "Cores:                                     ${Cores}" >> ${SummaryFile}
echo "Threads:                                   ${Threads}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "ROOTVersion:                               ${ROOTVersion}" >> ${SummaryFile}
echo "Geant4Version:                             ${Geant4Version}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "MEGAlibVersion:                            ${MEGAlibVersion}" >> ${SummaryFile}
echo "MEGAlibGitHash:                            ${MEGAlibGitHash}" >> ${SummaryFile}
echo "MEGAlibOptimization:                       ${MEGAlibOptimization}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "# Cosima" >> ${SummaryFile}
echo "DurationCosimaSingle:                      ${DurationCosimaSingle}" >> ${SummaryFile}
echo "EventsPerSecondCosimaSingle:               ${EventsPerSecondCosimaSingle}" >> ${SummaryFile}
echo "DurationCosimaMultiple:                    ${DurationCosimaMultiple}" >> ${SummaryFile}
echo "EventsPerSecondCosimaMultiple:             ${EventsPerSecondCosimaMultiple}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "# Revan" >> ${SummaryFile}
echo "DurationRevanSingle:                       ${DurationRevanSingle}" >> ${SummaryFile}
echo "EventsPerSecondRevanSingle:                ${EventsPerSecondRevanSingle}" >> ${SummaryFile}
echo "DurationRevanMultiple:                     ${DurationRevanMultiple}" >> ${SummaryFile}
echo "EventsPerSecondRevanMultiple:              ${EventsPerSecondRevanMultiple}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "# ResponseCreator" >> ${SummaryFile}
echo "DurationResponseCreatorSingle:             ${DurationResponseCreatorSingle}" >> ${SummaryFile}
echo "EventsPerSecondResponseCreatorSingle:      ${EventsPerSecondResponseCreatorSingle}" >> ${SummaryFile}
echo "DurationResponseCreatorMultiple:           ${DurationResponseCreatorMultiple}" >> ${SummaryFile}
echo "EventsPerSecondResponseCreatorMultiple:    ${EventsPerSecondResponseCreatorMultiple}" >> ${SummaryFile}
echo " " >> ${SummaryFile}
echo "# MimrecImaging" >> ${SummaryFile}
echo "DurationMimrecImagingSingle:               ${DurationMimrecImagingSingle}" >> ${SummaryFile}
echo "EventsPerSecondMimrecImagingSingle:        ${EventsPerSecondMimrecImagingSingle}" >> ${SummaryFile}
echo "DurationMimrecImagingMultiple:             ${DurationMimrecImagingMultiple}" >> ${SummaryFile}
echo "EventsPerSecondMimrecImagingMultiple:      ${EventsPerSecondMimrecImagingMultiple}" >> ${SummaryFile}
echo " " >> ${SummaryFile}


rm Benchmark*.sim.gz

rm ${TempEXE}
rm ${TimerC}

echo " "
echo "Done"

exit 0
