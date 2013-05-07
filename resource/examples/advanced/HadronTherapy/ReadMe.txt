Example Hadron-Therapy run
==========================


*** Description:

This example shows how to simulate a germanium-based hadron therapy detector


*** Geometry:

The geometry consists of 2 main parts:
- A set of arrays of double-sided strip detectors which are arranged in a ring
- In the center of the ring a simple PMMA phantom is situated


*** Look at the origin of ALL secondary particles:

HadronTherapy.p100MeV.source simulates a 100 MeV proton beam hitting the 
PMMA phantom - it stores all events not just the triggered ones

Launch the simulation via:
> cosima HadronTherapy.p100MeV.source

You can use the program "HadronTherapy" to show you the origins of all secondary 
particles generated (which are not atoms).

Compile:
> make -f $MEGALIB/resource/standalone/Makefile.StandAlone PRG=HadronTherapy only
Run:
> HadronTherapy -g HadronTherapy.geo.setup -f Protons100MeV.inc1.id1.sim -p 0 0 0 10

You might get images such as in HadronTherapy.origins.png.


*** Reconstruct at the origin of the photons:

HadronTherapy.p150MeV.source simulates a 120 MeV proton beam hitting the 
PMMA phantom - it stores just the triggered events

Launch the simulation via:
> cosima HadronTherapy.p120MeV.source
If you have lots of core you might run it with mcosima, in order to get more data,
and thus a nicer image
> mcosima HadronTherapy.p120MeV.source

Reconstruct the events:
revan -c HadronTherapy.revan.cfg -g HadronTherapy.geo.setup -f Protons120MeV.p1.sim -a

Image the events:
mimrec -c HadronTherapy.mimrec.cfg -g HadronTherapy.geo.setup -f Protons120MeV.p1.tra
And press "play"

You will get three images such as HadronTherapy.imageXY.png, HadronTherapy.imageXZ.png, 
HadronTherapy.imageYZ.png





