###According to Solidworks, the mass of the Cryostat shell, without the top and bottom plates, is 3.4 kg (currently 3.89 here)
###" ", the mass of the Cryostat top is 817 g (607 g here), and the bottom is 6636 g (6858 here).

###This file is so long because it includes all of the cut-outs of the cryostat shell. There are a lot and it's pretty messy. Also, it would be good to need to add a few more...


Include Intro.geo
Include Preamp_ThreeStack.geo



Constant Cryostat_xoffset 1.7793
Constant IRShield_thickness 0.0508
#Constant det_xoffset 4.594
#Constant det_yoffset 6.205
Constant Cryostat_Cutout_thickness {1.143/2}

Constant VacTubeCryostat_x_offset {5.715/2}
Constant VacTubeCryostat_z_offset {5.715/2}


Volume Cryostat
Cryostat.Visibility 0
Cryostat.Material Vacuum
Cryostat.Virtual True
Cryostat.Shape BRIK 20 16 15


Volume CryostatAluOutside
CryostatAluOutside.Visibility 0
CryostatAluOutside.Material al6061
CryostatAluOutside.Shape BRIK 14.136 15.803 9.7155
CryostatAluOutside.Position Cryostat_xoffset 0 0 
CryostatAluOutside.Mother Cryostat


Volume CryostatTop
CryostatTop.Visibility 1
CryostatTop.Material al6061
CryostatTop.Shape BRIK 14.136 15.803 0.3955 
CryostatTop.Mother Cryostat
CryostatTop.Position Cryostat_xoffset 0 10.111

Volume CryostatTopHoles
CryostatTopHoles.Visibility 1
CryostatTopHoles.Material Vacuum
CryostatTopHoles.Shape BRIK 2.5288 2.8621 0.3325

###could edit the Cryostat top and account for the thicker edge part (1.0922 cm from the edge and a total of 2.1082 cm thick and 0.178 cm wide edge)

For X 5 -10.3372 5.2354
  For Y 5 -11.6709 5.9019
    CryostatTopHoles.Copy CryostatTopHoles_%X%Y
    CryostatTopHoles_%X%Y.Position $X $Y -0.063
    CryostatTopHoles_%X%Y.Mother CryostatTop
    CryostatTopHoles_%X%Y.Visibility 1
  Done
Done


Volume CryostatAluBottom
CryostatAluBottom.Visibility 1
CryostatAluBottom.Material al6061
CryostatAluBottom.Shape BRIK 22.8473 23.7998 0.5842
CryostatAluBottom.Position 0 0 {-9.7155-0.5842}
CryostatAluBottom.Mother Cryostat


Volume CryostatAluInside
CryostatAluInside.Visibility 0
CryostatAluInside.Material Vacuum
CryostatAluInside.Shape BRIK 12.8686 14.5835 9.7155 ###ck (2/14) changed so that we can have the top and bottom cryostat shell pieces separate.
CryostatAluInside.Position 0 0 0
CryostatAluInside.Mother CryostatAluOutside

Volume CryostatCryocoolerHole
CryostatCryocoolerHole.Visibility 1
CryostatCryocoolerHole.Material Vacuum
CryostatCryocoolerHole.Shape BRIK 0.6098 2.9337 4.6292 
CryostatCryocoolerHole.Position -13.5262 11.5739 -4.1236
CryostatCryocoolerHole.Mother CryostatAluOutside


#The ColdfingerExtension2 piece extends through the CryostatCryocoolerHole hole. I need to add the extension piece here and also inside the doghouse...There is also the piece that extends up to the cryocooler tip (Chris refers to it as the whistle), which I need to add here.
#The ColdfingerExtension is 1.791 cm above the InnerSide of the Cryostat Bottom, and 0.531 cm away from the cryostat wall in the +y direction, but we should actually be referencing it within the cutout hole for the doghouse. The extension is 0.815cm above the bottom edge and 0.505 cm from the outer edge 

Include ColdFingerExtension_WithinCryostatShell.geo
ColdFingerExtension_WithinCryostatShell.Mother CryostatCryocoolerHole
ColdFingerExtension_WithinCryostatShell.Position 0 {2.9337-0.485-4.45/2} {-4.6292+0.82+4.496/2+1.27} 
#the above pieve is pretty off I think. Both in the y direction and in the z direction. I could just fudge it and do it visually, but I'm thinking that the reason it's off is due to the CryostatCryocoolerHole and I need a little more time to look into that.

#This is an extension to the "Coldfinger Extension". The part that meets with the volume contained within the IRShield and connects to the pieve above going into the doghouse.
Volume ColdFingerExtension_4 ###who knows how many i'm at now...
ColdFingerExtension_4.Visibility 1
ColdFingerExtension_4.Material al6061
ColdFingerExtension_4.Shape BRIK {1.4805/2} {4.45/2} {1.8288/2}
ColdFingerExtension_4.Position {-12.8686 + 1.4805/2} {14.5835 - 0.531 - 4.45/2} {-9.7155 + 1.791 + 1.8288/2}
ColdFingerExtension_4.Color 6
#The + 0.5538 comes from the fact that the thicker part of the Coldfinger Extension, which has a thickness of 1.8288 cm, lies 1.791 cm from the bottom of the cryostat. But, the coldfinger extension thins out to 1.27 cm before exciting the wall of the cryostat, so that would be 1.791 + 0.5538 cm from the bottom of the cryostat.
#ColdFingerExtension_5.Position {-12.8686 + 1.4805/2} 11.5739 {-9.7155+1.791+1.27/2}
ColdFingerExtension_4.Mother CryostatAluInside

Volume ColdFingerExtension_5
ColdFingerExtension_5.Visibility 1
ColdFingerExtension_5.Material al6061
ColdFingerExtension_5.Shape TRAP {1.8288/2} 0 0 {1.0301/2} {7.1088/2} {7.1088/2 - 0.8223/2} {21.76} {1.0301/2} {7.1088/2} {7.1088/2 - 0.8223/2} {21.76}
ColdFingerExtension_5.Position {-11.3881 + 3.37} {13.0224 + 1.0301/2} {-9.7155 + 1.791 + 1.8288/2}
ColdFingerExtension_5.Rotation 0 180 0
ColdFingerExtension_5.Color 6
ColdFingerExtension_5.Mother CryostatAluInside

###WIthin the cryostat lives the IR Shields and within those, the germanium detectors! Including them here...
Include IRShieldsAndGe.geo
IRShieldsAndGe.Mother CryostatAluInside
IRShieldsAndGe.Position 0 0 {-2.5145}




###LOTS OF CUTOUTS IN THE CRYOSTAT OUTSIDE!!!! WAHOO!...everything below here.

Volume PreampCryostatCutout
PreampCryostatCutout.Visibility 0
PreampCryostatCutout.Material Vacuum
PreampCryostatCutout.Shape BRIK {10.29/2} {1.016/2} {1.51/2} 

Volume PreampCutouts3Stack
PreampCutouts3Stack.Visibility 1
PreampCutouts3Stack.Virtual True
PreampCutouts3Stack.Material Vacuum
PreampCutouts3Stack.Shape BRIK {10.3/2} {1.016/2} 6

For Z 3 -preamps_zoffset preamps_zoffset
  PreampCryostatCutout.Copy PreampCutoutStack_%Z
  PreampCutoutStack_%Z.Position 0 0 $Z
  PreampCutoutStack_%Z.Mother PreampCutouts3Stack
Done


###For the back of the cryostat
Volume CryostatCutouts_1
CryostatCutouts_1.Visibility 0
CryostatCutouts_1.Material Vacuum
CryostatCutouts_1.Shape BRIK Cryostat_Cutout_thickness {3.3274/2}  {1.9812/2}

For Z 5 {-9.7155+1+1.9812/2} {1.9812+.15}
  For Y 3 {-15.803+1.27+3.3274/2+1.9812+1.03} {+3.3274+.15}
      CryostatCutouts_1.Copy CryostatCutout_Back_%Z%Y
      CryostatCutout_Back_%Z%Y.Position {-14.136+Cryostat_Cutout_thickness} $Y $Z
      CryostatCutout_Back_%Z%Y.Mother CryostatAluOutside
      #CryostatCutout_Back_%Z%Y.Color 4
      CryostatCutout_Back_%Z%Y.Visibility 0
  Done
Done
For Z 5 {-9.7155+1+1.9812/2} {1.9812+.15}
  For Y 2 {1.2+3.3274/2} {+3.3274+.15}
      CryostatCutouts_1.Copy CryostatCutout_Back2_%Z%Y
      CryostatCutout_Back2_%Z%Y.Position {-14.136+Cryostat_Cutout_thickness} $Y $Z
      CryostatCutout_Back2_%Z%Y.Mother CryostatAluOutside
      #CryostatCutout_Back2_%Z%Y.Color 4
      CryostatCutout_Back2_%Z%Y.Visibility 0
  Done
Done

Volume CryostatCutouts_2
CryostatCutouts_2.Visibility 0
CryostatCutouts_2.Material Vacuum
CryostatCutouts_2.Shape BRIK Cryostat_Cutout_thickness {1.688/2}  {1.9812/2}

For Z 8 {-9.7155+1+1.9812/2} {1.9812+.15}
  CryostatCutouts_2.Copy CryostatCutout_BackLeft_%Z
  CryostatCutout_BackLeft_%Z.Position {-14.136+Cryostat_Cutout_thickness} {-15.803+1.27+1.688/2} $Z
  CryostatCutout_BackLeft_%Z.Mother CryostatAluOutside
  #CryostatCutout_BackLeft_%Z.Color 5
  CryostatCutout_BackLeft_%Z.Visibility 0
Done

For Z 3 {-9.7155+1+1.9812/2+1.9812*5+.15*5} {1.9812+.15}
  CryostatCutouts_2.Copy CryostatCutout_BackRight_%Z
  CryostatCutout_BackRight_%Z.Position {-14.136+Cryostat_Cutout_thickness} {15.803-1.27-1.688/2} $Z
  CryostatCutout_BackRight_%Z.Mother CryostatAluOutside
  #CryostatCutout_BackRight_%Z.Color 5
  CryostatCutout_BackRight_%Z.Visibility 0
Done

Volume CryostatCutouts_3
CryostatCutouts_3.Visibility 1
CryostatCutouts_3.Material Vacuum
CryostatCutouts_3.Shape BRIK Cryostat_Cutout_thickness {1.05/2}  {1.9812/2}
For Z 8 {-9.7155+1+1.9812/2} {1.9812+.15}
  CryostatCutouts_3.Copy CryostatCutout_BackCenter_%Z
  CryostatCutout_BackCenter_%Z.Position {-14.136+Cryostat_Cutout_thickness} 0 $Z
  CryostatCutout_BackCenter_%Z.Mother CryostatAluOutside
 # CryostatCutout_BackCenter_%Z.Color 5
  CryostatCutout_BackCenter_%Z.Visibility 0
Done


For Y 2 -5.815 11.63
    PreampCutouts3Stack.Copy PreampCutouts_Back%Y
    PreampCutouts_Back%Y.Position {-14.136+1.016/2} $Y {8.384-preamps_zoffset}
    PreampCutouts_Back%Y.Rotation 0 0 90
    PreampCutouts_Back%Y.Mother CryostatAluOutside
   # PreampCutouts_Back%Y.Color 7
    PreampCutouts_Back%Y.Visibility 0
Done


###For the right and Left Sides

Volume VacuumTube_ThurHole
VacuumTube_ThurHole.Visibility 0
VacuumTube_ThurHole.Material Vacuum
VacuumTube_ThurHole.Shape TUBS 0 1.3 {1.2195/2}  0 360
VacuumTube_ThurHole.Rotation 90 0 0
VacuumTube_ThurHole.Position {-14.136 + VacTubeCryostat_x_offset} {-15.803+1.2195/2} {9.7155-VacTubeCryostat_z_offset}
VacuumTube_ThurHole.Mother CryostatAluOutside
#VacuumTube_ThurHole.Color 6

For Z 3 {9.7155-1.9812/2-1} {-1.9812-.15}
  For X 4 {14.136-.2-1.688/2} {-1.688-.15}
    For Y 2 {-15.803+Cryostat_Cutout_thickness} {15.803*2-Cryostat_Cutout_thickness*2}
      CryostatCutouts_2.Copy CryostatCutout_SidesA_%X%Y%Z
      CryostatCutout_SidesA_%X%Y%Z.Position $X $Y $Z
      CryostatCutout_SidesA_%X%Y%Z.Mother CryostatAluOutside
      CryostatCutout_SidesA_%X%Y%Z.Rotation 0 0 90
      #CryostatCutout_SidesA_%X%Y%Z.Color 6
      CryostatCutout_SidesA_%X%Y%Z.Visibility 0
    Done
  Done
Done

For Z 3 {9.7155-1.9812/2-1} {-1.9812-.15}
  For X 6 {14.136-.2-1.688/2-1.688*4-.15*4-1.5} {-1.688-.15}
    For Y 2 {-15.803+Cryostat_Cutout_thickness} {15.803*2-Cryostat_Cutout_thickness*2}
      CryostatCutouts_2.Copy CryostatCutout_SidesB_%X%Y%Z
      CryostatCutout_SidesB_%X%Y%Z.Position $X $Y $Z
      CryostatCutout_SidesB_%X%Y%Z.Mother CryostatAluOutside
      CryostatCutout_SidesB_%X%Y%Z.Rotation 0 0 90
     # CryostatCutout_SidesB_%X%Y%Z.Color 6
      CryostatCutout_SidesB_%X%Y%Z.Visibility 0
    Done
  Done
Done

For X 8 {14.136-.2-3.3274/2} {-3.3274-.15}
    For Y 2 {-15.803+Cryostat_Cutout_thickness} {15.803*2-Cryostat_Cutout_thickness*2}
      CryostatCutouts_1.Copy CryostatCutout_SidesC_%X%Y
      CryostatCutout_SidesC_%X%Y.Position $X $Y {-9.7155+1.9812/2+1}
      CryostatCutout_SidesC_%X%Y.Mother CryostatAluOutside
      CryostatCutout_SidesC_%X%Y.Rotation 0 0 90
    #  CryostatCutout_SidesC_%X%Y.Color 7
      CryostatCutout_SidesC_%X%Y.Visibility 0
    Done
Done

For Y 2 {-15.803+1.016/2} {15.803*2-1.016}
    PreampCutouts3Stack.Copy PreampCutouts_SideA%Y
    PreampCutouts_SideA%Y.Position {5.815-1.016/2} $Y {1.0438-preamps_zoffset}
    PreampCutouts_SideA%Y.Rotation 0 0 0
    PreampCutouts_SideA%Y.Mother CryostatAluOutside
   # PreampCutouts_SideA%Y.Color 6
Done

For Y 2 {-15.803+1.016/2} {15.803*2-1.016}
    PreampCutouts3Stack.Copy PreampCutouts_SideB%Y
    PreampCutouts_SideB%Y.Position {-5.815+1.016/2} $Y {-preamps_zoffset-0.1286}
    PreampCutouts_SideB%Y.Rotation 0 0 0
    PreampCutouts_SideB%Y.Mother CryostatAluOutside
    #PreampCutouts_SideB%Y.Color 6
Done



#For the Front side
For Z 3 {9.7155-1.9812/2-1} {-1.9812-.15}
  For Y 8 {-15.803+2+3.3274/2} {+3.3274+.15}
      CryostatCutouts_1.Copy CryostatCutout_Front_%Y%Z
      CryostatCutout_Front_%Y%Z.Position {14.136-Cryostat_Cutout_thickness} $Y $Z
      CryostatCutout_Front_%Y%Z.Mother CryostatAluOutside
     # CryostatCutout_Front_%Y%Z.Color 8
      CryostatCutout_Front_%Y%Z.Visibility 0
  Done
Done

For Y 8 {-15.803+2+3.3274/2} {+3.3274+.15}
      CryostatCutouts_1.Copy CryostatCutout_Front_%Y
      CryostatCutout_Front_%Y.Position {14.136-Cryostat_Cutout_thickness} $Y {-9.7155+1.9812/2+1}
      CryostatCutout_Front_%Y.Mother CryostatAluOutside
      #CryostatCutout_Front_%Y.Color 8
      CryostatCutout_Front_%Y.Visibility 0
Done



For Y 2 -5.815 {5.815*2}
    PreampCutouts3Stack.Copy PreampCutouts_Front%Y
    PreampCutouts_Front%Y.Position {14.136-1.016/2} $Y {1.0438-preamps_zoffset}
    PreampCutouts_Front%Y.Rotation 0 0 90
    PreampCutouts_Front%Y.Mother CryostatAluOutside
    #PreampCutouts_Front%Y.Color 6
Done

For Y 2 {-15.803+2+1.9812/2} {15.803*2-4-1.9812}
  For Z 2 {-9.7155+6} {3.3274+.1}
    CryostatCutouts_1.Copy CryostatCutout_FrontB_%Y%Z
    CryostatCutout_FrontB_%Y%Z.Position {14.136-Cryostat_Cutout_thickness} $Y $Z
    CryostatCutout_FrontB_%Y%Z.Mother CryostatAluOutside
    CryostatCutout_FrontB_%Y%Z.Rotation 90 0 0
    #CryostatCutout_FrontB_%Y%Z.Color 2
    CryostatCutout_FrontB_%Y%Z.Visibility 0
    Done
Done

