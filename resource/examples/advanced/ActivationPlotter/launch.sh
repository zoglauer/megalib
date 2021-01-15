#! /bin/bash
 
for S1 in `ls *Step1.source`; do
  mwait -p=cosima -i=cores
  echo "Launching ${S1}"
  cosima -z ${S1} > /dev/null &
done
wait
for S2 in `ls *Step2.source`; do
  mwait -p=cosima -i=cores
  echo "Launching ${S2}"
  cosima -z ${S2} > /dev/null &
done
wait
for S3 in `ls *Step3.source`; do
  mwait -p=cosima -i=cores
  echo "Launching ${S3}"
  cosima -z ${S3} > /dev/null &
done
wait
