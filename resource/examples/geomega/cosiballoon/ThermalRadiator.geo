#Files defines the geometry of a single thermal radiator.
#According to Solidworks, each thermal radiator is 1.44 kg and here I find a weight of 1.76 kg, (this is probably mostly due to the cylindrial approximation I make down below...)

Include Intro.geo

Constant HalfWidthofRadiator {22.86/2}
Constant HalfThicknessRadiator 0.32

Volume ThermalRadiator
ThermalRadiator.Visibility 1
ThermalRadiator.Virtual True
ThermalRadiator.Material Vacuum
ThermalRadiator.Shape BRIK 12 3 12

Volume ThermalRadiatorWing
ThermalRadiatorWing.Visibility 1
ThermalRadiatorWing.Material al6061
ThermalRadiatorWing.Shape BRIK HalfWidthofRadiator 0.32 12
ThermalRadiatorWing.Mother ThermalRadiator
ThermalRadiatorWing.Position 0 0 0

#I've approximated the triangluar prism as a quarter-cylindrical prism cause it was easier to code in geomega. The final mass is pretty close, so this can be changed if/when fine-tuning the mass-model.

Volume ThermalRadiatorWedge
ThermalRadiatorWedge.Visibility 1
ThermalRadiatorWedge.Material al6061
ThermalRadiatorWedge.Shape TUBS 0 4 12 0 90
ThermalRadiatorWedge.Mother ThermalRadiator
ThermalRadiatorWedge.Position -HalfWidthofRadiator 0.32 0

