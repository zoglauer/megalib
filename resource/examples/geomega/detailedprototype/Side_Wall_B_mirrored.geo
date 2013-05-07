// Setup file for the MEGA prototype in the Duke configuration

// This is almost the same file as Side_Wall_B.setup, but only mirrored with respect to the x-axis.

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// The Mother Volume of the Detectot Configuration:
Volume SideWallBm_Volume
SideWallBm_Volume.Material Air
SideWallBm_Volume.Visibility 0
SideWallBm_Volume.Virtual true
SideWallBm_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// SideWallBm_Volume.Mother 0



// Thin Wall (1)
Volume SideWallBm_ThinWall_1
SideWallBm_ThinWall_1.Material Aluminium
SideWallBm_ThinWall_1.Shape BRIK 13.425 6.35 0.01
SideWallBm_ThinWall_1.Position -0.225 0.15 0.01
SideWallBm_ThinWall_1.Color 1
SideWallBm_ThinWall_1.Visibility 1
SideWallBm_ThinWall_1.Mother SideWallBm_Volume

// Upper Wall,  "North Wall"  (2)
Volume SideWallBm_UpperWall_2
SideWallBm_UpperWall_2.Material Aluminium
SideWallBm_UpperWall_2.Shape BRIK 14.325 0.1 0.44
SideWallBm_UpperWall_2.Position -0.225 6.4 0.46
SideWallBm_UpperWall_2.Color 1
SideWallBm_UpperWall_2.Visibility 1
SideWallBm_UpperWall_2.Mother SideWallBm_Volume

// Lower Wall,  "South Wall"  (3)
Volume SideWallBm_LowerWall_3
SideWallBm_LowerWall_3.Material Aluminium
SideWallBm_LowerWall_3.Shape BRIK 14.325 0.1 0.44
SideWallBm_LowerWall_3.Position -0.225 -6.1 0.46
SideWallBm_LowerWall_3.Color 1
SideWallBm_LowerWall_3.Visibility 1
SideWallBm_LowerWall_3.Mother SideWallBm_Volume

// Left Wall,  "Western Wall"  (4)
Volume SideWallBm_LeftWall_4
SideWallBm_LeftWall_4.Material Aluminium
SideWallBm_LeftWall_4.Shape BRIK 0.1 6.15 0.44
SideWallBm_LeftWall_4.Position 14.0 0.15 0.46
SideWallBm_LeftWall_4.Color 1
SideWallBm_LeftWall_4.Visibility 1
SideWallBm_LeftWall_4.Mother SideWallBm_Volume

// Right Wall,  "Western Wall"  (5)
Volume SideWallBm_RightWall_5
SideWallBm_RightWall_5.Material Aluminium
SideWallBm_RightWall_5.Shape BRIK 0.1 6.15 0.44
SideWallBm_RightWall_5.Position -14.45 0.15 0.46
SideWallBm_RightWall_5.Color 1
SideWallBm_RightWall_5.Visibility 1
SideWallBm_RightWall_5.Mother SideWallBm_Volume

// (6)
Volume SideWallBm_Part_6
SideWallBm_Part_6.Material Aluminium
SideWallBm_Part_6.Shape BRIK 0.4 6.15 0.09
SideWallBm_Part_6.Position 13.5 0.15 0.11
SideWallBm_Part_6.Color 1
SideWallBm_Part_6.Visibility 1
SideWallBm_Part_6.Mother SideWallBm_Volume

// (6a)
//Volume SideWallBm_Part_6a
//SideWallBm_Part_6a.Material Aluminium
//SideWallBm_Part_6a.Shape BRIK 0.05 6.15 0.035
//SideWallBm_Part_6a.Position 13.15 0.15 0.015
//SideWallBm_Part_6a.Color 1
//SideWallBm_Part_6a.Visibility 1
//SideWallBm_Part_6a.Mother SideWallBm_Volume

// (7)
Volume SideWallBm_Part_7
SideWallBm_Part_7.Material Aluminium
SideWallBm_Part_7.Shape BRIK 0.4 0.1 0.35
SideWallBm_Part_7.Position 13.5 0.0 0.55
SideWallBm_Part_7.Color 1
SideWallBm_Part_7.Visibility 1
SideWallBm_Part_7.Mother SideWallBm_Volume

// (8)
Volume SideWallBm_Part_8
SideWallBm_Part_8.Material Aluminium
SideWallBm_Part_8.Shape BRIK 0.4 6.15 0.09
SideWallBm_Part_8.Position -13.95 0.15 0.11
SideWallBm_Part_8.Color 1
SideWallBm_Part_8.Visibility 1
SideWallBm_Part_8.Mother SideWallBm_Volume

// (8a)
//Volume SideWallBm_Part_8a
//SideWallBm_Part_8a.Material Aluminium
//SideWallBm_Part_8a.Shape BRIK 0.05 6.15 0.035
//SideWallBm_Part_8a.Position -13.6 0.15 0.015
//SideWallBm_Part_8a.Color 1
//SideWallBm_Part_8a.Visibility 1
//SideWallBm_Part_8a.Mother SideWallBm_Volume

// (9)
Volume SideWallBm_Part_9
SideWallBm_Part_9.Material Aluminium
SideWallBm_Part_9.Shape BRIK 0.4 0.1 0.35
SideWallBm_Part_9.Position -13.95 0.0 0.55
SideWallBm_Part_9.Color 1
SideWallBm_Part_9.Visibility 1
SideWallBm_Part_9.Mother SideWallBm_Volume

// (10)
Volume SideWallBm_Part_10
SideWallBm_Part_10.Material Aluminium
SideWallBm_Part_10.Shape BRIK 0.1 6.15 0.44
SideWallBm_Part_10.Position 4.55 0.15 0.46
SideWallBm_Part_10.Color 1
SideWallBm_Part_10.Visibility 1
SideWallBm_Part_10.Mother SideWallBm_Volume

// (11)
Volume SideWallBm_Part_11
SideWallBm_Part_11.Material Aluminium
SideWallBm_Part_11.Shape BRIK 0.1 6.15 0.44
SideWallBm_Part_11.Position -4.55 0.15 0.46
SideWallBm_Part_11.Color 1
SideWallBm_Part_11.Visibility 1
SideWallBm_Part_11.Mother SideWallBm_Volume

// (12)
Volume SideWallBm_Part_12
SideWallBm_Part_12.Material Aluminium
SideWallBm_Part_12.Shape BRIK 4.225 0.1 0.44
SideWallBm_Part_12.Position 8.875 0.0 0.46
SideWallBm_Part_12.Color 1
SideWallBm_Part_12.Visibility 1
SideWallBm_Part_12.Mother SideWallBm_Volume

// (13)
Volume SideWallBm_Part_13
SideWallBm_Part_13.Material Aluminium
SideWallBm_Part_13.Shape BRIK 4.45 0.1 0.44
SideWallBm_Part_13.Position 0.0 0.0 0.46
SideWallBm_Part_13.Color 1
SideWallBm_Part_13.Visibility 1
SideWallBm_Part_13.Mother SideWallBm_Volume

// (14)
Volume SideWallBm_Part_14
SideWallBm_Part_14.Material Aluminium
SideWallBm_Part_14.Shape BRIK 4.45 0.1 0.44
SideWallBm_Part_14.Position -9.1 0.0 0.46
SideWallBm_Part_14.Color 1
SideWallBm_Part_14.Visibility 1
SideWallBm_Part_14.Mother SideWallBm_Volume

// (15)
Volume SideWallBm_Part_15
SideWallBm_Part_15.Material Aluminium
SideWallBm_Part_15.Shape BRIK 4.225 0.05 0.09
SideWallBm_Part_15.Position 8.875 3.1 0.11
SideWallBm_Part_15.Color 1
SideWallBm_Part_15.Visibility 1
SideWallBm_Part_15.Mother SideWallBm_Volume

// (16)
Volume SideWallBm_Part_16
SideWallBm_Part_16.Material Aluminium
SideWallBm_Part_16.Shape BRIK 4.225 0.05 0.09
SideWallBm_Part_16.Position 8.875 -3.05 0.11
SideWallBm_Part_16.Color 1
SideWallBm_Part_16.Visibility 1
SideWallBm_Part_16.Mother SideWallBm_Volume

// (17)
Volume SideWallBm_Part_17
SideWallBm_Part_17.Material Aluminium
SideWallBm_Part_17.Shape BRIK 4.45 0.05 0.09
SideWallBm_Part_17.Position 0.0 3.1 0.11
SideWallBm_Part_17.Color 1
SideWallBm_Part_17.Visibility 1
SideWallBm_Part_17.Mother SideWallBm_Volume

// (18)
Volume SideWallBm_Part_18
SideWallBm_Part_18.Material Aluminium
SideWallBm_Part_18.Shape BRIK 4.45 0.05 0.09
SideWallBm_Part_18.Position 0.0 -3.05 0.11
SideWallBm_Part_18.Color 1
SideWallBm_Part_18.Visibility 1
SideWallBm_Part_18.Mother SideWallBm_Volume

// (19)
Volume SideWallBm_Part_19
SideWallBm_Part_19.Material Aluminium
SideWallBm_Part_19.Shape BRIK 4.45 0.05 0.09
SideWallBm_Part_19.Position -9.1 3.1 0.11
SideWallBm_Part_19.Color 1
SideWallBm_Part_19.Visibility 1
SideWallBm_Part_19.Mother SideWallBm_Volume

// (20)
Volume SideWallBm_Part_20
SideWallBm_Part_20.Material Aluminium
SideWallBm_Part_20.Shape BRIK 4.45 0.05 0.09
SideWallBm_Part_20.Position -9.1 -3.05 0.11
SideWallBm_Part_20.Color 1
SideWallBm_Part_20.Visibility 1
SideWallBm_Part_20.Mother SideWallBm_Volume



// (21)
Volume SideWallBm_Part_21
SideWallBm_Part_21.Material Aluminium
SideWallBm_Part_21.Shape BRIK 0.05 1.575 0.09
SideWallBm_Part_21.Position 8.875 4.725 0.11
SideWallBm_Part_21.Color 1
SideWallBm_Part_21.Visibility 1
SideWallBm_Part_21.Mother SideWallBm_Volume

// (22)
Volume SideWallBm_Part_22
SideWallBm_Part_22.Material Aluminium
SideWallBm_Part_22.Shape BRIK 0.05 1.575 0.09
SideWallBm_Part_22.Position 0.0 4.725 0.11
SideWallBm_Part_22.Color 1
SideWallBm_Part_22.Visibility 1
SideWallBm_Part_22.Mother SideWallBm_Volume

// (23)
Volume SideWallBm_Part_23
SideWallBm_Part_23.Material Aluminium
SideWallBm_Part_23.Shape BRIK 0.05 1.575 0.09
SideWallBm_Part_23.Position -9.1 4.725 0.11
SideWallBm_Part_23.Color 1
SideWallBm_Part_23.Visibility 1
SideWallBm_Part_23.Mother SideWallBm_Volume


// (24)
Volume SideWallBm_Part_24
SideWallBm_Part_24.Material Aluminium
SideWallBm_Part_24.Shape BRIK 0.05 1.475 0.09
SideWallBm_Part_24.Position 8.875 1.575 0.11
SideWallBm_Part_24.Color 1
SideWallBm_Part_24.Visibility 1
SideWallBm_Part_24.Mother SideWallBm_Volume

// (25)
Volume SideWallBm_Part_25
SideWallBm_Part_25.Material Aluminium
SideWallBm_Part_25.Shape BRIK 0.05 1.475 0.09
SideWallBm_Part_25.Position 0.0 1.575 0.11
SideWallBm_Part_25.Color 1
SideWallBm_Part_25.Visibility 1
SideWallBm_Part_25.Mother SideWallBm_Volume

// (26)
Volume SideWallBm_Part_26
SideWallBm_Part_26.Material Aluminium
SideWallBm_Part_26.Shape BRIK 0.05 1.475 0.09
SideWallBm_Part_26.Position -9.1 1.575 0.11
SideWallBm_Part_26.Color 1
SideWallBm_Part_26.Visibility 1
SideWallBm_Part_26.Mother SideWallBm_Volume


// (27)
Volume SideWallBm_Part_27
SideWallBm_Part_27.Material Aluminium
SideWallBm_Part_27.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_27.Position 8.875 -1.55 0.11
SideWallBm_Part_27.Color 1
SideWallBm_Part_27.Visibility 1
SideWallBm_Part_27.Mother SideWallBm_Volume

// (28)
Volume SideWallBm_Part_28
SideWallBm_Part_28.Material Aluminium
SideWallBm_Part_28.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_28.Position 0.0 -1.55 0.11
SideWallBm_Part_28.Color 1
SideWallBm_Part_28.Visibility 1
SideWallBm_Part_28.Mother SideWallBm_Volume

// (29)
Volume SideWallBm_Part_29
SideWallBm_Part_29.Material Aluminium
SideWallBm_Part_29.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_29.Position -9.1 -1.55 0.11
SideWallBm_Part_29.Color 1
SideWallBm_Part_29.Visibility 1
SideWallBm_Part_29.Mother SideWallBm_Volume


// (30)
Volume SideWallBm_Part_30
SideWallBm_Part_30.Material Aluminium
SideWallBm_Part_30.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_30.Position 8.875 -4.55 0.11
SideWallBm_Part_30.Color 1
SideWallBm_Part_30.Visibility 1
SideWallBm_Part_30.Mother SideWallBm_Volume

// (31)
Volume SideWallBm_Part_31
SideWallBm_Part_31.Material Aluminium
SideWallBm_Part_31.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_31.Position 0.0 -4.55 0.11
SideWallBm_Part_31.Color 1
SideWallBm_Part_31.Visibility 1
SideWallBm_Part_31.Mother SideWallBm_Volume

// (32)
Volume SideWallBm_Part_32
SideWallBm_Part_32.Material Aluminium
SideWallBm_Part_32.Shape BRIK 0.05 1.45 0.09
SideWallBm_Part_32.Position -9.1 -4.55 0.11
SideWallBm_Part_32.Color 1
SideWallBm_Part_32.Visibility 1
SideWallBm_Part_32.Mother SideWallBm_Volume

// Broadenings for the borings:

// (33)
Volume SideWallBm_Part_33
SideWallBm_Part_33.Material Aluminium
SideWallBm_Part_33.Shape BRIK 0.35 0.15 0.44
SideWallBm_Part_33.Position 12.1 6.15 0.46
SideWallBm_Part_33.Color 1
SideWallBm_Part_33.Visibility 1
SideWallBm_Part_33.Mother SideWallBm_Volume

// (34)
Volume SideWallBm_Part_34
SideWallBm_Part_34.Material Aluminium
SideWallBm_Part_34.Shape BRIK 0.575 0.15 0.44
SideWallBm_Part_34.Position 3.875 6.15 0.46
SideWallBm_Part_34.Color 1
SideWallBm_Part_34.Visibility 1
SideWallBm_Part_34.Mother SideWallBm_Volume

// (35)
Volume SideWallBm_Part_35
SideWallBm_Part_35.Material Aluminium
SideWallBm_Part_35.Shape BRIK 0.575 0.15 0.44
SideWallBm_Part_35.Position -3.875 6.15 0.46
SideWallBm_Part_35.Color 1
SideWallBm_Part_35.Visibility 1
SideWallBm_Part_35.Mother SideWallBm_Volume

// (36)
Volume SideWallBm_Part_36
SideWallBm_Part_36.Material Aluminium
SideWallBm_Part_36.Shape BRIK 0.35 0.15 0.44
SideWallBm_Part_36.Position -12.55 6.15 0.46
SideWallBm_Part_36.Color 1
SideWallBm_Part_36.Visibility 1
SideWallBm_Part_36.Mother SideWallBm_Volume



// The borings:

// on left side:

// (37)
Volume SideWallBm_Part_37
SideWallBm_Part_37.Material Steel
SideWallBm_Part_37.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_37.Position 0.0 5.35 0.0
SideWallBm_Part_37.Color 7
SideWallBm_Part_37.Visibility 1
SideWallBm_Part_37.Mother SideWallBm_Part_6

// (38)
Volume SideWallBm_Part_38
SideWallBm_Part_38.Material Steel
SideWallBm_Part_38.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_38.Position 0.0 -5.35 0.0
SideWallBm_Part_38.Color 7
SideWallBm_Part_38.Visibility 1
SideWallBm_Part_38.Mother SideWallBm_Part_6

// (39)
Volume SideWallBm_Part_39
SideWallBm_Part_39.Material Steel
SideWallBm_Part_39.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_39.Position 0.0 1.35 0.0
SideWallBm_Part_39.Color 7
SideWallBm_Part_39.Visibility 1
SideWallBm_Part_39.Mother SideWallBm_Part_6

// (40)
Volume SideWallBm_Part_40
SideWallBm_Part_40.Material Steel
SideWallBm_Part_40.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_40.Position 0.0 -1.65 0.0
SideWallBm_Part_40.Color 7
SideWallBm_Part_40.Visibility 1
SideWallBm_Part_40.Mother SideWallBm_Part_6


// on the right side:

// (41)
Volume SideWallBm_Part_41
SideWallBm_Part_41.Material Steel
SideWallBm_Part_41.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_41.Position 0.0 5.35 0.0
SideWallBm_Part_41.Color 7
SideWallBm_Part_41.Visibility 1
SideWallBm_Part_41.Mother SideWallBm_Part_8

// (42)
Volume SideWallBm_Part_42
SideWallBm_Part_42.Material Steel
SideWallBm_Part_42.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_42.Position 0.0 -5.35 0.0
SideWallBm_Part_42.Color 7
SideWallBm_Part_42.Visibility 1
SideWallBm_Part_42.Mother SideWallBm_Part_8

// (43)
Volume SideWallBm_Part_43
SideWallBm_Part_43.Material Steel
SideWallBm_Part_43.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_43.Position 0.0 1.35 0.0
SideWallBm_Part_43.Color 7
SideWallBm_Part_43.Visibility 1
SideWallBm_Part_43.Mother SideWallBm_Part_8

// (44)
Volume SideWallBm_Part_44
SideWallBm_Part_44.Material Steel
SideWallBm_Part_44.Shape TUBS 0.0 0.15 0.09 0.0 360.0
SideWallBm_Part_44.Position 0.0 -1.65 0.0
SideWallBm_Part_44.Color 7
SideWallBm_Part_44.Visibility 1
SideWallBm_Part_44.Mother SideWallBm_Part_8


// borings in the "North Wall":

// (45)
Volume SideWallBm_Part_45
SideWallBm_Part_45.Material Steel
SideWallBm_Part_45.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallBm_Part_45.Rotation 90.0 0.0 0.0
SideWallBm_Part_45.Position -12.325 0.0 0.04
SideWallBm_Part_45.Color 7
SideWallBm_Part_45.Visibility 1
SideWallBm_Part_45.Mother SideWallBm_UpperWall_2

// (46)
Volume SideWallBm_Part_46
SideWallBm_Part_46.Material Steel
SideWallBm_Part_46.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallBm_Part_46.Rotation 90.0 0.0 0.0
SideWallBm_Part_46.Position 12.325 0.0 0.04
SideWallBm_Part_46.Color 7
SideWallBm_Part_46.Visibility 1
SideWallBm_Part_46.Mother SideWallBm_UpperWall_2

// (47)
Volume SideWallBm_Part_47
SideWallBm_Part_47.Material Steel
SideWallBm_Part_47.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallBm_Part_47.Rotation 90.0 0.0 0.0
SideWallBm_Part_47.Position 3.875 0.0 0.04
SideWallBm_Part_47.Color 7
SideWallBm_Part_47.Visibility 1
SideWallBm_Part_47.Mother SideWallBm_UpperWall_2

// (48)
Volume SideWallBm_Part_48
SideWallBm_Part_48.Material Steel
SideWallBm_Part_48.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallBm_Part_48.Rotation 90.0 0.0 0.0
SideWallBm_Part_48.Position -3.425 0.0 0.04
SideWallBm_Part_48.Color 7
SideWallBm_Part_48.Visibility 1
SideWallBm_Part_48.Mother SideWallBm_UpperWall_2

// In the broadenings:

// (49)
Volume SideWallBm_Part_49
SideWallBm_Part_49.Material Steel
SideWallBm_Part_49.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallBm_Part_49.Rotation 90.0 0.0 0.0
SideWallBm_Part_49.Position 0.0 0.0 0.04
SideWallBm_Part_49.Color 7
SideWallBm_Part_49.Visibility 1
SideWallBm_Part_49.Mother SideWallBm_Part_33

// (50)
Volume SideWallBm_Part_50
SideWallBm_Part_50.Material Steel
SideWallBm_Part_50.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallBm_Part_50.Rotation 90.0 0.0 0.0
SideWallBm_Part_50.Position 0.0 0.0 0.04
SideWallBm_Part_50.Color 7
SideWallBm_Part_50.Visibility 1
SideWallBm_Part_50.Mother SideWallBm_Part_36

// (51)
Volume SideWallBm_Part_51
SideWallBm_Part_51.Material Steel
SideWallBm_Part_51.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallBm_Part_51.Rotation 90.0 0.0 0.0
SideWallBm_Part_51.Position -0.225 0.0 0.04
SideWallBm_Part_51.Color 7
SideWallBm_Part_51.Visibility 1
SideWallBm_Part_51.Mother SideWallBm_Part_34

// (52)
Volume SideWallBm_Part_52
SideWallBm_Part_52.Material Steel
SideWallBm_Part_52.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallBm_Part_52.Rotation 90.0 0.0 0.0
SideWallBm_Part_52.Position 0.225 0.0 0.04
SideWallBm_Part_52.Color 7
SideWallBm_Part_52.Visibility 1
SideWallBm_Part_52.Mother SideWallBm_Part_35
