Example on how to create an individualized cosima output
========================================================


*** Description:

This example shows how to modify the main cosima file "MCCosima.cc" in order to individualize the cosima output or, generally, to do anything you want with the freshly simulated event


*** Geometry:

The geometry is just a large world consisting out of air and a black aborber as ground.
Photons are started from 0/0/0 isotropically into the upper hemisphere


*** Modifications

In order to write your individualized cosima output or do whatever else you want to do with your newly generated sim event you need to do the following modifications (as can be seen in ModifiedCosima.cxx):
1. 
Define a (global) function and give it to the main cosima object ("MCMain" in "MCCosima")
g_Main->SetEventRelegator(Relegator);
When ever a new event is created, this function will be called with a pointer of the new event. Do whatever you want with it, but don't delete it.
2. 
If, for example, you want dump your own individualized sim file, you should also comment out the "FileName" directive in the source file.

Take a look at "ModofiedCosima.cxx" and "AirWorld.source" for an example


*** Compilation, simulation, analysis:

If you execute "make" in this directory, two programs will be built: modifiedcosima and quickparse.
Executing
> modifiedcosima AirWorld.source 
will start a small simulation run. It will generate an RANDOM, BINARY file named AirWorld_Run_XYZ.bsim.
Look at the output via:
> quickparse AirWorld_Run_XYZ.bsim




