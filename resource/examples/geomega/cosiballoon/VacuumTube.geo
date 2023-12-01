#File defines the vacuum tubes which attaches to the cryostat and to the pump that is removed before integration.
#Because of the bendy tubes, this geometry was tricky to get perfect. I left holes in corners and most of my tube lengths were approximate, but the final tube position within the cryostat and relative to the shields is very close to perfect.
###The mass from solidworks says 1.34 kg, and the mass here is only 1.17g, so I'm missing quite a bit still, and this can be improved by better approximating the flanges and valve.

Include Intro.geo

Constant CryoFlange_x_offset {15}
Constant CryoFlange_y_offset {-4}
Constant CryoFlange_z_offset {-4}
Constant KFlangeClamp1_x_pos {CryoFlange_x_offset - 8.5/1.41 - 2.54/1.41 - 13.5 - 2.54/4}
Constant KFlangeClamp1_y_pos {CryoFlange_y_offset + 8.5/1.41 + 2.54/2/1.41+1.2}
###The "+1.2" in the constant above to was my empirically determined fudge factor that resulted in the correct distance between the vacuum tube and the top of the shields (0.592 cm) 
Constant KFlangeClamp1_z_pos {CryoFlange_z_offset+3+2.54/2}

Volume VacuumTube
VacuumTube.Visibility 0
VacuumTube.Material Vacuum
VacuumTube.Virtual True
VacuumTube.Shape BRIK 20 10 8

Volume CryostatFlange
CryostatFlange.Visibility 1
CryostatFlange.Material msteel_18_8
CryostatFlange.Shape TUBS {2.54/2} {5.715/2} {0.635/2} 0 360
CryostatFlange.Mother VacuumTube
CryostatFlange.Position CryoFlange_x_offset CryoFlange_y_offset CryoFlange_z_offset

###I've just called my subsequent tube segments VacTube#

Volume VacTube1
VacTube1.Visibility 1
VacTube1.Material msteel_18_8
VacTube1.Shape TUBS {2.21/2} {2.54/2} {3/2} 0 360
VacTube1.Mother VacuumTube
VacTube1.Position CryoFlange_x_offset CryoFlange_y_offset {CryoFlange_z_offset+3/2}

Volume VacTube2
VacTube2.Visibility 1
VacTube2.Material msteel_18_8
VacTube2.Shape TUBS {2.21/2} {2.54/2} {8.5/2} 0 360
VacTube2.Mother VacuumTube
VacTube2.Rotation 0 90 -45 
VacTube2.Position {CryoFlange_x_offset + -2.54/2/1.414 - 8.5/2/1.414} {CryoFlange_y_offset+2.54/2/1.414 + 8.5/2/1.414} KFlangeClamp1_z_pos

Volume VacTube3
VacTube3.Visibility 1
VacTube3.Material msteel_18_8
VacTube3.Shape TUBS {2.21/2} {2.54/2} {13.5/2} 0 360
##The length of 13.5 was chosen to position the manual valve and second KFlangeClamp the correct distance from the outer shields
VacTube3.Mother VacuumTube
VacTube3.Rotation 0 90 0
VacTube3.Position {CryoFlange_x_offset - 8.5/1.41 - 2.54/1.41 - 13.5/2}  KFlangeClamp1_y_pos  KFlangeClamp1_z_pos


###Approximation for a KFlange Clamp. Mostly made out of aluminum. Not quite cylindrical, but close enough.
Volume KFlangeClamp
KFlangeClamp.Visibility 1
KFlangeClamp.Color 2
KFlangeClamp.Material al6061
KFlangeClamp.Shape TUBS {2.54/2} {2.5} {2.54/4} 0 360

KFlangeClamp.Copy KFlangeClamp1
KFlangeClamp1.Mother VacuumTube
KFlangeClamp1.Rotation 0 90 0
KFlangeClamp1.Position KFlangeClamp1_x_pos KFlangeClamp1_y_pos KFlangeClamp1_z_pos

Volume VacTube4
VacTube4.Visibility 1
VacTube4.Material msteel_18_8
VacTube4.Shape TUBS {2.21/2} {2.54/2} {3.9/2} 0 360
VacTube4.Mother VacuumTube
VacTube4.Rotation 0 90 0
VacTube4.Position {KFlangeClamp1_x_pos-2.54/4-3.9/2} KFlangeClamp1_y_pos KFlangeClamp1_z_pos


###loose approximation for the hand valve. Has the correct outter diameter and height, but is missing the innerworkings and the actual valvue part. I've made it out of steel, but I'm not positive of the material...
Volume VacHandValve
VacHandValve.Visibility 1
VacHandValve.Color 3
VacHandValve.Material msteel_18_8
VacHandValve.Shape TUBS {3/2} {4.57/2} {8.9/2} 0 360
VacHandValve.Mother VacuumTube
VacHandValve.Rotation 90 0 0
VacHandValve.Position  {KFlangeClamp1_x_pos- 2.54/4 - 3.9 - 4.57/2} KFlangeClamp1_y_pos KFlangeClamp1_z_pos

Volume VacTube5
VacTube5.Visibility 1
VacTube5.Material msteel_18_8
VacTube5.Shape TUBS {2.21/2} {2.54/2} {3.1/2} 0 360
VacTube5.Mother VacuumTube
VacTube5.Rotation 0 90 0
VacTube5.Position {KFlangeClamp1_x_pos-2.54/4-3.9 -4.57 -3.1/2} KFlangeClamp1_y_pos KFlangeClamp1_z_pos

KFlangeClamp.Copy KFlangeClamp2
KFlangeClamp2.Mother VacuumTube
KFlangeClamp2.Rotation 0 90 0
KFlangeClamp2.Position {KFlangeClamp1_x_pos-2.54/4-3.9 -4.57 -3.1 - 2.54/4} KFlangeClamp1_y_pos KFlangeClamp1_z_pos


