#This file defines the entire DetectorHead - Preamps, Cryocooler, Vacuum tube and entire cryostat (and everthere therein)


Include Intro.geo

#Constant VacTubeCryostat_x_offset {5.715/2}
#Constant VacTubeCryostat_z_offset {5.715/2}

Volume DetectorHead
DetectorHead.Visibility 0
DetectorHead.Material Vacuum
DetectorHead.Shape BRIK 30.0 30.0 30.0
DetectorHead.Virtual true
DetectorHead.Position 0 0 0

Include Cryostat.geo
Cryostat.Mother DetectorHead
Cryostat.Position 0 0 0

Include Cryocooler.geo
Cryocooler.Mother DetectorHead
Cryocooler.Position {-16.9568} 3.549 -4.1035  
#ck raised this up by .2 cm 09/2014 to avoid an overlap between the doghouse and the cryostat baseplate
Cryocooler.Rotation 90 0 180

Include VacuumTube.geo
###defines the vacuum tube and flange and valve.
VacuumTube.Mother DetectorHead
VacuumTube.Position {-14.136+Cryostat_xoffset-CryoFlange_x_offset+VacTubeCryostat_x_offset} {-15.803+CryoFlange_z_offset-0.635/2} {9.7155-CryoFlange_y_offset-VacTubeCryostat_z_offset}
VacuumTube.Rotation 90 0 0


Include Preamp_ThreeStack.geo
###Preamp_ThreeStack.geo defines a verticle stack of three pre-amps.
###Preamp positions are still a bit off for simplicity, but the approximations I have made should be good enough.
    ###I've made the pre-amp positions on the sides rotationally symmetric, not a mirror image, like it should be, but the offset is only about a centimeter, so it should be fine.
###Remember, the front of the geomerty is the sun facing-side. The back is therefore the cryocooler side.

For Y 2 -5.815 {5.815*2}
  Preamp_ThreeStack.Copy PreamplifierBox_FrontA%Y
  PreamplifierBox_FrontA%Y.Position {16.676+Cryostat_xoffset} $Y {1.0438-preamps_zoffset}
  PreamplifierBox_FrontA%Y.Mother DetectorHead
Done

For Y 2 -18.343 36.686
    Preamp_ThreeStack.Copy PreamplifierBox_SideA%Y
    PreamplifierBox_SideA%Y.Position {5.815+Cryostat_xoffset} $Y {1.0438-preamps_zoffset}
    PreamplifierBox_SideA%Y.Rotation 0.0 0.0 90.0
    PreamplifierBox_SideA%Y.Mother DetectorHead
Done
For Y 2 -18.343 36.686
    Preamp_ThreeStack.Copy PreamplifierBox_SideB%Y
    PreamplifierBox_SideB%Y.Position {-5.815+Cryostat_xoffset} $Y {-preamps_zoffset-0.1286}
    PreamplifierBox_SideB%Y.Rotation 0.0 0.0 90.0
    PreamplifierBox_SideB%Y.Mother DetectorHead
Done

For Y 2 -5.815 11.63
    Preamp_ThreeStack.Copy PreamplifierBox_Back%Y
    PreamplifierBox_Back%Y.Position {-16.676+Cryostat_xoffset} $Y {8.384-preamps_zoffset}
    PreamplifierBox_Back%Y.Mother DetectorHead
Done





