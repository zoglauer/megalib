// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// The Mother Volume of the Detector Configuration:
Volume GreenWall_Volume
GreenWall_Volume.Material Air
GreenWall_Volume.Visibility 0
GreenWall_Volume.Virtual true
GreenWall_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// GreenWall_Volume.Mother 0



// One big block from which parts are cut away later (1)
// The position of the block was chosen to make the origin
// coincides with the middle indicated by the blue lines in
// the blueprints of the detector setup:
Volume GreenWall_BigBlock_1
GreenWall_BigBlock_1.Material Aluminium
GreenWall_BigBlock_1.Shape BRIK 14.325 13.975 0.55
GreenWall_BigBlock_1.Position 1.775 -2.125 0.0
GreenWall_BigBlock_1.Color 8
GreenWall_BigBlock_1.Visibility 1
GreenWall_BigBlock_1.Mother GreenWall_Volume



// Four missing parts in the corners:

// (2)
Volume GreenWall_Part_2
GreenWall_Part_2.Material Aluminium
GreenWall_Part_2.Shape BRIK 0.875 1.25 0.55
GreenWall_Part_2.Position -13.425 10.6 0.0
GreenWall_Part_2.Color 8
GreenWall_Part_2.Visibility 1
GreenWall_Part_2.Mother GreenWall_Volume

// (3)
Volume GreenWall_Part_3
GreenWall_Part_3.Material Aluminium
GreenWall_Part_3.Shape BRIK 1.25 0.875 0.55
GreenWall_Part_3.Position 14.85 12.725 0.0
GreenWall_Part_3.Color 8
GreenWall_Part_3.Visibility 1
GreenWall_Part_3.Mother GreenWall_Volume

// (4)
Volume GreenWall_Part_4
GreenWall_Part_4.Material Aluminium
GreenWall_Part_4.Shape BRIK 0.875 1.25 0.55
GreenWall_Part_4.Position -13.425 -14.85 0.0
GreenWall_Part_4.Color 8
GreenWall_Part_4.Visibility 1
GreenWall_Part_4.Mother GreenWall_Volume

// (5)
Volume GreenWall_Part_5
GreenWall_Part_5.Material Aluminium
GreenWall_Part_5.Shape BRIK 1.25 0.875 0.55
GreenWall_Part_5.Position 14.85 -16.975 0.0
GreenWall_Part_5.Color 8
GreenWall_Part_5.Visibility 1
GreenWall_Part_5.Mother GreenWall_Volume



// Now things get cut away:
// All Coordinates are relative to GreenWall_BigBlock_1 !

// A large but thin area on bottom:
// (6)
Volume GreenWall_Part_6
GreenWall_Part_6.Material Air
GreenWall_Part_6.Shape BRIK 12.325 11.95 0.2
GreenWall_Part_6.Position -1.3 1.325 -0.35
GreenWall_Part_6.Color 8
GreenWall_Part_6.Visibility 1
GreenWall_Part_6.Mother GreenWall_BigBlock_1


// the big holes:
// (7)
Volume GreenWall_Part_7
GreenWall_Part_7.Material Air
GreenWall_Part_7.Shape BRIK 3.75 3.5 0.34
GreenWall_Part_7.Position -7.725 7.975 0.19
GreenWall_Part_7.Color 8
GreenWall_Part_7.Visibility 1
GreenWall_Part_7.Mother GreenWall_BigBlock_1

// (8)
Volume GreenWall_Part_8
GreenWall_Part_8.Material Air
GreenWall_Part_8.Shape BRIK 3.55 3.5 0.34
GreenWall_Part_8.Position -0.225 7.975 0.19
GreenWall_Part_8.Color 8
GreenWall_Part_8.Visibility 1
GreenWall_Part_8.Mother GreenWall_BigBlock_1

// (9)
Volume GreenWall_Part_9
GreenWall_Part_9.Material Air
GreenWall_Part_9.Shape BRIK 3.75 3.5 0.34
GreenWall_Part_9.Position 7.275 7.975 0.19
GreenWall_Part_9.Color 8
GreenWall_Part_9.Visibility 1
GreenWall_Part_9.Mother GreenWall_BigBlock_1

// (10)
Volume GreenWall_Part_10
GreenWall_Part_10.Material Air
GreenWall_Part_10.Shape BRIK 3.75 3.7 0.34
GreenWall_Part_10.Position -7.725 0.575 0.19
GreenWall_Part_10.Color 8
GreenWall_Part_10.Visibility 1
GreenWall_Part_10.Mother GreenWall_BigBlock_1

// (11)
Volume GreenWall_Part_11
GreenWall_Part_11.Material Air
GreenWall_Part_11.Shape BRIK 3.55 3.7 0.34
GreenWall_Part_11.Position -0.225 0.575 0.19
GreenWall_Part_11.Color 8
GreenWall_Part_11.Visibility 1
GreenWall_Part_11.Mother GreenWall_BigBlock_1

// (12)
Volume GreenWall_Part_12
GreenWall_Part_12.Material Air
GreenWall_Part_12.Shape BRIK 3.75 3.7 0.34
GreenWall_Part_12.Position 7.275 0.575 0.19
GreenWall_Part_12.Color 8
GreenWall_Part_12.Visibility 1
GreenWall_Part_12.Mother GreenWall_BigBlock_1

// (13)
Volume GreenWall_Part_13
GreenWall_Part_13.Material Air
GreenWall_Part_13.Shape BRIK 3.75 3.65 0.34
GreenWall_Part_13.Position -7.725 -6.975 0.19
GreenWall_Part_13.Color 8
GreenWall_Part_13.Visibility 1
GreenWall_Part_13.Mother GreenWall_BigBlock_1

// (14)
Volume GreenWall_Part_14
GreenWall_Part_14.Material Air
GreenWall_Part_14.Shape BRIK 3.55 3.65 0.34
GreenWall_Part_14.Position -0.225 -6.975 0.19
GreenWall_Part_14.Color 8
GreenWall_Part_14.Visibility 1
GreenWall_Part_14.Mother GreenWall_BigBlock_1

// (15)
Volume GreenWall_Part_15
GreenWall_Part_15.Material Air
GreenWall_Part_15.Shape BRIK 3.75 3.65 0.34
GreenWall_Part_15.Position 7.275 -6.975 0.19
GreenWall_Part_15.Color 8
GreenWall_Part_15.Visibility 1
GreenWall_Part_15.Mother GreenWall_BigBlock_1


// (16)
Volume GreenWall_Part_16
GreenWall_Part_16.Material Air
GreenWall_Part_16.Shape BRIK 3.75 0.8 0.34
GreenWall_Part_16.Position -7.725 12.475 0.19
GreenWall_Part_16.Color 8
GreenWall_Part_16.Visibility 1
GreenWall_Part_16.Mother GreenWall_BigBlock_1

// (17)
Volume GreenWall_Part_17
GreenWall_Part_17.Material Air
GreenWall_Part_17.Shape BRIK 3.55 0.8 0.34
GreenWall_Part_17.Position -0.225 12.475 0.19
GreenWall_Part_17.Color 8
GreenWall_Part_17.Visibility 1
GreenWall_Part_17.Mother GreenWall_BigBlock_1

// (18)
Volume GreenWall_Part_18
GreenWall_Part_18.Material Air
GreenWall_Part_18.Shape BRIK 3.75 0.8 0.34
GreenWall_Part_18.Position 7.275 12.475 0.19
GreenWall_Part_18.Color 8
GreenWall_Part_18.Visibility 1
GreenWall_Part_18.Mother GreenWall_BigBlock_1



// (19)
Volume GreenWall_Part_19
GreenWall_Part_19.Material Air
GreenWall_Part_19.Shape BRIK 0.975 3.5 0.34
GreenWall_Part_19.Position -12.65 7.975 0.19
GreenWall_Part_19.Color 8
GreenWall_Part_19.Visibility 1
GreenWall_Part_19.Mother GreenWall_BigBlock_1

// (20)
Volume GreenWall_Part_20
GreenWall_Part_20.Material Air
GreenWall_Part_20.Shape BRIK 0.975 3.7 0.34
GreenWall_Part_20.Position -12.65 0.575 0.19
GreenWall_Part_20.Color 8
GreenWall_Part_20.Visibility 1
GreenWall_Part_20.Mother GreenWall_BigBlock_1

// (21)
Volume GreenWall_Part_21
GreenWall_Part_21.Material Air
GreenWall_Part_21.Shape BRIK 0.975 3.65 0.34
GreenWall_Part_21.Position -12.65 -6.975 0.19
GreenWall_Part_21.Color 8
GreenWall_Part_21.Visibility 1
GreenWall_Part_21.Mother GreenWall_BigBlock_1



// (22)
Volume GreenWall_Part_22
GreenWall_Part_22.Material Air
GreenWall_Part_22.Shape BRIK 1.0 0.625 0.34
GreenWall_Part_22.Position 12.625 12.65 0.19
GreenWall_Part_22.Color 8
GreenWall_Part_22.Visibility 1
GreenWall_Part_22.Mother GreenWall_BigBlock_1

// (23)
Volume GreenWall_Part_23
GreenWall_Part_23.Material Air
GreenWall_Part_23.Shape BRIK 0.775 1.025 0.34
GreenWall_Part_23.Position -12.85 -12.25 0.19
GreenWall_Part_23.Color 8
GreenWall_Part_23.Visibility 1
GreenWall_Part_23.Mother GreenWall_BigBlock_1



// (24)
Volume GreenWall_Part_24
GreenWall_Part_24.Material Air
GreenWall_Part_24.Shape BRIK 3.65 0.2 0.54
GreenWall_Part_24.Position -7.825 13.475 -0.01
GreenWall_Part_24.Color 8
GreenWall_Part_24.Visibility 1
GreenWall_Part_24.Mother GreenWall_BigBlock_1

// (25)
Volume GreenWall_Part_25
GreenWall_Part_25.Material Air
GreenWall_Part_25.Shape BRIK 3.35 0.2 0.54
GreenWall_Part_25.Position -0.225 13.475 -0.01
GreenWall_Part_25.Color 8
GreenWall_Part_25.Visibility 1
GreenWall_Part_25.Mother GreenWall_BigBlock_1

// (26)
Volume GreenWall_Part_26
GreenWall_Part_26.Material Air
GreenWall_Part_26.Shape BRIK 3.65 0.2 0.54
GreenWall_Part_26.Position 7.375 13.475 -0.01
GreenWall_Part_26.Color 8
GreenWall_Part_26.Visibility 1
GreenWall_Part_26.Mother GreenWall_BigBlock_1


// (27)
Volume GreenWall_Part_27
GreenWall_Part_27.Material Air
GreenWall_Part_27.Shape BRIK 0.2 3.4 0.54
GreenWall_Part_27.Position -13.825 8.075 -0.01
GreenWall_Part_27.Color 8
GreenWall_Part_27.Visibility 1
GreenWall_Part_27.Mother GreenWall_BigBlock_1

// (28)
Volume GreenWall_Part_28
GreenWall_Part_28.Material Air
GreenWall_Part_28.Shape BRIK 0.2 3.5 0.54
GreenWall_Part_28.Position -13.825 0.575 -0.01
GreenWall_Part_28.Color 8
GreenWall_Part_28.Visibility 1
GreenWall_Part_28.Mother GreenWall_BigBlock_1

// (29)
Volume GreenWall_Part_29
GreenWall_Part_29.Material Air
GreenWall_Part_29.Shape BRIK 0.2 3.55 0.54
GreenWall_Part_29.Position -13.835 -7.075 -0.01
GreenWall_Part_29.Color 8
GreenWall_Part_29.Visibility 1
GreenWall_Part_29.Mother GreenWall_BigBlock_1


// (30)
Volume GreenWall_Part_30
GreenWall_Part_30.Material Air
GreenWall_Part_30.Shape BRIK 1.2 3.5 0.34
GreenWall_Part_30.Position 12.425 7.975 0.19
GreenWall_Part_30.Color 8
GreenWall_Part_30.Visibility 1
GreenWall_Part_30.Mother GreenWall_BigBlock_1

// (31)
Volume GreenWall_Part_31
GreenWall_Part_31.Material Air
GreenWall_Part_31.Shape BRIK 1.2 3.7 0.34
GreenWall_Part_31.Position 12.425 0.575 0.19
GreenWall_Part_31.Color 8
GreenWall_Part_31.Visibility 1
GreenWall_Part_31.Mother GreenWall_BigBlock_1


// (32)
Volume GreenWall_Part_32
GreenWall_Part_32.Material Air
GreenWall_Part_32.Shape BRIK 1.3 9.1 0.2
GreenWall_Part_32.Position 12.325 4.175 -0.35
GreenWall_Part_32.Color 8
GreenWall_Part_32.Visibility 1
GreenWall_Part_32.Mother GreenWall_BigBlock_1

// (33)
Volume GreenWall_Part_33
GreenWall_Part_33.Material Air
GreenWall_Part_33.Shape BRIK 0.55 2.85 0.2
GreenWall_Part_33.Position 11.575 -7.775 -0.35
GreenWall_Part_33.Color 8
GreenWall_Part_33.Visibility 1
GreenWall_Part_33.Mother GreenWall_BigBlock_1


// (34)
Volume GreenWall_Part_34
GreenWall_Part_34.Material Air
GreenWall_Part_34.Shape BRIK 0.2 3.4 0.54
GreenWall_Part_34.Position 13.825 8.075 -0.01
GreenWall_Part_34.Color 8
GreenWall_Part_34.Visibility 1
GreenWall_Part_34.Mother GreenWall_BigBlock_1

// (35)
Volume GreenWall_Part_35
GreenWall_Part_35.Material Air
GreenWall_Part_35.Shape BRIK 0.2 3.5 0.54
GreenWall_Part_35.Position 13.825 0.575 -0.01
GreenWall_Part_35.Color 8
GreenWall_Part_35.Visibility 1
GreenWall_Part_35.Mother GreenWall_BigBlock_1



// (36)
Volume GreenWall_Part_36
GreenWall_Part_36.Material Air
GreenWall_Part_36.Shape BRIK 1.3 0.8 0.34
GreenWall_Part_36.Position 12.325 -4.125 0.19
GreenWall_Part_36.Color 8
GreenWall_Part_36.Visibility 1
GreenWall_Part_36.Mother GreenWall_BigBlock_1

// (37)
Volume GreenWall_Part_37
GreenWall_Part_37.Material Air
GreenWall_Part_37.Shape BRIK 0.55 2.85 0.34
GreenWall_Part_37.Position 11.575 -7.775 0.19
GreenWall_Part_37.Color 8
GreenWall_Part_37.Visibility 1
GreenWall_Part_37.Mother GreenWall_BigBlock_1



// Holes for cables on low right and right low side (38)-(41):

// (38)
Volume GreenWall_Part_38
GreenWall_Part_38.Material Air
GreenWall_Part_38.Shape BRIK 0.35 2.0 0.55
GreenWall_Part_38.Position 13.075 -7.925 0.0
GreenWall_Part_38.Color 8
GreenWall_Part_38.Visibility 1
GreenWall_Part_38.Mother GreenWall_BigBlock_1

// (39)
Volume GreenWall_Part_39
GreenWall_Part_39.Material Air
GreenWall_Part_39.Shape BRIK 0.45 2.7 0.55
GreenWall_Part_39.Position 13.875 -7.925 0.0
GreenWall_Part_39.Color 8
GreenWall_Part_39.Visibility 1
GreenWall_Part_39.Mother GreenWall_BigBlock_1

// (40)
Volume GreenWall_Part_40
GreenWall_Part_40.Material Air
GreenWall_Part_40.Shape BRIK 2.0 0.35 0.55
GreenWall_Part_40.Position 8.275 -12.725 0.0
GreenWall_Part_40.Color 8
GreenWall_Part_40.Visibility 1
GreenWall_Part_40.Mother GreenWall_BigBlock_1

// (41)
Volume GreenWall_Part_41
GreenWall_Part_41.Material Air
GreenWall_Part_41.Shape BRIK 2.7 0.45 0.55
GreenWall_Part_41.Position 8.275 -13.525 0.0
GreenWall_Part_41.Color 8
GreenWall_Part_41.Visibility 1
GreenWall_Part_41.Mother GreenWall_BigBlock_1

// End of holes for cables.



// (42)
Volume GreenWall_Part_42
GreenWall_Part_42.Material Air
GreenWall_Part_42.Shape BRIK 9.375 1.325 0.2
GreenWall_Part_42.Position -4.25 -11.95  -0.35
GreenWall_Part_42.Color 8
GreenWall_Part_42.Visibility 1
GreenWall_Part_42.Mother GreenWall_BigBlock_1

// (43)
Volume GreenWall_Part_43
GreenWall_Part_43.Material Air
GreenWall_Part_43.Shape BRIK 2.95 0.55 0.54
GreenWall_Part_43.Position 8.075 -11.175  -0.01
GreenWall_Part_43.Color 8
GreenWall_Part_43.Visibility 1
GreenWall_Part_43.Mother GreenWall_BigBlock_1


// (44)
Volume GreenWall_Part_44
GreenWall_Part_44.Material Air
GreenWall_Part_44.Shape BRIK 3.75 1.225 0.34
GreenWall_Part_44.Position -7.725 -12.05 0.19
GreenWall_Part_44.Color 8
GreenWall_Part_44.Visibility 1
GreenWall_Part_44.Mother GreenWall_BigBlock_1

// (45)
Volume GreenWall_Part_45
GreenWall_Part_45.Material Air
GreenWall_Part_45.Shape BRIK 3.55 1.225 0.34
GreenWall_Part_45.Position -0.225 -12.05 0.19
GreenWall_Part_45.Color 8
GreenWall_Part_45.Visibility 1
GreenWall_Part_45.Mother GreenWall_BigBlock_1


// (46)
Volume GreenWall_Part_46
GreenWall_Part_46.Material Air
GreenWall_Part_46.Shape BRIK 0.8 1.325 0.34
GreenWall_Part_46.Position 4.325 -11.95 0.19
GreenWall_Part_46.Color 8
GreenWall_Part_46.Visibility 1
GreenWall_Part_46.Mother GreenWall_BigBlock_1


// (47)
Volume GreenWall_Part_47
GreenWall_Part_47.Material Air
GreenWall_Part_47.Shape BRIK 3.65 0.2 0.54
GreenWall_Part_47.Position -7.825 -13.475 -0.01
GreenWall_Part_47.Color 8
GreenWall_Part_47.Visibility 1
GreenWall_Part_47.Mother GreenWall_BigBlock_1

// (48)
Volume GreenWall_Part_48
GreenWall_Part_48.Material Air
GreenWall_Part_48.Shape BRIK 3.35 0.2 0.54
GreenWall_Part_48.Position -0.225 -13.475 -0.01
GreenWall_Part_48.Color 8
GreenWall_Part_48.Visibility 1
GreenWall_Part_48.Mother GreenWall_BigBlock_1

// (49)
Volume GreenWall_Part_49
GreenWall_Part_49.Material Air
GreenWall_Part_49.Shape BRIK 0.7 0.2 0.54
GreenWall_Part_49.Position 4.425 -13.475 -0.01
GreenWall_Part_49.Color 8
GreenWall_Part_49.Visibility 1
GreenWall_Part_49.Mother GreenWall_BigBlock_1



// Screw-holes:

// Big ones on each corner:

// (50)
Volume GreenWall_Part_50
GreenWall_Part_50.Material Aluminium
GreenWall_Part_50.Shape TUBS 0.0 0.275 0.55 0.0 360.0 
GreenWall_Part_50.Position 0.0 0.0 0.0
GreenWall_Part_50.Color 8
GreenWall_Part_50.Visibility 1
GreenWall_Part_50.Mother GreenWall_Part_2

// (51)
Volume GreenWall_Part_51
GreenWall_Part_51.Material Aluminium
GreenWall_Part_51.Shape TUBS 0.0 0.275 0.55 0.0 360.0 
GreenWall_Part_51.Position 0.0 0.0 0.0
GreenWall_Part_51.Color 8
GreenWall_Part_51.Visibility 1
GreenWall_Part_51.Mother GreenWall_Part_3

// (52)
Volume GreenWall_Part_52
GreenWall_Part_52.Material Aluminium
GreenWall_Part_52.Shape TUBS 0.0 0.275 0.55 0.0 360.0 
GreenWall_Part_52.Position 0.0 0.0 0.0
GreenWall_Part_52.Color 8
GreenWall_Part_52.Visibility 1
GreenWall_Part_52.Mother GreenWall_Part_4

// (53)
Volume GreenWall_Part_53
GreenWall_Part_53.Material Aluminium
GreenWall_Part_53.Shape TUBS 0.0 0.275 0.55 0.0 360.0 
GreenWall_Part_53.Position 0.0 0.0 0.0
GreenWall_Part_53.Color 8
GreenWall_Part_53.Visibility 1
GreenWall_Part_53.Mother GreenWall_Part_5



// small ones:
// for connection to side wall:

// (54)
Volume GreenWall_Part_54
GreenWall_Part_54.Material Aluminium
GreenWall_Part_54.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_54.Position -13.925 11.725 0.0
GreenWall_Part_54.Color 8
GreenWall_Part_54.Visibility 1
GreenWall_Part_54.Mother GreenWall_BigBlock_1

// (55)
Volume GreenWall_Part_55
GreenWall_Part_55.Material Aluminium
GreenWall_Part_55.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_55.Position -13.925 4.375 0.0
GreenWall_Part_55.Color 8
GreenWall_Part_55.Visibility 1
GreenWall_Part_55.Mother GreenWall_BigBlock_1

// (56)
Volume GreenWall_Part_56
GreenWall_Part_56.Material Aluminium
GreenWall_Part_56.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_56.Position -13.925 -3.225 0.0
GreenWall_Part_56.Color 8
GreenWall_Part_56.Visibility 1
GreenWall_Part_56.Mother GreenWall_BigBlock_1

// (57)
Volume GreenWall_Part_57
GreenWall_Part_57.Material Aluminium
GreenWall_Part_57.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_57.Position -13.925 -12.075 0.0
GreenWall_Part_57.Color 8
GreenWall_Part_57.Visibility 1
GreenWall_Part_57.Mother GreenWall_BigBlock_1


// (58)
Volume GreenWall_Part_58
GreenWall_Part_58.Material Aluminium
GreenWall_Part_58.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_58.Position -12.325 -13.575 0.0
GreenWall_Part_58.Color 8
GreenWall_Part_58.Visibility 1
GreenWall_Part_58.Mother GreenWall_BigBlock_1

// (59)
Volume GreenWall_Part_59
GreenWall_Part_59.Material Aluminium
GreenWall_Part_59.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_59.Position -3.875 -13.575 0.0
GreenWall_Part_59.Color 8
GreenWall_Part_59.Visibility 1
GreenWall_Part_59.Mother GreenWall_BigBlock_1

// (60)
Volume GreenWall_Part_60
GreenWall_Part_60.Material Aluminium
GreenWall_Part_60.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_60.Position 3.425 -13.575 0.0
GreenWall_Part_60.Color 8
GreenWall_Part_60.Visibility 1
GreenWall_Part_60.Mother GreenWall_BigBlock_1

// (61)
Volume GreenWall_Part_61
GreenWall_Part_61.Material Aluminium
GreenWall_Part_61.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_61.Position 12.325 -13.575 0.0
GreenWall_Part_61.Color 8
GreenWall_Part_61.Visibility 1
GreenWall_Part_61.Mother GreenWall_BigBlock_1


// (62)
Volume GreenWall_Part_62
GreenWall_Part_62.Material Aluminium
GreenWall_Part_62.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_62.Position -12.325 13.575 0.0
GreenWall_Part_62.Color 8
GreenWall_Part_62.Visibility 1
GreenWall_Part_62.Mother GreenWall_BigBlock_1

// (63)
Volume GreenWall_Part_63
GreenWall_Part_63.Material Aluminium
GreenWall_Part_63.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_63.Position -3.875 13.575 0.0
GreenWall_Part_63.Color 8
GreenWall_Part_63.Visibility 1
GreenWall_Part_63.Mother GreenWall_BigBlock_1

// (64)
Volume GreenWall_Part_64
GreenWall_Part_64.Material Aluminium
GreenWall_Part_64.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_64.Position 3.425 13.575 0.0
GreenWall_Part_64.Color 8
GreenWall_Part_64.Visibility 1
GreenWall_Part_64.Mother GreenWall_BigBlock_1

// (65)
Volume GreenWall_Part_65
GreenWall_Part_65.Material Aluminium
GreenWall_Part_65.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_65.Position 12.325 13.575 0.0
GreenWall_Part_65.Color 8
GreenWall_Part_65.Visibility 1
GreenWall_Part_65.Mother GreenWall_BigBlock_1


// (66)
Volume GreenWall_Part_66
GreenWall_Part_66.Material Aluminium
GreenWall_Part_66.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_66.Position 13.925 11.725 0.0
GreenWall_Part_66.Color 8
GreenWall_Part_66.Visibility 1
GreenWall_Part_66.Mother GreenWall_BigBlock_1

// (67)
Volume GreenWall_Part_67
GreenWall_Part_67.Material Aluminium
GreenWall_Part_67.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_67.Position 13.925 4.375 0.0
GreenWall_Part_67.Color 8
GreenWall_Part_67.Visibility 1
GreenWall_Part_67.Mother GreenWall_BigBlock_1

// (68)
Volume GreenWall_Part_68
GreenWall_Part_68.Material Aluminium
GreenWall_Part_68.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_68.Position 13.925 -3.225 0.0
GreenWall_Part_68.Color 8
GreenWall_Part_68.Visibility 1
GreenWall_Part_68.Mother GreenWall_BigBlock_1

// (69)
Volume GreenWall_Part_69
GreenWall_Part_69.Material Aluminium
GreenWall_Part_69.Shape TUBS 0.0 0.15 0.55 0.0 360.0 
GreenWall_Part_69.Position 13.925 -12.075 0.0
GreenWall_Part_69.Color 8
GreenWall_Part_69.Visibility 1
GreenWall_Part_69.Mother GreenWall_BigBlock_1


// Connection to detector-frames:

// (70)
Volume GreenWall_Part_70
GreenWall_Part_70.Material Aluminium
GreenWall_Part_70.Shape TUBS 0.0 0.15 0.35 0.0 360.0 
GreenWall_Part_70.Position -11.775 12.075 0.2
GreenWall_Part_70.Color 8
GreenWall_Part_70.Visibility 1
GreenWall_Part_70.Mother GreenWall_BigBlock_1

// (71)
Volume GreenWall_Part_71
GreenWall_Part_71.Material Aluminium
GreenWall_Part_71.Shape TUBS 0.0 0.15 0.35 0.0 360.0 
GreenWall_Part_71.Position -11.775 -10.925 0.2
GreenWall_Part_71.Color 8
GreenWall_Part_71.Visibility 1
GreenWall_Part_71.Mother GreenWall_BigBlock_1

// (72)
Volume GreenWall_Part_72
GreenWall_Part_72.Material Aluminium
GreenWall_Part_72.Shape TUBS 0.0 0.15 0.35 0.0 360.0 
GreenWall_Part_72.Position 11.325 12.075 0.2
GreenWall_Part_72.Color 8
GreenWall_Part_72.Visibility 2
GreenWall_Part_72.Mother GreenWall_BigBlock_1

// (73)
Volume GreenWall_Part_73
GreenWall_Part_73.Material Aluminium
GreenWall_Part_73.Shape TUBS 0.0 0.15 0.35 0.0 360.0 
GreenWall_Part_73.Position 11.325 -10.925 0.2
GreenWall_Part_73.Color 8
GreenWall_Part_73.Visibility 1
GreenWall_Part_73.Mother GreenWall_BigBlock_1


// Purging - Connection:

// (74)
Volume GreenWall_Part_74
GreenWall_Part_74.Material Aluminium
GreenWall_Part_74.Shape TUBS 0.0 0.2 0.35 0.0 360.0 
GreenWall_Part_74.Position -12.875 12.075 0.2
GreenWall_Part_74.Color 6
GreenWall_Part_74.Visibility 1
GreenWall_Part_74.Mother GreenWall_BigBlock_1

// (75)
Volume GreenWall_Part_75
GreenWall_Part_75.Material Aluminium
GreenWall_Part_75.Shape TUBS 0.0 0.2 0.35 0.0 360.0 
GreenWall_Part_75.Position 12.375 -12.075 0.2
GreenWall_Part_75.Color 6
GreenWall_Part_75.Visibility 1
GreenWall_Part_75.Mother GreenWall_BigBlock_1




// Cable-Tree:

// Plasic Isolation:
// (76):
Volume GreenWall_Cable_76
GreenWall_Cable_76.Material Plastic_Isolation
GreenWall_Cable_76.Shape BRIK 1.55 0.305 0.55
GreenWall_Cable_76.Position 0.0 0.14 0.0
GreenWall_Cable_76.Color 6
GreenWall_Cable_76.Visibility 1
GreenWall_Cable_76.Mother GreenWall_Part_41

// Cu:
// (77):
Volume GreenWall_Cable_77
GreenWall_Cable_77.Material Copper
GreenWall_Cable_77.Shape BRIK 1.55 0.055 0.55
GreenWall_Cable_77.Position 0.0 0.0 0.0
GreenWall_Cable_77.Color 6
GreenWall_Cable_77.Visibility 1
GreenWall_Cable_77.Mother GreenWall_Cable_76



// Plasic Isolation:
// (78):
Volume GreenWall_Cable_78
GreenWall_Cable_78.Material Plastic_Isolation
GreenWall_Cable_78.Shape BRIK 0.305 1.55 0.55
GreenWall_Cable_78.Position -0.14 0.0 0.0
GreenWall_Cable_78.Color 6
GreenWall_Cable_78.Visibility 1
GreenWall_Cable_78.Mother GreenWall_Part_39

// Cu:
// (79):
Volume GreenWall_Cable_79
GreenWall_Cable_79.Material Copper
GreenWall_Cable_79.Shape BRIK 0.055 1.55 0.55
GreenWall_Cable_79.Position 0.0 0.0 0.0
GreenWall_Cable_79.Color 6
GreenWall_Cable_79.Visibility 1
GreenWall_Cable_79.Mother GreenWall_Cable_78



