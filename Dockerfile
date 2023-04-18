# Dockerfile for MEGAlib
# 
# Build the docker with:
# docker build -t megalib-main - < Dockerfile

FROM ubuntu:22.04

MAINTAINER Andreas Zoglauer <zoglauer@berkeley.edu>

# Install all the MEGAlib, ROOT, and Geant4 prerequisites
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq gosu vim nano less gzip git gawk dpkg-dev make g++ gcc gfortran gdb valgrind binutils libx11-dev libxpm-dev libxft-dev libxext-dev libssl-dev libpcre3-dev libglu1-mesa-dev libglew-dev libftgl-dev libmysqlclient-dev libfftw3-dev libgraphviz-dev libavahi-compat-libdnssd-dev libldap2-dev python3 python3-pip python3-dev python3-tk python3-venv libxml2-dev libkrb5-dev libgsl-dev cmake libxmu-dev curl doxygen libblas-dev liblapack-dev expect dos2unix libncurses5-dev libboost-all-dev libcfitsio-dev mlocate libxerces-c-dev

# Add Mr. MEGAlib user
RUN groupadd -g 2468000 mrmegalib && useradd -u 2468000 -g 2468000 -ms /bin/bash mrmegalib

# Create ROOT directory
RUN mkdir /opt/MEGAlib && chown -R mrmegalib:mrmegalib /opt/MEGAlib

# Switch to Mr. MEGAlib
USER mrmegalib

# Setup MEGAlib
RUN cd /home/mrmegalib && git clone https://github.com/zoglauer/megalib.git MEGAlib && cd /home/mrmegalib/MEGAlib && /bin/bash setup.sh --ex=/opt/MEGAlib  --branch=main --clean=yes
RUN echo . /home/mrmegalib/MEGAlib/bin/source-megalib.sh >> /home/mrmegalib/.bashrc

# Switch back to ROOT
USER root

# Setup rootpy
RUN /bin/bash -c ". /home/mrmegalib/MEGAlib/bin/source-megalib.sh && pip3 install rootpy"

# Install tensorflow
RUN pip3 install tensorflow

# Create entry-point script - changes the UID of mrmegalib to the local USER and group for full access to the exchange directory on all machines
RUN    cd /usr/local/bin \
    && echo '#!/bin/bash' >> entrypoint.sh \
    && echo 'if [ "${USERID}" != "" ]; then usermod -u ${USERID} mrmegalib; fi' >> entrypoint.sh \
    && echo 'if [ "${GROUPID}" != "" ]; then groupmod -g ${GROUPID} mrmegalib; fi' >> entrypoint.sh \
    && echo 'if [ "${USERID}" != "" ] || [ "${GROUPID}" != "" ]; then chown -R mrmegalib:mrmegalib /home/mrmegalib; fi' >> entrypoint.sh \
    && echo 'gosu mrmegalib bash' >> entrypoint.sh \
    && chmod a+rx /usr/local/bin/entrypoint.sh

# The working directory is the home directory
WORKDIR /home/mrmegalib

ENTRYPOINT ["/usr/local/bin/entrypoint.sh"]
