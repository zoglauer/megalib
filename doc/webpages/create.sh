#!/bin/bash

# create.sh
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

if [[ ! -d neodym ]]; then
  git clone https://github.com/zoglauer/neodym.git
fi

if [[ ! -d templates ]]; then
  mkdir templates
  cd templates
  ln -s ../neodym/templates/neodym.css neodym.css
  ln -s ../neodym/templates/neodym.js neodym.js
  ln -s ../neodym/templates/neodym-pictureshow-dual.ss neodym-pictureshow-dual.css
  ln -s ../neodym/templates/neodym-pictureshow.js neodym-pictureshow.js
  cd ..
fi

if [[ ! -d python-env ]]; then
  python3 -m venv python-env
  source python-env/bin/activate
  pip install bibtexparser
  pip install bs4
else
  source python-env/bin/activate
fi

python3 neodym/neodym.py
