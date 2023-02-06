#File defines the mechanical cryocooler, copper supports, and aluminum Cryostat_Cooler_Box (referred to colloquially as the "doghouse")

###Alex originally made a simple model of the cryocooler in 2012 (?), and he chose the inner radius of CryocoolerBody such that the total mass of the cryostat was 3.1 kg (mass without the collar, bellows or copper supports from the cryocooler manual). I've done the same here.

Include Intro.geo

Constant Cryocooler_axis_yoffset 1.116

Volume Cryocooler
Cryocooler.Visibility 0
Cryocooler.Material Vacuum
Cryocooler.Virtual True
Cryocooler.Shape BRIK 4.7 5.6 12.2
###Due to the TUBS shape, I thought it would be easier to draw the cryocooler so that z would be along the axis of rotation of the pump. - its +1.116cm off of the center axis of the BRIK

Volume CryocoolerBody
CryocoolerBody.Visibility 1
CryocoolerBody.Material msteel_18_8 
CryocoolerBody.Mother Cryocooler
CryocoolerBody.Color 4
CryocoolerBody.Shape TUBS 2.6 4.15 4.55 0 360 
###Inner radius used to be 3.309cm, from Alex's _0.9 model. I changed inner radius to get mass to agree with that reported in the cryocooler manual.
CryocoolerBody.Position 0 Cryocooler_axis_yoffset -7.6

Volume HeatRejectCollar
HeatRejectCollar.Visibility 1
HeatRejectCollar.Material mcopper
HeatRejectCollar.Mother Cryocooler
HeatRejectCollar.Color 5
HeatRejectCollar.Shape TUBS 2.61 4.4 1.485 0 360
HeatRejectCollar.Position 0 Cryocooler_axis_yoffset -1.565

###Because I am uncreative, I've just used the term "CryocoolerBody_#" to represent the different sections of the cooler that are permenantly attached. The numbers increase working from the main body up to the tip.

Volume CryocoolerBody_2
CryocoolerBody_2.Visibility 1
CryocoolerBody_2.Material msteel_18_8
CryocoolerBody_2.Mother Cryocooler
CryocoolerBody_2.Color 6
CryocoolerBody_2.Shape TUBS 0 2.61 1.485 0 360
CryocoolerBody_2.Position 0 Cryocooler_axis_yoffset -1.565


Volume CryocoolerBellows
CryocoolerBellows.Visibility 1
CryocoolerBellows.Material al6061 
####To improve this model, I should make this a lower density aluminum to better simulate the bellows
CryocoolerBellows.Mother Cryocooler
CryocoolerBellows.Shape TUBS 2.55 3.39 1.995 0 360
CryocoolerBellows.Position 0 Cryocooler_axis_yoffset 1.915

##I had this part wrong, cause I had the cryocooler tip going into the doghouse - so there were obviously overlaps there. Using the new geomega geomeotry overlap checker, i foudn that this pieve was 3.11 cm into the doghouse, so I just cut it that much shorter
Volume CryocoolerBody_3
CryocoolerBody_3.Visibility 1
CryocoolerBody_3.Material msteel_18_8
CryocoolerBody_3.Mother Cryocooler
CryocoolerBody_3.Color 7
CryocoolerBody_3.Shape TUBS 0 1.005 {3.55-3.11/2} 0 360
CryocoolerBody_3.Position 0 Cryocooler_axis_yoffset {3.47-3.11/2}

#The piece below belongs in the doghouse mother volume
#Volume CryocoolerCopperTip
#CryocoolerCopperTip.Visibility 1
#CryocoolerCopperTip.Material mcopper
#CryocoolerCopperTip.Mother Cryocooler
#CryocoolerCopperTip.Color 8
#CryocoolerCopperTip.Shape TUBS 0 2.1 0.3 0 360
#CryocoolerCopperTip.Position 0 Cryocooler_axis_yoffset 7.32


###The Cryocooler_Support* are those designed by Chris Smith, composed mostly of smashed together thin copper sheets. I've just modeled them as BRIKs of copper with the correct width and height.

Volume Cryocooler_Support_Wide
Cryocooler_Support_Wide.Visibility 1
Cryocooler_Support_Wide.Material mcopper
Cryocooler_Support_Wide.Mother Cryocooler
Cryocooler_Support_Wide.Color 4
Cryocooler_Support_Wide.Shape BRIK 4 1 0.65
Cryocooler_Support_Wide.Position 0 {-5.512+1} -1.565

Volume Cryocooler_Support_Narrow
Cryocooler_Support_Narrow.Visibility 1
Cryocooler_Support_Narrow.Material mcopper
Cryocooler_Support_Narrow.Color 4
Cryocooler_Support_Narrow.Shape BRIK 1 1 0.65

Cryocooler_Support_Narrow.Copy Cryocooler_Support_Narrow_1
Cryocooler_Support_Narrow_1.Mother Cryocooler
Cryocooler_Support_Narrow_1.Position 3 {-5.512+1} -9

Cryocooler_Support_Narrow.Copy Cryocooler_Support_Narrow_2
Cryocooler_Support_Narrow_2.Mother Cryocooler
Cryocooler_Support_Narrow_2.Position -3 {-5.512+1} -9



### Cryostat_Cooler_Box is the "doghouse". The aluminum extension to the cryostat within which the cryocooler tip is located.
Include Cryostat_Cooler_Box.geo
Cryostat_Cooler_Box.Position 0 0 8.025 
Cryostat_Cooler_Box.Mother Cryocooler

