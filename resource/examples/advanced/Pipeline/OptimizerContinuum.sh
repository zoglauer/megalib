#!/bin/bash

# OptimizerContinuum.sh
#
# Copyright (C) by the MEGAlib contributors.
#
# This file is part of MEGAlib.
#
# MEGAlib is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# MEGAlib is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
# License (License.md) for more details.
#
# SPDX-License-Identifier: LGPL-3.0-or-later


source OptimizerOptions.dat


if [ "${OnAxis}" = "true" ]; then

time="-t 1000000"
source="-k ${sourcedir}/PS_Continuum_80keV_0deg.inc1.id1.tra${gzipped}   ${started}   ${area}   2.0 80.0 100000   0.0 0.0 \
--pphi 0 0 1 \
--ptheta 0 0 1"
type="OnAxis"

else

time="-t 63113472"
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



mdelay SensitivityOptimizer $(( $(nproc) / 2 ))

SensitivityOptimizer \
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
-n Cont_${type}_100_300.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))



SensitivityOptimizer \
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
-n Cont_${type}_175_525.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_250_750.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_500_1500.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_1000_3000.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_2500_7500.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_5000_15000.${version} &

mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


SensitivityOptimizer \
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
-n Cont_${type}_10000_30000.${version} &



exit 0


mdelay SensitivityOptimizer $(( $(nproc) / 2 ))


