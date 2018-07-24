# Dockerfile for MEGAlib
# 
# Build the docker with:
# docker build -t Dockerfile

FROM ubuntu:18.04

MAINTAINER Andreas Zoglauer <zoglauer@berkeley.edu>

# Install all the MEGAlib, ROOT, and Geant4 prerequisites
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -yq vim nano less gzip git gawk dpkg-dev make g++ gcc gfortran gdb valgrind binutils libx11-dev libxpm-dev libxft-dev libxext-dev libssl-dev libpcre3-dev libglu1-mesa-dev libglew-dev libftgl-dev libmysqlclient-dev libfftw3-dev graphviz-dev libavahi-compat-libdnssd-dev libldap2-dev python-dev libxml2-dev libkrb5-dev libgsl-dev cmake libxmu-dev curl doxygen libblas-dev liblapack-dev expect dos2unix libncurses5-dev

# Add Mr. MEGAlib user
RUN useradd -ms /bin/bash mrmegalib

# Switch to Mr. MEGAlib
USER mrmegalib

# Setup MEGAlib
RUN cd /home/mrmegalib && git clone https://github.com/zoglauer/megalib.git MEGAlib 
RUN cd /home/mrmegalib/MEGAlib && /bin/bash setup.sh --release=dev --branch=experimental --clean=yes
RUN cd /home/mrmegalib && echo ". /home/mrmegalib/MEGAlib/bin/source-megalib.sh" >> ~/.bashrc

# Set a working directory
WORKDIR /home/mrmegalib

