#This file descibes the "doghouse", or the extension to the cryostat within which the cryocooler tip is located.
#Mass of this box according to Solidworks is:______. This model is 367 g.

Include Intro.geo

Constant Cryocooler_axis_yoffset 1.116
###The above constant is defined in Cryocooler.geo

Constant CryocoolerBox_WallThickness 0.8890
Constant CryocoolerBox_RibThickness 0.127

Volume Cryostat_Cooler_Box
Cryostat_Cooler_Box.Visibility 0
Cryostat_Cooler_Box.Virtual True
Cryostat_Cooler_Box.Material Vacuum
Cryostat_Cooler_Box.Shape BRIK 4.6 5.6 4.2

Volume CryoBoxOutside
CryoBoxOutside.Visibility 1
CryoBoxOutside.Material al6061
CryoBoxOutside.Shape BRIK 4.535 {5.512+CryocoolerBox_RibThickness/2} 4.115
CryoBoxOutside.Position 0 0 0 
CryoBoxOutside.Mother Cryostat_Cooler_Box


###The hole that the cryocooler tip goes through...
Volume CryoBoxThruHole
CryoBoxThruHole.Visibility 1
CryoBoxThruHole.Material Vacuum
CryoBoxThruHole.Shape TUBS 0 2.805 {CryocoolerBox_WallThickness/2} 0 360
CryoBoxThruHole.Position 0 Cryocooler_axis_yoffset {-4.115+CryocoolerBox_WallThickness/2} 
CryoBoxThruHole.Mother CryoBoxOutside


###The hole that goes to the rest of the cryostat...
Volume CryoBoxSideCutOut
CryoBoxSideCutOut.Visibility 1
CryoBoxSideCutOut.Material Vacuum
CryoBoxSideCutOut.Shape BRIK 0.95 1.1 0.381


###Similar to the cryostat, the surface of this box is highly cutout. I've included a lot of cutouts here of approximately the correct dimensions.
For X 4 {-3*0.95-3*CryocoolerBox_RibThickness/2} {0.95*2+CryocoolerBox_RibThickness}
  For Y 4 {-3*1.1-3*CryocoolerBox_RibThickness/2} {1.1*2+CryocoolerBox_RibThickness}
    CryoBoxSideCutOut.Copy CryoBoxSideCutOut_%X%Y
    CryoBoxSideCutOut_%X%Y.Position $X $Y {4.115-0.381}
    CryoBoxSideCutOut_%X%Y.Mother CryoBoxOutside
    CryoBoxSideCutOut_%X%Y.Visibility 1
  Done
Done

For X 4 {-3*0.95-3*CryocoolerBox_RibThickness/2} {0.95*2+CryocoolerBox_RibThickness}
  CryoBoxSideCutOut.Copy CryoBoxSideBCutOut_%X
  CryoBoxSideBCutOut_%X.Position $X {-3*1.1-3*CryocoolerBox_RibThickness/2} {-4.115+0.381}
  CryoBoxSideBCutOut_%X.Mother CryoBoxOutside
  CryoBoxSideBCutOut_%X.Visibility 1
Done

Volume CryoBoxTopBottCutOut
CryoBoxTopBottCutOut.Visibility 1
CryoBoxTopBottCutOut.Material Vacuum
CryoBoxTopBottCutOut.Shape BRIK 0.95 0.381 1.5

For X 4 {-3*0.95-3*CryocoolerBox_RibThickness/2} {0.95*2+CryocoolerBox_RibThickness}
  For Z 2 {-1.5-CryocoolerBox_RibThickness/2} {1.5*2+CryocoolerBox_RibThickness}
    For Y 2 {5.512-0.381} {-(5.512-0.381)*2}
      CryoBoxTopBottCutOut.Copy CryoBoxTopBottCutOut_%X%Y%Z
      CryoBoxTopBottCutOut_%X%Y%Z.Position $X $Y $Z
      CryoBoxTopBottCutOut_%X%Y%Z.Mother CryoBoxOutside
      CryoBoxTopBottCutOut_%X%Y%Z.Visibility 1
    Done
  Done
Done


Volume CryoBoxInside
CryoBoxInside.Visibility 1
CryoBoxInside.Material Vacuum
CryoBoxInside.Shape BRIK 4.535 {5.512-CryocoolerBox_WallThickness} {4.115-CryocoolerBox_WallThickness}
#CryoBoxInside.Position {-CryocoolerBox_RibThickness/2} 0 0
CryoBoxInside.Position 0 0 0 
CryoBoxInside.Mother CryoBoxOutside



###The coldfinger extension starts in about the center of the cryostat (bolted to the coldfinger) and extends into the doghouse attaching to the cryocooler tip through the whistle. Unfortunately, this means that the coldfinger extenstion needs to be made up of multiple geo volumes. This is the one that's contained in the doghouse. There is also a volume within the cryostat walls, and within the inner cryostat volume.

Include ColdFingerExtension_WithinTheDoghouse.geo
ColdFingerExtension_WithinTheDoghouse.Mother CryoBoxInside
ColdFingerExtension_WithinTheDoghouse.Rotation -90 0 0
ColdFingerExtension_WithinTheDoghouse.Position {-4.535+2.165/2} {-5.512+CryocoolerBox_WallThickness + 0.815 + 1.27 + 4.496/2} 0.225




Volume CryocoolerBody_4
CryocoolerBody_4.Visibility 1
CryocoolerBody_4.Material msteel_18_8
CryocoolerBody_4.Mother CryoBoxInside
CryocoolerBody_4.Shape TUBS 0 1.005 {3.11/2 - CryocoolerBox_WallThickness/2} 0 360
CryocoolerBody_4.Position 0 {Cryocooler_axis_yoffset} {-4.115+CryocoolerBox_WallThickness/2+3.11/2}
CryocoolerBody_4.Color 7

Volume CryocoolerBody_3_5
CryocoolerBody_3_5.Visibility 1
CryocoolerBody_3_5.Material msteel_18_8
CryocoolerBody_3_5.Mother CryoBoxThruHole
CryocoolerBody_3_5.Shape TUBS 0 1.005 {CryocoolerBox_WallThickness/2} 0 360
CryocoolerBody_3_5.Position 0 0 0
CryocoolerBody_3_5.Color 7

Volume CryocoolerCopperTip
CryocoolerCopperTip.Visibility 1
CryocoolerCopperTip.Material mcopper
CryocoolerCopperTip.Mother CryoBoxInside
CryocoolerCopperTip.Color 8
CryocoolerCopperTip.Shape TUBS 0 2.1 0.3 0 360
CryocoolerCopperTip.Position 0 Cryocooler_axis_yoffset {-4.115+3.11+0.3}




