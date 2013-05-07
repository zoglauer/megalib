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



// The Mother Volume of Part B:
Volume Part_B_Volume             
Part_B_Volume.Material Air
Part_B_Volume.Visibility 0
Part_B_Volume.Shape BRIK 100. 100. 100.
Part_B_Volume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// Part_B_Volume.Mother 0




// The element parts of Part B:

// Back Wall:
Volume Part_B_1
Part_B_1.Material Aluminium
Part_B_1.Shape BRIK 1.9 2.65 0.1
Part_B_1.Position 0.0 0.15 0.1
Part_B_1.Color 1
Part_B_1.Visibility 1
Part_B_1.Mother Part_B_Volume

// Low Border:
Volume Part_B_2
Part_B_2.Material Aluminium
Part_B_2.Shape BRIK 1.9 0.45 0.45
Part_B_2.Position 0.0 -2.05 0.65
Part_B_2.Color 1
Part_B_2.Visibility 1
Part_B_2.Mother Part_B_Volume

// Up Border:
Volume Part_B_3
Part_B_3.Material Aluminium
Part_B_3.Shape BRIK 1.9 0.6 0.45
Part_B_3.Position 0.0 2.2 0.65
Part_B_3.Color 1
Part_B_3.Visibility 1
Part_B_3.Mother Part_B_Volume

// Right Border:
Volume Part_B_4
Part_B_4.Material Aluminium
Part_B_4.Shape BRIK 0.1 1.6 0.45
Part_B_4.Position 1.8 0.0 0.65
Part_B_4.Color 1
Part_B_4.Visibility 1
Part_B_4.Mother Part_B_Volume

// Left Border:
Volume Part_B_5
Part_B_5.Material Aluminium
Part_B_5.Shape BRIK 0.1 1.6 0.45
Part_B_5.Position -1.8 0.0 0.65
Part_B_5.Color 1
Part_B_5.Visibility 1
Part_B_5.Mother Part_B_Volume

// B6:
Volume Part_B_6
Part_B_6.Material Aluminium
Part_B_6.Shape BRIK 0.5 1.75 0.225
Part_B_6.Position 0.0 4.55 0.875
Part_B_6.Color 1
Part_B_6.Visibility 1
Part_B_6.Mother Part_B_Volume



