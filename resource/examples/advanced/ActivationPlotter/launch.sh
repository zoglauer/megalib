#! /bin/bash

# launch.sh
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
