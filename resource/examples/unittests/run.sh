#!/bin/bash

make -f ${MEGALIB}/resource/standalone/Makefile.StandAlone PRG=CompareHistograms only

cd shells
make -f ${MEGALIB}/resource/standalone/Makefile.StandAlone PRG=Shells only
bash run.sh
cd ..