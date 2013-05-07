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



// The Mother Volume of Part C:
Volume Part_C_Volume             
Part_C_Volume.Material Air
Part_C_Volume.Visibility 0
Part_C_Volume.Shape BRIK 100. 100. 100.
Part_C_Volume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// Part_C_Volume.Mother 0




// The element parts of Part C:

// C1
Volume Part_C_1
Part_C_1.Material Aluminium
Part_C_1.Shape BRIK 0.5 0.5 0.65
Part_C_1.Position 1.0 0.5 0.65
Part_C_1.Color 1
Part_C_1.Visibility 1
Part_C_1.Mother Part_C_Volume

// C2:
Volume Part_C_2
Part_C_2.Material Aluminium
Part_C_2.Shape BRIK 0.25 4.0 0.65
Part_C_2.Position 0.25 4.0 0.65
Part_C_2.Color 1
Part_C_2.Visibility 1
Part_C_2.Mother Part_C_Volume

// C3:
Volume Part_C_3
Part_C_3.Material Aluminium
Part_C_3.Shape BRIK 0.95 2.5 0.55
Part_C_3.Position -0.95 5.5 0.55
Part_C_3.Color 1
Part_C_3.Visibility 1
Part_C_3.Mother Part_C_Volume


// the mirrored parts:

// C4
Volume Part_C_4
Part_C_4.Material Aluminium
Part_C_4.Shape BRIK 0.5 0.5 0.65
Part_C_4.Position 1.0 0.5 -0.65
Part_C_4.Color 1
Part_C_4.Visibility 1
Part_C_4.Mother Part_C_Volume

// C5:
Volume Part_C_5
Part_C_5.Material Aluminium
Part_C_5.Shape BRIK 0.25 4.0 0.65
Part_C_5.Position 0.25 4.0 -0.65
Part_C_5.Color 1
Part_C_5.Visibility 1
Part_C_5.Mother Part_C_Volume

// C6:
Volume Part_C_6
Part_C_6.Material Aluminium
Part_C_6.Shape BRIK 0.95 2.5 0.55
Part_C_6.Position -0.95 5.5 -0.55
Part_C_6.Color 1
Part_C_6.Visibility 1
Part_C_6.Mother Part_C_Volume
