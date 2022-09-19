###Because the GeD's live within the IR Shields, I've used this file to define the geometry of the IR Shields, and then I've included the 12 GeD's and the coldfinger.
##check the thikcness of IR shields. Is the constant the half thickness!?


Include Intro.geo

Constant IRShield_thickness 0.0508
##Constant GeDs_Z_withinIRShields 0.486
Constant GeDs_Z_withinIRShields {6.027-5.5435}


Volume IRShieldsAndGe
IRShieldsAndGe.Visibility 0
IRShieldsAndGe.Virtual True
IRShieldsAndGe.Material Vacuum
IRShieldsAndGe.Shape BRIK 12 13.5 6

Volume IRShieldAluOutside
IRShieldAluOutside.Visibility 0
IRShieldAluOutside.Material al6061
##IRShieldAluOutside.Shape BRIK 11.3881 13.0224 5.4674 ##edited on 06/04/15
IRShieldAluOutside.Shape BRIK 11.3881 13.0224 5.5435
IRShieldAluOutside.Position 0 0 0
IRShieldAluOutside.Mother IRShieldsAndGe
IRShieldAluOutside.Color 3


Volume IRShieldTopHoles
IRShieldTopHoles.Visibility 1
IRShieldTopHoles.Material Vacuum
IRShieldTopHoles.Color 6
IRShieldTopHoles.Shape BRIK 4.3245 5.1415 {IRShield_thickness/2}
For X 2 -4.9561 9.9122
  For Y 2 -5.7731 11.5462
    IRShieldTopHoles.Copy IRShieldTopHoles_%X%Y
    IRShieldTopHoles_%X%Y.Position $X $Y {5.5435 - IRShield_thickness/2}
    IRShieldTopHoles_%X%Y.Mother IRShieldAluOutside
    IRShieldTopHoles_%X%Y.Visibility 1
  Done
Done

Volume IRShieldSideHoles
IRShieldSideHoles.Visibility 1
IRShieldSideHoles.Material Vacuum
IRShieldSideHoles.Color 6
IRShieldSideHoles.Shape BRIK 3.2893 {IRShield_thickness/2} 1.06
For Y 2 {-13.0224+IRShield_thickness/2} {26.0448-IRShield_thickness}
  For Z 3 3.2334 -2.54
    IRShieldSideHoles.Copy IRShieldSideHoles_SideA%Y%Z
    IRShieldSideHoles_SideA%Y%Z.Position -5.708 $Y $Z
    IRShieldSideHoles_SideA%Y%Z.Mother IRShieldAluOutside
    IRShieldSideHoles_SideA%Y%Z.Visibility 1
  Done
Done
For Y 2 {-13.0224+IRShield_thickness/2} {26.0448-IRShield_thickness}
  For Z 3 2.8214 -2.54
    IRShieldSideHoles.Copy IRShieldSideHoles_SideB%Y%Z
    IRShieldSideHoles_SideB%Y%Z.Position 5.708 $Y $Z
    IRShieldSideHoles_SideB%Y%Z.Mother IRShieldAluOutside
    IRShieldSideHoles_SideB%Y%Z.Visibility 0
  Done
Done
For X 2 {-11.3881+IRShield_thickness/2} {22.7762-IRShield_thickness}
  For Z 3 3.2334 -2.54
    IRShieldSideHoles.Copy IRShieldSideHoles_FrontA%X%Z ###Front and Back
    IRShieldSideHoles_FrontA%X%Z.Position $X -6.4325 $Z
    IRShieldSideHoles_FrontA%X%Z.Mother IRShieldAluOutside
    IRShieldSideHoles_FrontA%X%Z.Rotation 0 0 90
    IRShieldSideHoles_FrontA%X%Z.Visibility 0
  Done
Done
For X 2 {-11.3881+IRShield_thickness/2} {22.7762-IRShield_thickness}
  For Z 3 2.8214 -2.54
    IRShieldSideHoles.Copy IRShieldSideHoles_FrontB%X%Z ###Front and Back
    IRShieldSideHoles_FrontB%X%Z.Position $X 6.4325 $Z
    IRShieldSideHoles_FrontB%X%Z.Mother IRShieldAluOutside
    IRShieldSideHoles_FrontB%X%Z.Rotation 0 0 90
    IRShieldSideHoles_FrontB%X%Z.Visibility 0
  Done
Done


Volume IRShieldAluInside
IRShieldAluInside.Visibility 0
IRShieldAluInside.Material Vacuum
IRShieldAluInside.Shape BRIK {11.3881-IRShield_thickness} {13.0224-IRShield_thickness} {5.5435 -IRShield_thickness}
IRShieldAluInside.Position 0 0 0
IRShieldAluInside.Mother IRShieldAluOutside
IRShieldAluInside.Color 4




###The coldfinger is including within this volume as well.
Include ColdFinger.geo
ColdFinger.Position 0 0 -0.1144  
###The -0.1144 above mates the coldfinger and IRshield on bottom
#ColdFinger.Visibility 0
ColdFinger.Mother IRShieldAluInside

###Including the twelve germanium detectors into the Volume contained within the IR Shields
Include GeD_12Stack.geo  
GeD_12Stack.Mother IRShieldAluInside
GeD_12Stack.Position 0 0  GeDs_Z_withinIRShields

Include ColdFingerExtension_WithinIRShield.geo
ColdFingerExtension_WithinIRShield.Position 0 0 0
#ColdFingerExtension_WithinIRShield.Visibility 0 
ColdFingerExtension_WithinIRShield.Mother IRShieldAluInside
