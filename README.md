# MEGAlib - the Medium-Energy Gamma-ray Astronomy library

## Overview

The Medium Energy Gamma-ray Astronomy library (MEGAlib) is a set of software tools which are designed to simulate, calibrate, and analyze data of hard X-ray and gamma-ray detectors, with a specialization on Compton telescopes. While MEGAlib was originally developed for astrophysics, it has been expanded and used for ground based applications such as medical imaging and environmental monitoring. The library comprises all necessary data analysis steps for these telescopes, from simulation/measurements via calibration, event reconstruction to image reconstruction.

MEGAlib contains a geometry and detector description tool for the detailed modeling of different detector types and characteristics, and provides an easy to use simulation program based on Geant4. For different Compton telescope detector types (electron tracking, multiple Compton or time of flight based) specialized Compton event reconstruction algorithms are implemented in different approaches (Chi-square and Bayesian). The high level data analysis tools allow to calculate response matrices, perform image deconvolution (specialized in list-mode-likelihood-based Compton image reconstruction), determine detector resolutions and sensitivities, retrieve spectra, determine polarization modulations, etc.

For a more complete list of features and more documentation please visit the [MEGAlib's homepage](http://megalibtoolkit.com).



## Installation

# The short version of the MEGAlib setup

```
git clone https://github.com/zoglauer/megalib MEGAlib
cd MEGAlib
bash setup.sh
```

This script will first download, compile, and install MEGAlib's own version of ROOT and Geant4, and then compile MEGAlib. In the end it will create a script called "source-megalib.sh" in the MEGAlib bin directory, which you have to source in order to run MEGAlib

```
. bin\source-megalib.sh
```


# The long version of the MEGAlib setup

Please consult [MEGAlib's homepage](http://megalibtoolkit.com).