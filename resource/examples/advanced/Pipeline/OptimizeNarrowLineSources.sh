#!/bin/bash

# Example script to run SensitivityOptimizer

# Part of the output files
version="1"

app="SensitivityOptimizer"

# Working directory
mydir="."
# Directory wioth all the source files
sourcedir="${mydir}/PointSources"
# Mimrec configuration file -- should be all open unless otherwise desired
cfg="-c AllOpen.cfg"
# The geometry
geo="-g ${MEGALIB}/resource/examples/geomega/special/Max.geo.setup"

# List of energies to use
energies="100 130 200 350 550 700 1000 1300 2000 3500 5000 7000 10000"

# Start are of the simulastions
area="20106.2"
# Number of simulated events
started="5000000"
# If the files are gzipped use this otehrwise set this to ""
gzipped=".gz"

# Observation time in seconds
time="-t 1000000"

# Should we look on-axis or all-sky scanning
OnAxis="false"

# Set the background files and the observations times in seconds
# Remember: concat tra files with mtraconcatter, and determine the time with "mtime *.inc*.tra.gz"
backgroundall=" \
-b ${mydir}/TrappedHadronicBackgroundDecay.p1.tra.gz  TIME \
-b ${mydir}/HadronicBackgroundDecay.p1.tra.gz         TIME \ 
-b ${mydir}/PhotonicBackground.p1.tra.gz              TIME \
-b ${mydir}/HadronicBackgroundPrompt.p1.tra.gz        TIME \
-b ${mydir}/LeptonicBackground.p1.tra.gz              TIME "


setsources() {


if [ "${OnAxis}" = "true" ]; then

source="-p ${sourcedir}/PS_${energy}keV_0deg.inc1.id1.tra${gzipped} ${started} ${area}  0.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 0 1"
type="OnAxis"

else

width=""
if [[ ${energywidth} != "0" ]]; then
  width="_${energywidth}keV"
fi

source=" \
-p ${sourcedir}/PS_${energy}keV${width}_0deg.inc1.id1.tra${gzipped} ${started} ${area}       0.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_10deg.inc1.id1.tra${gzipped} ${started} ${area}     10.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_20deg.inc1.id1.tra${gzipped} ${started} ${area}     20.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_30deg.inc1.id1.tra${gzipped} ${started} ${area}     30.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_40deg.inc1.id1.tra${gzipped} ${started} ${area}     40.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_50deg.inc1.id1.tra${gzipped} ${started} ${area}     50.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_60deg.inc1.id1.tra${gzipped} ${started} ${area}     60.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_70deg.inc1.id1.tra${gzipped} ${started} ${area}     70.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_80deg.inc1.id1.tra${gzipped} ${started} ${area}     80.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_90deg.inc1.id1.tra${gzipped} ${started} ${area}     90.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_100deg.inc1.id1.tra${gzipped} ${started} ${area}    100.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_100deg.inc1.id1.tra${gzipped} ${started} ${area}    110.0 0.0 \
-p ${sourcedir}/PS_${energy}keV${width}_100deg.inc1.id1.tra${gzipped} ${started} ${area}    120.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 120 13"

type="AllSky"

fi

}


for energy in ${energies}; do
energywidth="0"
setsources

mdelay ${app} $(( $(nproc) / 2 ))

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 2 \
--fdi 0.25 0.75 3 \
--cqf 1.0 1.0 1 \
--arm 1.0 9.0 17 \
--egy ${energy} 1.0 `echo "0.025*${energy}" | bc -l` 10 \
--ehc 0 90 2 \
-w `echo "0.94*${energy}" | bc -l` `echo "1.06*${energy}" | bc -l` \
-n NarrowLine_${type}_${energy}.v${version} &

done



energy="511"
energywidth="1"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 1.0 1.0 1 \
--arm 2.0 6.0 9 \
--egy ${energy} 1.0 20.0 19 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


energy="847"
energywidth="14"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 1.0 1.0 1 \
--arm 2.0 6.0 9 \
--egy ${energy} 2.0 20.0 19 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))



energy="1157"
energywidth="4"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 1 1 1 \
--arm 1.0 5.0 9 \
--egy ${energy} 2.0 20.0 19 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


energy="1173.2"
energywidth="0"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 1.0 1.0 1 \
--arm 1.0 5.0 9 \
--egy ${energy} 2.0 10.0 9 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


energy="1332.5"
energywidth="0"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 1.0 1.0 1 \
--arm 1.0 5.0 9 \
--egy ${energy} 2.0 10.0 9 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))



energy="1808.7"
energywidth="0"
setsources

${app} \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 2 \
--fdi 0.25 0.75 3 \
--cqf 1 1 1 \
--arm 1.0 5.0 9 \
--egy ${energy} 2.0 10.0 9 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))








