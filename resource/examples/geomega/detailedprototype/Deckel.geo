// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo



Volume CapVolume
CapVolume.Material Air
CapVolume.Visibility 0
CapVolume.Shape BRIK 70. 70. 70.
CapVolume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// CapVolume.Mother 0







// Vier "Zapfen":
// (50)
Volume Cap_Part50
Cap_Part50.Material Aluminium
Cap_Part50.Shape BRIK 0.625 0.15 0.45
Cap_Part50.Position 1.75 1.45 0.45
Cap_Part50.Color 9
Cap_Part50.Visibility 1
Cap_Part50.Mother CapVolume

// (51)
Volume Cap_Part51
Cap_Part51.Material Aluminium
Cap_Part51.Shape BRIK 0.625 0.15 0.45
Cap_Part51.Position 1.75 -1.45 0.45
Cap_Part51.Color 9
Cap_Part51.Visibility 1
Cap_Part51.Mother CapVolume

// (52)
Volume Cap_Part52
Cap_Part52.Material Aluminium
Cap_Part52.Shape BRIK 0.625 0.15 0.45
Cap_Part52.Position -1.75 1.45 0.45
Cap_Part52.Color 9
Cap_Part52.Visibility 1
Cap_Part52.Mother CapVolume

// (53)
Volume Cap_Part53
Cap_Part53.Material Aluminium
Cap_Part53.Shape BRIK 0.625 0.15 0.45
Cap_Part53.Position -1.75 -1.45 0.45
Cap_Part53.Color 9
Cap_Part53.Visibility 1
Cap_Part53.Mother CapVolume



// rechte Wand aussen:
// (60)
Volume Cap_Part60
Cap_Part60.Material Aluminium
Cap_Part60.Shape BRIK 0.3 4.65 0.4
Cap_Part60.Position 4.3 0.0 -0.1
Cap_Part60.Color 9
Cap_Part60.Visibility 1
Cap_Part60.Mother CapVolume

// linke Wand aussen:
// (61)
Volume Cap_Part61
Cap_Part61.Material Aluminium
Cap_Part61.Shape BRIK 0.3 4.65 0.4
Cap_Part61.Position -4.3 0.0 -0.1
Cap_Part61.Color 9
Cap_Part61.Visibility 1
Cap_Part61.Mother CapVolume


// Big Plate on Bottom (62):
// Recalculate the both z-values!!  
Volume Cap_Part62
Cap_Part62.Material Aluminium
Cap_Part62.Shape BRIK 3.9 4.255 0.14
Cap_Part62.Position 0. 0. 1.04
Cap_Part62.Color 9
Cap_Part62.Visibility 1
Cap_Part62.Mother CapVolume


// rechte Innenwand:
// (35):
Volume Cap_Part35
Cap_Part35.Material Aluminium
Cap_Part35.Shape BRIK 0.05 4.45 0.5
Cap_Part35.Position 3.95 0. 0.5
Cap_Part35.Color 9
Cap_Part35.Visibility 1
Cap_Part35.Mother CapVolume

// linke Innenwand:
// (36):
Volume Cap_Part36
Cap_Part36.Material Aluminium
Cap_Part36.Shape BRIK 0.05 4.45 0.5
Cap_Part36.Position  -3.95 0. 0.5
Cap_Part36.Color 9
Cap_Part36.Visibility 1
Cap_Part36.Mother CapVolume


// vordere Innenwand:
// (63)
Volume Cap_Part63
Cap_Part63.Material Air
Cap_Part63.Shape BRIK 3.9 0.995 0.5
Cap_Part63.Position 0.0 4.35 0.5
Cap_Part63.Color 9
Cap_Part63.Visibility 1
Cap_Part63.Mother CapVolume

// hintere Innenwand:
// (64)
Volume Cap_Part64
Cap_Part64.Material Air
Cap_Part64.Shape BRIK 3.9 0.995 0.5
Cap_Part64.Position 0.0 -4.35 0.5
Cap_Part64.Color 9
Cap_Part64.Visibility 1
Cap_Part64.Mother CapVolume


// vorder Aussenwand:
// (65):
Volume Cap_Part65
Cap_Part65.Material Air
Cap_Part65.Shape BRIK 4.0 0.1 0.4
Cap_Part65.Position 0.0 4.55 -0.1
Cap_Part65.Color 9
Cap_Part65.Visibility 1
Cap_Part65.Mother CapVolume

// hintere Aussenwand:
// (66):
Volume Cap_Part66
Cap_Part66.Material Air
Cap_Part66.Shape BRIK  4.0 0.1 0.4
Cap_Part66.Position 0.0 -4.55 -0.1
Cap_Part66.Color 9
Cap_Part66.Visibility 1
Cap_Part66.Mother CapVolume



// (20):
Volume Cap_Screw1
Cap_Screw1.Material Iron
Cap_Screw1.Shape TUBS 0. 0.2 0.1 0. 360.
Cap_Screw1.Position  4.3 2.6 0.4
Cap_Screw1.Color 7
Cap_Screw1.Visibility 1
Cap_Screw1.Mother CapVolume

// (21):
Volume Cap_Screw2
Cap_Screw2.Material Iron
Cap_Screw2.Shape TUBS 0. 0.2 0.1 0. 360.
Cap_Screw2.Position -4.3 2.6 0.4
Cap_Screw2.Color 7
Cap_Screw2.Visibility 1
Cap_Screw2.Mother CapVolume

// (22):
Volume Cap_Screw3
Cap_Screw3.Material Iron
Cap_Screw3.Shape TUBS 0. 0.2 0.1 0. 360.
Cap_Screw3.Position 4.3 -2.6 0.4
Cap_Screw3.Color 7
Cap_Screw3.Visibility 1
Cap_Screw3.Mother CapVolume

// (23):
Volume Cap_Screw4
Cap_Screw4.Material Iron
Cap_Screw4.Shape TUBS 0. 0.2 0.1 0. 360.
Cap_Screw4.Position -4.3 -2.6 0.4
Cap_Screw4.Color 7
Cap_Screw4.Visibility 1
Cap_Screw4.Mother CapVolume



// Hole on short side of the cap:
// part of the hole in the inner front wall:
// (58)
Volume Cap_Window58
Cap_Window58.Material Air
Cap_Window58.Shape BRIK 2.1 0.1 0.4
Cap_Window58.Position 0.0 0.0 -0.1
Cap_Window58.Color 6
Cap_Window58.Visibility 1
Cap_Window58.Mother Cap_Part63

// part of the hole in the outer front wall:
// (67):
Volume Cap_Window67
Cap_Window67.Material Air
Cap_Window67.Shape BRIK 2.1 0.1 0.15
Cap_Window67.Position 0.0 0.0 0.25
Cap_Window67.Color 6
Cap_Window67.Visibility 1
Cap_Window67.Mother Cap_Part65


