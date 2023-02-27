#!/bin/bash

docker run --rm -it \
    --volume /Users/andreas/Documents/Science/Software/COSItools/megalib/doc/Papers:/data \
    --user $(id -u):$(id -g) \
    --env JOURNAL=joss \
    openjournals/inara fretalon.md
