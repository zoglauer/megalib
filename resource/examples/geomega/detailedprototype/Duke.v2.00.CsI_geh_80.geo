// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo

Include Duke.v2.00.8cm.geo
Include Deckel.geo


// World volume section

// Volume WorldVolume             
// WorldVolume.Material Air
// WorldVolume.Visibility 0
// WorldVolume.Shape BRIK 1000. 1000. 1000.
// WorldVolume.Mother 0


// The MOTHER VOLUME:
// ------------------

Volume Geh_80_Volume             
Geh_80_Volume.Material Air
Geh_80_Volume.Visibility 0
Geh_80_Volume.Shape BRIK 100. 100. 100.
Geh_80_Volume.Virtual true
// NEEDS THIS LINE TO VIEW ALONE:
// Geh_80_Volume.Mother 0




// The COMPONENTS:
// ---------------

// The DETECTOR UNIT consisting of 12 x 10 CsI Crystals an surrounding Millipore (1):
8cm_CsICore.Copy 8cm_Unit
8cm_Unit.Position 0. 0. 0.
8cm_Unit.Mother Geh_80_Volume




// Circuit board and PIN-Diodes:
// upper side:
// circuit board (2):
// PUT THE RIGHT MATERIAL HERE, WHEN DEFINED IN matertials.geo:              <---
Volume 8cm_upperCircuitBoard_2
8cm_upperCircuitBoard_2.Material PCB
8cm_upperCircuitBoard_2.Shape BRIK 3.94 4.45 0.075   // y = 4.455, 26.3.02, changed because collision with cap
8cm_upperCircuitBoard_2.Position 0. 0. 4.125
8cm_upperCircuitBoard_2.Color 2
8cm_upperCircuitBoard_2.Visibility 1
8cm_upperCircuitBoard_2.Mother Geh_80_Volume


// PIN-Diodes (3):
Volume 8cm_upperPINDiodes_3
8cm_upperPINDiodes_3.Material SiliconPIN
8cm_upperPINDiodes_3.Shape BRIK 2.965 3.485 0.025
8cm_upperPINDiodes_3.Position 0. 0. 4.025
8cm_upperPINDiodes_3.Color 5
8cm_upperPINDiodes_3.Visibility 1
8cm_upperPINDiodes_3.Mother Geh_80_Volume

// lower side:
// circuit board (4):
// PUT THE RIGHT MATERIAL HERE, WHEN DEFINED IN matertials.geo:              <---
Volume 8cm_lowerCircuitBoard_4
8cm_lowerCircuitBoard_4.Material PCB
8cm_lowerCircuitBoard_4.Shape BRIK 3.94 4.45 0.075    // y = 4.455, 26.3.02, changed because collision with cap
8cm_lowerCircuitBoard_4.Position 0. 0. -4.125
8cm_lowerCircuitBoard_4.Color 2
8cm_lowerCircuitBoard_4.Visibility 1
8cm_lowerCircuitBoard_4.Mother Geh_80_Volume


// PIN-Diodes (5):
Volume 8cm_lowerPINDiodes_5
8cm_lowerPINDiodes_5.Material SiliconPIN
8cm_lowerPINDiodes_5.Shape BRIK 2.965 3.485 0.025
8cm_lowerPINDiodes_5.Position 0. 0. -4.025
8cm_lowerPINDiodes_5.Color 5
8cm_lowerPINDiodes_5.Visibility 1
8cm_lowerPINDiodes_5.Mother Geh_80_Volume



// 4 CsICore surrounding walls:

// front wall (6):
Volume 8cm_FrontWall_6
8cm_FrontWall_6.Material Aluminium
8cm_FrontWall_6.Shape BRIK 4.0 0.125 3.8
8cm_FrontWall_6.Position 0.0 3.58 0.0
8cm_FrontWall_6.Color 1
8cm_FrontWall_6.Visibility 1
8cm_FrontWall_6.Mother Geh_80_Volume

// back wall (7):
Volume 8cm_BackWall_7
8cm_BackWall_7.Material Aluminium
8cm_BackWall_7.Shape BRIK 4.0 0.125 3.8
8cm_BackWall_7.Position 0.0 -3.58 0.0
8cm_BackWall_7.Color 1
8cm_BackWall_7.Visibility 1
8cm_BackWall_7.Mother Geh_80_Volume

// left wall (8):
Volume 8cm_LeftWall_8
8cm_LeftWall_8.Material Aluminium
8cm_LeftWall_8.Shape BRIK 0.1 3.455 3.65
8cm_LeftWall_8.Position -2.985 0.0 -0.0
8cm_LeftWall_8.Color 1
8cm_LeftWall_8.Visibility 1
8cm_LeftWall_8.Mother Geh_80_Volume

// right wall (9):
Volume 8cm_RightWall_9
8cm_RightWall_9.Material Aluminium
8cm_RightWall_9.Shape BRIK 0.1 3.455 3.65
8cm_RightWall_9.Position 2.985 0.0 0.0
8cm_RightWall_9.Color 1
8cm_RightWall_9.Visibility 1
8cm_RightWall_9.Mother Geh_80_Volume





// Part (10):
Volume 8cm_Part_10
8cm_Part_10.Material Aluminium
8cm_Part_10.Shape BRIK 0.4575 3.455 0.4
8cm_Part_10.Position 3.5425 0.0 3.25
8cm_Part_10.Color 1
8cm_Part_10.Visibility 1
8cm_Part_10.Mother Geh_80_Volume

// Part (11):
Volume 8cm_Part_11
8cm_Part_11.Material Aluminium
8cm_Part_11.Shape BRIK 0.4575 3.455 0.4
8cm_Part_11.Position 3.5425 0.0 -3.25
8cm_Part_11.Color 1
8cm_Part_11.Visibility 1
8cm_Part_11.Mother Geh_80_Volume

// Part (12):
Volume 8cm_Part_12
8cm_Part_12.Material Aluminium
8cm_Part_12.Shape BRIK 0.4575 3.455 0.4
8cm_Part_12.Position -3.5425 0.0 3.25
8cm_Part_12.Color 1
8cm_Part_12.Visibility 1
8cm_Part_12.Mother Geh_80_Volume

// Part (13):
Volume 8cm_Part_13
8cm_Part_13.Material Aluminium
8cm_Part_13.Shape BRIK 0.4575 3.455 0.4
8cm_Part_13.Position -3.5425 0.0 -3.25
8cm_Part_13.Color 1
8cm_Part_13.Visibility 1
8cm_Part_13.Mother Geh_80_Volume




// The outgoing cubics which catch the screws for the caps:
// Part (14):
Volume 8cm_Part_14
8cm_Part_14.Material Aluminium
8cm_Part_14.Shape BRIK 0.3 3.7 0.4
8cm_Part_14.Position 4.3 0.0 3.25
8cm_Part_14.Color 1
8cm_Part_14.Visibility 1
8cm_Part_14.Mother Geh_80_Volume

// Part (15):
Volume 8cm_Part_15
8cm_Part_15.Material Aluminium
8cm_Part_15.Shape BRIK 0.3 3.7 0.4
8cm_Part_15.Position -4.3 0.0 3.25
8cm_Part_15.Color 1
8cm_Part_15.Visibility 1
8cm_Part_15.Mother Geh_80_Volume

// Part (16):
Volume 8cm_Part_16
8cm_Part_16.Material Aluminium
8cm_Part_16.Shape BRIK 0.3 3.7 0.4
8cm_Part_16.Position 4.3 0.0 -3.25
8cm_Part_16.Color 1
8cm_Part_16.Visibility 1
8cm_Part_16.Mother Geh_80_Volume

// Part (17):
Volume 8cm_Part_17
8cm_Part_17.Material Aluminium
8cm_Part_17.Shape BRIK 0.3 3.7 0.4
8cm_Part_17.Position -4.3 0.0 -3.25
8cm_Part_17.Color 1
8cm_Part_17.Visibility 1
8cm_Part_17.Mother Geh_80_Volume



// Surroundings of the x-z-borderarea:
// thick long cubics along x-axis:

// Part (18):
Volume 8cm_Part_18
8cm_Part_18.Material Aluminium
8cm_Part_18.Shape BRIK 4.0 0.4725 0.25
8cm_Part_18.Position 0.0 4.1775 3.4
8cm_Part_18.Color 1
8cm_Part_18.Visibility 1
8cm_Part_18.Mother Geh_80_Volume

// Part (19):
Volume 8cm_Part_19
8cm_Part_19.Material Aluminium
8cm_Part_19.Shape BRIK 4.0 0.4725 0.25
8cm_Part_19.Position 0.0 4.1775 -3.4
8cm_Part_19.Color 1
8cm_Part_19.Visibility 1
8cm_Part_19.Mother Geh_80_Volume

// Part (20):
Volume 8cm_Part_20
8cm_Part_20.Material Aluminium
8cm_Part_20.Shape BRIK 4.0 0.4725 0.25
8cm_Part_20.Position 0.0 -4.1775 3.4
8cm_Part_20.Color 1
8cm_Part_20.Visibility 1
8cm_Part_20.Mother Geh_80_Volume

// Part (21):
Volume 8cm_Part_21
8cm_Part_21.Material Aluminium
8cm_Part_21.Shape BRIK 4.0 0.4725 0.25
8cm_Part_21.Position 0.0 -4.1775 -3.4
8cm_Part_21.Color 1
8cm_Part_21.Visibility 1
8cm_Part_21.Mother Geh_80_Volume



// lean and some shorter cubics along z-axis:
// Part (22):
Volume 8cm_Part_22
8cm_Part_22.Material Aluminium
8cm_Part_22.Shape BRIK 0.075 0.4725 3.15
8cm_Part_22.Position 3.925 4.1775 0.0
8cm_Part_22.Color 1
8cm_Part_22.Visibility 1
8cm_Part_22.Mother Geh_80_Volume

// Part (23):
Volume 8cm_Part_23
8cm_Part_23.Material Aluminium
8cm_Part_23.Shape BRIK 0.075 0.4725 3.15
8cm_Part_23.Position 3.925 -4.1775 0.0
8cm_Part_23.Color 2
8cm_Part_23.Visibility 1
8cm_Part_23.Mother Geh_80_Volume

// Part (24):
Volume 8cm_Part_24
8cm_Part_24.Material Aluminium
8cm_Part_24.Shape BRIK 0.075 0.4725 3.15
8cm_Part_24.Position -3.925 4.1775 0.0
8cm_Part_24.Color 1
8cm_Part_24.Visibility 1
8cm_Part_24.Mother Geh_80_Volume

// Part (25):
Volume 8cm_Part_25
8cm_Part_25.Material Aluminium
8cm_Part_25.Shape BRIK 0.075 0.4725 3.15
8cm_Part_25.Position -3.925 -4.1775 0.0
8cm_Part_25.Color 1
8cm_Part_25.Visibility 1
8cm_Part_25.Mother Geh_80_Volume





// Part (26):
Volume 8cm_Part_26
8cm_Part_26.Material Aluminium
8cm_Part_26.Shape BRIK 0.225 4.45 0.175
8cm_Part_26.Position 3.775 0.0 3.825
8cm_Part_26.Color 1
8cm_Part_26.Visibility 1
8cm_Part_26.Mother Geh_80_Volume

// Part (27):
Volume 8cm_Part_27
8cm_Part_27.Material Aluminium
8cm_Part_27.Shape BRIK 0.225 4.45 0.175
8cm_Part_27.Position -3.775 0.0 3.825
8cm_Part_27.Color 1
8cm_Part_27.Visibility 1
8cm_Part_27.Mother Geh_80_Volume

// Part (28):
Volume 8cm_Part_28
8cm_Part_28.Material Aluminium
8cm_Part_28.Shape BRIK 0.225 4.45 0.175
8cm_Part_28.Position 3.775 0.0 -3.825
8cm_Part_28.Color 1
8cm_Part_28.Visibility 1
8cm_Part_28.Mother Geh_80_Volume

// Part (29):
Volume 8cm_Part_29
8cm_Part_29.Material Aluminium
8cm_Part_29.Shape BRIK 0.225 4.45 0.175
8cm_Part_29.Position -3.775 0.0 -3.825
8cm_Part_29.Color 1
8cm_Part_29.Visibility 1
8cm_Part_29.Mother Geh_80_Volume





// Part (30):
Volume 8cm_Part_30
8cm_Part_30.Material Aluminium
8cm_Part_30.Shape BRIK 3.55 0.225 0.175
8cm_Part_30.Position 0.0 4.225 3.825
8cm_Part_30.Color 1
8cm_Part_30.Visibility 1
8cm_Part_30.Mother Geh_80_Volume

// Part (31):
Volume 8cm_Part_31
8cm_Part_31.Material Aluminium
8cm_Part_31.Shape BRIK 3.55 0.225 0.175
8cm_Part_31.Position 0.0 4.225 -3.825
8cm_Part_31.Color 1
8cm_Part_31.Visibility 1
8cm_Part_31.Mother Geh_80_Volume

// Part (32):
Volume 8cm_Part_32
8cm_Part_32.Material Aluminium
8cm_Part_32.Shape BRIK 3.55 0.225 0.175
8cm_Part_32.Position 0.0 -4.225 3.825
8cm_Part_32.Color 1
8cm_Part_32.Visibility 1
8cm_Part_32.Mother Geh_80_Volume

// Part (33):
Volume 8cm_Part_33
8cm_Part_33.Material Aluminium
8cm_Part_33.Shape BRIK 3.55 0.225 0.175
8cm_Part_33.Position 0.0 -4.225 -3.825
8cm_Part_33.Color 1
8cm_Part_33.Visibility 1
8cm_Part_33.Mother Geh_80_Volume



// CAP on top and on bottom:
CapVolume.Copy 8cm_UpperCap_34
8cm_UpperCap_34.Position 0.0 0.0 4.2
8cm_UpperCap_34.Mother Geh_80_Volume

CapVolume.Copy 8cm_LowerCap_35
8cm_LowerCap_35.Position 0.0 0.0 -4.2
8cm_LowerCap_35.Rotation 0. 180. 0.
8cm_LowerCap_35.Mother Geh_80_Volume