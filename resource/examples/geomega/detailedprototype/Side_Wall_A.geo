// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// The Mother Volume of the Detectot Configuration:
Volume SideWallA_Volume
SideWallA_Volume.Material Air
SideWallA_Volume.Visibility 0
SideWallA_Volume.Virtual true
SideWallA_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// SideWallA_Volume.Mother 0



// Upper Side Wall (1)
Volume SideWallA_UpperWall_1
SideWallA_UpperWall_1.Material Aluminium
SideWallA_UpperWall_1.Shape BRIK 13.175 0.1 0.44
SideWallA_UpperWall_1.Position -0.575 6.4 0.46
SideWallA_UpperWall_1.Color 1
SideWallA_UpperWall_1.Visibility 1
SideWallA_UpperWall_1.Mother SideWallA_Volume

// Left Side Wall (2)
Volume SideWallA_LeftWall_2
SideWallA_LeftWall_2.Material Aluminium
SideWallA_LeftWall_2.Shape BRIK 0.1 6.15 0.44
SideWallA_LeftWall_2.Position -13.65 0.15 0.46
SideWallA_LeftWall_2.Color 1
SideWallA_LeftWall_2.Visibility 1
SideWallA_LeftWall_2.Mother SideWallA_Volume

// Right Side Wall (3)
Volume SideWallA_RightWall_3
SideWallA_RightWall_3.Material Aluminium
SideWallA_RightWall_3.Shape BRIK 0.1 6.15 0.44
SideWallA_RightWall_3.Position 12.5 0.15 0.46
SideWallA_RightWall_3.Color 1
SideWallA_RightWall_3.Visibility 1
SideWallA_RightWall_3.Mother SideWallA_Volume

// Lower Side Wall (4)
Volume SideWallA_LowerWall_4
SideWallA_LowerWall_4.Material Aluminium
SideWallA_LowerWall_4.Shape BRIK 13.175 0.1 0.44
SideWallA_LowerWall_4.Position -0.575 -6.1 0.46
SideWallA_LowerWall_4.Color 1
SideWallA_LowerWall_4.Visibility 1
SideWallA_LowerWall_4.Mother SideWallA_Volume


// Thin Wall (5)
Volume SideWallA_ThinWall_5
SideWallA_ThinWall_5.Material Aluminium
SideWallA_ThinWall_5.Shape BRIK 13.175 6.35 0.01
SideWallA_ThinWall_5.Position -0.575 0.15 0.01
SideWallA_ThinWall_5.Color 1
SideWallA_ThinWall_5.Visibility 1
SideWallA_ThinWall_5.Mother SideWallA_Volume


// (6):
Volume SideWallA_Part_6
SideWallA_Part_6.Material Aluminium
SideWallA_Part_6.Shape BRIK 0.6 0.15 0.44
SideWallA_Part_6.Position -12.95 6.15 0.46
SideWallA_Part_6.Color 1
SideWallA_Part_6.Visibility 1
SideWallA_Part_6.Mother SideWallA_Volume

// (7):
Volume SideWallA_Part_7
SideWallA_Part_7.Material Aluminium
SideWallA_Part_7.Shape BRIK 0.15 0.375 0.44
SideWallA_Part_7.Position -13.4 5.625 0.46
SideWallA_Part_7.Color 1
SideWallA_Part_7.Visibility 1
SideWallA_Part_7.Mother SideWallA_Volume

// (8):
Volume SideWallA_Part_8
SideWallA_Part_8.Material Aluminium
SideWallA_Part_8.Shape BRIK 0.75 0.15 0.44
SideWallA_Part_8.Position 11.65 6.15 0.46
SideWallA_Part_8.Color 1
SideWallA_Part_8.Visibility 1
SideWallA_Part_8.Mother SideWallA_Volume

// (9):
Volume SideWallA_Part_9
SideWallA_Part_9.Material Aluminium
SideWallA_Part_9.Shape BRIK 0.15 0.375 0.44
SideWallA_Part_9.Position 12.25 5.625 0.46
SideWallA_Part_9.Color 1
SideWallA_Part_9.Visibility 1
SideWallA_Part_9.Mother SideWallA_Volume



// (10):
Volume SideWallA_Part_10
SideWallA_Part_10.Material Aluminium
SideWallA_Part_10.Shape BRIK 0.5 0.15 0.44
SideWallA_Part_10.Position 4.0 6.15 0.46
SideWallA_Part_10.Color 1
SideWallA_Part_10.Visibility 1
SideWallA_Part_10.Mother SideWallA_Volume

// (11):
Volume SideWallA_Part_11
SideWallA_Part_11.Material Aluminium
SideWallA_Part_11.Shape BRIK 0.5 0.15 0.44
SideWallA_Part_11.Position -4.0 6.15 0.46
SideWallA_Part_11.Color 1
SideWallA_Part_11.Visibility 1
SideWallA_Part_11.Mother SideWallA_Volume

// (12):
Volume SideWallA_Part_12
SideWallA_Part_12.Material Aluminium
SideWallA_Part_12.Shape BRIK 0.1 6.15 0.44
SideWallA_Part_12.Position -4.6 0.15 0.46
SideWallA_Part_12.Color 1
SideWallA_Part_12.Visibility 1
SideWallA_Part_12.Mother SideWallA_Volume

// (13):
Volume SideWallA_Part_13
SideWallA_Part_13.Material Aluminium
SideWallA_Part_13.Shape BRIK 0.1 6.15 0.44
SideWallA_Part_13.Position 4.6 0.15 0.46
SideWallA_Part_13.Color 1
SideWallA_Part_13.Visibility 1
SideWallA_Part_13.Mother SideWallA_Volume

// (14):
Volume SideWallA_Part_14
SideWallA_Part_14.Material Aluminium
SideWallA_Part_14.Shape BRIK 4.425 0.1 0.44
SideWallA_Part_14.Position -9.125 0.0 0.46
SideWallA_Part_14.Color 1
SideWallA_Part_14.Visibility 1
SideWallA_Part_14.Mother SideWallA_Volume

// (15):
Volume SideWallA_Part_15
SideWallA_Part_15.Material Aluminium
SideWallA_Part_15.Shape BRIK 4.5 0.1 0.44
SideWallA_Part_15.Position 0.0 0.0 0.46
SideWallA_Part_15.Color 1
SideWallA_Part_15.Visibility 1
SideWallA_Part_15.Mother SideWallA_Volume

// (16):
Volume SideWallA_Part_16
SideWallA_Part_16.Material Aluminium
SideWallA_Part_16.Shape BRIK 3.85 0.1 0.44
SideWallA_Part_16.Position 8.55 0.0 0.46
SideWallA_Part_16.Color 1
SideWallA_Part_16.Visibility 1
SideWallA_Part_16.Mother SideWallA_Volume


// (17):
Volume SideWallA_Part_17
SideWallA_Part_17.Material Aluminium
SideWallA_Part_17.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_17.Position -13.4 1.5 0.46
SideWallA_Part_17.Color 1
SideWallA_Part_17.Visibility 1
SideWallA_Part_17.Mother SideWallA_Volume

// (18):
Volume SideWallA_Part_18
SideWallA_Part_18.Material Aluminium
SideWallA_Part_18.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_18.Position -13.4 -1.5 0.46
SideWallA_Part_18.Color 1
SideWallA_Part_18.Visibility 1
SideWallA_Part_18.Mother SideWallA_Volume

// (19):
Volume SideWallA_Part_19
SideWallA_Part_19.Material Aluminium
SideWallA_Part_19.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_19.Position -13.4 -5.2 0.46
SideWallA_Part_19.Color 1
SideWallA_Part_19.Visibility 1
SideWallA_Part_19.Mother SideWallA_Volume


// (20):
Volume SideWallA_Part_20
SideWallA_Part_20.Material Aluminium
SideWallA_Part_20.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_20.Position 12.25 1.5 0.46
SideWallA_Part_20.Color 1
SideWallA_Part_20.Visibility 1
SideWallA_Part_20.Mother SideWallA_Volume

// (21):
Volume SideWallA_Part_21
SideWallA_Part_21.Material Aluminium
SideWallA_Part_21.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_21.Position 12.25 -1.5 0.46
SideWallA_Part_21.Color 1
SideWallA_Part_21.Visibility 1
SideWallA_Part_21.Mother SideWallA_Volume

// (22):
Volume SideWallA_Part_22
SideWallA_Part_22.Material Aluminium
SideWallA_Part_22.Shape BRIK 0.15 0.25 0.44
SideWallA_Part_22.Position 12.25 -5.2 0.46
SideWallA_Part_22.Color 1
SideWallA_Part_22.Visibility 1
SideWallA_Part_22.Mother SideWallA_Volume



// (23):
Volume SideWallA_Part_23
SideWallA_Part_23.Material Aluminium
SideWallA_Part_23.Shape BRIK 0.05 3.1 0.0625
SideWallA_Part_23.Position -8.975 3.2 0.0825
SideWallA_Part_23.Color 1
SideWallA_Part_23.Visibility 1
SideWallA_Part_23.Mother SideWallA_Volume

// (24):
Volume SideWallA_Part_24
SideWallA_Part_24.Material Aluminium
SideWallA_Part_24.Shape BRIK 2.2625 0.05 0.0625
SideWallA_Part_24.Position -11.2875 3.1 0.0825
SideWallA_Part_24.Color 1
SideWallA_Part_24.Visibility 1
SideWallA_Part_24.Mother SideWallA_Volume

// (25):
Volume SideWallA_Part_25
SideWallA_Part_25.Material Aluminium
SideWallA_Part_25.Shape BRIK 2.1125 0.05 0.0625
SideWallA_Part_25.Position -6.8125 3.1 0.0825
SideWallA_Part_25.Color 1
SideWallA_Part_25.Visibility 1
SideWallA_Part_25.Mother SideWallA_Volume

// (26):
Volume SideWallA_Part_26
SideWallA_Part_26.Material Aluminium
SideWallA_Part_26.Shape BRIK 0.05 2.95 0.0625
SideWallA_Part_26.Position -8.975 -3.05 0.0825
SideWallA_Part_26.Color 1
SideWallA_Part_26.Visibility 1
SideWallA_Part_26.Mother SideWallA_Volume

// (27):
Volume SideWallA_Part_27
SideWallA_Part_27.Material Aluminium
SideWallA_Part_27.Shape BRIK 2.2625 0.05 0.0625
SideWallA_Part_27.Position -11.2875 -3.05 0.0825
SideWallA_Part_27.Color 1
SideWallA_Part_27.Visibility 1
SideWallA_Part_27.Mother SideWallA_Volume

// (28):
Volume SideWallA_Part_28
SideWallA_Part_28.Material Aluminium
SideWallA_Part_28.Shape BRIK 2.1125 0.05 0.0625
SideWallA_Part_28.Position -6.8125 -3.05 0.0825
SideWallA_Part_28.Color 1
SideWallA_Part_28.Visibility 1
SideWallA_Part_28.Mother SideWallA_Volume



// (29):
Volume SideWallA_Part_29
SideWallA_Part_29.Material Aluminium
SideWallA_Part_29.Shape BRIK 2.225 0.05 0.0625
SideWallA_Part_29.Position -2.275 3.1 0.0825
SideWallA_Part_29.Color 1
SideWallA_Part_29.Visibility 1
SideWallA_Part_29.Mother SideWallA_Volume

// (30):
Volume SideWallA_Part_30
SideWallA_Part_30.Material Aluminium
SideWallA_Part_30.Shape BRIK 2.225 0.05 0.0625
SideWallA_Part_30.Position 2.275 3.1 0.0825
SideWallA_Part_30.Color 1
SideWallA_Part_30.Visibility 1
SideWallA_Part_30.Mother SideWallA_Volume

// (31):
Volume SideWallA_Part_31
SideWallA_Part_31.Material Aluminium
SideWallA_Part_31.Shape BRIK 2.225 0.05 0.0625
SideWallA_Part_31.Position -2.275 -3.05 0.0825
SideWallA_Part_31.Color 1
SideWallA_Part_31.Visibility 1
SideWallA_Part_31.Mother SideWallA_Volume

// (32):
Volume SideWallA_Part_32
SideWallA_Part_32.Material Aluminium
SideWallA_Part_32.Shape BRIK 2.225 0.05 0.0625
SideWallA_Part_32.Position 2.275 -3.05 0.0825
SideWallA_Part_32.Color 1
SideWallA_Part_32.Visibility 1
SideWallA_Part_32.Mother SideWallA_Volume

// (33):
Volume SideWallA_Part_33
SideWallA_Part_33.Material Aluminium
SideWallA_Part_33.Shape BRIK 0.05 3.1 0.0625
SideWallA_Part_33.Position 0.0 3.2 0.0825
SideWallA_Part_33.Color 1
SideWallA_Part_33.Visibility 1
SideWallA_Part_33.Mother SideWallA_Volume

// (34):
Volume SideWallA_Part_34
SideWallA_Part_34.Material Aluminium
SideWallA_Part_34.Shape BRIK 0.05 2.95 0.0625
SideWallA_Part_34.Position 0.0 -3.05 0.0825
SideWallA_Part_34.Color 1
SideWallA_Part_34.Visibility 1
SideWallA_Part_34.Mother SideWallA_Volume

// (35):
Volume SideWallA_Part_35
SideWallA_Part_35.Material Aluminium
SideWallA_Part_35.Shape BRIK 0.05 3.1 0.0625
SideWallA_Part_35.Position 8.4 3.2 0.0825
SideWallA_Part_35.Color 1
SideWallA_Part_35.Visibility 1
SideWallA_Part_35.Mother SideWallA_Volume

// (36):
Volume SideWallA_Part_36
SideWallA_Part_36.Material Aluminium
SideWallA_Part_36.Shape BRIK 0.05 2.95 0.0625
SideWallA_Part_36.Position 8.4 -3.05 0.0825
SideWallA_Part_36.Color 1
SideWallA_Part_36.Visibility 1
SideWallA_Part_36.Mother SideWallA_Volume


// (37):
Volume SideWallA_Part_37
SideWallA_Part_37.Material Aluminium
SideWallA_Part_37.Shape BRIK 1.825 0.05 0.0625
SideWallA_Part_37.Position 6.525 3.1 0.0825
SideWallA_Part_37.Color 1
SideWallA_Part_37.Visibility 1
SideWallA_Part_37.Mother SideWallA_Volume

// (38):
Volume SideWallA_Part_38
SideWallA_Part_38.Material Aluminium
SideWallA_Part_38.Shape BRIK 1.825 0.05 0.0625
SideWallA_Part_38.Position 6.525 -3.05 0.0825
SideWallA_Part_38.Color 1
SideWallA_Part_38.Visibility 1
SideWallA_Part_38.Mother SideWallA_Volume

// (39):
Volume SideWallA_Part_39
SideWallA_Part_39.Material Aluminium
SideWallA_Part_39.Shape BRIK 1.975 0.05 0.0625
SideWallA_Part_39.Position 10.425 3.1 0.0825
SideWallA_Part_39.Color 1
SideWallA_Part_39.Visibility 1
SideWallA_Part_39.Mother SideWallA_Volume

// (40):
Volume SideWallA_Part_40
SideWallA_Part_40.Material Aluminium
SideWallA_Part_40.Shape BRIK 1.975 0.05 0.0625
SideWallA_Part_40.Position 10.425 -3.05 0.0825
SideWallA_Part_40.Color 1
SideWallA_Part_40.Visibility 1
SideWallA_Part_40.Mother SideWallA_Volume


// borings:

// (41):
Volume SideWallA_Part_41
SideWallA_Part_41.Material Steel
SideWallA_Part_41.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_41.Rotation 90. 0. 0.
SideWallA_Part_41.Position -12.075 0.0 0.04
SideWallA_Part_41.Color 7
SideWallA_Part_41.Visibility 1
SideWallA_Part_41.Mother SideWallA_UpperWall_1

// (42):
Volume SideWallA_Part_42
SideWallA_Part_42.Material Steel
SideWallA_Part_42.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_42.Rotation 90. 0. 0.
SideWallA_Part_42.Position 0.3 0.0 0.04
SideWallA_Part_42.Color 7
SideWallA_Part_42.Visibility 1
SideWallA_Part_42.Mother SideWallA_Part_6

// (43):
Volume SideWallA_Part_43
SideWallA_Part_43.Material Steel
SideWallA_Part_43.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_43.Rotation 90. 0. 0.
SideWallA_Part_43.Position -3.225 0.0 0.04
SideWallA_Part_43.Color 7
SideWallA_Part_43.Visibility 1
SideWallA_Part_43.Mother SideWallA_UpperWall_1

// (44):
Volume SideWallA_Part_44
SideWallA_Part_44.Material Steel
SideWallA_Part_44.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_44.Rotation 90. 0. 0.
SideWallA_Part_44.Position -0.2 0.0 0.04
SideWallA_Part_44.Color 7
SideWallA_Part_44.Visibility 1
SideWallA_Part_44.Mother SideWallA_Part_10

// (45):
Volume SideWallA_Part_45
SideWallA_Part_45.Material Steel
SideWallA_Part_45.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_45.Rotation 90. 0. 0.
SideWallA_Part_45.Position 4.375 0.0 0.04
SideWallA_Part_45.Color 7
SideWallA_Part_45.Visibility 1
SideWallA_Part_45.Mother SideWallA_UpperWall_1

// (46):
Volume SideWallA_Part_46
SideWallA_Part_46.Material Steel
SideWallA_Part_46.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_46.Rotation 90. 0. 0.
SideWallA_Part_46.Position 0.2 0.0 0.04
SideWallA_Part_46.Color 7
SideWallA_Part_46.Visibility 1
SideWallA_Part_46.Mother SideWallA_Part_11

// (47):
Volume SideWallA_Part_47
SideWallA_Part_47.Material Steel
SideWallA_Part_47.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_47.Rotation 90. 0. 0.
SideWallA_Part_47.Position 11.775 0.0 0.04
SideWallA_Part_47.Color 7
SideWallA_Part_47.Visibility 1
SideWallA_Part_47.Mother SideWallA_UpperWall_1

// (48):
Volume SideWallA_Part_48
SideWallA_Part_48.Material Steel
SideWallA_Part_48.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_48.Rotation 90. 0. 0.
SideWallA_Part_48.Position -0.45 0.0 0.04
SideWallA_Part_48.Color 7
SideWallA_Part_48.Visibility 1
SideWallA_Part_48.Mother SideWallA_Part_8


// (49):
Volume SideWallA_Part_49
SideWallA_Part_49.Material Steel
SideWallA_Part_49.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_49.Rotation 90. 0. 90.
SideWallA_Part_49.Position 0.0 5.35 -0.11	// z = 0.04
SideWallA_Part_49.Color 7
SideWallA_Part_49.Visibility 1
SideWallA_Part_49.Mother SideWallA_LeftWall_2

// (50):
Volume SideWallA_Part_50
SideWallA_Part_50.Material Steel
SideWallA_Part_50.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_50.Rotation 90. 0. 90.
SideWallA_Part_50.Position 0.0 -0.125 -0.11	// 0.04
SideWallA_Part_50.Color 7
SideWallA_Part_50.Visibility 1
SideWallA_Part_50.Mother SideWallA_Part_7

// (51):
Volume SideWallA_Part_51
SideWallA_Part_51.Material Steel
SideWallA_Part_51.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_51.Rotation 90. 0. 90.
SideWallA_Part_51.Position 0.0 1.35 -0.11	//0.04
SideWallA_Part_51.Color 7
SideWallA_Part_51.Visibility 1
SideWallA_Part_51.Mother SideWallA_LeftWall_2

// (52):
Volume SideWallA_Part_52
SideWallA_Part_52.Material Steel
SideWallA_Part_52.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_52.Rotation 90. 0. 90.
SideWallA_Part_52.Position 0.0 0.0 -0.11	//0.04
SideWallA_Part_52.Color 7
SideWallA_Part_52.Visibility 1
SideWallA_Part_52.Mother SideWallA_Part_17

// (53):
Volume SideWallA_Part_53
SideWallA_Part_53.Material Steel
SideWallA_Part_53.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_53.Rotation 90. 0. 90.
SideWallA_Part_53.Position 0.0 -1.65 -0.11	//0.04
SideWallA_Part_53.Color 7
SideWallA_Part_53.Visibility 1
SideWallA_Part_53.Mother SideWallA_LeftWall_2

// (54):
Volume SideWallA_Part_54
SideWallA_Part_54.Material Steel
SideWallA_Part_54.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_54.Rotation 90. 0. 90.
SideWallA_Part_54.Position 0.0 0.0 -0.11	//0.04
SideWallA_Part_54.Color 7
SideWallA_Part_54.Visibility 1
SideWallA_Part_54.Mother SideWallA_Part_18

// (55):
Volume SideWallA_Part_55
SideWallA_Part_55.Material Steel
SideWallA_Part_55.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_55.Rotation 90. 0. 90.
SideWallA_Part_55.Position 0.0 -5.35 -0.11	//0.04
SideWallA_Part_55.Color 7
SideWallA_Part_55.Visibility 1
SideWallA_Part_55.Mother SideWallA_LeftWall_2

// (56):
Volume SideWallA_Part_56
SideWallA_Part_56.Material Steel
SideWallA_Part_56.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_56.Rotation 90. 0. 90.
SideWallA_Part_56.Position 0.0 0.0 -0.11	//0.04
SideWallA_Part_56.Color 7
SideWallA_Part_56.Visibility 1
SideWallA_Part_56.Mother SideWallA_Part_19


// (57):
Volume SideWallA_Part_57
SideWallA_Part_57.Material Steel
SideWallA_Part_57.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_57.Rotation 90. 0. 90.
SideWallA_Part_57.Position 0.0 5.35 -0.11  // z = 0.04
SideWallA_Part_57.Color 7
SideWallA_Part_57.Visibility 1
SideWallA_Part_57.Mother SideWallA_RightWall_3

// (58):
Volume SideWallA_Part_58
SideWallA_Part_58.Material Steel
SideWallA_Part_58.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_58.Rotation 90. 0. 90.
SideWallA_Part_58.Position 0.0 -0.125 -0.11  // 0.04
SideWallA_Part_58.Color 7
SideWallA_Part_58.Visibility 1
SideWallA_Part_58.Mother SideWallA_Part_9

// (59):
Volume SideWallA_Part_59
SideWallA_Part_59.Material Steel
SideWallA_Part_59.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_59.Rotation 90. 0. 90.
SideWallA_Part_59.Position 0.0 1.35 -0.11  	//0.04
SideWallA_Part_59.Color 7
SideWallA_Part_59.Visibility 1
SideWallA_Part_59.Mother SideWallA_RightWall_3

// (60):
Volume SideWallA_Part_60
SideWallA_Part_60.Material Steel
SideWallA_Part_60.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_60.Rotation 90. 0. 90.
SideWallA_Part_60.Position 0.0 0.0 -0.11 	//0.04
SideWallA_Part_60.Color 7
SideWallA_Part_60.Visibility 1
SideWallA_Part_60.Mother SideWallA_Part_20

// (61):
Volume SideWallA_Part_61
SideWallA_Part_61.Material Steel
SideWallA_Part_61.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_61.Rotation 90. 0. 90.
SideWallA_Part_61.Position 0.0 -1.65 -0.11 	//0.04
SideWallA_Part_61.Color 7
SideWallA_Part_61.Visibility 1
SideWallA_Part_61.Mother SideWallA_RightWall_3

// (62):
Volume SideWallA_Part_62
SideWallA_Part_62.Material Steel
SideWallA_Part_62.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_62.Rotation 90. 0. 90.
SideWallA_Part_62.Position 0.0 0.0 -0.11	//0.04
SideWallA_Part_62.Color 7
SideWallA_Part_62.Visibility 1
SideWallA_Part_62.Mother SideWallA_Part_21

// (63):
Volume SideWallA_Part_63
SideWallA_Part_63.Material Steel
SideWallA_Part_63.Shape TUBS 0.0 0.15 0.1 0.0 360.0
SideWallA_Part_63.Rotation 90. 0. 90.
SideWallA_Part_63.Position 0.0 -5.35 -0.11	//0.04
SideWallA_Part_63.Color 7
SideWallA_Part_63.Visibility 1
SideWallA_Part_63.Mother SideWallA_RightWall_3

// (64):
Volume SideWallA_Part_64
SideWallA_Part_64.Material Steel
SideWallA_Part_64.Shape TUBS 0.0 0.15 0.15 0.0 360.0
SideWallA_Part_64.Rotation 90. 0. 90.
SideWallA_Part_64.Position 0.0 0.0 -0.11 	//0.04
SideWallA_Part_64.Color 7
SideWallA_Part_64.Visibility 1
SideWallA_Part_64.Mother SideWallA_Part_22

