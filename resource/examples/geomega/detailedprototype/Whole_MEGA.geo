// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
//Include Materials.geo


Include Rahmen.geo
Include PassiveRahmen.geo
Include 80mm_Detector_Config.geo
Include Duke.v2.00.CsI_geh_20.geo
Include Duke.v2.00.CsI_geh_40.geo
Include Green_Wall.geo
Include Red_Wall.geo
Include Side_Wall_A.geo
Include Side_Wall_A_mirrored.geo
Include Side_Wall_B.geo
Include Side_Wall_B_mirrored.geo



// The Mother Volume of the Detector Configuration:
Volume MEGA_Volume
MEGA_Volume.Material Air
MEGA_Volume.Visibility 0
MEGA_Volume.Virtual true
MEGA_Volume.Shape BRIK 200. 200. 200.
// NEEDS THIS LINE TO VIEW ALONE:
//MEGA_Volume.Mother 0


// 10 Layers of Si detectors:



// In the drawing the distance between two layers seems to be 1.05 cm. In Reality the distance was measured as exactly 1.00 cm!
// I changed the values in this file, but kept the former values in comments behind.
// I "defined" the position of Layer 5 as right, beause this one is in the middle of all layers, and also closest to the origin!
// April 12th: I redefined the z-posititions, to have an exact distance of 50 mm beteween the upper surface of the lowest Si-Wafer,
// and the upper surface of the 8 cm CsI Crystals.

// (1):
Frame_Volume.Copy bachus
bachus.Position 0.0 0.0 4.58		// z = 4.45
bachus.Mother MEGA_Volume

// (2):
Frame_Volume.Copy neptun
neptun.Position 0.0 0.0 3.58		// z = 3.4
neptun.Mother MEGA_Volume

// (3):
Frame_Volume.Copy poseidon
poseidon.Position 0.0 0.0 2.58		// z = 2.35
poseidon.Mother MEGA_Volume

// (4):
Frame_Volume.Copy achilles
achilles.Position 0.0 0.0 1.58		// z = 1.3     
achilles.Mother MEGA_Volume

// (5):
Frame_Volume.Copy merkur
merkur.Position 0.0 0.0 0.58      // I "define" this one as right positioned!
merkur.Mother MEGA_Volume

// (6):
Frame_Volume.Copy odysseus
odysseus.Position 0.0 0.0 -0.42 		// z = -0.8
odysseus.Mother MEGA_Volume

// (7):
Frame_Volume.Copy apoll
apoll.Position 0.0 0.0 -1.42		// z = -1.85
apoll.Mother MEGA_Volume

// (8):
Frame_Volume.Copy ikarus
ikarus.Position 0.0 0.0 -2.42		// z = -2.9
ikarus.Mother MEGA_Volume

// (9):
Frame_Volume.Copy sisyphus
sisyphus.Position 0.0 0.0 -3.42		// z = -3.95
sisyphus.Mother MEGA_Volume

// (10):
Frame_Volume.Copy vulkan
vulkan.Position 0.0 0.0 -4.42		// z = -5.0
vulkan.Mother MEGA_Volume

// Optional two Layers:
// (OL1):
PassiveFrame_Volume.Copy hermes
hermes.Position 0.0 0.0 5.58
hermes.Mother MEGA_Volume

// (OL2):
//Frame_Volume.Copy Layer_OL2
//Layer_OL2.Position 0.0 0.0 6.08
//Layer_OL2.Mother MEGA_Volume




// The 8cm CsI Detector Device:
// (11):
80mm_DC_Volume.Copy DetectorBlock
DetectorBlock.Position 0.0 0.0 -13.37
DetectorBlock.Mother MEGA_Volume



// The 2cm CsI Detectors:
// (12):
Geh_20_Volume.Copy pallas
pallas.Position 17.75 6.7 -1.2
pallas.Rotation 0. 90. 0.
pallas.Mother MEGA_Volume

// (13):
Geh_20_Volume.Copy diane
diane.Position 17.75 -6.7 -1.2
diane.Rotation 0. 90. 0.
diane.Mother MEGA_Volume

// (14):
Geh_20_Volume.Copy thetis
thetis.Position -17.75 6.7 -1.2
thetis.Rotation 0. -90. 0.
thetis.Mother MEGA_Volume

// (15):
Geh_20_Volume.Copy antigone
antigone.Position -17.75 -6.7 -1.2
antigone.Rotation 0. -90. 0.
antigone.Mother MEGA_Volume

 
// (16):
Geh_20_Volume.Copy minerva
minerva.Position 6.7 17.75 -1.2
minerva.Rotation 90. -90. 180.
minerva.Mother MEGA_Volume

// (17):
Geh_20_Volume.Copy persephone
persephone.Position 6.7 -17.75 -1.2
persephone.Rotation -90. -90. 180.
persephone.Mother MEGA_Volume

// (18):
Geh_20_Volume.Copy circe
circe.Position -6.7 17.75 -1.2
circe.Rotation 90. -90. 180.
circe.Mother MEGA_Volume

// (19):
Geh_20_Volume.Copy europa
europa.Position -6.7 -17.75 -1.2
europa.Rotation -90. -90. 180.
europa.Mother MEGA_Volume




// The 4cm Detectors:

// (20):
Geh_40_Volume.Copy hydra
hydra.Position 17.4698 6.7 -10.12545			// z = -10.05	// x = 17.55 	// z = -10.15 
hydra.Rotation 0.0 119.8 0.
hydra.Mother MEGA_Volume

// (21):
Geh_40_Volume.Copy fortuna
fortuna.Position 17.4698 -6.7 -10.12545
fortuna.Rotation 0.0 119.8 0.
fortuna.Mother MEGA_Volume

// (22):
Geh_40_Volume.Copy aetna
aetna.Position -17.4698 6.7 -10.12545
aetna.Rotation 0.0 -119.8 0.
aetna.Mother MEGA_Volume

// (23):
Geh_40_Volume.Copy medusa
medusa.Position -17.4698 -6.7 -10.12545
medusa.Rotation 0.0 -119.8 0.
medusa.Mother MEGA_Volume


// (24):
Geh_40_Volume.Copy venus
venus.Position 6.7 17.4698 -10.12545
//venus.Rotation 90. -90. 180.
venus.Rotation 0. 119.8 90.
venus.Mother MEGA_Volume

// (25):
Geh_40_Volume.Copy penelope
penelope.Position 6.7 -17.4698 -10.12545
//penelope.Rotation -90. -90. 180.
penelope.Rotation 0. -119.8 90.
penelope.Mother MEGA_Volume

// (26):
Geh_40_Volume.Copy ariadne
ariadne.Position -6.7 17.4698 -10.12545
//ariadne.Rotation 90. -90. 180.
ariadne.Rotation 0. 119.8 90.
ariadne.Mother MEGA_Volume

// (27):
Geh_40_Volume.Copy helena
helena.Position -6.7 -17.4698 -10.12545
// helena.Rotation -90. -90. 180.
helena.Rotation 0. -119.8 90.
helena.Mother MEGA_Volume



// -- till here identically with "JustForTesting.geo" ------------------------------

// Case of the D1 detectors:

RedWall_Volume.Copy Top_Wall
Top_Wall.Position 0.0 0.0 7.7
Top_Wall.Mother MEGA_Volume

GreenWall_Volume.Copy Bottom_Wall
Bottom_Wall.Position 0.0 0.0 -5.55
Bottom_Wall.Mother MEGA_Volume

SideWallA_Volume.Copy Right_Wall
Right_Wall.Position 15.2 -1.6 1.2
Right_Wall.Rotation 90. 0. 90.
Right_Wall.Mother MEGA_Volume

SideWallAm_Volume.Copy Left_Wall
Left_Wall.Position -11.67 -1.6 1.2
Left_Wall.Rotation 90. 0. -90.
Left_Wall.Mother MEGA_Volume

SideWallBm_Volume.Copy Front_Wall
Front_Wall.Position 1.975 -15.36 1.2
Front_Wall.Rotation 90. 0. 0.
Front_Wall.Mother MEGA_Volume

SideWallB_Volume.Copy Back_Wall
Back_Wall.Position 1.975 11.0 1.2
Back_Wall.Rotation -90. 180. 0.
Back_Wall.Mother MEGA_Volume





// Definitions of the connection rods between detectors and case:

Volume BigBrick
BigBrick.Material Aluminium
BigBrick.Shape BRIK 1.5 1.0 13.675
BigBrick.Color 1
BigBrick.Visibility 1

Volume Hole1_in_BigBrick
Hole1_in_BigBrick.Material Air
Hole1_in_BigBrick.Shape BRIK 1.3 0.9 6.4
Hole1_in_BigBrick.Position 0 -0.1 6.5
Hole1_in_BigBrick.Visibility 1
Hole1_in_BigBrick.Color 1
Hole1_in_BigBrick.Mother BigBrick

Volume Hole2_in_BigBrick
Hole2_in_BigBrick.Material Air
Hole2_in_BigBrick.Shape BRIK 1.3 0.9 6.4
Hole2_in_BigBrick.Position 0. -0.1 -6.5
Hole2_in_BigBrick.Visibility 1
Hole2_in_BigBrick.Color 1
Hole2_in_BigBrick.Mother BigBrick

Volume UpScrew1_in_BigBrick
UpScrew1_in_BigBrick.Material Steel
UpScrew1_in_BigBrick.Shape TUBS 0.0 0.2 0.3875 0.0 360.0
UpScrew1_in_BigBrick.Position 0.0 -0.3 13.2875
UpScrew1_in_BigBrick.Color 7
UpScrew1_in_BigBrick.Visibility 1
UpScrew1_in_BigBrick.Mother BigBrick

Volume DownScrew1_in_BigBrick
DownScrew1_in_BigBrick.Material Steel
DownScrew1_in_BigBrick.Shape TUBS 0.0 0.2 0.3875 0.0 360.0
DownScrew1_in_BigBrick.Position 0.0 -0.3 -13.2875
DownScrew1_in_BigBrick.Color 7
DownScrew1_in_BigBrick.Visibility 1
DownScrew1_in_BigBrick.Mother BigBrick

Volume SmallBrick
SmallBrick.Material Aluminium
SmallBrick.Shape BRIK 1.3 0.75 13.675
SmallBrick.Color 1
SmallBrick.Visibility 1

Volume Hole1_in_SmallBrick
Hole1_in_SmallBrick.Material Air
Hole1_in_SmallBrick.Shape BRIK 1.1 0.65 6.4
Hole1_in_SmallBrick.Position 0 -0.1 6.5
Hole1_in_SmallBrick.Visibility 1
Hole1_in_SmallBrick.Color 1
Hole1_in_SmallBrick.Mother SmallBrick

Volume Hole2_in_SmallBrick
Hole2_in_SmallBrick.Material Air
Hole2_in_SmallBrick.Shape BRIK 1.1 0.65 6.4
Hole2_in_SmallBrick.Position 0. -0.1 -6.5
Hole2_in_SmallBrick.Visibility 1
Hole2_in_SmallBrick.Color 1
Hole2_in_SmallBrick.Mother SmallBrick

//--- Old definitition of the InnerBrick before collision with detector was found -----
//Volume InnerBrick
//InnerBrick.Material Aluminium
//InnerBrick.Shape BRIK 1.25 0.75 6.825
//InnerBrick.Color 1
//InnerBrick.Visibility 1

//Volume Hole1_in_InnerBrick
//Hole1_in_InnerBrick.Material Air
//Hole1_in_InnerBrick.Shape BRIK 1.05 0.65 6.025
//Hole1_in_InnerBrick.Position 0 -0.1 0.0
//Hole1_in_InnerBrick.Visibility 1
//Hole1_in_InnerBrick.Color 1
//Hole1_in_InnerBrick.Mother InnerBrick
//-------------------------------------------------------------------------------------


Volume InnerBrick
InnerBrick.Material Air
InnerBrick.Shape BRIK 1.25 0.75 6.825
InnerBrick.Visibility 0
InnerBrick.Virtual true

Volume InnerBrick_BackWall
InnerBrick_BackWall.Material Aluminium
InnerBrick_BackWall.Shape BRIK 1.25 0.1 6.825
InnerBrick_BackWall.Position 0.0 0.65 0.0
InnerBrick_BackWall.Visibility 1
InnerBrick_BackWall.Color 1
InnerBrick_BackWall.Mother InnerBrick

Volume InnerBrick_UpperWall
InnerBrick_UpperWall.Material Aluminium
InnerBrick_UpperWall.Shape BRIK 1.25 0.65 0.1
InnerBrick_UpperWall.Position 0.0 -0.1 6.725
InnerBrick_UpperWall.Visibility 1
InnerBrick_UpperWall.Color 1
InnerBrick_UpperWall.Mother InnerBrick

Volume InnerBrick_LowerWall
InnerBrick_LowerWall.Material Aluminium
InnerBrick_LowerWall.Shape BRIK 1.25 0.65 0.1
InnerBrick_LowerWall.Position 0.0 -0.1 -6.725
InnerBrick_LowerWall.Visibility 1
InnerBrick_LowerWall.Color 1
InnerBrick_LowerWall.Mother InnerBrick

Volume InnerBrick_LeftWall
InnerBrick_LeftWall.Material Aluminium
InnerBrick_LeftWall.Shape BRIK 0.1 0.65 6.625
InnerBrick_LeftWall.Position -1.15 -0.1 0.0
InnerBrick_LeftWall.Visibility 1
InnerBrick_LeftWall.Color 1
InnerBrick_LeftWall.Mother InnerBrick

Volume InnerBrick_RightWall
InnerBrick_RightWall.Material Aluminium
InnerBrick_RightWall.Shape BRIK 0.1 0.65 6.625
InnerBrick_RightWall.Position 1.15 -0.1 0.0
InnerBrick_RightWall.Visibility 1
InnerBrick_RightWall.Color 1
InnerBrick_RightWall.Mother InnerBrick



// Positioning of these rods:

BigBrick.Copy Connect1
Connect1.Position -17.35 0.0 -6.075
Connect1.Rotation 0. 0. -90.
Connect1.Mother MEGA_Volume

BigBrick.Copy Connect2
Connect2.Position 17.35 0.0 -6.075
Connect2.Rotation 0. 0. 90.
Connect2.Mother MEGA_Volume

BigBrick.Copy Connect7
Connect7.Position 0.0 -17.35 -6.075
//Connect7.Rotation 0. 0. 180.
Connect7.Mother MEGA_Volume

BigBrick.Copy Connect8
Connect8.Position 0.0 17.35 -6.075
Connect8.Rotation 0. 0. 180.
Connect8.Mother MEGA_Volume


SmallBrick.Copy Connect3
Connect3.Position 17.6 12.65 -6.075
Connect3.Rotation 0. 0. 180.
Connect3.Mother MEGA_Volume

SmallBrick.Copy Connect4
Connect4.Position -17.6 12.65 -6.075
Connect4.Rotation 0. 0. 180.
Connect4.Mother MEGA_Volume

SmallBrick.Copy Connect5
Connect5.Position 17.6 -12.65 -6.075
Connect5.Mother MEGA_Volume

SmallBrick.Copy Connect6
Connect6.Position -17.6 -12.65 -6.075
Connect6.Mother MEGA_Volume

SmallBrick.Copy Connect9
Connect9.Position 12.65 17.6 -6.075
Connect9.Rotation 0. 0. 90.
Connect9.Mother MEGA_Volume

SmallBrick.Copy Connect10
Connect10.Position 12.65 -17.6 -6.075
Connect10.Rotation 0. 0. 90.
Connect10.Mother MEGA_Volume

SmallBrick.Copy Connect11
Connect11.Position -12.65 17.6 -6.075
Connect11.Rotation 0. 0. -90.
Connect11.Mother MEGA_Volume

SmallBrick.Copy Connect12
Connect12.Position -12.65 -17.6 -6.075
Connect12.Rotation 0. 0. -90.
Connect12.Mother MEGA_Volume


InnerBrick.Copy Connect13
Connect13.Position -13.525 -14.9 -12.925
Connect13.Rotation 0. 0. -90.
Connect13.Mother MEGA_Volume

InnerBrick.Copy Connect14
Connect14.Position 14.8 12.9 -12.925
Connect14.Rotation 0. 0. 180.
Connect14.Mother MEGA_Volume

InnerBrick.Copy Connect15
Connect15.Position -13.525 10.65 -12.925
Connect15.Rotation 0. 0. -45.0
Connect15.Mother MEGA_Volume

InnerBrick.Copy Connect16
Connect16.Position 14.8 -17.15 -12.925
//Connect16.Rotation 0. 0. -90.
Connect16.Mother MEGA_Volume




// The Cable Tree:
// Cable simulated to be hanging down.
// Tree 1 is on the right front side, Tree 2 on the front right side.

// Cable Tree 1:
// Plasic Isolation:
// (CT1_1):
Volume CableTree1_1
CableTree1_1.Material Plastic_Isolation
CableTree1_1.Shape BRIK 1.55 0.305 0.95
CableTree1_1.Position 10.05 -15.51 -7.05
CableTree1_1.Color 2
CableTree1_1.Visibility 1
CableTree1_1.Mother MEGA_Volume

// Cu:
// (CT1_2):
Volume CableTree1_2
CableTree1_2.Material Copper
CableTree1_2.Shape BRIK 1.55 0.055 0.95
CableTree1_2.Position 0.0 0.0 0.0
CableTree1_2.Color 2
CableTree1_2.Visibility 1
CableTree1_2.Mother CableTree1_1

// Plasic Isolation:
// (CT1_3):
Volume CableTree1_3
CableTree1_3.Material Plastic_Isolation
CableTree1_3.Shape BRIK 1.55 0.305 2.36
CableTree1_3.Position 10.05 -14.15 -10.16
CableTree1_3.Rotation 32. 0. 0.
CableTree1_3.Color 2
CableTree1_3.Visibility 1
CableTree1_3.Mother MEGA_Volume

// Cu:
// (CT1_4):
Volume CableTree1_4
CableTree1_4.Material Copper
CableTree1_4.Shape BRIK 1.55 0.055 2.36
CableTree1_4.Position 0.0 0.0 0.0
CableTree1_4.Color 2
CableTree1_4.Visibility 1
CableTree1_4.Mother CableTree1_3

// Plasic Isolation:
// (CT1_5):
Volume CableTree1_5
CableTree1_5.Material Plastic_Isolation
CableTree1_5.Shape BRIK 1.55 0.305 3.75
CableTree1_5.Position 10.05 -14.0 -16.02
CableTree1_5.Rotation -14.9 0. 0.
CableTree1_5.Color 2
CableTree1_5.Visibility 1
CableTree1_5.Mother MEGA_Volume

// Cu:
// (CT1_6):
Volume CableTree1_6
CableTree1_6.Material Copper
CableTree1_6.Shape BRIK 1.55 0.055 3.75
CableTree1_6.Position 0.0 0.0 0.0
CableTree1_6.Color 2
CableTree1_6.Visibility 1
CableTree1_6.Mother CableTree1_5

// Plasic Isolation:
// (CT1_7):
Volume CableTree1_7
CableTree1_7.Material Plastic_Isolation
CableTree1_7.Shape BRIK 1.55 0.305 2.25
CableTree1_7.Position 10.05 -17.55 -19.4
CableTree1_7.Rotation 90. 0. 0.
CableTree1_7.Color 2
CableTree1_7.Visibility 1
CableTree1_7.Mother MEGA_Volume

// Cu:
// (CT1_8):
Volume CableTree1_8
CableTree1_8.Material Copper
CableTree1_8.Shape BRIK 1.55 0.055 2.25
CableTree1_8.Position 0.0 0.0 0.0
CableTree1_8.Color 2
CableTree1_8.Visibility 1
CableTree1_8.Mother CableTree1_7



// the part that goes up from the green bottom plate to the single boards:

Volume UpperCableTree
UpperCableTree.Visibility 0
UpperCableTree.Virtual true
UpperCableTree.Shape BRIK 20.0 20.0 6.0
UpperCableTree.Material Air

// line 1
// Plasic Isolation:
// (CT1_9):
Volume CableTree1_9
CableTree1_9.Material Plastic_Isolation
CableTree1_9.Shape BRIK 1.55 0.0305 0.33
CableTree1_9.Position 10.05 -14.235 -4.67
CableTree1_9.Color 2
CableTree1_9.Visibility 1
CableTree1_9.Mother UpperCableTree

// Cu:
// (CT1_10):
Volume CableTree1_10
CableTree1_10.Material Copper
CableTree1_10.Shape BRIK 1.55 0.0055 0.33
CableTree1_10.Position 0.0 0.0 0.0
CableTree1_10.Color 2
CableTree1_10.Visibility 1
CableTree1_10.Mother CableTree1_9

// line 2
// Plasic Isolation:
// (CT1_11):
Volume CableTree1_11
CableTree1_11.Material Plastic_Isolation
CableTree1_11.Shape BRIK 1.55 0.0305 0.83
CableTree1_11.Position 10.05 -14.296 -4.17
CableTree1_11.Color 2
CableTree1_11.Visibility 1
CableTree1_11.Mother UpperCableTree

// Cu:
// (CT1_12):
Volume CableTree1_12
CableTree1_12.Material Copper
CableTree1_12.Shape BRIK 1.55 0.0055 0.83
CableTree1_12.Position 0.0 0.0 0.0
CableTree1_12.Color 2
CableTree1_12.Visibility 1
CableTree1_12.Mother CableTree1_11

// line 3
// Plasic Isolation:
// (CT1_13):
Volume CableTree1_13
CableTree1_13.Material Plastic_Isolation
CableTree1_13.Shape BRIK 1.55 0.0305 1.33
CableTree1_13.Position 10.05 -14.357 -3.67
CableTree1_13.Color 2
CableTree1_13.Visibility 1
CableTree1_13.Mother UpperCableTree

// Cu:
// (CT1_14):
Volume CableTree1_14
CableTree1_14.Material Copper
CableTree1_14.Shape BRIK 1.55 0.0055 1.33
CableTree1_14.Position 0.0 0.0 0.0
CableTree1_14.Color 2
CableTree1_14.Visibility 1
CableTree1_14.Mother CableTree1_13

// line 4
// Plasic Isolation:
// (CT1_15):
Volume CableTree1_15
CableTree1_15.Material Plastic_Isolation
CableTree1_15.Shape BRIK 1.55 0.0305 1.83
CableTree1_15.Position 10.05 -14.418 -3.17
CableTree1_15.Color 2
CableTree1_15.Visibility 1
CableTree1_15.Mother UpperCableTree

// Cu:
// (CT1_16):
Volume CableTree1_16
CableTree1_16.Material Copper
CableTree1_16.Shape BRIK 1.55 0.0055 1.83
CableTree1_16.Position 0.0 0.0 0.0
CableTree1_16.Color 2
CableTree1_16.Visibility 1
CableTree1_16.Mother CableTree1_15

// line 5
// Plasic Isolation:
// (CT1_17):
Volume CableTree1_17
CableTree1_17.Material Plastic_Isolation
CableTree1_17.Shape BRIK 1.55 0.0305 2.33
CableTree1_17.Position 10.05 -14.479 -2.67
CableTree1_17.Color 2
CableTree1_17.Visibility 1
CableTree1_17.Mother UpperCableTree

// Cu:
// (CT1_18):
Volume CableTree1_18
CableTree1_18.Material Copper
CableTree1_18.Shape BRIK 1.55 0.0055 2.33
CableTree1_18.Position 0.0 0.0 0.0
CableTree1_18.Color 2
CableTree1_18.Visibility 1
CableTree1_18.Mother CableTree1_17

// line 6
// Plasic Isolation:
// (CT1_19):
Volume CableTree1_19
CableTree1_19.Material Plastic_Isolation
CableTree1_19.Shape BRIK 1.55 0.0305 2.83
CableTree1_19.Position 10.05 -14.54 -2.17
CableTree1_19.Color 2
CableTree1_19.Visibility 1
CableTree1_19.Mother UpperCableTree

// Cu:
// (CT1_20):
Volume CableTree1_20
CableTree1_20.Material Copper
CableTree1_20.Shape BRIK 1.55 0.0055 2.83
CableTree1_20.Position 0.0 0.0 0.0
CableTree1_20.Color 2
CableTree1_20.Visibility 1
CableTree1_20.Mother CableTree1_19

// line 7
// Plasic Isolation:
// (CT1_21):
Volume CableTree1_21
CableTree1_21.Material Plastic_Isolation
CableTree1_21.Shape BRIK 1.55 0.0305 3.33
CableTree1_21.Position 10.05 -14.601 -1.67
CableTree1_21.Color 2
CableTree1_21.Visibility 1
CableTree1_21.Mother UpperCableTree

// Cu:
// (CT1_22):
Volume CableTree1_22
CableTree1_22.Material Copper
CableTree1_22.Shape BRIK 1.55 0.0055 3.33
CableTree1_22.Position 0.0 0.0 0.0
CableTree1_22.Color 2
CableTree1_22.Visibility 1
CableTree1_22.Mother CableTree1_21

// line 8
// Plasic Isolation:
// (CT1_23):
Volume CableTree1_23
CableTree1_23.Material Plastic_Isolation
CableTree1_23.Shape BRIK 1.55 0.0305 3.83
CableTree1_23.Position 10.05 -14.662 -1.17
CableTree1_23.Color 2
CableTree1_23.Visibility 1
CableTree1_23.Mother UpperCableTree

// Cu:
// (CT1_24):
Volume CableTree1_24
CableTree1_24.Material Copper
CableTree1_24.Shape BRIK 1.55 0.0055 3.83
CableTree1_24.Position 0.0 0.0 0.0
CableTree1_24.Color 2
CableTree1_24.Visibility 1
CableTree1_24.Mother CableTree1_23

// line 9
// Plasic Isolation:
// (CT1_25):
Volume CableTree1_25
CableTree1_25.Material Plastic_Isolation
CableTree1_25.Shape BRIK 1.55 0.0305 4.33
CableTree1_25.Position 10.05 -14.723 -0.67
CableTree1_25.Color 2
CableTree1_25.Visibility 1
CableTree1_25.Mother UpperCableTree

// Cu:
// (CT1_26):
Volume CableTree1_26
CableTree1_26.Material Copper
CableTree1_26.Shape BRIK 1.55 0.0055 4.33
CableTree1_26.Position 0.0 0.0 0.0
CableTree1_26.Color 2
CableTree1_26.Visibility 1
CableTree1_26.Mother CableTree1_25

// line 10
// Plasic Isolation:
// (CT1_27):
Volume CableTree1_27
CableTree1_27.Material Plastic_Isolation
CableTree1_27.Shape BRIK 1.55 0.0305 4.83
CableTree1_27.Position 10.05 -14.784 -0.17
CableTree1_27.Color 2
CableTree1_27.Visibility 1
CableTree1_27.Mother UpperCableTree

// Cu:
// (CT1_28):
Volume CableTree1_28
CableTree1_28.Material Copper
CableTree1_28.Shape BRIK 1.55 0.0055 4.83
CableTree1_28.Position 0.0 0.0 0.0
CableTree1_28.Color 2
CableTree1_28.Visibility 1
CableTree1_28.Mother CableTree1_27

// line 11
// Plasic Isolation:
// (CT1_29):
Volume CableTree1_29
CableTree1_29.Material Plastic_Isolation
CableTree1_29.Shape BRIK 1.55 0.0305 5.33
CableTree1_29.Position 10.05 -14.845 0.33
CableTree1_29.Color 2
CableTree1_29.Visibility 1
CableTree1_29.Mother UpperCableTree

// Cu:
// (CT1_30):
Volume CableTree1_30
CableTree1_30.Material Copper
CableTree1_30.Shape BRIK 1.55 0.0055 5.33
CableTree1_30.Position 0.0 0.0 0.0
CableTree1_30.Color 2
CableTree1_30.Visibility 1
CableTree1_30.Mother CableTree1_29

// line 12
// Plasic Isolation:
// (CT1_31):
Volume CableTree1_31
CableTree1_31.Material Plastic_Isolation
CableTree1_31.Shape BRIK 1.55 0.0305 5.83
CableTree1_31.Position 10.05 -14.906 0.83
CableTree1_31.Color 2
CableTree1_31.Visibility 1
CableTree1_31.Mother UpperCableTree

// Cu:
// (CT1_32):
Volume CableTree1_32
CableTree1_32.Material Copper
CableTree1_32.Shape BRIK 1.55 0.0055 5.83
CableTree1_32.Position 0.0 0.0 0.0
CableTree1_32.Color 2
CableTree1_32.Visibility 1
CableTree1_32.Mother CableTree1_31


UpperCableTree.Copy UCT1
UCT1.Position 0. 0. 0.
UCT1.Mother MEGA_Volume



// Cable Tree 2:

// Plasic Isolation:
// (CT2_1):
Volume CableTree2_1
CableTree2_1.Material Plastic_Isolation
CableTree2_1.Shape BRIK 0.305 1.55 0.95
CableTree2_1.Position 15.51 -10.05 -7.05
CableTree2_1.Color 2
CableTree2_1.Visibility 1
CableTree2_1.Mother MEGA_Volume

// Cu:
// (CT2_2):
Volume CableTree2_2
CableTree2_2.Material Copper
CableTree2_2.Shape BRIK 0.055 1.55 0.95
CableTree2_2.Position 0.0 0.0 0.0
CableTree2_2.Color 2
CableTree2_2.Visibility 1
CableTree2_2.Mother CableTree2_1

// Plasic Isolation:
// (CT2_3):
Volume CableTree2_3
CableTree2_3.Material Plastic_Isolation
CableTree2_3.Shape BRIK 0.305 1.55 2.36
CableTree2_3.Position 14.15 -10.05 -10.16
CableTree2_3.Rotation 0. 32. 0.
CableTree2_3.Color 2
CableTree2_3.Visibility 1
CableTree2_3.Mother MEGA_Volume

// Cu:
// (CT2_4):
Volume CableTree2_4
CableTree2_4.Material Copper
CableTree2_4.Shape BRIK 0.055 1.55 2.36
CableTree2_4.Position 0.0 0.0 0.0
CableTree2_4.Color 2
CableTree2_4.Visibility 1
CableTree2_4.Mother CableTree2_3

// Plasic Isolation:
// (CT2_5):
Volume CableTree2_5
CableTree2_5.Material Plastic_Isolation
CableTree2_5.Shape BRIK 0.305 1.55 3.75
CableTree2_5.Position 14.0 -10.05 -16.02
CableTree2_5.Rotation 0. -14.9 0.
CableTree2_5.Color 2
CableTree2_5.Visibility 1
CableTree2_5.Mother MEGA_Volume

// Cu:
// (CT2_6):
Volume CableTree2_6
CableTree2_6.Material Copper
CableTree2_6.Shape BRIK 0.055 1.55 3.75
CableTree2_6.Position 0.0 0.0 0.0
CableTree2_6.Color 2
CableTree2_6.Visibility 1
CableTree2_6.Mother CableTree2_5

// Plasic Isolation:
// (CT2_7):
Volume CableTree2_7
CableTree2_7.Material Plastic_Isolation
CableTree2_7.Shape BRIK 0.305 1.55 2.25
CableTree2_7.Position 17.55 -10.05 -19.4
CableTree2_7.Rotation 0. 90. 0.
CableTree2_7.Color 2
CableTree2_7.Visibility 1
CableTree2_7.Mother MEGA_Volume

// Cu:
// (CT2_8):
Volume CableTree2_8
CableTree2_8.Material Copper
CableTree2_8.Shape BRIK 0.055 1.55 2.25
CableTree2_8.Position 0.0 0.0 0.0
CableTree2_8.Color 2
CableTree2_8.Visibility 1
CableTree2_8.Mother CableTree2_7


// the part that goes up from the green bottom plate to the single boards:


UpperCableTree.Copy UCT2
UCT2.Position 0. -20.1 0.
UCT2.Rotation 0. 0. 90.
UCT2.Mother MEGA_Volume






//// Ground Plate:
//
//
//Volume GroundPlate
//GroundPlate.Material Aluminium
//GroundPlate.Shape BRIK 23.95 23.95 0.5
//GroundPlate.Position 0. 0. -20.15
//GroundPlate.Visibility 1
//GroundPlate.Color 1
//GroundPlate.Mother MEGA_Volume


