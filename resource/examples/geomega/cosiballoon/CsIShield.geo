###This file defines a single CsI shield, including the crystal, aluminum housing (with screw holes!) and PMTs.
###This was not edited from MassModel_0.9 -> MassModel_1.0. I'm not sure the original author, but Carolyn checked all dimensions and made comments where I found any descrepancies with physical shields

Include Intro.geo
Include PMT.geo
Include Shields.det

###distances between the shield housing and the crystal:
Constant Lgap 3.8169
Constant Wgap 0.58
Constant Hgap 2.1429
###distances defining the size of the shield housing:
Constant recL 23.8
Constant recH 12.0
Constant recW 2.55

Constant PMT1_offset -14.0 ###ck (2/14) should truly be 13.7, but this is close enough
Constant PMT2_offset -24.0 ###ck (2/14) should truly be 13.7 + 8.5 = 22.2, again, this is close enough

###distance between center of the Shield and center of DetectorHead:
Constant Zpos 1.1161

###defines the trapezoid shape of the crystal:
Constant wedgeoffset 0.93

Volume RectangularShieldModule
RectangularShieldModule.Visibility 1
RectangularShieldModule.Material Vacuum
RectangularShieldModule.Virtual true
RectangularShieldModule.Shape BRIK recL {10.*recW} recH

Volume RectangularShieldModule_hull
RectangularShieldModule_hull.Visibility 1
RectangularShieldModule_hull.Color 1
RectangularShieldModule_hull.Material al6061
RectangularShieldModule_hull.Shape BRIK recL recW recH
RectangularShieldModule_hull.Mother RectangularShieldModule

Volume RectangularShieldModule_CSI
RectangularShieldModule_CSI.Visibility 1
RectangularShieldModule_CSI.Color 4
RectangularShieldModule_CSI.Material CsI
RectangularShieldModule_CSI.Shape TRD1 {recH-Hgap} {recH-Hgap-wedgeoffset} {recL-Lgap} {recW-Wgap}
RectangularShieldModule_CSI.Mother RectangularShieldModule_hull
RectangularShieldModule_CSI.Position 0 0 0
RectangularShieldModule_CSI.Rotation 90 90 0


SinglePMT.Copy PMT_1
PMT_1.Visibility 1
PMT_1.Mother RectangularShieldModule
PMT_1.Position {recL+PMT1_offset} {recW + 8.25} 0
PMT_1.Rotation -90 0 0 

SinglePMT.Copy PMT_2
PMT_2.Visibility 1
PMT_2.Mother RectangularShieldModule
PMT_2.Position {recL+PMT2_offset} {recW + 8.25} 0
PMT_2.Rotation -90 0 0 



###Someone patient took the time to add all of the M8 holes machined in the housing. There are more holes now, but I haven't felt it would be super helpful to add them.

Volume M8_Hole
M8_Hole.Visibility 0
M8_Hole.Material Vacuum
M8_Hole.Shape TUBS 0 0.34 1.275 0 360

M8_Hole.Copy M8_Hole_Copy1
M8_Hole_Copy1.Mother RectangularShieldModule_hull
M8_Hole_Copy1.Rotation 0 90 0
M8_Hole_Copy1.Position {recL-1.275} {recW-1.5} {recH-3}

M8_Hole.Copy M8_Hole_Copy2
M8_Hole_Copy2.Mother RectangularShieldModule_hull
M8_Hole_Copy2.Rotation 0 90 0
M8_Hole_Copy2.Position {recL-1.275} {recW-3.7} {recH-3}

M8_Hole.Copy M8_Hole_Copy3
M8_Hole_Copy3.Mother RectangularShieldModule_hull
M8_Hole_Copy3.Rotation 0 90 0
M8_Hole_Copy3.Position {recL-1.275} {recW-1.5} {recH-12}

M8_Hole.Copy M8_Hole_Copy4
M8_Hole_Copy4.Mother RectangularShieldModule_hull
M8_Hole_Copy4.Rotation 0 90 0
M8_Hole_Copy4.Position {recL-1.275} {recW-3.7} {recH-12}

M8_Hole.Copy M8_Hole_Copy5
M8_Hole_Copy5.Mother RectangularShieldModule_hull
M8_Hole_Copy5.Rotation 0 90 0
M8_Hole_Copy5.Position {recL-1.275} {recW-1.5} {recH-21}

M8_Hole.Copy M8_Hole_Copy6
M8_Hole_Copy6.Mother RectangularShieldModule_hull
M8_Hole_Copy6.Rotation 0 90 0
M8_Hole_Copy6.Position {recL-1.275} {recW-3.7} {recH-21}

M8_Hole.Copy M8_Hole_Copy7
M8_Hole_Copy7.Mother RectangularShieldModule_hull
M8_Hole_Copy7.Rotation 0 90 0
M8_Hole_Copy7.Position {-recL+1.275} {recW-1.5} {recH-3}

M8_Hole.Copy M8_Hole_Copy8
M8_Hole_Copy8.Mother RectangularShieldModule_hull
M8_Hole_Copy8.Rotation 0 90 0
M8_Hole_Copy8.Position {-recL+1.275} {recW-3.7} {recH-3}

M8_Hole.Copy M8_Hole_Copy9
M8_Hole_Copy9.Mother RectangularShieldModule_hull
M8_Hole_Copy9.Rotation 0 90 0
M8_Hole_Copy9.Position {-recL+1.275} {recW-1.5} {recH-12}

M8_Hole.Copy M8_Hole_Copy10
M8_Hole_Copy10.Mother RectangularShieldModule_hull
M8_Hole_Copy10.Rotation 0 90 0
M8_Hole_Copy10.Position {-recL+1.275} {recW-3.7} {recH-12}

M8_Hole.Copy M8_Hole_Copy11
M8_Hole_Copy11.Mother RectangularShieldModule_hull
M8_Hole_Copy11.Rotation 0 90 0
M8_Hole_Copy11.Position {-recL+1.275} {recW-1.5} {recH-21}

M8_Hole.Copy M8_Hole_Copy12
M8_Hole_Copy12.Mother RectangularShieldModule_hull
M8_Hole_Copy12.Rotation 0 90 0
M8_Hole_Copy12.Position {-recL+1.275} {recW-3.7} {recH-21}

#Scintillator CsI_Shield
#CsI_Shield.DetectorVolume RectangularShieldModule_CSI
#CsI_Shield.SensitiveVolume RectangularShieldModule_CSI
#CsI_Shield.EnergyResolution   Gauss     30.0     30.0     5.00
#CsI_Shield.EnergyResolution   Gauss    100.0    100.0    15.25
#CsI_Shield.EnergyResolution   Gauss   1000.0   1000.0    48.62
#CsI_Shield.EnergyResolution   Gauss  10000.0  10000.0   425.00
#CsI_Shield.TriggerThreshold 80.0
#CsI_Shield.FailureRate 0.0
#
#Trigger CsI_Shield_Trigger
#CsI_Shield_Trigger.Veto true
#CsI_Shield_Trigger.TriggerByDetector True
#CsI_Shield_Trigger.Detector CsI_Shield 1
#### Carolyn added the following line as a result of geomega prompt, see below
#CsI_Shield.NoiseThresholdEqualsTriggerThreshold true

####Jan 13th, 2014:
#Errors reported:
#   ***  Info  ***  for detector CsI_Shield
#No noise threshold defined --- setting it to zero
#   ***  Error  ***  Triggers with vetoes
#A detector (here: CsI_Shield), which only has veto triggers, must have the flag "NoiseThresholdEqualsTriggerThreshold true"!"
###Therefore I added "CsI_Shield.NoiseThresholdEqualsTriggerThreshold true" above
