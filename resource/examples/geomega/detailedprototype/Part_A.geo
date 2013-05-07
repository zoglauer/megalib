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



// The Mother Volume of Part A:
Volume Part_A_Volume             
Part_A_Volume.Material Air
Part_A_Volume.Visibility 0
Part_A_Volume.Shape BRIK 100. 100. 100.
Part_A_Volume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// Part_A_Volume.Mother 0




// The parts of Part A:

// Border on top:
Volume Part_A_1
Part_A_1.Material Aluminium
Part_A_1.Shape BRIK 4.0 0.1 0.5
Part_A_1.Position 0.0 7.9 -0.15
Part_A_1.Color 1
Part_A_1.Visibility 1
Part_A_1.Mother Part_A_Volume

// back Back Wall:
Volume Part_A_2
Part_A_2.Material Aluminium
Part_A_2.Shape BRIK 4.0 2.4 0.15
Part_A_2.Position 0.0 5.4 -0.5
Part_A_2.Color 1
Part_A_2.Visibility 1
Part_A_2.Mother Part_A_Volume

// Connection from back Back Wall to front Back Wall:
Volume Part_A_3
Part_A_3.Material Aluminium
Part_A_3.Shape BRIK 4.0 0.1 0.5
Part_A_3.Position 0.0 2.9 -0.15
Part_A_3.Color 1
Part_A_3.Visibility 1
Part_A_3.Mother Part_A_Volume

// front Back Wall:
Volume Part_A_4
Part_A_4.Material Aluminium
Part_A_4.Shape BRIK 4.0 1.2 0.175
Part_A_4.Position 0.0 1.6 0.175
Part_A_4.Color 1
Part_A_4.Visibility 1
Part_A_4.Mother Part_A_Volume

// Border on bottom:
Volume Part_A_5
Part_A_5.Material Aluminium
Part_A_5.Shape BRIK 4.0 0.2 0.675
Part_A_5.Position 0.0 0.2 0.675
Part_A_5.Color 1
Part_A_5.Visibility 1
Part_A_5.Mother Part_A_Volume


// Side Walls:

// upper side walls:
// right:
Volume Part_A_6
Part_A_6.Material Aluminium
Part_A_6.Shape BRIK 0.1 2.4 0.525
Part_A_6.Position 3.9 5.4 0.175
Part_A_6.Color 1
Part_A_6.Visibility 1
Part_A_6.Mother Part_A_Volume

//left:
Volume Part_A_7
Part_A_7.Material Aluminium
Part_A_7.Shape BRIK 0.1 2.4 0.525
Part_A_7.Position -3.9 5.4 0.175
Part_A_7.Color 1
Part_A_7.Visibility 1
Part_A_7.Mother Part_A_Volume



// lower side walls:

// right:
Volume Part_A_8
Part_A_8.Material Aluminium
Part_A_8.Shape BRIK 0.1 1.3 0.45
Part_A_8.Position 3.9 1.7 0.8
Part_A_8.Color 1
Part_A_8.Visibility 1
Part_A_8.Mother Part_A_Volume

// left:
Volume Part_A_9
Part_A_9.Material Aluminium
Part_A_9.Shape BRIK 0.1 1.3 0.45
Part_A_9.Position -3.9 1.7 0.8
Part_A_9.Color 1
Part_A_9.Visibility 1
Part_A_9.Mother Part_A_Volume


// ----->> Geometry OK untill here <<---------------------------------------------------------------------------------------------------------
