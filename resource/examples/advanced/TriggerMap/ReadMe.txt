Example illustrating the creation and use of trigger maps

The underlying geometry is a simple electron tracker, in which Compton scattering or pair creation happens, a calorimeter on the bottom, as well as a simple segmented anti-coincidence system. Volumes and detectors are defined in Base.geo.setup, the triggers in TriggerMap.geo.setup - this file then in turn includes Base.geo.setup.


1) Create the trigger maps

Take a look in the file CreateTriggerMap.cxx to see an easy way to create the trigger maps for an existsing geometry file.
To create the trigger maps, execute:
>> make -f ${MEGALIB}/resource/standalone/Makefile.StandAlone PRG=CreateTriggerMap
and then
>> CreateTriggerMap -g Base.geo.setup

This will create several trigger maps:
a) OneSiteTriggers.trig - vetoable trigger: at least one hit in tracker and at least one hit in the calorimeter
b) TwoSiteTriggers.trig - vetoable trigger: at least two hits in successive tracker layers and at least one hit in the calorimeter
c) GuardRingVeto.trig - veto trigger: one hit in the guard ring
d) ACSVeto.trig - veto trigger: any hit in the anticoincidence system
e) PairTrigger.trig - non-vetoable trigger: at least 6 hits in successive tracker layers and at least 6 hits in the calorimeter, no hit in the top ACS or the first tracker layer
f) Combined.trig - combined the last 4 in one file

Remarks:
While it is straight forward to combine trigger criteria b)-e) in one file f), this is not recommended since this will not allow any diagnostics on how many events have been actually vetoed.
Finally, it is also possible to move the veto c) & d) into b) by requiring exactly 0 hits in the guard rings and the anti-coincidence system, this is also not recommended, since it also hides the number of vetoes events - the vetoes events are then just not triggered...


2) Geometry and simulations

Before simulating the example take a look at the end of the geometry to see how the new trigger maps are called.

Then just run the simulations:
>> cosima TriggerMap.source

This will generate two sim files, one for lower energies (TriggerMapLowSource.inc1.id1.sim), and one for higher energies (TriggerMapHighSource.inc1.id1.sim).
For both you can take a look at the trigger statistics via revan:

>> revan -f TriggerMapHighSource.inc1.id1.sim -g TriggerMap.geo.setup
And the click Analysis -> Show trigger statistics

Trigger statistics:
  Not triggered events: ..................................      0
  Number of vetoed events: ...............................     36
    Raised vetoes (multiples possible)
      ACSVeto: ...........................................     36
      GuardRingVeto: .....................................      8
    Raised triggers canceled by a veto (multiples possible)
      TwoSiteTriggers: ...................................     33
  Number of triggered events: ............................     57
    Raised triggers (multiples possible)
      PairTrigger: .......................................     57
      TwoSiteTriggers: ...................................     57


As homework, change the TN (non-vetoable trigger) to TV (vetoable trigger) in PairTrigger.trig, to determine the amount of self-vetoed pair events for the sim file TriggerMapHighSource.inc1.id1.sim.

Trigger statistics:
  Not triggered events: ..................................      0
  Number of vetoed events: ...............................     91
    Raised vetoes (multiples possible)
      ACSVeto: ...........................................     91
      GuardRingVeto: .....................................     16
    Raised triggers canceled by a veto (multiples possible)
      PairTrigger: .......................................     55
      TwoSiteTriggers: ...................................     88
  Number of triggered events: ............................      2
    Raised triggers (multiples possible)
      PairTrigger: .......................................      2
      TwoSiteTriggers: ...................................      2

In this case 55/57 events are self vetoed...      
      
      