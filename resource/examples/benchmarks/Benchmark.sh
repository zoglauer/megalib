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

# For compatibility with macOS Catalina, compile a little C program...

TempEXE=$(mktemp)
TimerC="${TempEXE}.c"

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



# First run a single thread
checkload

echo "Running single core benchmark"
Start=$( ${TempEXE} )
cosima -z -v 0 -s 1 -t 0 Benchmark.source > /dev/null
Stop=$( ${TempEXE} )

Duration=$( echo "${Stop} - ${Start}" | bc -l )
EventsPerSecond=$( echo "100000 / (${Stop} - ${Start})" | bc -l )
echo "Duration single core: ${Duration} seconds"
echo "Events per second single core: ${EventsPerSecond} seconds"


# Then run all threads:
checkload

echo "Running multi core benchmark"
Processes=$( nproc )
Start=$( ${TempEXE} )
for i in $( seq 1 ${Processes} ); do
  cosima -z -v 0 -s $i -t $i Benchmark.source > /dev/null &
done
wait
Stop=$( ${TempEXE} )

Duration=$( echo "${Stop} - ${Start}" | bc -l )
EventsPerSecond=$( echo "100000 * ${Processes} / (${Stop} - ${Start})" | bc -l )
echo "Duration multi core: ${Duration} seconds"
echo "Events per second multi core: ${EventsPerSecond} seconds"

rm Benchmark*.sim.gz

rm ${TempEXE}
rm ${TimerC}

exit 0


