#!/bin/bash


source OptimizerOptions.dat


setsources() {

if [ "${OnAxis}" = "true" ]; then
time="-t 1000000"
source="-p ${sourcedir}/PS_${energy}keV_0deg.inc1.id1.tra${gzipped} ${started} ${area}  0.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 0 1"
type="OnAxis"
else
time="-t 63113472"
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
--pphi 0 0 1 \
--ptheta 0 100 11"
type="AllSky"
fi


}



for energy in ${energies}; do
energywidth="0"
setsources

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 0.95 1.0 3 \
--arm 1.0 7.0 7 \
--egy ${energy} 1.0 `echo "0.025*${energy}" | bc -l` 10 \
--ehc 0 90 2 \
-w `echo "0.94*${energy}" | bc -l` `echo "1.06*${energy}" | bc -l` \
-n NarrowLine_${type}_${energy}.v${version} &

done



energy="511"
energywidth="1"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.5 2 \
--cqf 0.95 1.0 6 \
--arm 2.0 7.0 6 \
--egy ${energy} 1.0 10.0 9 \
--ehc 0 90 2 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


energy="847"
energywidth="14"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.5 2 \
--cqf 1.0 1.0 1 \
--arm 3.0 6.0 4 \
--egy ${energy} 2.0 10.0 9 \
--ehc 0 0 1 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))



energy="1157"
energywidth="4"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.75 3 \
--cqf 0.95 1.0 6 \
--arm 2.0 4.0 5 \
--egy ${energy} 2.0 15.0 14 \
--ehc 0 0 1 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))



energy="1157.04"
energywidth="0"
setsources

SensitivityOptimizer \
  ${time} \
  ${source} \
  ${backgroundall} \
  ${cfg} \
  ${geo} \
  --csl 2 2 7 \
  --phi 180 180 1 \
  --fdi 0.25 0.75 3 \
  --cqf 0.95 1.0 6 \
  --arm 2.0 4.0 5 \
  --egy ${energy} 1.0 5.0 9 \
  --ehc 0 0 1 \
  -n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))




energy="1173.2"
energywidth="0"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.5 2 \
--cqf 0.95 1.0 6 \
--arm 2.0 4.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))



SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.5 2 \
--cqf 0.95 1.0 6 \
--arm 2.0 4.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-w 1153 1193 \
-n NarrowLine_${type}_${energy}.wide.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))



energy="1332.5"
energywidth="0"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.50 2 \
--cqf 0.95 1.0 6 \
--arm 1.0 3.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.25 0.50 2 \
--cqf 0.95 1.0 6 \
--arm 1.0 3.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-w 1312 1352 \
-n NarrowLine_${type}_${energy}.wide.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))





energy="1808.7"
energywidth="0"
setsources

SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.5 0.5 1 \
--cqf 0.95 1 6 \
--arm 1.0 3.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-w 1759 1859 \
-n NarrowLine_${type}_${energy}.wide.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
${time} \
${source} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 180 180 1 \
--fdi 0.5 0.5 1 \
--cqf 0.95 1 6 \
--arm 1.0 3.0 5 \
--egy ${energy} 1.0 5.0 9 \
--ehc 0 0 1 \
-n NarrowLine_${type}_${energy}.v${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))





