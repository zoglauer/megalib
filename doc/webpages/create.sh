#!/bin/bash

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
  virtualenv -p python3 python-env
  source python-env/bin/activate
  pip install bibtexparser
  pip install bs4
else
  source python-env/bin/activate
fi

python3 neodym/neodym.py