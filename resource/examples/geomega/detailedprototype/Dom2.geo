// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0


// The Dom2.geo.setup - file is just the same like the Dom.geo.setup - file,
// but in addition with Connection Cylinders to the surrounding egg of the "Fluggerrüst".


// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo



Include Whole_MEGA.geo
Include PMT.geo


// The Mother Volume of the Detector Configuration:
Volume Dom_Volume
Dom_Volume.Material Air
Dom_Volume.Visibility 0
Dom_Volume.Virtual true
Dom_Volume.Shape BRIK 200. 200. 200.
// NEEDS THIS LINE TO VIEW ALONE:
// Dom_Volume.Mother 0



MEGA_Volume.Position 0.0 0.0 0.05
MEGA_Volume.Mother Dom_Volume


// ---------------------------------------------------------------------------------------
// NE110 Walls

Volume DV_Wall_1
DV_Wall_1.Material NE110
DV_Wall_1.Shape BRIK 26.27 0.6 12.45
DV_Wall_1.Position 0.0 25.67 -6.35
DV_Wall_1.Visibility 1
DV_Wall_1.Color 7
DV_Wall_1.Mother Dom_Volume

Volume DV_Wall_2
DV_Wall_2.Material NE110
DV_Wall_2.Shape BRIK 26.27 0.6 12.45
DV_Wall_2.Position 0.0 -25.67 -6.35
DV_Wall_2.Visibility 1
DV_Wall_2.Color 7
DV_Wall_2.Mother Dom_Volume

Volume DV_Wall_3
DV_Wall_3.Material NE110
DV_Wall_3.Shape BRIK 0.6 25.07 12.45
DV_Wall_3.Position -25.67 0.0 -6.35
DV_Wall_3.Visibility 1
DV_Wall_3.Color 7
DV_Wall_3.Mother Dom_Volume

Volume DV_Wall_4
DV_Wall_4.Material NE110
DV_Wall_4.Shape BRIK 0.6 25.07 12.45
DV_Wall_4.Position 25.67 0.0 -6.35
DV_Wall_4.Visibility 1
DV_Wall_4.Color 7
DV_Wall_4.Mother Dom_Volume

// -------------------------------------------------------------------------------------------




//-- The old definitions of the roof. They caused crevices between the consisting parts. -------------------
//// Roof:
//Volume DV_Roof_Template
//DV_Roof_Template.Material NE110
//DV_Roof_Template.Shape TRD1 25.3 0.0 0.6 17.9
//DV_Roof_Template.Visibility 1
//DV_Roof_Template.Color 8
//
//
//
//DV_Roof_Template.Copy DV_Roof_5
//DV_Roof_5.Rotation 45. 0. 0.
//DV_Roof_5.Position 0. 13.1 19.2
//DV_Roof_5.Mother Dom_Volume
//
//DV_Roof_Template.Copy DV_Roof_6
//DV_Roof_6.Rotation 45. 0. 90.
//DV_Roof_6.Position -13.1 0. 19.2
//DV_Roof_6.Mother Dom_Volume
//
//DV_Roof_Template.Copy DV_Roof_7
//DV_Roof_7.Rotation 45. 0. 180.
//DV_Roof_7.Position 0. -13.1 19.2
//DV_Roof_7.Mother Dom_Volume
//
//DV_Roof_Template.Copy DV_Roof_8
//DV_Roof_8.Rotation 45. 0. 270.
//DV_Roof_8.Position 13.1 0. 19.2
//DV_Roof_8.Mother Dom_Volume
//---------------------------------------------------------------------------------------------------


// --------------------------------------------------------------------------------------------------
// The Roof (new definition):

// NOTE: The Roof is going a little too far to the outside, for this was easier to realize. It causes
// corners on the outside, that are there in reality. It also causes some problems with the outer aluminium
// roof (see below). But it was easier so after all.

Volume DV_Roof_Template
DV_Roof_Template.Material NE110
// This line seems to be most correct until now:
// DV_Roof_Template.Shape TRAP 0.6 0. 0. 18.927167004 26.27 0. 0. 17.727167004 25.07 0. 0.
// but here changes for improvement:
//DV_Roof_Template.Shape TRAP 0.6 0. 0. 18.927167004 26.767056274 0. 0. 17.727167004 25.07 0. 0.
// Change for geant4.6.0-p01
DV_Roof_Template.Shape TRAP 0.6 0. 0. 18.927167004 26.767056274 0.0000001 0. 17.727167004 25.07 0.0000001 0.
//DV_Roof_Template.Position 0 0 0
//DV_Roof_Template.Rotation 135. 0. 0.
DV_Roof_Template.Visibility 1
//DV_Roof_Template.Color 8
//DV_Roof_Template.Mother Dom_Volume

DV_Roof_Template.Copy DV_Roof_5
DV_Roof_5.Rotation 135. 0. 0.
DV_Roof_5.Position 0.0 12.9592646687 19.0592640687
DV_Roof_5.Color 7
DV_Roof_5.Mother Dom_Volume

DV_Roof_Template.Copy DV_Roof_6
DV_Roof_6.Rotation 135. 0. -90.
DV_Roof_6.Position 12.9592646687 0.0 19.0592640687
DV_Roof_6.Color 7
DV_Roof_6.Mother Dom_Volume

DV_Roof_Template.Copy DV_Roof_7
DV_Roof_7.Rotation 135. 0. 180.
DV_Roof_7.Position 0.0 -12.9592646687 19.0592640687
DV_Roof_7.Color 7
DV_Roof_7.Mother Dom_Volume

DV_Roof_Template.Copy DV_Roof_8
DV_Roof_8.Rotation 135. 0. 90.
DV_Roof_8.Position -12.9592646687 0.0 19.0592640687
DV_Roof_8.Color 7
DV_Roof_8.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------

// Inner Aluminium Walls:

// vertical walls:

Volume DV_Wall_9
DV_Wall_9.Material Aluminium
DV_Wall_9.Shape BRIK 24.72 0.015 12.3			// BRIK 24.72 0.015 12.2
DV_Wall_9.Position 0.0 24.705 -6.20			// Position 0.0 24.705 -6.30
DV_Wall_9.Visibility 1
DV_Wall_9.Color 2
DV_Wall_9.Mother Dom_Volume

Volume DV_Wall_10
DV_Wall_10.Material Aluminium
DV_Wall_10.Shape BRIK 24.72 0.015 12.3
DV_Wall_10.Position 0.0 -24.705 -6.20
DV_Wall_10.Visibility 1
DV_Wall_10.Color 2
DV_Wall_10.Mother Dom_Volume

Volume DV_Wall_11
DV_Wall_11.Material Aluminium
DV_Wall_11.Shape BRIK 0.015 24.69 12.3
DV_Wall_11.Position -24.705 0.0 -6.20
DV_Wall_11.Visibility 1
DV_Wall_11.Color 2
DV_Wall_11.Mother Dom_Volume

Volume DV_Wall_12
DV_Wall_12.Material Aluminium
DV_Wall_12.Shape BRIK 0.015 24.69 12.3
DV_Wall_12.Position 24.705 0.0 -6.20
DV_Wall_12.Visibility 1
DV_Wall_12.Color 2
DV_Wall_12.Mother Dom_Volume


// inner angular aluminium walls:

Volume DV_RoofTemplate_17
DV_RoofTemplate_17.Material Aluminium
DV_RoofTemplate_17.Shape TRAP 0.015 0. 0. 17.4884664275 24.7324264069 0.0000001 0. 17.4584664275 24.69 0.0000001 0.
DV_RoofTemplate_17.Visibility 1
DV_RoofTemplate_17.Color 2



DV_RoofTemplate_17.Copy DV_Roof_18
DV_Roof_18.Position 12.355606601 0.0 18.455606601
DV_Roof_18.Rotation 135. 0. -90.
DV_Roof_18.Mother Dom_Volume

DV_RoofTemplate_17.Copy DV_Roof_19
DV_Roof_19.Position -12.355606601 0.0 18.455606601
DV_Roof_19.Rotation 135. 0. 90.
DV_Roof_19.Mother Dom_Volume

DV_RoofTemplate_17.Copy DV_Roof_20
DV_Roof_20.Position 0.0 12.355606601 18.455606601
DV_Roof_20.Rotation 135. 0. 0.
DV_Roof_20.Mother Dom_Volume

DV_RoofTemplate_17.Copy DV_Roof_21
DV_Roof_21.Position 0.0 -12.355606601 18.455606601
DV_Roof_21.Rotation 135. 0. 180.
DV_Roof_21.Mother Dom_Volume





// Outer Aluminium Walls:

Volume DV_Wall_13
DV_Wall_13.Material Aluminium
// going to bottom:
//DV_Wall_13.Shape BRIK 26.65 0.015 12.3
//DV_Wall_13.Position 0.0 26.635 -6.2
// going to tabletop
DV_Wall_13.Shape BRIK 26.65 0.015 8.9
DV_Wall_13.Position 0.0 26.635 -2.8
DV_Wall_13.Visibility 1
DV_Wall_13.Color 2
DV_Wall_13.Mother Dom_Volume

Volume DV_Wall_14
DV_Wall_14.Material Aluminium
// going to bottom:
//DV_Wall_14.Shape BRIK 26.65 0.015 12.3
//DV_Wall_14.Position 0.0 -26.635 -6.2
// going to tabletop:
DV_Wall_14.Shape BRIK 26.65 0.015 8.9
DV_Wall_14.Position 0.0 -26.635 -2.8
DV_Wall_14.Visibility 1
DV_Wall_14.Color 2
DV_Wall_14.Mother Dom_Volume

Volume DV_Wall_15
DV_Wall_15.Material Aluminium
// going to bottom:
//DV_Wall_15.Shape BRIK 0.015 26.62 12.3
//DV_Wall_15.Position 26.635 0.0 -6.2
// going to tabletop:
DV_Wall_15.Shape BRIK 0.015 26.62 8.9
DV_Wall_15.Position 26.635 0.0 -2.8
DV_Wall_15.Visibility 1
DV_Wall_15.Color 2
DV_Wall_15.Mother Dom_Volume

Volume DV_Wall_16
DV_Wall_16.Material Aluminium
// goint to bottom:
//DV_Wall_16.Shape BRIK 0.015 26.62 12.3
//DV_Wall_16.Position -26.635 0.0 -6.2
// going to tabletop:
DV_Wall_16.Shape BRIK 0.015 26.62 8.9
DV_Wall_16.Position -26.635 0.0 -2.8
DV_Wall_16.Visibility 1
DV_Wall_16.Color 2
DV_Wall_16.Mother Dom_Volume


// outer angular aluminium walls:

// NOTE: Due to practical changes from the orinal design of the NE110 roof (see above),
// the outer aluminium roof is also changed in this model. The roof is also going out a little too far
// and has NO connection to the vertical outer aluminium walls beneath! Thus there is a small gap of about
// 0.5 cm width in a horizontal plane, going all around the model.

Volume DV_RoofTemplate_22
DV_RoofTemplate_22.Material Aluminium
DV_RoofTemplate_22.Shape TRAP 0.015 0. 0. 19.307167004 27.304057428 0.0000001 0. 19.2774494113 27.262031021 0.0000001 0.
DV_RoofTemplate_22.Visibility 1
DV_RoofTemplate_22.Color 2


DV_RoofTemplate_22.Copy DV_Roof_23
DV_Roof_23.Position 13.641622112 0.0 19.741622112
DV_Roof_23.Rotation 135. 0. -90.
DV_Roof_23.Mother Dom_Volume

DV_RoofTemplate_22.Copy DV_Roof_24
DV_Roof_24.Position -13.641622112 0.0 19.741622112
DV_Roof_24.Rotation 135. 0. 90.
DV_Roof_24.Mother Dom_Volume

DV_RoofTemplate_22.Copy DV_Roof_25
DV_Roof_25.Position 0.0 13.641622112 19.741622112
DV_Roof_25.Rotation 135. 0. 0.
DV_Roof_25.Mother Dom_Volume

DV_RoofTemplate_22.Copy DV_Roof_26
DV_Roof_26.Position 0.0 -13.641622112 19.741622112
DV_Roof_26.Rotation 135. 0. 180.
DV_Roof_26.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------------
// Inner / Outer Paper Layer:

// Inner Paper Layer:

// inner vertical Paper walls:
Volume DV_Wall_27
DV_Wall_27.Material ACSPaper
DV_Wall_27.Shape BRIK 24.90 0.01 12.3
DV_Wall_27.Position 0.0 24.89 -6.20
DV_Wall_27.Visibility 1
DV_Wall_27.Color 3
DV_Wall_27.Mother Dom_Volume

Volume DV_Wall_28
DV_Wall_28.Material ACSPaper
DV_Wall_28.Shape BRIK 24.90 0.01 12.3d
DV_Wall_28.Position 0.0 -24.89 -6.20
DV_Wall_28.Visibility 1
DV_Wall_28.Color 3
DV_Wall_28.Mother Dom_Volume

Volume DV_Wall_29
DV_Wall_29.Material ACSPaper
DV_Wall_29.Shape BRIK 0.01 24.88 12.3
DV_Wall_29.Position -24.89 0.0 -6.20
DV_Wall_29.Visibility 1
DV_Wall_29.Color 3
DV_Wall_29.Mother Dom_Volume

Volume DV_Wall_30
DV_Wall_30.Material ACSPaper
DV_Wall_30.Shape BRIK 0.01 24.88 12.3
DV_Wall_30.Position 24.89 0.0 -6.20
DV_Wall_30.Visibility 1
DV_Wall_30.Color 3
DV_Wall_30.Mother Dom_Volume


// inner paper roof:

Volume DV_RoofTemplate_31
DV_RoofTemplate_31.Material ACSPaper
DV_RoofTemplate_31.Shape TRAP 0.01 0. 0. 17.612816715 24.908284271 0.0000001 0. 17.592816715 24.88 0.0000001 0.
DV_RoofTemplate_31.Visibility 1
DV_RoofTemplate_31.Color 3


DV_RoofTemplate_31.Copy DV_Roof_32
DV_Roof_32.Position 12.447071067 0.0 18.547071067
DV_Roof_32.Rotation 135. 0. -90.
DV_Roof_32.Mother Dom_Volume

DV_RoofTemplate_31.Copy DV_Roof_33
DV_Roof_33.Position -12.447071067 0.0 18.547071067
DV_Roof_33.Rotation 135. 0. 90.
DV_Roof_33.Mother Dom_Volume

DV_RoofTemplate_31.Copy DV_Roof_34
DV_Roof_34.Position 0.0 12.447071067 18.547071067
DV_Roof_34.Rotation 135. 0. 0.
DV_Roof_34.Mother Dom_Volume

DV_RoofTemplate_31.Copy DV_Roof_35
DV_Roof_35.Position 0.0 -12.447071067 18.547071067
DV_Roof_35.Rotation 135. 0. 180.
DV_Roof_35.Mother Dom_Volume



// Outer Paper Layer:

// outer vertical Paper walls:

Volume DV_Wall_50
DV_Wall_50.Material ACSPaper
DV_Wall_50.Shape BRIK 26.45 0.01 12.3
DV_Wall_50.Position 0.0 26.44 -6.20
DV_Wall_50.Visibility 1
DV_Wall_50.Color 3
DV_Wall_50.Mother Dom_Volume

Volume DV_Wall_51
DV_Wall_51.Material ACSPaper
DV_Wall_51.Shape BRIK 26.45 0.01 12.3
DV_Wall_51.Position 0.0 -26.44 -6.20
DV_Wall_51.Visibility 1
DV_Wall_51.Color 3
DV_Wall_51.Mother Dom_Volume

Volume DV_Wall_52
DV_Wall_52.Material ACSPaper
DV_Wall_52.Shape BRIK 0.01 26.43 12.3
DV_Wall_52.Position -26.44 0.0 -6.20
DV_Wall_52.Visibility 1
DV_Wall_52.Color 3
DV_Wall_52.Mother Dom_Volume

Volume DV_Wall_53
DV_Wall_53.Material ACSPaper
DV_Wall_53.Shape BRIK 0.01 26.43 12.3
DV_Wall_53.Position 26.44 0.0 -6.20
DV_Wall_53.Visibility 1
DV_Wall_53.Color 3
DV_Wall_53.Mother Dom_Volume



// outer paper roof:

// NOTE: Due to practical changes from the orinal design of the NE110 roof (see above),
// the outer paper roof is (as well as the aluminium roof) also changed in this model.
// The roof is also going out a little too far, and has NO connection to the vertical 
// outer aluminium walls beneath! Thus there is a small gap of about 0.5 cm width 
// in a horizontal plane, going all around the model.


Volume DV_RoofTemplate_54
DV_RoofTemplate_54.Material ACSPaper
DV_RoofTemplate_54.Shape TRAP 0.01 0. 0. 19.041172413 26.928284271 0.0000001 0. 19.021172413 26.9 0.0000001 0.
DV_RoofTemplate_54.Visibility 1
DV_RoofTemplate_54.Color 3


DV_RoofTemplate_54.Copy DV_Roof_55
DV_Roof_55.Position 13.457071067 0.0 19.557071067
DV_Roof_55.Rotation 135. 0. -90.
DV_Roof_55.Mother Dom_Volume

DV_RoofTemplate_54.Copy DV_Roof_56
DV_Roof_56.Position -13.457071067 0.0 19.557071067
DV_Roof_56.Rotation 135. 0. 90.
DV_Roof_56.Mother Dom_Volume

DV_RoofTemplate_54.Copy DV_Roof_57
DV_Roof_57.Position 0.0 13.457071067 19.557071067
DV_Roof_57.Rotation 135. 0. 0.
DV_Roof_57.Mother Dom_Volume

DV_RoofTemplate_54.Copy DV_Roof_58
DV_Roof_58.Position 0.0 -13.457071067 19.557071067
DV_Roof_58.Rotation 135. 0. 180.
DV_Roof_58.Mother Dom_Volume



// ----------------------------------------------------------------------------------------------------

// Aluminium Connection from outer Alu-Wall to PMTTable:

// Long Side

Volume DV_AC_60
DV_AC_60.Material Aluminium
DV_AC_60.Shape BRIK 26.85 0.1 0.55
DV_AC_60.Position 0.0 26.75 -10.95
DV_AC_60.Visibility 1
DV_AC_60.Color 2
DV_AC_60.Mother Dom_Volume

Volume DV_AC_61
DV_AC_61.Material Aluminium
DV_AC_61.Shape BRIK 26.85 0.1 0.55
DV_AC_61.Position 0.0 -26.75 -10.95
DV_AC_61.Visibility 1
DV_AC_61.Color 2
DV_AC_61.Mother Dom_Volume

Volume DV_AC_62
DV_AC_62.Material Aluminium
DV_AC_62.Shape BRIK 28.15 0.75 0.1
DV_AC_62.Position 0.0 27.4 -11.6
DV_AC_62.Visibility 1
DV_AC_62.Color 2
DV_AC_62.Mother Dom_Volume

Volume DV_AC_63
DV_AC_63.Material Aluminium
DV_AC_63.Shape BRIK 28.15 0.75 0.1
DV_AC_63.Position 0.0 -27.4 -11.6
DV_AC_63.Visibility 1
DV_AC_63.Color 2
DV_AC_63.Mother Dom_Volume


// Short Side:

Volume DV_AC_64
DV_AC_64.Material Aluminium
DV_AC_64.Shape BRIK 0.1 26.65 0.55
DV_AC_64.Position 26.75 0.0 -10.95
DV_AC_64.Visibility 1
DV_AC_64.Color 2
DV_AC_64.Mother Dom_Volume

Volume DV_AC_65
DV_AC_65.Material Aluminium
DV_AC_65.Shape BRIK 0.1 26.65 0.55
DV_AC_65.Position -26.75 0.0 -10.95
DV_AC_65.Visibility 1
DV_AC_65.Color 2
DV_AC_65.Mother Dom_Volume

Volume DV_AC_66
DV_AC_66.Material Aluminium
DV_AC_66.Shape BRIK 0.75 26.65 0.1
DV_AC_66.Position 27.4 0.0 -11.6
DV_AC_66.Visibility 1
DV_AC_66.Color 2
DV_AC_66.Mother Dom_Volume

Volume DV_AC_67
DV_AC_67.Material Aluminium
DV_AC_67.Shape BRIK 0.75 26.65 0.1
DV_AC_67.Position -27.4 0.0 -11.6
DV_AC_67.Visibility 1
DV_AC_67.Color 2
DV_AC_67.Mother Dom_Volume



// Aluminium Connections from PMTTable to green bottom-ring:

// The connection layers on the outside of the PMT Table are arranged in an octagonal shape.
// Since no specifications are made in the drawing, exact position of corners are guessed.
// Nate that unlike in realitiy the aluminium layers is not totaly closed around the whole detector,
// but has several crevices at the corners!


Volume DV_AC_68
DV_AC_68.Material Aluminium
//DV_AC_68.Shape TRD1 17.0 16.0 0.015 0.8
DV_AC_68.Shape BRIK 17.0 0.015 0.8
DV_AC_68.Position 0.0 36.8 -11.685
DV_AC_68.Rotation 90. 0. 0.
DV_AC_68.Visibility 1
DV_AC_68.Color 2
DV_AC_68.Mother Dom_Volume

Volume DV_AC_69
DV_AC_69.Material Aluminium
//DV_AC_69.Shape TRD1 17.0 16.0 0.015 0.8
DV_AC_69.Shape BRIK 17.0 0.015 0.8
DV_AC_69.Position 0.0 -36.8 -11.685
DV_AC_69.Rotation 90. 0. 180.
DV_AC_69.Visibility 1
DV_AC_69.Color 2
DV_AC_69.Mother Dom_Volume

Volume DV_AC_70
DV_AC_70.Material Aluminium
//DV_AC_70.Shape TRD1 17.0 16.0 0.015 0.8
DV_AC_70.Shape BRIK 17.0 0.015 0.8
DV_AC_70.Position 36.8 0.0 -11.685
DV_AC_70.Rotation 90. 0. -90.
DV_AC_70.Visibility 1
DV_AC_70.Color 2
DV_AC_70.Mother Dom_Volume

Volume DV_AC_71
DV_AC_71.Material Aluminium
//DV_AC_71.Shape TRD1 17.0 16.0 0.015 0.8
DV_AC_71.Shape BRIK 17.0 0.015 0.8
DV_AC_71.Position -36.8 0.0 -11.685
DV_AC_71.Rotation 90. 0. 90.
DV_AC_71.Visibility 1
DV_AC_71.Color 2
DV_AC_71.Mother Dom_Volume


Volume DV_AC_72
DV_AC_72.Material Aluminium
DV_AC_72.Shape BRIK 5.25 0.8 0.015
DV_AC_72.Position 32.45 20.9 -11.685
DV_AC_72.Rotation 0. 0. 135.
DV_AC_72.Visibility 1
DV_AC_72.Color 2
DV_AC_72.Mother Dom_Volume

Volume DV_AC_73
DV_AC_73.Material Aluminium
DV_AC_73.Shape BRIK 5.25 0.8 0.015
DV_AC_73.Position -32.45 20.9 -11.685
DV_AC_73.Rotation 0. 0. 45.
DV_AC_73.Visibility 1
DV_AC_73.Color 2
DV_AC_73.Mother Dom_Volume

Volume DV_AC_74
DV_AC_74.Material Aluminium
DV_AC_74.Shape BRIK 5.25 0.8 0.015
DV_AC_74.Position 32.45 -20.9 -11.685
DV_AC_74.Rotation 0. 0. 45.
DV_AC_74.Visibility 1
DV_AC_74.Color 2
DV_AC_74.Mother Dom_Volume

Volume DV_AC_75
DV_AC_75.Material Aluminium
DV_AC_75.Shape BRIK 5.25 0.8 0.015
DV_AC_75.Position -32.45 -20.9 -11.685
DV_AC_75.Rotation 0. 0. 135.
DV_AC_75.Visibility 1
DV_AC_75.Color 2
DV_AC_75.Mother Dom_Volume


Volume DV_AC_76
DV_AC_76.Material Aluminium
DV_AC_76.Shape BRIK 5.25 0.8 0.015
DV_AC_76.Position 20.9 32.45 -11.685
DV_AC_76.Rotation 0. 0. 135.
DV_AC_76.Visibility 1
DV_AC_76.Color 2
DV_AC_76.Mother Dom_Volume

Volume DV_AC_77
DV_AC_77.Material Aluminium
DV_AC_77.Shape BRIK 5.25 0.8 0.015
DV_AC_77.Position -20.9 -32.45 -11.685
DV_AC_77.Rotation 0. 0. 135.
DV_AC_77.Visibility 1
DV_AC_77.Color 2
DV_AC_77.Mother Dom_Volume

Volume DV_AC_78
DV_AC_78.Material Aluminium
DV_AC_78.Shape BRIK 5.25 0.8 0.015
DV_AC_78.Position 20.9 -32.45 -11.685
DV_AC_78.Rotation 0. 0. 45.
DV_AC_78.Visibility 1
DV_AC_78.Color 2
DV_AC_78.Mother Dom_Volume

Volume DV_AC_79
DV_AC_79.Material Aluminium
DV_AC_79.Shape BRIK 5.25 0.8 0.015
DV_AC_79.Position -20.9 32.45 -11.685
DV_AC_79.Rotation 0. 0. 45.
DV_AC_79.Visibility 1
DV_AC_79.Color 2
DV_AC_79.Mother Dom_Volume


Volume DV_AC_80
DV_AC_80.Material Aluminium
DV_AC_80.Shape BRIK 0.015 17.0 3.7
DV_AC_80.Position -37.585 0.0 -15.4
DV_AC_80.Visibility 1
DV_AC_80.Color 2
DV_AC_80.Mother Dom_Volume

Volume DV_AC_81
DV_AC_81.Material Aluminium
DV_AC_81.Shape BRIK 0.015 17.0 3.7
DV_AC_81.Position 37.585 0.0 -15.4
DV_AC_81.Visibility 1
DV_AC_81.Color 2
DV_AC_81.Mother Dom_Volume

Volume DV_AC_82
DV_AC_82.Material Aluminium
DV_AC_82.Shape BRIK 0.015 17.0 3.7
DV_AC_82.Position 0.0 -37.585 -15.4
DV_AC_82.Rotation 0. 0. 90.
DV_AC_82.Visibility 1
DV_AC_82.Color 2
DV_AC_82.Mother Dom_Volume

Volume DV_AC_83
DV_AC_83.Material Aluminium
DV_AC_83.Shape BRIK 0.015 17.0 3.7
DV_AC_83.Position 0.0 37.585 -15.4
DV_AC_83.Rotation 0. 0. 90.
DV_AC_83.Visibility 1
DV_AC_83.Color 2
DV_AC_83.Mother Dom_Volume


Volume DV_AC_84
DV_AC_84.Material Aluminium
DV_AC_84.Shape BRIK 6.55 0.015 3.7
DV_AC_84.Position 21.65 32.8 -15.4
DV_AC_84.Rotation 0. 0. 135.
DV_AC_84.Visibility 1
DV_AC_84.Color 2
DV_AC_84.Mother Dom_Volume

Volume DV_AC_84a
DV_AC_84a.Material Aluminium
DV_AC_84a.Shape BRIK 6.55 0.1 0.75
DV_AC_84a.Position 21.568  32.718 -18.35
DV_AC_84a.Rotation 0. 0. 135.
DV_AC_84a.Visibility 1
DV_AC_84a.Color 2
DV_AC_84a.Mother Dom_Volume

Volume DV_AC_84b
DV_AC_84b.Material Aluminium
DV_AC_84b.Shape BRIK 6.55 0.635 0.1
DV_AC_84b.Position 22.11 33.26 -19.0
DV_AC_84b.Rotation 0. 0. 135.
DV_AC_84b.Visibility 1
DV_AC_84b.Color 2
DV_AC_84b.Mother Dom_Volume

Volume DV_AC_85
DV_AC_85.Material Aluminium
DV_AC_85.Shape BRIK 6.55 0.015 3.7
DV_AC_85.Position 21.65 -32.8 -15.4
DV_AC_85.Rotation 0. 0. 45.
DV_AC_85.Visibility 1
DV_AC_85.Color 2
DV_AC_85.Mother Dom_Volume

Volume DV_AC_85a
DV_AC_85a.Material Aluminium
DV_AC_85a.Shape BRIK 6.55 0.1 0.75
DV_AC_85a.Position 21.568  -32.718 -18.35
DV_AC_85a.Rotation 0. 0. 45.
DV_AC_85a.Visibility 1
DV_AC_85a.Color 2
DV_AC_85a.Mother Dom_Volume

Volume DV_AC_85b
DV_AC_85b.Material Aluminium
DV_AC_85b.Shape BRIK 6.55 0.635 0.1
DV_AC_85b.Position 22.11 -33.26 -19.0
DV_AC_85b.Rotation 0. 0. 45.
DV_AC_85b.Visibility 1
DV_AC_85b.Color 2
DV_AC_85b.Mother Dom_Volume

Volume DV_AC_86
DV_AC_86.Material Aluminium
DV_AC_86.Shape BRIK 6.55 0.015 3.7
DV_AC_86.Position -21.65 32.8 -15.4
DV_AC_86.Rotation 0. 0. 45.
DV_AC_86.Visibility 1
DV_AC_86.Color 2
DV_AC_86.Mother Dom_Volume

Volume DV_AC_86a
DV_AC_86a.Material Aluminium
DV_AC_86a.Shape BRIK 6.55 0.1 0.75
DV_AC_86a.Position -21.568  32.718 -18.35
DV_AC_86a.Rotation 0. 0. 45.
DV_AC_86a.Visibility 1
DV_AC_86a.Color 2
DV_AC_86a.Mother Dom_Volume

Volume DV_AC_86b
DV_AC_86b.Material Aluminium
DV_AC_86b.Shape BRIK 6.55 0.635 0.1
DV_AC_86b.Position -22.11 33.26 -19.0
DV_AC_86b.Rotation 0. 0. 45.
DV_AC_86b.Visibility 1
DV_AC_86b.Color 2
DV_AC_86b.Mother Dom_Volume

Volume DV_AC_87
DV_AC_87.Material Aluminium
DV_AC_87.Shape BRIK 6.55 0.015 3.7
DV_AC_87.Position -21.65 -32.8 -15.4
DV_AC_87.Rotation 0. 0. 135.
DV_AC_87.Visibility 1
DV_AC_87.Color 2
DV_AC_87.Mother Dom_Volume

Volume DV_AC_87a
DV_AC_87a.Material Aluminium
DV_AC_87a.Shape BRIK 6.55 0.1 0.75
DV_AC_87a.Position -21.568  -32.718 -18.35
DV_AC_87a.Rotation 0. 0. 135.
DV_AC_87a.Visibility 1
DV_AC_87a.Color 2
DV_AC_87a.Mother Dom_Volume

Volume DV_AC_87b
DV_AC_87b.Material Aluminium
DV_AC_87b.Shape BRIK 6.55 0.635 0.1
DV_AC_87b.Position -22.11 -33.26 -19.0
DV_AC_87b.Rotation 0. 0. 135.
DV_AC_87b.Visibility 1
DV_AC_87b.Color 2
DV_AC_87b.Mother Dom_Volume



Volume DV_AC_88
DV_AC_88.Material Aluminium
DV_AC_88.Shape BRIK 6.55 0.015 3.7
DV_AC_88.Position 32.8 21.65 -15.4
DV_AC_88.Rotation 0. 0. 135.
DV_AC_88.Visibility 1
DV_AC_88.Color 2
DV_AC_88.Mother Dom_Volume

Volume DV_AC_88a
DV_AC_88a.Material Aluminium
DV_AC_88a.Shape BRIK 6.55 0.1 0.75
DV_AC_88a.Position 32.718 21.568 -18.35
DV_AC_88a.Rotation 0. 0. 135.
DV_AC_88a.Visibility 1
DV_AC_88a.Color 2
DV_AC_88a.Mother Dom_Volume

Volume DV_AC_88b
DV_AC_88b.Material Aluminium
DV_AC_88b.Shape BRIK 6.55 0.635 0.1
DV_AC_88b.Position 33.26 22.11 -19.0
DV_AC_88b.Rotation 0. 0. 135.
DV_AC_88b.Visibility 1
DV_AC_88b.Color 2
DV_AC_88b.Mother Dom_Volume

Volume DV_AC_89
DV_AC_89.Material Aluminium
DV_AC_89.Shape BRIK 6.55 0.015 3.7
DV_AC_89.Position -32.8 21.65 -15.4
DV_AC_89.Rotation 0. 0. 45.
DV_AC_89.Visibility 1
DV_AC_89.Color 2
DV_AC_89.Mother Dom_Volume

Volume DV_AC_89a
DV_AC_89a.Material Aluminium
DV_AC_89a.Shape BRIK 6.55 0.1 0.75
DV_AC_89a.Position -32.718 21.568 -18.35
DV_AC_89a.Rotation 0. 0. 45.
DV_AC_89a.Visibility 1
DV_AC_89a.Color 2
DV_AC_89a.Mother Dom_Volume

Volume DV_AC_89b
DV_AC_89b.Material Aluminium
DV_AC_89b.Shape BRIK 6.55 0.635 0.1
DV_AC_89b.Position -33.26 22.11 -19.0
DV_AC_89b.Rotation 0. 0. 45.
DV_AC_89b.Visibility 1
DV_AC_89b.Color 2
DV_AC_89b.Mother Dom_Volume

Volume DV_AC_90
DV_AC_90.Material Aluminium
DV_AC_90.Shape BRIK 6.55 0.015 3.7
DV_AC_90.Position 32.8 -21.65 -15.4
DV_AC_90.Rotation 0. 0. 45.
DV_AC_90.Visibility 1
DV_AC_90.Color 2
DV_AC_90.Mother Dom_Volume

Volume DV_AC_90a
DV_AC_90a.Material Aluminium
DV_AC_90a.Shape BRIK 6.55 0.1 0.75
DV_AC_90a.Position 32.718 -21.568 -18.35
DV_AC_90a.Rotation 0. 0. 45.
DV_AC_90a.Visibility 1
DV_AC_90a.Color 2
DV_AC_90a.Mother Dom_Volume

Volume DV_AC_90b
DV_AC_90b.Material Aluminium
DV_AC_90b.Shape BRIK 6.55 0.635 0.1
DV_AC_90b.Position 33.26 -22.11 -19.0
DV_AC_90b.Rotation 0. 0. 45.
DV_AC_90b.Visibility 1
DV_AC_90b.Color 2
DV_AC_90b.Mother Dom_Volume

Volume DV_AC_91
DV_AC_91.Material Aluminium
DV_AC_91.Shape BRIK 6.55 0.015 3.7
DV_AC_91.Position -32.8 -21.65 -15.4
DV_AC_91.Rotation 0. 0. 135.
DV_AC_91.Visibility 1
DV_AC_91.Color 2
DV_AC_91.Mother Dom_Volume

Volume DV_AC_91a
DV_AC_91a.Material Aluminium
DV_AC_91a.Shape BRIK 6.55 0.1 0.75
DV_AC_91a.Position -32.718 -21.568 -18.35
DV_AC_91a.Rotation 0. 0. 135.
DV_AC_91a.Visibility 1
DV_AC_91a.Color 2
DV_AC_91a.Mother Dom_Volume

Volume DV_AC_91b
DV_AC_91b.Material Aluminium
DV_AC_91b.Shape BRIK 6.55 0.635 0.1
DV_AC_91b.Position -33.26 -22.11 -19.0
DV_AC_91b.Rotation 0. 0. 135.
DV_AC_91b.Visibility 1
DV_AC_91b.Color 2
DV_AC_91b.Mother Dom_Volume


Volume DV_AC_92
DV_AC_92.Material Aluminium
DV_AC_92.Shape BRIK 17.0 0.1 0.75
DV_AC_92.Position 0.0 37.47 -18.35
DV_AC_92.Visibility 1
DV_AC_92.Color 2
DV_AC_92.Mother Dom_Volume

Volume DV_AC_92a
DV_AC_92a.Material Aluminium
DV_AC_92a.Shape BRIK 17.0 0.635 0.1
DV_AC_92a.Position 0.0 38.235 -19.0
DV_AC_92a.Visibility 1
DV_AC_92a.Color 2
DV_AC_92a.Mother Dom_Volume

Volume DV_AC_93
DV_AC_93.Material Aluminium
DV_AC_93.Shape BRIK 17.0 0.1 0.75
DV_AC_93.Position 0.0 -37.47 -18.35
DV_AC_93.Visibility 1
DV_AC_93.Color 2
DV_AC_93.Mother Dom_Volume

Volume DV_AC_93a
DV_AC_93a.Material Aluminium
DV_AC_93a.Shape BRIK 17.0 0.635 0.1
DV_AC_93a.Position 0.0 -38.235 -19.0
DV_AC_93a.Visibility 1
DV_AC_93a.Color 2
DV_AC_93a.Mother Dom_Volume

Volume DV_AC_94
DV_AC_94.Material Aluminium
DV_AC_94.Shape BRIK 0.1 17.0 0.75
DV_AC_94.Position 37.47 0.0 -18.35
DV_AC_94.Visibility 1
DV_AC_94.Color 2
DV_AC_94.Mother Dom_Volume

Volume DV_AC_94a
DV_AC_94a.Material Aluminium
DV_AC_94a.Shape BRIK 0.635 17.0 0.1
DV_AC_94a.Position 38.235 0.0 -19.0
DV_AC_94a.Visibility 1
DV_AC_94a.Color 2
DV_AC_94a.Mother Dom_Volume

Volume DV_AC_95
DV_AC_95.Material Aluminium
DV_AC_95.Shape BRIK 0.1 17.0 0.75
DV_AC_95.Position -37.47 0.0 -18.35
DV_AC_95.Visibility 1
DV_AC_95.Color 2
DV_AC_95.Mother Dom_Volume

Volume DV_AC_95a
DV_AC_95a.Material Aluminium
DV_AC_95a.Shape BRIK 0.635 17.0 0.1
DV_AC_95a.Position -38.235 0.0 -19.0
DV_AC_95a.Visibility 1
DV_AC_95a.Color 2
DV_AC_95a.Mother Dom_Volume



// ----------------------------------------------------------------------------------------------------
// Green drawed Ring on bottom:

Volume DV_GreenRing_100
DV_GreenRing_100.Material Air
DV_GreenRing_100.Shape BRIK 39.5 39.5 0.3
DV_GreenRing_100.Position 0. 0. -19.70   // ggf. z-Koordinate korrigieren
DV_GreenRing_100.Visibility 0
DV_GreenRing_100.Virtual true
DV_GreenRing_100.Mother Dom_Volume


Volume DV_GRTempl_101
DV_GRTempl_101.Material Aluminium
DV_GRTempl_101.Shape TRD1 20.5060966544 13.3643181644 0.3 3.57088924499
DV_GRTempl_101.Visibility 1
DV_GRTempl_101.Color 8


DV_GRTempl_101.Copy DV_Part_102
DV_Part_102.Position 27.525 27.525 0.3
DV_Part_102.Rotation -90. 0. -45.
DV_Part_102.Mother DV_GreenRing_100

DV_GRTempl_101.Copy DV_Part_103
DV_Part_103.Position -27.525 27.525 0.3
DV_Part_103.Rotation -90. 0. 45.
DV_Part_103.Mother DV_GreenRing_100

DV_GRTempl_101.Copy DV_Part_104
DV_Part_104.Position -27.525 -27.525 0.3
DV_Part_104.Rotation -90. 0. 135.
DV_Part_104.Mother DV_GreenRing_100

DV_GRTempl_101.Copy DV_Part_105
DV_Part_105.Position 27.525 -27.525 0.3
DV_Part_105.Rotation -90. 0. 225.
DV_Part_105.Mother DV_GreenRing_100


Volume DV_GRTempl_106
DV_GRTempl_106.Material Aluminium
DV_GRTempl_106.Shape TRD1 25.0 10.5 0.3 7.25
DV_GRTempl_106.Visibility 1
DV_GRTempl_106.Color 8


DV_GRTempl_106.Copy DV_Part_107
DV_Part_107.Position 32.25 0.0 0.3
DV_Part_107.Rotation -90. 0. -90.
DV_Part_107.Mother DV_GreenRing_100

DV_GRTempl_106.Copy DV_Part_108
DV_Part_108.Position 0.0 32.25 0.3
DV_Part_108.Rotation -90. 0. 0.
DV_Part_108.Mother DV_GreenRing_100

DV_GRTempl_106.Copy DV_Part_109
DV_Part_109.Position -32.25 0.0 0.3
DV_Part_109.Rotation -90. 0. 90.
DV_Part_109.Mother DV_GreenRing_100

DV_GRTempl_106.Copy DV_Part_110
DV_Part_110.Position 0.0 -32.25 0.3
DV_Part_110.Rotation -90. 0. 180.
DV_Part_110.Mother DV_GreenRing_100


// ---------------------------------------------------------------------------------------------------
// Tables for the PMT:

Volume DV_PMTTTable_120
DV_PMTTTable_120.Material Air
DV_PMTTTable_120.Shape BRIK 25.0 5.0 3.7
DV_PMTTTable_120.Visibility 0
DV_PMTTTable_120.Virtual true


Volume DV_PMTTTop_121
DV_PMTTTop_121.Material Aluminium
DV_PMTTTop_121.Shape BRIK 25.0 0.7 0.3
DV_PMTTTop_121.Position 0.0 -4.3 3.4
DV_PMTTTop_121.Visibility 1
DV_PMTTTop_121.Color 1
DV_PMTTTop_121.Mother DV_PMTTTable_120

Volume DV_PMTTTop_122
DV_PMTTTop_122.Material Aluminium
DV_PMTTTop_122.Shape TRD1 25.0 16.5 0.3 4.3
DV_PMTTTop_122.Position 0.0 0.7 3.4
DV_PMTTTop_122.Rotation -90. 0. 0.
DV_PMTTTop_122.Visibility 1
DV_PMTTTop_122.Color 1
DV_PMTTTop_122.Mother DV_PMTTTable_120


Volume DV_PMTTTempl_123
DV_PMTTTempl_123.Material Aluminium
DV_PMTTTempl_123.Shape TUBS 0. 0.4 3.4 0. 360.
DV_PMTTTempl_123.Visibility 1
DV_PMTTTempl_123.Color 1

DV_PMTTTempl_123.Copy DV_Part_124
DV_Part_124.Position 15.5 3.9 -0.3
DV_Part_124.Mother DV_PMTTTable_120

DV_PMTTTempl_123.Copy DV_Part_125
DV_Part_125.Position -15.5 3.9 -0.3
DV_Part_125.Mother DV_PMTTTable_120

DV_PMTTTempl_123.Copy DV_Part_126
DV_Part_126.Position -15.5 -3.9 -0.3
DV_Part_126.Mother DV_PMTTTable_120

DV_PMTTTempl_123.Copy DV_Part_127
DV_Part_127.Position 15.5 -3.9 -0.3
DV_Part_127.Mother DV_PMTTTable_120

//
DV_PMTTTable_120.Copy DV_PMTTable_128
DV_PMTTable_128.Position 0. 32.15 -15.40   	// überprüfe Koordinaten
DV_PMTTable_128.Mother Dom_Volume

DV_PMTTTable_120.Copy DV_PMTTable_129
DV_PMTTable_129.Position 0. -32.15 -15.40   	// überprüfe Koordinaten
DV_PMTTable_129.Rotation 0. 0. 180.
DV_PMTTable_129.Mother Dom_Volume

DV_PMTTTable_120.Copy DV_PMTTable_130
DV_PMTTable_130.Position -32.15 0. -15.40   	// überprüfe Koordinaten
DV_PMTTable_130.Rotation 0. 0. 90.
DV_PMTTable_130.Mother Dom_Volume

DV_PMTTTable_120.Copy DV_PMTTable_131
DV_PMTTable_131.Position 32.15 0. -15.40   	// überprüfe Koordinaten
DV_PMTTable_131.Rotation 0. 0. 270.
DV_PMTTable_131.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------------

// The PMTs imported from PMT.geo.setup

// Note the PMTs are only consisting of about the right materials and case dimensions,
// but have no inner details!

PMT_Volume.Copy DV_PMT_141
DV_PMT_141.Position 32.1 0.0 -4.40
DV_PMT_141.Rotation 0. 90. -90.
DV_PMT_141.Mother Dom_Volume

PMT_Volume.Copy DV_PMT_142
DV_PMT_142.Position 0.0 32.1 -4.40
DV_PMT_142.Rotation 0. 90. 0.
DV_PMT_142.Mother Dom_Volume

PMT_Volume.Copy DV_PMT_143
DV_PMT_143.Position -32.1 0.0 -4.40
DV_PMT_143.Rotation 0. 90. 90.
DV_PMT_143.Mother Dom_Volume

PMT_Volume.Copy DV_PMT_144
DV_PMT_144.Position 0.0 -32.1 -4.40
DV_PMT_144.Rotation 0. 90. 180.
DV_PMT_144.Mother Dom_Volume


PMT_Volume.Copy DV_PMT_145
DV_PMT_145.Position 0.0 -33.5 -39.4
DV_PMT_145.Rotation 0. 90. 180.
DV_PMT_145.Mother Dom_Volume

PMT_Volume.Copy DV_PMT_146
DV_PMT_146.Position 0.0 33.5 -39.4
DV_PMT_146.Rotation 0. 90. 0.
DV_PMT_146.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------------

// Black drawed Bottom-Plate beneath green-drwaed ring:

Volume DV_BBP_150
DV_BBP_150.Material Aluminium
DV_BBP_150.Shape BRIK 23.6 39.9 0.40
DV_BBP_150.Position 0. 0. -20.1
DV_BBP_150.Visibility 1
DV_BBP_150.Color 1
DV_BBP_150.Mother Dom_Volume

Volume DV_BBP_151
DV_BBP_151.Material Aluminium
DV_BBP_151.Shape TRD1 39.9 23.6 0.4 8.15
DV_BBP_151.Position -31.75 0.0 -20.1
DV_BBP_151.Rotation 90. 0. -90
DV_BBP_151.Visibility 1
DV_BBP_151.Color 1
DV_BBP_151.Mother Dom_Volume

Volume DV_BBP_152
DV_BBP_152.Material Aluminium
DV_BBP_152.Shape TRD1 39.9 23.6 0.4 8.15
DV_BBP_152.Position 31.75 0.0 -20.1
DV_BBP_152.Rotation 90. 0. 90.
DV_BBP_152.Visibility 1
DV_BBP_152.Color 1
DV_BBP_152.Mother Dom_Volume



// ----------------------------------------------------------------------------------------------------

// Carrying parts:

Volume DV_CPTempl_160 
DV_CPTempl_160.Shape BRIK 1.5 0.5 8.47
DV_CPTempl_160.Visibility 0
DV_CPTempl_160.Virtual true
DV_CPTempl_160.Material Air


Volume DV_CPTempl_161
DV_CPTempl_161.Material Aluminium
DV_CPTempl_161.Shape BRIK 1.5 0.5 0.15
DV_CPTempl_161.Position 0. 0. 0.15
DV_CPTempl_161.Color 1
DV_CPTempl_161.Visibility 1
DV_CPTempl_161.Mother DV_CPTempl_160

Volume DV_CPTempl_162
DV_CPTempl_162.Material Aluminium
DV_CPTempl_162.Shape BRIK 1.5 0.1 3.25
DV_CPTempl_162.Position 0.0 0.4 3.55
DV_CPTempl_162.Color 1
DV_CPTempl_162.Visibility 1
DV_CPTempl_162.Mother DV_CPTempl_160

Volume DV_CPTempl_163
DV_CPTempl_163.Material Aluminium
DV_CPTempl_163.Shape BRIK 0.1 0.4 3.25
DV_CPTempl_163.Position 0.0 -0.1 3.55
DV_CPTempl_163.Color 1
DV_CPTempl_163.Visibility 1
DV_CPTempl_163.Mother DV_CPTempl_160

Volume DV_CPTempl_164
DV_CPTempl_164.Material Aluminium
DV_CPTempl_164.Shape BRIK 1.5 0.5 0.1
DV_CPTempl_164.Position 0.0 0.0 6.9
DV_CPTempl_164.Color 1
DV_CPTempl_164.Visibility 1
DV_CPTempl_164.Mother DV_CPTempl_160

Volume DV_CPTempl_165
DV_CPTempl_165.Material Aluminium
DV_CPTempl_165.Shape BRIK 1.5 0.2 0.635
DV_CPTempl_165.Position 0.0 -0.3 7.635
DV_CPTempl_165.Color 1
DV_CPTempl_165.Visibility 1
DV_CPTempl_165.Mother DV_CPTempl_160

Volume DV_CPTempl_166
DV_CPTempl_166.Material Aluminium
DV_CPTempl_166.Shape BRIK 1.5 0.5 0.1
DV_CPTempl_166.Position 0.0 0.0 8.37
DV_CPTempl_166.Color 1
DV_CPTempl_166.Visibility 1
DV_CPTempl_166.Mother DV_CPTempl_160


DV_CPTempl_160.Copy DV_CP_167
DV_CP_167.Position 12.0 34.4 -31.5
DV_CP_167.Rotation 0. 0. 180.
DV_CP_167.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_168
DV_CP_168.Position -12.0 34.4 -31.5
DV_CP_168.Rotation 0. 0. 180.
DV_CP_168.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_169
DV_CP_169.Position 12.0 -34.4 -31.5
//DV_CP_169.Rotation 0. 0. 180.
DV_CP_169.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_170
DV_CP_170.Position -12.0 -34.4 -31.5
//DV_CP_170.Rotation 0. 0. 180.
DV_CP_170.Mother Dom_Volume



DV_CPTempl_160.Copy DV_CP_171
DV_CP_171.Position 34.4 7.2 -31.5
DV_CP_171.Rotation 0. 0. 90.
DV_CP_171.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_172
DV_CP_172.Position 34.4 -7.2 -31.5
DV_CP_172.Rotation 0. 0. 90.
DV_CP_172.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_173
DV_CP_173.Position 34.4 21.2 -31.5
DV_CP_173.Rotation 0. 0. 90.
DV_CP_173.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_174
DV_CP_174.Position 34.4 -21.2 -31.5
DV_CP_174.Rotation 0. 0. 90.
DV_CP_174.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_175
DV_CP_175.Position -34.4 7.2 -31.5
DV_CP_175.Rotation 0. 0. -90.
DV_CP_175.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_176
DV_CP_176.Position -34.4 -7.2 -31.5
DV_CP_176.Rotation 0. 0. -90.
DV_CP_176.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_177
DV_CP_177.Position -34.4 21.2 -31.5
DV_CP_177.Rotation 0. 0. -90.
DV_CP_177.Mother Dom_Volume

DV_CPTempl_160.Copy DV_CP_178
DV_CP_178.Position -34.4 -21.2 -31.5
DV_CP_178.Rotation 0. 0. -90.
DV_CP_178.Mother Dom_Volume

// ----------------------------------------------------------------------------------------------------


// The bottom NE110 - Plate


Volume DV_ABP_180
DV_ABP_180.Material NE110
DV_ABP_180.Shape BRIK 24.5 34.5 0.635
DV_ABP_180.Position 0. 0. -23.865
DV_ABP_180.Visibility 1
DV_ABP_180.Color 7
DV_ABP_180.Mother Dom_Volume

Volume DV_ABP_181
DV_ABP_181.Material NE110
DV_ABP_181.Shape TRD1 34.5 24.5 0.635 5.0
DV_ABP_181.Position -29.5 0.0 -23.865
DV_ABP_181.Rotation 90. 0. -90
DV_ABP_181.Visibility 1
DV_ABP_181.Color 7
DV_ABP_181.Mother Dom_Volume

Volume DV_ABP_182
DV_ABP_182.Material NE110
DV_ABP_182.Shape TRD1 34.5 24.5 0.635 5.0
DV_ABP_182.Position 29.5 0.0 -23.865
DV_ABP_182.Rotation 90. 0. 90.
DV_ABP_182.Visibility 1
DV_ABP_182.Color 7
DV_ABP_182.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------------

// The absolute bottom-plate:

Volume DV_ABP_190
DV_ABP_190.Material Aluminium
DV_ABP_190.Shape BRIK 23.6 39.9 0.3
DV_ABP_190.Position 0. 0. -31.8
DV_ABP_190.Visibility 1
DV_ABP_190.Color 1
DV_ABP_190.Mother Dom_Volume

Volume DV_ABP_191
DV_ABP_191.Material Aluminium
DV_ABP_191.Shape TRD1 39.9 23.6 0.3 8.15
DV_ABP_191.Position -31.75 0.0 -31.8
DV_ABP_191.Rotation 90. 0. -90
DV_ABP_191.Visibility 1
DV_ABP_191.Color 1
DV_ABP_191.Mother Dom_Volume

Volume DV_ABP_192
DV_ABP_192.Material Aluminium
DV_ABP_192.Shape TRD1 39.9 23.6 0.3 8.15
DV_ABP_192.Position 31.75 0.0 -31.8
DV_ABP_192.Rotation 90. 0. 90.
DV_ABP_192.Visibility 1
DV_ABP_192.Color 1
DV_ABP_192.Mother Dom_Volume


// ----------------------------------------------------------------------------------------------------


// The Shoe for the NE110 Walls:

// some very very small parts for binding the aluminium layer to the shoe are neglected.

Volume DV_Shoe_200
DV_Shoe_200.Material Aluminium
DV_Shoe_200.Shape BRIK 27.8 1.54 0.15
DV_Shoe_200.Position 0.0 26.26 -18.95
DV_Shoe_200.Visibility 1
DV_Shoe_200.Color 1
DV_Shoe_200.Mother Dom_Volume

Volume DV_Shoe_201
DV_Shoe_201.Material Aluminium
DV_Shoe_201.Shape BRIK 27.8 1.54 0.15
DV_Shoe_201.Position 0.0 -26.26 -18.95
DV_Shoe_201.Visibility 1
DV_Shoe_201.Color 1
DV_Shoe_201.Mother Dom_Volume

Volume DV_Shoe_202
DV_Shoe_202.Material Aluminium
DV_Shoe_202.Shape BRIK 1.54 24.72 0.15
DV_Shoe_202.Position 26.26 0.0 -18.95
DV_Shoe_202.Visibility 1
DV_Shoe_202.Color 1
DV_Shoe_202.Mother Dom_Volume

Volume DV_Shoe_203
DV_Shoe_203.Material Aluminium
DV_Shoe_203.Shape BRIK 1.54 24.72 0.15
DV_Shoe_203.Position -26.26 0.0 -18.95
DV_Shoe_203.Visibility 1
DV_Shoe_203.Color 1
DV_Shoe_203.Mother Dom_Volume


Volume DV_Shoe_204
DV_Shoe_204.Material Aluminium
DV_Shoe_204.Shape BRIK 26.62 0.175 0.15
DV_Shoe_204.Position 0.0 26.445 -18.65
DV_Shoe_204.Visibility 1
DV_Shoe_204.Color 1
DV_Shoe_204.Mother Dom_Volume

Volume DV_Shoe_205
DV_Shoe_205.Material Aluminium
DV_Shoe_205.Shape BRIK 26.62 0.175 0.15
DV_Shoe_205.Position 0.0 -26.445 -18.65
DV_Shoe_205.Visibility 1
DV_Shoe_205.Color 1
DV_Shoe_205.Mother Dom_Volume

Volume DV_Shoe_206
DV_Shoe_206.Material Aluminium
DV_Shoe_206.Shape BRIK 0.175 26.27 0.15
DV_Shoe_206.Position 26.445 0.0 -18.65
DV_Shoe_206.Visibility 1
DV_Shoe_206.Color 1
DV_Shoe_206.Mother Dom_Volume

Volume DV_Shoe_207
DV_Shoe_207.Material Aluminium
DV_Shoe_207.Shape BRIK 0.175 26.27 0.15
DV_Shoe_207.Position -26.445 0.0 -18.65
DV_Shoe_207.Visibility 1
DV_Shoe_207.Color 1
DV_Shoe_207.Mother Dom_Volume


Volume DV_Shoe_208
DV_Shoe_208.Material Aluminium
DV_Shoe_208.Shape BRIK 25.07 0.175 0.15
DV_Shoe_208.Position 0.0 24.895 -18.65
DV_Shoe_208.Visibility 1
DV_Shoe_208.Color 1
DV_Shoe_208.Mother Dom_Volume

Volume DV_Shoe_209
DV_Shoe_209.Material Aluminium
DV_Shoe_209.Shape BRIK 25.07 0.175 0.15
DV_Shoe_209.Position 0.0 -24.895 -18.65
DV_Shoe_209.Visibility 1
DV_Shoe_209.Color 1
DV_Shoe_209.Mother Dom_Volume

Volume DV_Shoe_210
DV_Shoe_210.Material Aluminium
DV_Shoe_210.Shape BRIK 0.175 24.72 0.15
DV_Shoe_210.Position 24.895 0.0 -18.65
DV_Shoe_210.Visibility 1
DV_Shoe_210.Color 1
DV_Shoe_210.Mother Dom_Volume

Volume DV_Shoe_211
DV_Shoe_211.Material Aluminium
DV_Shoe_211.Shape BRIK 0.175 24.72 0.15
DV_Shoe_211.Position -24.895 0.0 -18.65
DV_Shoe_211.Visibility 1
DV_Shoe_211.Color 1
DV_Shoe_211.Mother Dom_Volume





// ----------------------------------------------------------------------------------------------------

// The thin aluminium cover for the bottom ACS NE110 Plate:

// This is supposed to be of ocatgonal shape, but since the exact position of the corners is not 
// notified in the drawing, I guessed possible values. However right is the thickness and maximum width.


Volume DV_Part_220
DV_Part_220.Material Aluminium
DV_Part_220.Shape BRIK 25.5 35.3 0.015
DV_Part_220.Position 0.0 0.0 -22.815
DV_Part_220.Visibility 1
DV_Part_220.Color 2
DV_Part_220.Mother Dom_Volume

Volume DV_Part_221
DV_Part_221.Material Aluminium
DV_Part_221.Shape TRD1 25.5 35.3 0.015 4.9
DV_Part_221.Position 30.4 0.0 -22.815
DV_Part_221.Rotation -90. 0. 90.
DV_Part_221.Visibility 1
DV_Part_221.Color 2
DV_Part_221.Mother Dom_Volume

Volume DV_Part_222
DV_Part_222.Material Aluminium
DV_Part_222.Shape TRD1 25.5 35.3 0.015 4.9
DV_Part_222.Position -30.4 0.0 -22.815
DV_Part_222.Rotation -90. 0. -90.
DV_Part_222.Visibility 1
DV_Part_222.Color 2
DV_Part_222.Mother Dom_Volume




Volume DV_Part_223
DV_Part_223.Material Aluminium
DV_Part_223.Shape BRIK 25.5 0.015 4.335
DV_Part_223.Position 0.0 35.285 -27.165
DV_Part_223.Visibility 1
DV_Part_223.Color 2
DV_Part_223.Mother Dom_Volume

Volume DV_Part_223a
DV_Part_223a.Material Aluminium
DV_Part_223a.Shape BRIK 25.5 0.635 0.1
DV_Part_223a.Position 0.0 35.935 -31.4
DV_Part_223a.Visibility 1
DV_Part_223a.Color 2
DV_Part_223a.Mother Dom_Volume

Volume DV_Part_223b
DV_Part_223b.Material Aluminium
DV_Part_223b.Shape BRIK 25.5 0.1 0.75
DV_Part_223b.Position 0.0 35.17 -30.75
DV_Part_223b.Visibility 1
DV_Part_223b.Color 2
DV_Part_223b.Mother Dom_Volume

Volume DV_Part_224
DV_Part_224.Material Aluminium
DV_Part_224.Shape BRIK 25.5 0.015 4.335
DV_Part_224.Position 0.0 -35.285 -27.165
DV_Part_224.Visibility 1
DV_Part_224.Color 2
DV_Part_224.Mother Dom_Volume

Volume DV_Part_224a
DV_Part_224a.Material Aluminium
DV_Part_224a.Shape BRIK 25.5 0.635 0.1
DV_Part_224a.Position 0.0 -35.935 -31.4
DV_Part_224a.Visibility 1
DV_Part_224a.Color 2
DV_Part_224a.Mother Dom_Volume

Volume DV_Part_224b
DV_Part_224b.Material Aluminium
DV_Part_224b.Shape BRIK 25.5 0.1 0.75
DV_Part_224b.Position 0.0 -35.17 -30.75
DV_Part_224b.Visibility 1
DV_Part_224b.Color 2
DV_Part_224b.Mother Dom_Volume

Volume DV_Part_225
DV_Part_225.Material Aluminium
DV_Part_225.Shape BRIK 0.015 25.5 4.335
DV_Part_225.Position 35.285 0.0  -27.165
DV_Part_225.Visibility 1
DV_Part_225.Color 2
DV_Part_225.Mother Dom_Volume

Volume DV_Part_225a
DV_Part_225a.Material Aluminium
DV_Part_225a.Shape BRIK 0.635 25.5 0.1
DV_Part_225a.Position 35.935 0.0 -31.4
DV_Part_225a.Visibility 1
DV_Part_225a.Color 2
DV_Part_225a.Mother Dom_Volume

Volume DV_Part_225b
DV_Part_225b.Material Aluminium
DV_Part_225b.Shape BRIK 0.1 25.5 0.75
DV_Part_225b.Position 35.17 0.0 -30.75
DV_Part_225b.Visibility 1
DV_Part_225b.Color 2
DV_Part_225b.Mother Dom_Volume

Volume DV_Part_226
DV_Part_226.Material Aluminium
DV_Part_226.Shape BRIK 0.015 25.5 4.335
DV_Part_226.Position -35.285 0.0 -27.165
DV_Part_226.Visibility 1
DV_Part_226.Color 2
DV_Part_226.Mother Dom_Volume

Volume DV_Part_226a
DV_Part_226a.Material Aluminium
DV_Part_226a.Shape BRIK 0.635 25.5 0.1
DV_Part_226a.Position -35.935 0.0 -31.4
DV_Part_226a.Visibility 1
DV_Part_226a.Color 2
DV_Part_226a.Mother Dom_Volume

Volume DV_Part_226b
DV_Part_226b.Material Aluminium
DV_Part_226b.Shape BRIK 0.1 25.5 0.75
DV_Part_226b.Position -35.17 0.0 -30.75
DV_Part_226b.Visibility 1
DV_Part_226b.Color 2
DV_Part_226b.Mother Dom_Volume


Volume DV_Part_227
DV_Part_227.Material Aluminium
DV_Part_227.Shape BRIK 6.9 0.015 4.335
DV_Part_227.Position 30.39 30.39 -27.165
DV_Part_227.Rotation 0.0 0.0 135.0
DV_Part_227.Visibility 1
DV_Part_227.Color 2
DV_Part_227.Mother Dom_Volume

Volume DV_Part_227a
DV_Part_227a.Material Aluminium
DV_Part_227a.Shape BRIK 6.93 0.635  0.1
DV_Part_227a.Position 30.85 30.85 -31.4
DV_Part_227a.Rotation 0.0 0.0 135.0
DV_Part_227a.Visibility 1
DV_Part_227a.Color 2
DV_Part_227a.Mother Dom_Volume

Volume DV_Part_227b
DV_Part_227b.Material Aluminium
DV_Part_227b.Shape BRIK 6.75 0.1 0.75
DV_Part_227b.Position 30.307 30.307 -30.75
DV_Part_227b.Rotation 0.0 0.0 135.0
DV_Part_227b.Visibility 1
DV_Part_227b.Color 2
DV_Part_227b.Mother Dom_Volume



Volume DV_Part_228
DV_Part_228.Material Aluminium
DV_Part_228.Shape BRIK 6.9 0.015 4.335
DV_Part_228.Position 30.39 -30.39 -27.165
DV_Part_228.Rotation 0.0 0.0 45.0
DV_Part_228.Visibility 1
DV_Part_228.Color 2
DV_Part_228.Mother Dom_Volume

Volume DV_Part_228a
DV_Part_228a.Material Aluminium
DV_Part_228a.Shape BRIK 6.93 0.635  0.1
DV_Part_228a.Position 30.85 -30.85 -31.4
DV_Part_228a.Rotation 0.0 0.0 45.0
DV_Part_228a.Visibility 1
DV_Part_228a.Color 2
DV_Part_228a.Mother Dom_Volume

Volume DV_Part_228b
DV_Part_228b.Material Aluminium
DV_Part_228b.Shape BRIK 6.75 0.1 0.75
DV_Part_228b.Position 30.307 -30.307 -30.75
DV_Part_228b.Rotation 0.0 0.0 45.0
DV_Part_228b.Visibility 1
DV_Part_228b.Color 2
DV_Part_228b.Mother Dom_Volume



Volume DV_Part_229
DV_Part_229.Material Aluminium
DV_Part_229.Shape BRIK 6.9 0.015 4.335
DV_Part_229.Position -30.39 30.39 -27.165
DV_Part_229.Rotation 0.0 0.0 45.0
DV_Part_229.Visibility 1
DV_Part_229.Color 2
DV_Part_229.Mother Dom_Volume

Volume DV_Part_229a
DV_Part_229a.Material Aluminium
DV_Part_229a.Shape BRIK 6.93 0.635  0.1
DV_Part_229a.Position -30.85 30.85 -31.4
DV_Part_229a.Rotation 0.0 0.0 45.0
DV_Part_229a.Visibility 1
DV_Part_229a.Color 2
DV_Part_229a.Mother Dom_Volume

Volume DV_Part_229b
DV_Part_229b.Material Aluminium
DV_Part_229b.Shape BRIK 6.75 0.1 0.75
DV_Part_229b.Position -30.307 30.307 -30.75
DV_Part_229b.Rotation 0.0 0.0 45.0
DV_Part_229b.Visibility 1
DV_Part_229b.Color 2
DV_Part_229b.Mother Dom_Volume


Volume DV_Part_230
DV_Part_230.Material Aluminium
DV_Part_230.Shape BRIK 6.9 0.015 4.335
DV_Part_230.Position -30.39 -30.39 -27.165
DV_Part_230.Rotation 0.0 0.0 135.0
DV_Part_230.Visibility 1
DV_Part_230.Color 2
DV_Part_230.Mother Dom_Volume

Volume DV_Part_230a
DV_Part_230a.Material Aluminium
DV_Part_230a.Shape BRIK 6.93 0.635  0.1
DV_Part_230a.Position -30.85 -30.85 -31.4
DV_Part_230a.Rotation 0.0 0.0 135.0
DV_Part_230a.Visibility 1
DV_Part_230a.Color 2
DV_Part_230a.Mother Dom_Volume

Volume DV_Part_230b
DV_Part_230b.Material Aluminium
DV_Part_230b.Shape BRIK 6.75 0.1 0.75
DV_Part_230b.Position -30.307 -30.307 -30.75
DV_Part_230b.Rotation 0.0 0.0 135.0
DV_Part_230b.Visibility 1
DV_Part_230b.Color 2
DV_Part_230b.Mother Dom_Volume




// ----------------------------------------------------------------------------------------------------

// The Cable Tree:

// The very exact position of the cable tree is not known yet, so I guessed one.
// Also there is a gap in the cable tree of less than 1 cm though the bottom plate.
// I hope this doesn't matter.


// Cable Tree 1:
// Plasic Isolation:
Volume DV_CableTree1_240
DV_CableTree1_240.Material Plastic_Isolation
DV_CableTree1_240.Shape BRIK 1.55 0.305 10.0
DV_CableTree1_240.Position 10.0 -30.0  -22.0
DV_CableTree1_240.Rotation 90. 0. 0.
DV_CableTree1_240.Color 2
DV_CableTree1_240.Visibility 1
DV_CableTree1_240.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_241
DV_CableTree1_241.Material Copper
DV_CableTree1_241.Shape BRIK 1.55 0.055 10.0
DV_CableTree1_241.Position 0.0 0.0 0.0
DV_CableTree1_241.Color 2
DV_CableTree1_241.Visibility 1
DV_CableTree1_241.Mother DV_CableTree1_240


// Plasic Isolation:
Volume DV_CableTree1_242
DV_CableTree1_242.Material Plastic_Isolation
DV_CableTree1_242.Shape BRIK 1.55 0.305 0.9
DV_CableTree1_242.Position 10.0 -19.65  -21.4
//DV_CableTree1_242.Rotation 90. 0. 0.
DV_CableTree1_242.Color 2
DV_CableTree1_242.Visibility 1
DV_CableTree1_242.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_243
DV_CableTree1_243.Material Copper
DV_CableTree1_243.Shape BRIK 1.55 0.055 0.9
DV_CableTree1_243.Position 0.0 0.0 0.0
DV_CableTree1_243.Color 2
DV_CableTree1_243.Visibility 1
DV_CableTree1_243.Mother DV_CableTree1_242


// Plasic Isolation:
Volume DV_CableTree1_244
DV_CableTree1_244.Material Plastic_Isolation
DV_CableTree1_244.Shape BRIK 1.55 0.305 10.0
DV_CableTree1_244.Position 10.0 -40.305  -31.7
//DV_CableTree1_244.Rotation 90. 0. 0.
DV_CableTree1_244.Color 2
DV_CableTree1_244.Visibility 1
DV_CableTree1_244.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_245
DV_CableTree1_245.Material Copper
DV_CableTree1_245.Shape BRIK 1.55 0.055 10.0
DV_CableTree1_245.Position 0.0 0.0 0.0
DV_CableTree1_245.Color 2
DV_CableTree1_245.Visibility 1
DV_CableTree1_245.Mother DV_CableTree1_244




// Cable Tree 2:
// Plasic Isolation:
Volume DV_CableTree1_250
DV_CableTree1_250.Material Plastic_Isolation
DV_CableTree1_250.Shape BRIK 0.305 1.55 10.0
DV_CableTree1_250.Position 30.0 -10.0  -22.0
DV_CableTree1_250.Rotation 0. 90. 0.
DV_CableTree1_250.Color 2
DV_CableTree1_250.Visibility 1
DV_CableTree1_250.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_251
DV_CableTree1_251.Material Copper
DV_CableTree1_251.Shape BRIK 0.055 1.55 10.0
DV_CableTree1_251.Position 0.0 0.0 0.0
DV_CableTree1_251.Color 2
DV_CableTree1_251.Visibility 1
DV_CableTree1_251.Mother DV_CableTree1_250


// Plasic Isolation:
Volume DV_CableTree1_252
DV_CableTree1_252.Material Plastic_Isolation
DV_CableTree1_252.Shape BRIK 0.305 1.55 0.9
DV_CableTree1_252.Position 19.65 -10.0  -21.4
DV_CableTree1_252.Color 2
DV_CableTree1_252.Visibility 1
DV_CableTree1_252.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_253
DV_CableTree1_253.Material Copper
DV_CableTree1_253.Shape BRIK 0.055 1.55 0.9
DV_CableTree1_253.Position 0.0 0.0 0.0
DV_CableTree1_253.Color 2
DV_CableTree1_253.Visibility 1
DV_CableTree1_253.Mother DV_CableTree1_252


// Plasic Isolation:
Volume DV_CableTree1_254
DV_CableTree1_254.Material Plastic_Isolation
DV_CableTree1_254.Shape BRIK 0.305 1.55 10.0
DV_CableTree1_254.Position 40.305 -10.0  -31.7
//DV_CableTree1_254.Rotation 90. 0. 0.
DV_CableTree1_254.Color 2
DV_CableTree1_254.Visibility 1
DV_CableTree1_254.Mother Dom_Volume

// Cu:
Volume DV_CableTree1_255
DV_CableTree1_255.Material Copper
DV_CableTree1_255.Shape BRIK 0.055 1.55 10.0
DV_CableTree1_255.Position 0.0 0.0 0.0
DV_CableTree1_255.Color 2
DV_CableTree1_255.Visibility 1
DV_CableTree1_255.Mother DV_CableTree1_254



// ----------------------------------------------------------------------------------------------------

// Cylindrical Connections to the outside world:

// Much about these Connections is roughly approximated !!
// Also there is a safe distance from the connections to the egg wall, for I don't know if in the simulation a circle is 
// approximated by a polygon (and if so, how good?) or really is a circle. In the Viewer first is the case.



Volume DV_CC_260a
DV_CC_260a.Material Aluminium
DV_CC_260a.Shape TUBS 0.0 3.0 0.9 0. 360.
DV_CC_260a.Rotation 90. 0. -45.
DV_CC_260a.Position 32. 32. -26.
DV_CC_260a.Visibility 1
DV_CC_260a.Color 4
DV_CC_260a.Mother Dom_Volume

Volume DV_CC_260b
DV_CC_260b.Material Aluminium
DV_CC_260b.Shape TUBS 0.0 2.2 0.4 0. 360.
DV_CC_260b.Rotation 90. 0. -45.
DV_CC_260b.Position 32.95 32.95 -26.
DV_CC_260b.Visibility 1
DV_CC_260b.Color 4
DV_CC_260b.Mother Dom_Volume

Volume DV_CC_260c
DV_CC_260c.Material Aluminium
DV_CC_260c.Shape TUBS 0.0 5.3 0.7 0. 360.
DV_CC_260c.Rotation 90. 0. -45.
DV_CC_260c.Position 33.8 33.8 -26.
DV_CC_260c.Visibility 1
DV_CC_260c.Color 4
DV_CC_260c.Mother Dom_Volume

Volume DV_CC_260d
DV_CC_260d.Material Aluminium
DV_CC_260d.Shape TUBS 0.0 4.2 0.5 0. 360.
DV_CC_260d.Rotation 90. 0. -45.
DV_CC_260d.Position 34.7 34.7 -26.
DV_CC_260d.Visibility 1
DV_CC_260d.Color 4
DV_CC_260d.Mother Dom_Volume



Volume DV_CC_261a
DV_CC_261a.Material Aluminium
DV_CC_261a.Shape TUBS 0.0 3.0 0.9 0. 360.
DV_CC_261a.Rotation 90. 0. 45.
DV_CC_261a.Position 32. -32. -26.
DV_CC_261a.Visibility 1
DV_CC_261a.Color 4
DV_CC_261a.Mother Dom_Volume

Volume DV_CC_261b
DV_CC_261b.Material Aluminium
DV_CC_261b.Shape TUBS 0.0 2.2 0.4 0. 360.
DV_CC_261b.Rotation 90. 0. 45.
DV_CC_261b.Position 32.95 -32.95 -26.
DV_CC_261b.Visibility 1
DV_CC_261b.Color 4
DV_CC_261b.Mother Dom_Volume

Volume DV_CC_261c
DV_CC_261c.Material Aluminium
DV_CC_261c.Shape TUBS 0.0 5.3 0.7 0. 360.
DV_CC_261c.Rotation 90. 0. 45.
DV_CC_261c.Position 33.8 -33.8 -26.
DV_CC_261c.Visibility 1
DV_CC_261c.Color 4
DV_CC_261c.Mother Dom_Volume

Volume DV_CC_261d
DV_CC_261d.Material Aluminium
DV_CC_261d.Shape TUBS 0.0 4.2 0.5 0. 360.
DV_CC_261d.Rotation 90. 0. 45.
DV_CC_261d.Position 34.7 -34.7 -26.
DV_CC_261d.Visibility 1
DV_CC_261d.Color 4
DV_CC_261d.Mother Dom_Volume



Volume DV_CC_262a
DV_CC_262a.Material Aluminium
DV_CC_262a.Shape TUBS 0.0 3.0 0.9 0. 360.
DV_CC_262a.Rotation 90. 0. 45.
DV_CC_262a.Position -32. 32. -26.
DV_CC_262a.Visibility 1
DV_CC_262a.Color 4
DV_CC_262a.Mother Dom_Volume

Volume DV_CC_262b
DV_CC_262b.Material Aluminium
DV_CC_262b.Shape TUBS 0.0 2.2 0.4 0. 360.
DV_CC_262b.Rotation 90. 0. 45.
DV_CC_262b.Position -32.95 32.95 -26.
DV_CC_262b.Visibility 1
DV_CC_262b.Color 4
DV_CC_262b.Mother Dom_Volume

Volume DV_CC_262c
DV_CC_262c.Material Aluminium
DV_CC_262c.Shape TUBS 0.0 5.3 0.7 0. 360.
DV_CC_262c.Rotation 90. 0. 45.
DV_CC_262c.Position -33.8 33.8 -26.
DV_CC_262c.Visibility 1
DV_CC_262c.Color 4
DV_CC_262c.Mother Dom_Volume

Volume DV_CC_262d
DV_CC_262d.Material Aluminium
DV_CC_262d.Shape TUBS 0.0 4.2 0.5 0. 360.
DV_CC_262d.Rotation 90. 0. 45.
DV_CC_262d.Position -34.7 34.7 -26.
DV_CC_262d.Visibility 1
DV_CC_262d.Color 4
DV_CC_262d.Mother Dom_Volume



Volume DV_CC_263a
DV_CC_263a.Material Aluminium
DV_CC_263a.Shape TUBS 0.0 3.0 0.9 0. 360.
DV_CC_263a.Rotation 90. 0. -45.
DV_CC_263a.Position -32. -32. -26.
DV_CC_263a.Visibility 1
DV_CC_263a.Color 4
DV_CC_263a.Mother Dom_Volume

Volume DV_CC_263b
DV_CC_263b.Material Aluminium
DV_CC_263b.Shape TUBS 0.0 2.2 0.4 0. 360.
DV_CC_263b.Rotation 90. 0. -45.
DV_CC_263b.Position -32.95 -32.95 -26.
DV_CC_263b.Visibility 1
DV_CC_263b.Color 4
DV_CC_263b.Mother Dom_Volume

Volume DV_CC_263c
DV_CC_263c.Material Aluminium
DV_CC_263c.Shape TUBS 0.0 5.3 0.7 0. 360.
DV_CC_263c.Rotation 90. 0. -45.
DV_CC_263c.Position -33.8 -33.8 -26.
DV_CC_263c.Visibility 1
DV_CC_263c.Color 4
DV_CC_263c.Mother Dom_Volume

Volume DV_CC_263d
DV_CC_263d.Material Aluminium
DV_CC_263d.Shape TUBS 0.0 4.2 0.5 0. 360.
DV_CC_263d.Rotation 90. 0. -45.
DV_CC_263d.Position -34.7 -34.7 -26.
DV_CC_263d.Visibility 1
DV_CC_263d.Color 4
DV_CC_263d.Mother Dom_Volume



// ----------------------------------------------------------------------------------------------------

