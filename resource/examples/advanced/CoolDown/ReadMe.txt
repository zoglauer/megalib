Example Cool Down
=================


*** Description:

This example shows how to simulate and analyze proton induced activation and the simulation of the subsequent cool down.


*** Geometry:

The geometry consists of 1 main parts, a LaBr3 cylinder which is hit by a proton beam.


*** Simulation:

In order to perform cool down simulation, two other simulations need to be perfromed first:

In Phase 1 a constant proton beam irradiates the LaBr3 cylinder. The task in this phase is to determine the which secondary isotopes are generated per time period. The relevant ouput file is "CoolDown.Phase1.BuildUp.dat" containing a list of the generated isotopes. The total observation time is not important, just the gathered statistics on the isotopes.

In Phase 2 the isotope files is used to determine the activation after a certain time (60 seconds) of constant radiation and subsequent cool down without measurement/simulation for (30 seconds)

In Phase 3 the cool down following the initial 30 seconds without measurement is simulated for 5000 seconds. 



*** Analysis:

The example comes with its own analysis program "CoolDown".

Compile it with:
make -f $MEGALIB/resource/standalone/Makefile.StandAlone PRG=CoolDown only

Run it with:
CoolDown -f CoolDown.Phase3.TheCoolDown.inc1.id1.sim -g LaBr3Sample.geo.setup

The displayed figures show the count rate as a function of time and as a function of time and energy.




