// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo




// World volume section

// Volume WorldVolume             
// WorldVolume.Material Air
// WorldVolume.Visibility 0
// WorldVolume.Shape BRIK 1000. 1000. 1000.
// WorldVolume.Mother 0



// The Mother Volume of the Blue Frame:
Volume Part_BF_Volume             
Part_BF_Volume.Material Air
Part_BF_Volume.Visibility 0
Part_BF_Volume.Shape BRIK 100. 100. 100.
Part_BF_Volume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// Part_BF_Volume.Mother 0




// The element parts of the Blue Frame:

// The hole plate (BF1):
Volume Part_BF_1
Part_BF_1.Material Aluminium
Part_BF_1.Shape BRIK 11.25 11.25 0.4
Part_BF_1.Position 0.0 0.0 -0.4
Part_BF_1.Color 4
Part_BF_1.Visibility 1
Part_BF_1.Mother Part_BF_Volume

// 4 recesses:

// (BF2)
Volume Part_BF_2
Part_BF_2.Material Aluminium
Part_BF_2.Shape BRIK 4.75 4.75 0.3
Part_BF_2.Position 5.15 5.15 -0.1
Part_BF_2.Color 4
Part_BF_2.Visibility 1
Part_BF_2.Mother Part_BF_1

// (BF3)
Volume Part_BF_3
Part_BF_3.Material Aluminium
Part_BF_3.Shape BRIK 4.75 4.75 0.3
Part_BF_3.Position 5.15 -5.15 -0.1
Part_BF_3.Color 4
Part_BF_3.Visibility 1
Part_BF_3.Mother Part_BF_1

// (BF4)
Volume Part_BF_4
Part_BF_4.Material Aluminium
Part_BF_4.Shape BRIK 4.75 4.75 0.3
Part_BF_4.Position -5.15 5.15 -0.1
Part_BF_4.Color 4
Part_BF_4.Visibility 1
Part_BF_4.Mother Part_BF_1

// (BF5)
Volume Part_BF_5
Part_BF_5.Material Aluminium
Part_BF_5.Shape BRIK 4.75 4.75 0.3
Part_BF_5.Position -5.15 -5.15 -0.1
Part_BF_5.Color 4
Part_BF_5.Visibility 1
Part_BF_5.Mother Part_BF_1