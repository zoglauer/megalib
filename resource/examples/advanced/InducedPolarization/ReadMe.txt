Example Induced Polarization
============================


*** Description:

This example shows how to simulate and analyze induced Compton polarization using a Compton detector.


*** Geometry:

The geometry consists of 4 main parts:
- A silicon "polarizer" on which the incident photons are Compton scattered
- A lead collimator which blocks all photons from reaching the detector which are not scattered under 90 degrees (around 90 degrees is the Compton polarization maximum)
- A plastic Compton scatterer
- A set of CZT detectors which absorbs the scattered photons


*** Simulation:

In order to perform polarization analysis two simulations are necessary, one with enabled Compton polarization and one without in order to be able to correct for geometry effects such as pixelization, etc.

Thus perform the following runs:
cosima UnpolarizedBeamPolBeam.source
cosima UnpolarizedBeamNoPolSim.source

The only difference between the simulation files is that in the first case the physics list Livermore-Pol is used and in the second case only Livermore.

Since the induced polarization simulation is quite in efficient, you might use mcosima on a multi-core machine.

The simulation itself shoots 662 keV photons onto the "polarizer", and stores events which trigger the plastic detector as well as the CZT detector.



*** Analysis:

The example comes with its own analysis program "PolAnalyzer", since the default revan and mimrec programs cannot be used since the plastic scatterer has no position resolution and the event reconstruction would fail in most cases.

Compile it with:
make -f $MEGALIB/resource/standalone/Makefile.StandAlone PRG=PolAnalyzer only

Run it with:
PolAnalyzer -p UnpolarizedBeamPolSim.inc1.id1.sim -u UnpolarizedBeamNoPolSim.inc1.id1.sim -g Polarimeter.geo.setup

The displayed figure shows the geometry corrected polarization signature in the azimuthal scatter angle distribution.
An example output is shown in CorrectedPolarizationSignature.png



