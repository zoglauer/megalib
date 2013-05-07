// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo

Include Duke.v2.00.4cm.geo



// World volume section

// Volume WorldVolume             
// WorldVolume.Material Air
// WorldVolume.Visibility 0
// WorldVolume.Shape BRIK 1000. 1000. 1000.
// WorldVolume.Mother 0


Volume Geh_40_Volume             
Geh_40_Volume.Material Air
Geh_40_Volume.Visibility 0
Geh_40_Volume.Virtual true
Geh_40_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// Geh_40_Volume.Mother 0



// The DETECTOR UNIT consisting of 12 x 10 CsI Crystals an surrounding Millipore:

4cm_CsICore.Copy 4cm_Unit
4cm_Unit.Position 0. 0. 0.
4cm_Unit.Mother Geh_40_Volume


// SURROUNDING ALUMINIUM:


// Plate on top (1):
Volume 4cm_TopPlate
4cm_TopPlate.Material Aluminium
4cm_TopPlate.Visibility 1
4cm_TopPlate.Color 1
// 4cm_TopPlate.Shape BRIK 3.1 3.65  0.1
4cm_TopPlate.Shape BRIK 3.085 3.655 0.1
4cm_TopPlate.Position 0. 0. -2.1
4cm_TopPlate.Mother Geh_40_Volume

// Plates on short side:
// FrontPlate (2):
Volume 4cm_FrontSidePlate
4cm_FrontSidePlate.Material Aluminium
// 4cm_FrontSidePlate.Shape BRIK 3.1 0.1 1.925.
4cm_FrontSidePlate.Shape BRIK 3.085 0.1 1.9
4cm_FrontSidePlate.Position 0. -3.555 -0.1
4cm_FrontSidePlate.Color 1
4cm_FrontSidePlate.Visibility 1
4cm_FrontSidePlate.Mother Geh_40_Volume

// BackPlate (3):
Volume 4cm_BackSidePlate
4cm_BackSidePlate.Material Aluminium
// 4cm_BackSidePlate.Shape BRIK 3.1 0.1 1.925.
4cm_BackSidePlate.Shape BRIK 3.085 0.1 1.9
4cm_BackSidePlate.Position 0. 3.555 -0.1
4cm_BackSidePlate.Color 1
4cm_BackSidePlate.Visibility 1
4cm_BackSidePlate.Mother Geh_40_Volume

// Plates on long side:
// RightPlate (4):
Volume 4cm_RightSidePlate
4cm_RightSidePlate.Material Aluminium
4cm_RightSidePlate.Shape BRIK 0.1 3.455 1.9
4cm_RightSidePlate.Position -2.985 0. -0.1
4cm_RightSidePlate.Color 1
4cm_RightSidePlate.Visibility 1
4cm_RightSidePlate.Mother Geh_40_Volume

// LeftPlate (5):
Volume 4cm_LeftSidePlate
4cm_LeftSidePlate.Material Aluminium
4cm_LeftSidePlate.Shape BRIK 0.1 3.455 1.9
4cm_LeftSidePlate.Position 2.985 0. -0.1
4cm_LeftSidePlate.Color 1
4cm_LeftSidePlate.Visibility 1
4cm_LeftSidePlate.Mother Geh_40_Volume



// Wings on the long side:
// Right side front wing (6):
Volume 4cm_LRightFrontWing
4cm_LRightFrontWing.Material Aluminium
4cm_LRightFrontWing.Shape BRIK 0.45 0.15 1.375
4cm_LRightFrontWing.Position 3.535 3.05 -0.275
4cm_LRightFrontWing.Color 1
4cm_LRightFrontWing.Visibility 1
4cm_LRightFrontWing.Mother Geh_40_Volume

// Right side back wing (7):
Volume 4cm_LRightBackWing
4cm_LRightBackWing.Material Aluminium
4cm_LRightBackWing.Shape BRIK 0.45 0.15 1.375
4cm_LRightBackWing.Position 3.535 -3.05 -0.275
4cm_LRightBackWing.Color 1
4cm_LRightBackWing.Visibility 1
4cm_LRightBackWing.Mother Geh_40_Volume

// Left side front wing (8):
Volume 4cm_LLeftFrontWing
4cm_LLeftFrontWing.Material Aluminium
4cm_LLeftFrontWing.Shape BRIK 0.45 0.15 1.375
4cm_LLeftFrontWing.Position -3.535 3.05 -0.275
4cm_LLeftFrontWing.Color 1
4cm_LLeftFrontWing.Visibility 1
4cm_LLeftFrontWing.Mother Geh_40_Volume

// Left side back wing (9):
Volume 4cm_LLeftBackWing
4cm_LLeftBackWing.Material Aluminium
4cm_LLeftBackWing.Shape BRIK 0.45 0.15 1.375
4cm_LLeftBackWing.Position -3.535 -3.05 -0.275
4cm_LLeftBackWing.Color 1
4cm_LLeftBackWing.Visibility 1
4cm_LLeftBackWing.Mother Geh_40_Volume


// Podests under wings on the long side:
// (10):
Volume 4cm_LRightFrontPodest
4cm_LRightFrontPodest.Material Aluminium
4cm_LRightFrontPodest.Shape BRIK 0.725 0.45 0.35
4cm_LRightFrontPodest.Position 3.81 2.75 1.45
4cm_LRightFrontPodest.Color 1
4cm_LRightFrontPodest.Visibility 1
4cm_LRightFrontPodest.Mother Geh_40_Volume

// (11):
Volume 4cm_LRightBackPodest
4cm_LRightBackPodest.Material Aluminium
4cm_LRightBackPodest.Shape BRIK 0.725 0.45 0.35
4cm_LRightBackPodest.Position 3.81 -2.75 1.45
4cm_LRightBackPodest.Color 1
4cm_LRightBackPodest.Visibility 1
4cm_LRightBackPodest.Mother Geh_40_Volume

// Podests under wings on the long side:
// (12)
Volume 4cm_LLeftFrontPodest
4cm_LLeftFrontPodest.Material Aluminium
4cm_LLeftFrontPodest.Shape BRIK 0.725 0.45 0.35
4cm_LLeftFrontPodest.Position -3.81 2.75 1.45
4cm_LLeftFrontPodest.Color 1
4cm_LLeftFrontPodest.Visibility 1
4cm_LLeftFrontPodest.Mother Geh_40_Volume

// Podests under wings on the long side:
// (13)
Volume 4cm_LLeftBackPodest
4cm_LLeftBackPodest.Material Aluminium
4cm_LLeftBackPodest.Shape BRIK 0.725 0.45 0.35
4cm_LLeftBackPodest.Position -3.81 -2.75 1.45
4cm_LLeftBackPodest.Color 1
4cm_LLeftBackPodest.Visibility 1
4cm_LLeftBackPodest.Mother Geh_40_Volume



// Wings on the Short side:
// Right side front wing (14):
Volume 4cm_SRightFrontWing
4cm_SRightFrontWing.Material Aluminium
4cm_SRightFrontWing.Shape BRIK 0.1 0.5 1.75
4cm_SRightFrontWing.Position 0.9 4.155 0.05
4cm_SRightFrontWing.Color 1
4cm_SRightFrontWing.Visibility 1
4cm_SRightFrontWing.Mother Geh_40_Volume

// Right side back wing (15):
Volume 4cm_SRightBackWing
4cm_SRightBackWing.Material Aluminium
4cm_SRightBackWing.Shape BRIK 0.1 0.5 1.75
4cm_SRightBackWing.Position 0.9 -4.155 0.05
4cm_SRightBackWing.Color 1
4cm_SRightBackWing.Visibility 1
4cm_SRightBackWing.Mother Geh_40_Volume

// Left side front wing (16):
Volume 4cm_SLeftFrontWing
4cm_SLeftFrontWing.Material Aluminium
4cm_SLeftFrontWing.Shape BRIK 0.1 0.5 1.75
4cm_SLeftFrontWing.Position -0.9 4.155 0.05
4cm_SLeftFrontWing.Color 1
4cm_SLeftFrontWing.Visibility 1
4cm_SLeftFrontWing.Mother Geh_40_Volume

// Left side back wing (17):
Volume 4cm_SLeftBackWing
4cm_SLeftBackWing.Material Aluminium
4cm_SLeftBackWing.Shape BRIK 0.1 0.5 1.75
4cm_SLeftBackWing.Position -0.9 -4.155 0.05
4cm_SLeftBackWing.Color 1
4cm_SLeftBackWing.Visibility 1
4cm_SLeftBackWing.Mother Geh_40_Volume


// (18) (right side):
Volume 4cm_Part18
4cm_Part18.Material Aluminium
4cm_Part18.Shape BRIK 0.725 2.3 0.225
4cm_Part18.Position 3.81 0.0 1.575
4cm_Part18.Color 1
4cm_Part18.Visibility 1
4cm_Part18.Mother Geh_40_Volume

// (19) (left side):
Volume 4cm_Part19
4cm_Part19.Material Aluminium
4cm_Part19.Shape BRIK 0.725 2.3 0.225
4cm_Part19.Position -3.81 0.0 1.575
4cm_Part19.Color 1
4cm_Part19.Visibility 1
4cm_Part19.Mother Geh_40_Volume





// Screws:
// PUT THE RIGHT MATERIAL HERE, WHEN DEFINED IN Materials.geo:               <---

// (20):
Volume 4cm_Screw1
4cm_Screw1.Material Iron
4cm_Screw1.Shape TUBS 0. 0.2 0.1 0. 360.
4cm_Screw1.Position  4.3 2.6 2.6
4cm_Screw1.Color 7
4cm_Screw1.Visibility 1
4cm_Screw1.Mother Geh_40_Volume

// (21):
Volume 4cm_Screw2
4cm_Screw2.Material Iron
4cm_Screw2.Shape TUBS 0. 0.2 0.1 0. 360.
4cm_Screw2.Position -4.3 2.6 2.6
4cm_Screw2.Color 7
4cm_Screw2.Visibility 1
4cm_Screw2.Mother Geh_40_Volume

// (22):
Volume 4cm_Screw3
4cm_Screw3.Material Iron
4cm_Screw3.Shape TUBS 0. 0.2 0.1 0. 360.
4cm_Screw3.Position 4.3 -2.6 2.6
4cm_Screw3.Color 7
4cm_Screw3.Visibility 1
4cm_Screw3.Mother Geh_40_Volume

// (23):
Volume 4cm_Screw4
4cm_Screw4.Material Iron
4cm_Screw4.Shape TUBS 0. 0.2 0.1 0. 360.
4cm_Screw4.Position -4.3 -2.6 2.6
4cm_Screw4.Color 7
4cm_Screw4.Visibility 1
4cm_Screw4.Mother Geh_40_Volume


// (24): not existent


// No more srews from here.

// Four aluminium cuboids as lower borders:
// (25):
Volume 4cm_Part25
4cm_Part25.Material Aluminium
4cm_Part25.Shape BRIK 0.30 4.65 0.35
4cm_Part25.Position 4.24 0.0 2.15
4cm_Part25.Color 1
4cm_Part25.Visibility 1
4cm_Part25.Mother Geh_40_Volume

// (26):
Volume 4cm_Part26
4cm_Part26.Material Aluminium
4cm_Part26.Shape BRIK 0.30 4.65 0.35
4cm_Part26.Position -4.24 0.0 2.15
4cm_Part26.Color 1
4cm_Part26.Visibility 1
4cm_Part26.Mother Geh_40_Volume

// (27):
Volume 4cm_Part27
4cm_Part27.Material Aluminium
4cm_Part27.Shape BRIK 3.94 0.1 0.35
4cm_Part27.Position 0.0 4.555 2.15
4cm_Part27.Color 1
4cm_Part27.Visibility 1
4cm_Part27.Mother Geh_40_Volume

// (28):
Volume 4cm_Part28
4cm_Part28.Material Aluminium
4cm_Part28.Shape BRIK 3.94 0.1 0.35
4cm_Part28.Position 0.0 -4.555 2.15
4cm_Part28.Color 1
4cm_Part28.Visibility 1
4cm_Part28.Mother Geh_40_Volume


// Circuit board and PIN-Diodes:
// circuit board (29):
// PUT THE RIGHT MATERIAL HERE, WHEN DEFINED IN matertials.geo:              <---
Volume 4cm_CircuitBoard
4cm_CircuitBoard.Material PCB
4cm_CircuitBoard.Shape BRIK 3.94 4.455 0.075
4cm_CircuitBoard.Position 0. 0. 2.125
4cm_CircuitBoard.Color 2
4cm_CircuitBoard.Visibility 1
4cm_CircuitBoard.Mother Geh_40_Volume


// PIN-Diodes (30):
Volume 4cm_PINDiodes
4cm_PINDiodes.Material SiliconPIN
4cm_PINDiodes.Shape BRIK 2.965 3.485 0.025
4cm_PINDiodes.Position 0. 0. 2.025
4cm_PINDiodes.Color 5
4cm_PINDiodes.Visibility 1
4cm_PINDiodes.Mother Geh_40_Volume


// (31):
Volume 4cm_Part31
4cm_Part31.Material Aluminium
4cm_Part31.Shape BRIK 0.2 4.455 0.1
4cm_Part31.Position 3.74 0.0 1.9
4cm_Part31.Color 1
4cm_Part31.Visibility 1
4cm_Part31.Mother Geh_40_Volume

// (32):
Volume 4cm_Part32
4cm_Part32.Material Aluminium
4cm_Part32.Shape BRIK 0.2 4.455 0.1
4cm_Part32.Position -3.74 0.0 1.9
4cm_Part32.Color 1
4cm_Part32.Visibility 1
4cm_Part32.Mother Geh_40_Volume

// (33):
Volume 4cm_Part33
4cm_Part33.Material Aluminium
4cm_Part33.Shape BRIK 3.54 0.2 0.1
4cm_Part33.Position 0.0 4.255 1.9
4cm_Part33.Color 1
4cm_Part33.Visibility 1
4cm_Part33.Mother Geh_40_Volume

// (34):
Volume 4cm_Part34
4cm_Part34.Material Aluminium
4cm_Part34.Shape BRIK 3.54 0.2 0.1
4cm_Part34.Position 0.0 -4.255 1.9
4cm_Part34.Color 1
4cm_Part34.Visibility 1
4cm_Part34.Mother Geh_40_Volume





// (35):
Volume 4cm_Part35
4cm_Part35.Material Aluminium
4cm_Part35.Shape BRIK 0.05 4.455 0.45
4cm_Part35.Position 3.89 0. 2.65
4cm_Part35.Color 1
4cm_Part35.Visibility 1
4cm_Part35.Mother Geh_40_Volume

// (36):
Volume 4cm_Part36
4cm_Part36.Material Aluminium
4cm_Part36.Shape BRIK 0.05 4.455 0.45
4cm_Part36.Position  -3.89 0. 2.65
4cm_Part36.Color 1
4cm_Part36.Visibility 1
4cm_Part36.Mother Geh_40_Volume

// (37):
Volume 4cm_Part37
4cm_Part37.Material Aluminium
4cm_Part37.Shape BRIK 3.84 0.1 0.55
4cm_Part37.Position 0.0 4.355 2.75
4cm_Part37.Color 1
4cm_Part37.Visibility 1
4cm_Part37.Mother Geh_40_Volume

// (38):
Volume 4cm_Part38
4cm_Part38.Material Aluminium
4cm_Part38.Shape BRIK 3.84 0.1 0.55
4cm_Part38.Position 0.0 -4.355 2.75
4cm_Part38.Color 1
4cm_Part38.Visibility 1
4cm_Part38.Mother Geh_40_Volume



// Big Plate on Bottom (39):
// Recalculate the both z-values!!  
Volume 4cm_Part39
4cm_Part39.Material Aluminium
4cm_Part39.Shape BRIK 3.84 4.235 .14
4cm_Part39.Position 0. 0. 3.24
4cm_Part39.Color 1
4cm_Part39.Visibility 1
4cm_Part39.Mother Geh_40_Volume


// Some holes to fill:

// (40)  
Volume 4cm_Part40
4cm_Part40.Material Aluminium
4cm_Part40.Shape BRIK 0.7275 0.725 0.075
4cm_Part40.Position 3.8125 3.925 1.725 
4cm_Part40.Color 1
4cm_Part40.Visibility 1
4cm_Part40.Mother Geh_40_Volume

// (41)
Volume 4cm_Part41
4cm_Part41.Material Aluminium
4cm_Part41.Shape BRIK 0.7275 0.725 0.075
4cm_Part41.Position 3.8125 -3.925 1.725 
4cm_Part41.Color 1
4cm_Part41.Visibility 1
4cm_Part41.Mother Geh_40_Volume

// (42)
Volume 4cm_Part42
4cm_Part42.Material Aluminium
4cm_Part42.Shape BRIK 0.7275 0.725 0.075
4cm_Part42.Position -3.8125 3.925 1.725 
4cm_Part42.Color 1
4cm_Part42.Visibility 1
4cm_Part42.Mother Geh_40_Volume

// (43)
Volume 4cm_Part43
4cm_Part43.Material Aluminium
4cm_Part43.Shape BRIK 0.7275 0.725 0.075
4cm_Part43.Position -3.8125 -3.925 1.725 
4cm_Part43.Color 1
4cm_Part43.Visibility 1
4cm_Part43.Mother Geh_40_Volume




// (44)  
Volume 4cm_Part44
4cm_Part44.Material Aluminium
4cm_Part44.Shape BRIK 1.0425 0.4975 0.075
4cm_Part44.Position 2.0425 4.1525 1.725
4cm_Part44.Color 1
4cm_Part44.Visibility 1
4cm_Part44.Mother Geh_40_Volume

// (45)
Volume 4cm_Part45
4cm_Part45.Material Aluminium
4cm_Part45.Shape BRIK 1.0425 0.4975 0.075
4cm_Part45.Position 2.0425 -4.1525 1.725
4cm_Part45.Color 1
4cm_Part45.Visibility 1
4cm_Part45.Mother Geh_40_Volume

// (46)
Volume 4cm_Part46
4cm_Part46.Material Aluminium
4cm_Part46.Shape BRIK 1.0425 0.4975 0.075
4cm_Part46.Position -2.0425 4.1525 1.725
4cm_Part46.Color 1
4cm_Part46.Visibility 1
4cm_Part46.Mother Geh_40_Volume

// (47)
Volume 4cm_Part47
4cm_Part47.Material Aluminium
4cm_Part47.Shape BRIK 1.0425 0.4975 0.075
4cm_Part47.Position -2.0425 -4.1525 1.725
4cm_Part47.Color 1
4cm_Part47.Visibility 1
4cm_Part47.Mother Geh_40_Volume



// (48)
Volume 4cm_Part48
4cm_Part48.Material Aluminium
4cm_Part48.Shape BRIK 0.8 0.4975 0.075
4cm_Part48.Position 0.0 4.1525 1.725
4cm_Part48.Color 1
4cm_Part48.Visibility 1
4cm_Part48.Mother Geh_40_Volume

// (49)
Volume 4cm_Part49
4cm_Part49.Material Aluminium
4cm_Part49.Shape BRIK 0.8 0.4975 0.075
4cm_Part49.Position 0.0 -4.1525 1.725
4cm_Part49.Color 1
4cm_Part49.Visibility 1
4cm_Part49.Mother Geh_40_Volume



// (50)
Volume 4cm_Part50
4cm_Part50.Material Aluminium
4cm_Part50.Shape BRIK 0.625 0.15 0.45
4cm_Part50.Position 1.75 1.45 2.65
4cm_Part50.Color 1
4cm_Part50.Visibility 1
4cm_Part50.Mother Geh_40_Volume

// (51)
Volume 4cm_Part51
4cm_Part51.Material Aluminium
4cm_Part51.Shape BRIK 0.625 0.15 0.45
4cm_Part51.Position 1.75 -1.45 2.65
4cm_Part51.Color 1
4cm_Part51.Visibility 1
4cm_Part51.Mother Geh_40_Volume

// (52)
Volume 4cm_Part52
4cm_Part52.Material Aluminium
4cm_Part52.Shape BRIK 0.625 0.15 0.45
4cm_Part52.Position -1.75 1.45 2.65
4cm_Part52.Color 1
4cm_Part52.Visibility 1
4cm_Part52.Mother Geh_40_Volume

// (53)
Volume 4cm_Part53
4cm_Part53.Material Aluminium
4cm_Part53.Shape BRIK 0.625 0.15 0.45
4cm_Part53.Position -1.75 -1.45 2.65
4cm_Part53.Color 1
4cm_Part53.Visibility 1
4cm_Part53.Mother Geh_40_Volume




// Big Discs on the short sides:
// (54)
Volume 4cm_Part54
4cm_Part54.Material Aluminium
4cm_Part54.Shape TUBS 0.25 1.6 .25 0. 360.
4cm_Part54.Position 0.0 4.905 0.15
4cm_Part54.Rotation 90. 0. 0.
4cm_Part54.Color 1
4cm_Part54.Visibility 1
4cm_Part54.Mother Geh_40_Volume

// (55)
Volume 4cm_Part55
4cm_Part55.Material Aluminium
4cm_Part55.Shape TUBS 0.25 1.6 0.25 0. 360.
4cm_Part55.Position 0.0 -4.905 0.15
4cm_Part55.Rotation 90. 0. 0.
4cm_Part55.Color 1
4cm_Part55.Visibility 1
4cm_Part55.Mother Geh_40_Volume


// Small Discs/Cylider behind big discs:
// (56)
Volume 4cm_Part56
4cm_Part56.Material Aluminium
4cm_Part56.Shape TUBS 0.25 0.8 0.45 0. 360.
4cm_Part56.Position 0.0 4.205 0.15
4cm_Part56.Rotation 90. 0. 0.
4cm_Part56.Color 1
4cm_Part56.Visibility 1
4cm_Part56.Mother Geh_40_Volume

// (57)
Volume 4cm_Part57
4cm_Part57.Material Aluminium
4cm_Part57.Shape TUBS 0.25 0.8 0.45 0. 360.
4cm_Part57.Position 0.0 -4.205 0.15
4cm_Part57.Rotation 90. 0. 0.
4cm_Part57.Color 1
4cm_Part57.Visibility 1
4cm_Part57.Mother Geh_40_Volume

// Srews through the discs:
// (59):
Volume 4cm_Screw59
4cm_Screw59.Material Aluminium
4cm_Screw59.Shape TUBS 0.0 0.25 0.7 0. 360.
4cm_Screw59.Position 0.0 4.455 0.15
4cm_Screw59.Rotation 90. 0. 0.
4cm_Screw59.Color 9
4cm_Screw59.Visibility 1
4cm_Screw59.Mother Geh_40_Volume

// (60):
Volume 4cm_Screw60
4cm_Screw60.Material Aluminium
4cm_Screw60.Shape TUBS 0.0 0.25 0.7 0. 360.
4cm_Screw60.Position 0.0 -4.455 0.15
4cm_Screw60.Rotation 90. 0. 0.
4cm_Screw60.Color 9
4cm_Screw60.Visibility 1
4cm_Screw60.Mother Geh_40_Volume




// Hole on short side of the cap:
// (58)
Volume 4cm_Part58
4cm_Part58.Material Air
4cm_Part58.Shape BRIK 2.1 0.1 0.4
4cm_Part58.Position 0.0 0.0 -0.15
4cm_Part58.Color 8
4cm_Part58.Visibility 1
4cm_Part58.Mother 4cm_Part37


// -- Geometry OK till here -------------------------------------// 
