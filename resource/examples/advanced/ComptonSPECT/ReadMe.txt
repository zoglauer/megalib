Example Ge-SPECT with Phantom
=============================


*** Description:

This example shows how to simulate a medical SPECT device based on germanium detectors and how to convert phantom data into the Geomega format for fast simulations


*** Geometry:

The geometry consists of 2 main parts:
- A set of arrays of double-sided strip detectors which are arranged in a ring
- In the center of the ring a phantom is situated

Due to the complexity of the phantom, the geometry will take a minute to load...


*** Phantom conversion

The phantom has been created using the program ConvertPhantom.cxx on the Phantom.mhd input file

Compile:
> make -f $MEGALIB/resource/standalone/Makefile.StandAlone PRG=ConvertPhantom only
Run:
> ConvertPhantom -f Phantom.mhd

It convertes the binary format into the MEGAlib format with special optimizations to allow for fast simulations:
1. Thw whole phantom is contained in a box volume 
2. The box volume is built up of volume planes
3. The planes are build up of volume lines
4. The lines contain the individual voxels, where voxels wih identical content have been combined and empty voxels are skipped.

This setup greatly optimizes the volume navigation within Geant4 and therefore speeds up the simulation.

Thanks to Voichita Maxim for providing the original phantom files.


*** Simulation:

18F.source contains a single point source at location (-1, -3, 2) cm. For simplifications, not the decay of 18-F itself is simulated, but positrons with an energy of 250 keV (the mean energy of the decay) are emitted into 4pi.

Launch the simulation via:
> cosima 18F.source


*** Analysis:

You can either use the supplied CSPECT.revan.cfg and CSPECT.mimrec.cfg to reconstruct the data with revan and then look at the data with mimrec
Or you can use the program "Analyzer" to show you a quick look at the reconstructed image in 3D.

Compile:
> make -f $MEGALIB/resource/standalone/Makefile.StandAlone PRG=CSPECTAnalyzer only
Run:
> CSPECTAnalyzer -g CSPECT.geo.setup -s CSPECT.inc1.id1.sim -r CSPECT.revan.cfg -m CSPECT.mimrec.cfg

Since 3D image reconstructions are memory intensive only a 50x50x050 grid has been used. If you have enough memory you can try to increase the image size and to more iterations to further sharpen the image.

Projections of the resulting 3D image are shown in XY.png, YZ.png and XZ.png.

Attention:
The current version of MEGAlib is not capable of distinguishing multiple coincident events (i.e. detection of two 511 photons due to emission of two 511-photons in adjacent directions during annihilation). Those will be wrongly reconstructed as one 1022 event.


