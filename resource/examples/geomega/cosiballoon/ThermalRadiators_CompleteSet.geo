###File defines the set of thermal radiators, all four of them, that mount to the butt ends of the shields that face the sides of the total geometry. The geometry of an individual thermal radiator (sometimes referred to as thermal wings) is defined in ThermalRadiators.geo

Include Intro.geo

Include ThermalRadiator.geo
###Defines a single ThermalRadiator

Include CsIShield.geo
###Included here to get access to the constants defined within: recL, Zpos...


Volume ThermalRadiators_CompleteSet
ThermalRadiators_CompleteSet.Visibility 1
ThermalRadiators_CompleteSet.Virtual True
ThermalRadiators_CompleteSet.Material Vacuum
ThermalRadiators_CompleteSet.Shape BRIK 50 30 12

ThermalRadiator.Copy ThermalRadiator_1
ThermalRadiator_1.Mother ThermalRadiators_CompleteSet
ThermalRadiator_1.Position {recL+HalfWidthofRadiator} {recL+HalfThicknessRadiator} Zpos

ThermalRadiator.Copy ThermalRadiator_2
ThermalRadiator_2.Mother ThermalRadiators_CompleteSet
ThermalRadiator_2.Rotation 180 0 0
ThermalRadiator_2.Position {recL+HalfWidthofRadiator} {-recL-HalfThicknessRadiator} Zpos

ThermalRadiator.Copy ThermalRadiator_3
ThermalRadiator_3.Mother ThermalRadiators_CompleteSet
ThermalRadiator_3.Rotation 0 180 0
ThermalRadiator_3.Position {-recL-HalfWidthofRadiator} {recL+HalfThicknessRadiator} Zpos

ThermalRadiator.Copy ThermalRadiator_4
ThermalRadiator_4.Mother ThermalRadiators_CompleteSet
ThermalRadiator_4.Rotation 180 180 0
ThermalRadiator_4.Position {-recL-HalfWidthofRadiator} {-recL-HalfThicknessRadiator} Zpos
