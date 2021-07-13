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



if [ "${OnAxis}" = "true" ]; then

source="-k ${sourcedir}/PS_Continuum_80keV_0deg.inc1.id1.tra${gzipped}   ${started}   ${area}   2.0 80.0 100000   0.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 0 1"
type="OnAxis"

else

sourcelow=" \
-k ${sourcedir}/PS_Continuum_80keV_0deg.inc1.id1.tra${gzipped}   ${started}   ${area}   2.0 80.0 100000   0.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_10deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  10.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_20deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  20.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_30deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  30.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_40deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  40.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_50deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  50.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_60deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  60.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_70deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  70.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_80deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  80.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_90deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 80.0 100000  90.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_100deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 80.0 100000 100.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_110deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 80.0 100000 110.0 0.0 \
-k ${sourcedir}/PS_Continuum_80keV_120deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 80.0 100000 120.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 120 13"

sourcehigh=" \
  -k ${sourcedir}/PS_Continuum_800keV_0deg.inc1.id1.tra${gzipped}   ${started}   ${area}   2.0 800.0 100000   0.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_10deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  10.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_20deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  20.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_30deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  30.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_40deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  40.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_50deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  50.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_60deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  60.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_70deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  70.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_80deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  80.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_90deg.inc1.id1.tra${gzipped}  ${started}   ${area}   2.0 800.0 100000  90.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_100deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 800.0 100000 100.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_110deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 800.0 100000 110.0 0.0 \
  -k ${sourcedir}/PS_Continuum_800keV_120deg.inc1.id1.tra${gzipped} ${started}   ${area}   2.0 800.0 100000 120.0 0.0 \
  --pphi 0 0 1 \
  --ptheta 0 120 13"


type="AllSky"
fi


${app} \
${time} \
${sourcelow} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 4.0 10.0 7 \
--contegy 100 300 \
--ehc 0 0 1 \
-n Cont_${type}_100_300.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))



${app} \
${time} \
${sourcelow} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 3.0 10.0 8 \
--contegy 175 525 \
--ehc 0 0 1 \
-n Cont_${type}_175_525.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcelow} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 3.0 10.0 8 \
--contegy 250 750 \
--ehc 0 0 1 \
-n Cont_${type}_250_750.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcelow} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 2.0 10.0 9 \
--contegy 500 1500 \
--ehc 0 0 1 \
-n Cont_${type}_500_1500.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcehigh} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 1.0 8.0 8 \
--contegy 1000 3000 \
--ehc 0 0 1 \
-n Cont_${type}_1000_3000.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcehigh} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.0 4 \
--cqf 0.25 1.75 3 \
--arm 1.0 8.0 8 \
--contegy 2500 7500 \
--ehc 0 0 1 \
-n Cont_${type}_2500_7500.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcehigh} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 20 100 5 \
--fdi 0.4 1.6 4 \
--cqf 0.25 1.75 3 \
--arm 2.0 6.0 5 \
--contegy 5000 15000 \
--ehc 0 0 1 \
-n Cont_${type}_5000_15000.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


${app} \
${time} \
${sourcehigh} \
${backgroundall} \
${cfg} \
${geo} \
--csl 2 2 7 \
--phi 90 180 3 \
--fdi 0.4 1.6 4 \
--cqf 0.25 1.75 3 \
--arm 1.0 8.0 8 \
--contegy 10000 30000 \
--ehc 0 0 1 \
-n Cont_${type}_10000_30000.v${version} &

mdelay ${app} $(( $(nproc) / 2 ))


