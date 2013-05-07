// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo

Include Duke.v2.00.2cm.geo
Include Deckel.geo



// World volume section

// Volume WorldVolume             
// WorldVolume.Material Air
// WorldVolume.Visibility 0
// WorldVolume.Shape BRIK 1000. 1000. 1000.
// WorldVolume.Mother 0




Volume Geh_20_Volume             
Geh_20_Volume.Material Air
Geh_20_Volume.Visibility 0
Geh_20_Volume.Virtual true
Geh_20_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// Geh_20_Volume.Mother 0



// The DETECTOR UNIT consisting of 12 x 10 CsI Crystals an surrounding Millipore:

2cm_CsICore.Copy 2cm_Unit
2cm_Unit.Position 0. 0. 0.
2cm_Unit.Mother Geh_20_Volume


// The CAP:

CapVolume.Copy 2cm_Cap
2cm_Cap.Position 0. 0. 1.2
2cm_Cap.Mother Geh_20_Volume



// SURROUNDING ALUMINIUM:


// Plate on top (1):
Volume 2cm_TopPlate
2cm_TopPlate.Material Aluminium
2cm_TopPlate.Visibility 1
2cm_TopPlate.Color 1
// 2cm_TopPlate.Shape BRIK 3.1 3.65  0.1
2cm_TopPlate.Shape BRIK 3.085 3.655 0.1
2cm_TopPlate.Position 0. 0. -1.1
2cm_TopPlate.Mother Geh_20_Volume

// Plates on short side:
// FrontPlate (2):
Volume 2cm_FrontSidePlate
2cm_FrontSidePlate.Material Aluminium
// 2cm_FrontSidePlate.Shape BRIK 3.1 0.1 1.925.
2cm_FrontSidePlate.Shape BRIK 3.085 0.1 0.9
2cm_FrontSidePlate.Position 0. -3.555 -0.1
2cm_FrontSidePlate.Color 1
2cm_FrontSidePlate.Visibility 1
2cm_FrontSidePlate.Mother Geh_20_Volume

// BackPlate (3):
Volume 2cm_BackSidePlate
2cm_BackSidePlate.Material Aluminium
// 2cm_BackSidePlate.Shape BRIK 3.1 0.1 1.925.
2cm_BackSidePlate.Shape BRIK 3.085 0.1 0.9
2cm_BackSidePlate.Position 0. 3.555 -0.1
2cm_BackSidePlate.Color 1
2cm_BackSidePlate.Visibility 1
2cm_BackSidePlate.Mother Geh_20_Volume

// Plates on long side:
// RightPlate (4):
Volume 2cm_RightSidePlate
2cm_RightSidePlate.Material Aluminium
2cm_RightSidePlate.Shape BRIK 0.1 3.455 0.9
2cm_RightSidePlate.Position -2.985 0. -0.1
2cm_RightSidePlate.Color 1
2cm_RightSidePlate.Visibility 1
2cm_RightSidePlate.Mother Geh_20_Volume

// LeftPlate (5):
Volume 2cm_LeftSidePlate
2cm_LeftSidePlate.Material Aluminium
2cm_LeftSidePlate.Shape BRIK 0.1 3.455 0.9
2cm_LeftSidePlate.Position 2.985 0. -0.1
2cm_LeftSidePlate.Color 1
2cm_LeftSidePlate.Visibility 1
2cm_LeftSidePlate.Mother Geh_20_Volume



// Wings on short side:
// (6) right front wing:
Volume 2cm_SRightFrontWing
2cm_SRightFrontWing.Material Aluminium
2cm_SRightFrontWing.Shape BRIK 0.2 0.5 0.825
2cm_SRightFrontWing.Position 0.98 4.155 -0.375
2cm_SRightFrontWing.Color 1
2cm_SRightFrontWing.Visibility 1
2cm_SRightFrontWing.Mother Geh_20_Volume

// (7) right back wing:
Volume 2cm_SRightBackWing
2cm_SRightBackWing.Material Aluminium
2cm_SRightBackWing.Shape BRIK 0.2 0.5 0.825
2cm_SRightBackWing.Position 0.98 -4.155 -0.375
2cm_SRightBackWing.Color 1
2cm_SRightBackWing.Visibility 1
2cm_SRightBackWing.Mother Geh_20_Volume

// (8) right front wing:
Volume 2cm_SLeftFrontWing
2cm_SLeftFrontWing.Material Aluminium
2cm_SLeftFrontWing.Shape BRIK 0.2 0.5 0.825
2cm_SLeftFrontWing.Position -0.98 4.155 -0.375
2cm_SLeftFrontWing.Color 1
2cm_SLeftFrontWing.Visibility 1
2cm_SLeftFrontWing.Mother Geh_20_Volume

// (9) right back wing:
Volume 2cm_SLeftBackWing
2cm_SLeftBackWing.Material Aluminium
2cm_SLeftBackWing.Shape BRIK 0.2 0.5 0.825
2cm_SLeftBackWing.Position -0.98 -4.155 -0.375
2cm_SLeftBackWing.Color 1
2cm_SLeftBackWing.Visibility 1
2cm_SLeftBackWing.Mother Geh_20_Volume


// Wings on long side:
// (10) right front wing:
Volume 2cm_LRightFrontWing
2cm_LRightFrontWing.Material Aluminium
2cm_LRightFrontWing.Shape BRIK 0.45 0.15 0.3625
2cm_LRightFrontWing.Position 3.535 3.05 -0.2625
2cm_LRightFrontWing.Color 1
2cm_LRightFrontWing.Visibility 1
2cm_LRightFrontWing.Mother Geh_20_Volume

// (11) right back wing:
Volume 2cm_LRightBackWing
2cm_LRightBackWing.Material Aluminium
2cm_LRightBackWing.Shape BRIK 0.45 0.15 0.3625
2cm_LRightBackWing.Position 3.535 -3.05 -0.2625
2cm_LRightBackWing.Color 1
2cm_LRightBackWing.Visibility 1
2cm_LRightBackWing.Mother Geh_20_Volume

// (12) right front wing:
Volume 2cm_LLeftFrontWing
2cm_LLeftFrontWing.Material Aluminium
2cm_LLeftFrontWing.Shape BRIK 0.45 0.15 0.3625
2cm_LLeftFrontWing.Position -3.535 3.05 -0.2625
2cm_LLeftFrontWing.Color 1
2cm_LLeftFrontWing.Visibility 1
2cm_LLeftFrontWing.Mother Geh_20_Volume

// (13) right back wing:
Volume 2cm_LLeftBackWing
2cm_LLeftBackWing.Material Aluminium
2cm_LLeftBackWing.Shape BRIK 0.45 0.15 0.3625
2cm_LLeftBackWing.Position -3.535 -3.05 -0.2625
2cm_LLeftBackWing.Color 1
2cm_LLeftBackWing.Visibility 1
2cm_LLeftBackWing.Mother Geh_20_Volume


// -- everything seems to be fine to this point ------------------------------------------------------------
// -- correction from this point ---------------------------------------------------------------------------


// podests under wings on the long side:
// (14) right front wing:
Volume 2cm_LRightFrontPodest
2cm_LRightFrontPodest.Material Aluminium
2cm_LRightFrontPodest.Shape BRIK 0.725 0.45 0.275   //  z = 0.45
2cm_LRightFrontPodest.Position 3.81 2.75 0.375      //  z = 0.55
2cm_LRightFrontPodest.Color 1
2cm_LRightFrontPodest.Visibility 1
2cm_LRightFrontPodest.Mother Geh_20_Volume

// (15) right back wing:
Volume 2cm_LRightBackPodest
2cm_LRightBackPodest.Material Aluminium
2cm_LRightBackPodest.Shape BRIK  0.725 0.45 0.275   // z = 0.45
2cm_LRightBackPodest.Position 3.81 -2.75 0.375      // z = 0.55
2cm_LRightBackPodest.Color 1
2cm_LRightBackPodest.Visibility 1
2cm_LRightBackPodest.Mother Geh_20_Volume

// (16) right front wing:
Volume 2cm_LLeftFrontPodest
2cm_LLeftFrontPodest.Material Aluminium
2cm_LLeftFrontPodest.Shape BRIK  0.725 0.45 0.275   // z = 0.45
2cm_LLeftFrontPodest.Position -3.81 2.75 0.375      // z = 0.55
2cm_LLeftFrontPodest.Color 1
2cm_LLeftFrontPodest.Visibility 1
2cm_LLeftFrontPodest.Mother Geh_20_Volume

// (17) right back wing:
Volume 2cm_LLeftBackPodest
2cm_LLeftBackPodest.Material Aluminium
2cm_LLeftBackPodest.Shape BRIK  0.725 0.45 0.275    // z = 0.45
2cm_LLeftBackPodest.Position -3.81 -2.75 0.375      // z = 0.55
2cm_LLeftBackPodest.Color 1
2cm_LLeftBackPodest.Visibility 1
2cm_LLeftBackPodest.Mother Geh_20_Volume


// connections between podests:
// (18) right connection:
Volume 2cm_LRightConnection
2cm_LRightConnection.Material Aluminium
2cm_LRightConnection.Shape BRIK 0.725 2.3 0.15   // z = 0.325
2cm_LRightConnection.Position 3.81 0.0 0.5       // z = 0.675
2cm_LRightConnection.Color 1
2cm_LRightConnection.Visibility 1
2cm_LRightConnection.Mother Geh_20_Volume

// (19) left connection:
Volume 2cm_LLeftConnection
2cm_LLeftConnection.Material Aluminium
2cm_LLeftConnection.Shape BRIK  0.725 2.3 0.15   // z = 0.325
2cm_LLeftConnection.Position -3.81 0.0 0.5       // z = 0.675
2cm_LLeftConnection.Color 1
2cm_LLeftConnection.Visibility 1
2cm_LLeftConnection.Mother Geh_20_Volume




// (20) :
Volume 2cm_Part20
2cm_Part20.Material Aluminium
2cm_Part20.Shape BRIK 0.45 0.625 0.1	// z = 0.275
2cm_Part20.Position 3.535 3.825 0.55	// z = 0.725
2cm_Part20.Color 1
2cm_Part20.Visibility 1
2cm_Part20.Mother Geh_20_Volume

// (21) :
Volume 2cm_Part21
2cm_Part21.Material Aluminium
2cm_Part21.Shape BRIK  0.45 0.625 0.1	// z = 0.275
2cm_Part21.Position 3.535 -3.825 0.55 	// z = 0.725
2cm_Part21.Color 1
2cm_Part21.Visibility 1
2cm_Part21.Mother Geh_20_Volume

// (22) :
Volume 2cm_Part22
2cm_Part22.Material Aluminium
2cm_Part22.Shape BRIK  0.45 0.625 0.1 	// z = 0.275
2cm_Part22.Position -3.535 3.825 0.55 	// z = 0.725
2cm_Part22.Color 1
2cm_Part22.Visibility 1
2cm_Part22.Mother Geh_20_Volume

// (23) :
Volume 2cm_Part23
2cm_Part23.Material Aluminium
2cm_Part23.Shape BRIK  0.45 0.625 0.1	// z = 0.275
2cm_Part23.Position -3.535 -3.825 0.55 	// z = 0.725
2cm_Part23.Color 1
2cm_Part23.Visibility 1
2cm_Part23.Mother Geh_20_Volume

// -- corrected to this point -----------------------------------------------------


// (24) :
Volume 2cm_Part24
2cm_Part24.Material Aluminium
2cm_Part24.Shape BRIK 3.085 0.3975 0.175 	// z = 0.275
2cm_Part24.Position 0.0 4.0525 0.625		// z = 0.725
2cm_Part24.Color 1
2cm_Part24.Visibility 1
2cm_Part24.Mother Geh_20_Volume

// (25) :
Volume 2cm_Part25
2cm_Part25.Material Aluminium
2cm_Part25.Shape BRIK 3.085 0.3975 0.175 	// z = 0.275
2cm_Part25.Position 0.0 -4.0525 0.625 		// z = 0.725
2cm_Part25.Color 1
2cm_Part25.Visibility 1
2cm_Part25.Mother Geh_20_Volume





// (26) :
Volume 2cm_Part26
2cm_Part26.Material Aluminium
2cm_Part26.Shape BRIK 1.18 0.1025 0.125
2cm_Part26.Position 0.0 4.5525 0.575
2cm_Part26.Color 8
2cm_Part26.Visibility 1
2cm_Part26.Mother Geh_20_Volume

// (27) :
Volume 2cm_Part27
2cm_Part27.Material Aluminium
2cm_Part27.Shape BRIK 1.18 0.1025 0.125
2cm_Part27.Position 0.0 -4.5525 0.575
2cm_Part27.Color 8
2cm_Part27.Visibility 1
2cm_Part27.Mother Geh_20_Volume





// Circuit board and PIN-Diodes:
// circuit board (28):
// PUT THE RIGHT MATERIAL HERE, WHEN DEFINED IN matertials.geo:              <---
Volume 2cm_CircuitBoard
2cm_CircuitBoard.Material PCB
2cm_CircuitBoard.Shape BRIK 3.94 4.45 0.075    // y = 4.455, 26.03.02
2cm_CircuitBoard.Position 0. 0. 1.125
2cm_CircuitBoard.Color 2
2cm_CircuitBoard.Visibility 1
2cm_CircuitBoard.Mother Geh_20_Volume


// PIN-Diodes (29):
Volume 2cm_PINDiodes
2cm_PINDiodes.Material SiliconPIN
2cm_PINDiodes.Shape BRIK 2.965 3.485 0.025
2cm_PINDiodes.Position 0. 0. 1.025
2cm_PINDiodes.Color 5
2cm_PINDiodes.Visibility 1
2cm_PINDiodes.Mother Geh_20_Volume



// Big Discs on the short sides:
// (54)
Volume 2cm_Part54
2cm_Part54.Material Aluminium
2cm_Part54.Shape TUBS 0.25 1.6 .25 0. 360.
2cm_Part54.Position 0.0 4.905 -0.55
2cm_Part54.Rotation 90. 0. 0.
2cm_Part54.Color 1
2cm_Part54.Visibility 1
2cm_Part54.Mother Geh_20_Volume

// (55)
Volume 2cm_Part55
2cm_Part55.Material Aluminium
2cm_Part55.Shape TUBS 0.25 1.6 0.25 0. 360.
2cm_Part55.Position 0.0 -4.905 -0.55
2cm_Part55.Rotation 90. 0. 0.
2cm_Part55.Color 1
2cm_Part55.Visibility 1
2cm_Part55.Mother Geh_20_Volume


// Small Discs/Cylider behind big discs:
// (56)
Volume 2cm_Part56
2cm_Part56.Material Aluminium
2cm_Part56.Shape TUBS 0.25 0.8 0.45 0. 360.
2cm_Part56.Position 0.0 4.205 -0.55
2cm_Part56.Rotation 90. 0. 0.
2cm_Part56.Color 1
2cm_Part56.Visibility 1
2cm_Part56.Mother Geh_20_Volume

// (57)
Volume 2cm_Part57
2cm_Part57.Material Aluminium
2cm_Part57.Shape TUBS 0.25 0.8 0.45 0. 360.
2cm_Part57.Position 0.0 -4.205 -0.55
2cm_Part57.Rotation 90. 0. 0.
2cm_Part57.Color 1
2cm_Part57.Visibility 1
2cm_Part57.Mother Geh_20_Volume

// Srews through the discs:
// (59):
Volume 2cm_Screw59
2cm_Screw59.Material Aluminium
2cm_Screw59.Shape TUBS 0.0 0.25 0.7 0. 360.
2cm_Screw59.Position 0.0 4.455 -0.55
2cm_Screw59.Rotation 90. 0. 0.
2cm_Screw59.Color 9
2cm_Screw59.Visibility 1
2cm_Screw59.Mother Geh_20_Volume

// (60):
Volume 2cm_Screw60
2cm_Screw60.Material Aluminium
2cm_Screw60.Shape TUBS 0.0 0.25 0.7 0. 360.
2cm_Screw60.Position 0.0 -4.455 -0.55
2cm_Screw60.Rotation 90. 0. 0.
2cm_Screw60.Color 9
2cm_Screw60.Visibility 1
2cm_Screw60.Mother Geh_20_Volume



// some additional parts between podests and cap:


// (61) :
Volume 2cm_Part61
2cm_Part61.Material Aluminium
2cm_Part61.Shape BRIK 0.45 3.655 0.075
2cm_Part61.Position 3.535 0.0 0.725
2cm_Part61.Color 1
2cm_Part61.Visibility 1
2cm_Part61.Mother Geh_20_Volume

// (62) :
Volume 2cm_Part62
2cm_Part62.Material Aluminium
2cm_Part62.Shape BRIK 0.45 3.655 0.075
2cm_Part62.Position -3.535 0.0 0.725
2cm_Part62.Color 1
2cm_Part62.Visibility 1
2cm_Part62.Mother Geh_20_Volume


