// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
//Include Materials.geo


// The Mother Volume of the Detectot Configuration:
Volume RedWall_Volume
RedWall_Volume.Material Air
RedWall_Volume.Visibility 0
RedWall_Volume.Virtual true
RedWall_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
//RedWall_Volume.Mother 0


// biggest part of the thin wall:
// this one goes though from total left to total right.
// Thin Wall (1)
Volume RedWall_ThinWall_1
RedWall_ThinWall_1.Material Aluminium
RedWall_ThinWall_1.Shape BRIK 18.2 12.75 0.01
RedWall_ThinWall_1.Position 0.0 0.0 0.01
RedWall_ThinWall_1.Color 1
RedWall_ThinWall_1.Visibility 1
RedWall_ThinWall_1.Mother RedWall_Volume

// rectangular border-parts of the thin wall:
// these fill out regions above and beneath (1):
// (2)
Volume RedWall_ThinWall_2
RedWall_ThinWall_2.Material Aluminium
RedWall_ThinWall_2.Shape BRIK 12.75 2.725 0.01
RedWall_ThinWall_2.Position 0.0 15.475 0.01
RedWall_ThinWall_2.Color 1
RedWall_ThinWall_2.Visibility 1
RedWall_ThinWall_2.Mother RedWall_Volume

// (3)
Volume RedWall_ThinWall_3
RedWall_ThinWall_3.Material Aluminium
RedWall_ThinWall_3.Shape BRIK 12.75 2.725 0.01
RedWall_ThinWall_3.Position 0.0 -15.475 0.01
RedWall_ThinWall_3.Color 1
RedWall_ThinWall_3.Visibility 1
RedWall_ThinWall_3.Mother RedWall_Volume



// the triangular corner parts of the thin wall:
// these almost perfectly fit in, but not 100%, because of some calculation roundings.

// (4)
Volume RedWall_ThinWall_4
RedWall_ThinWall_4.Material Aluminium
RedWall_ThinWall_4.Shape TRD1 3.8535 0.0 0.01 1.92675
RedWall_ThinWall_4.Rotation -90.0 0.0 -45.0
RedWall_ThinWall_4.Position -14.1125 -14.1125 0.01
RedWall_ThinWall_4.Color 1
RedWall_ThinWall_4.Visibility 1
RedWall_ThinWall_4.Mother RedWall_Volume

// (5)
Volume RedWall_ThinWall_5
RedWall_ThinWall_5.Material Aluminium
RedWall_ThinWall_5.Shape TRD1 3.8535 0.0 0.01 1.92675
RedWall_ThinWall_5.Rotation -90.0 0.0 -135.0
RedWall_ThinWall_5.Position -14.1125 14.1125 0.01
RedWall_ThinWall_5.Color 1
RedWall_ThinWall_5.Visibility 1
RedWall_ThinWall_5.Mother RedWall_Volume

// (6)
Volume RedWall_ThinWall_6
RedWall_ThinWall_6.Material Aluminium
RedWall_ThinWall_6.Shape TRD1 3.8535 0.0 0.01 1.92675
RedWall_ThinWall_6.Rotation -90.0 0.0 45.0
RedWall_ThinWall_6.Position 14.1125 -14.1125 0.01
RedWall_ThinWall_6.Color 1
RedWall_ThinWall_6.Visibility 1
RedWall_ThinWall_6.Mother RedWall_Volume

// (7)
Volume RedWall_ThinWall_7
RedWall_ThinWall_7.Material Aluminium
RedWall_ThinWall_7.Shape TRD1 3.8535 0.0 0.01 1.92675
RedWall_ThinWall_7.Rotation -90.0 0.0 135.0
RedWall_ThinWall_7.Position 14.1125 14.1125 0.01
RedWall_ThinWall_7.Color 1
RedWall_ThinWall_7.Visibility 1
RedWall_ThinWall_7.Mother RedWall_Volume



// parts (8) and (9) yet undefined.




// (10)
Volume RedWall_Part_10
RedWall_Part_10.Material Aluminium
RedWall_Part_10.Shape BRIK 0.2 17.8 0.19
RedWall_Part_10.Position -10.0 0.0 0.21
RedWall_Part_10.Color 1
RedWall_Part_10.Visibility 1
RedWall_Part_10.Mother RedWall_Volume


// (11)
Volume RedWall_Part_11
RedWall_Part_11.Material Aluminium
RedWall_Part_11.Shape BRIK 12.75 0.2 0.19
RedWall_Part_11.Position 0.0 18.0 0.21
RedWall_Part_11.Color 1
RedWall_Part_11.Visibility 1
RedWall_Part_11.Mother RedWall_Volume

// (12)
Volume RedWall_Part_12
RedWall_Part_12.Material Aluminium
RedWall_Part_12.Shape BRIK 12.75 0.2 0.19
RedWall_Part_12.Position 0.0 -18.0 0.21
RedWall_Part_12.Color 1
RedWall_Part_12.Visibility 1
RedWall_Part_12.Mother RedWall_Volume


// (13)
Volume RedWall_Part_13
RedWall_Part_13.Material Aluminium
RedWall_Part_13.Shape BRIK 0.2 12.75 0.19
RedWall_Part_13.Position 18.0 0.0 0.21
RedWall_Part_13.Color 1
RedWall_Part_13.Visibility 1
RedWall_Part_13.Mother RedWall_Volume

// (14)
Volume RedWall_Part_14
RedWall_Part_14.Material Aluminium
RedWall_Part_14.Shape BRIK 0.2 12.75 0.19
RedWall_Part_14.Position -18.0 0.0 0.21
RedWall_Part_14.Color 1
RedWall_Part_14.Visibility 1
RedWall_Part_14.Mother RedWall_Volume


// (15)
Volume RedWall_Part_15
RedWall_Part_15.Material Aluminium
RedWall_Part_15.Shape BRIK 3.8 0.2 0.19
RedWall_Part_15.Position -14.0 10.0 0.21
RedWall_Part_15.Color 1
RedWall_Part_15.Visibility 1
RedWall_Part_15.Mother RedWall_Volume

// (16)
Volume RedWall_Part_16
RedWall_Part_16.Material Aluminium
RedWall_Part_16.Shape BRIK 13.8 0.2 0.19
RedWall_Part_16.Position 4.0 10.0 0.21
RedWall_Part_16.Color 1
RedWall_Part_16.Visibility 1
RedWall_Part_16.Mother RedWall_Volume


// (17)
Volume RedWall_Part_17
RedWall_Part_17.Material Aluminium
RedWall_Part_17.Shape BRIK 13.8 0.1 0.19
RedWall_Part_17.Position 4.0 2.25 0.21
RedWall_Part_17.Color 1
RedWall_Part_17.Visibility 1
RedWall_Part_17.Mother RedWall_Volume

// (18)
Volume RedWall_Part_18
RedWall_Part_18.Material Aluminium
RedWall_Part_18.Shape BRIK 13.8 0.1 0.19
RedWall_Part_18.Position 4.0 -5.35 0.21
RedWall_Part_18.Color 1
RedWall_Part_18.Visibility 1
RedWall_Part_18.Mother RedWall_Volume


// (19)
Volume RedWall_Part_19
RedWall_Part_19.Material Aluminium
RedWall_Part_19.Shape BRIK 3.8 0.1 0.19
RedWall_Part_19.Position -14.0 2.25 0.21
RedWall_Part_19.Color 1
RedWall_Part_19.Visibility 1
RedWall_Part_19.Mother RedWall_Volume

// (20)
Volume RedWall_Part_20
RedWall_Part_20.Material Aluminium
RedWall_Part_20.Shape BRIK 3.8 0.1 0.19
RedWall_Part_20.Position -14.0 -5.35 0.21
RedWall_Part_20.Color 1
RedWall_Part_20.Visibility 1
RedWall_Part_20.Mother RedWall_Volume



// (21)
Volume RedWall_Part_21
RedWall_Part_21.Material Aluminium
RedWall_Part_21.Shape BRIK 0.1 3.475 0.19
RedWall_Part_21.Position -12.15 5.825 0.21
RedWall_Part_21.Color 1
RedWall_Part_21.Visibility 1
RedWall_Part_21.Mother RedWall_Volume

// (22)
Volume RedWall_Part_22
RedWall_Part_22.Material Aluminium
RedWall_Part_22.Shape BRIK 0.1 3.725 0.19
RedWall_Part_22.Position -2.1 6.075 0.21
RedWall_Part_22.Color 1
RedWall_Part_22.Visibility 1
RedWall_Part_22.Mother RedWall_Volume

// (23)
Volume RedWall_Part_23
RedWall_Part_23.Material Aluminium
RedWall_Part_23.Shape BRIK 0.1 3.725 0.19
RedWall_Part_23.Position 5.2 6.075 0.21
RedWall_Part_23.Color 1
RedWall_Part_23.Visibility 1
RedWall_Part_23.Mother RedWall_Volume

// (24)
Volume RedWall_Part_24
RedWall_Part_24.Material Aluminium
RedWall_Part_24.Shape BRIK 0.2 3.725 0.19
RedWall_Part_24.Position 13.075 6.075 0.21
RedWall_Part_24.Color 1
RedWall_Part_24.Visibility 1
RedWall_Part_24.Mother RedWall_Volume

// (25)
Volume RedWall_Part_25
RedWall_Part_25.Material Aluminium
RedWall_Part_25.Shape BRIK 0.1 3.475 0.19
RedWall_Part_25.Position 15.675 5.825 0.21
RedWall_Part_25.Color 1
RedWall_Part_25.Visibility 1
RedWall_Part_25.Mother RedWall_Volume


// (26)
Volume RedWall_Part_26
RedWall_Part_26.Material Aluminium
RedWall_Part_26.Shape BRIK 0.1 3.7 0.19
RedWall_Part_26.Position -12.15 -1.55 0.21
RedWall_Part_26.Color 1
RedWall_Part_26.Visibility 1
RedWall_Part_26.Mother RedWall_Volume

// (27)
Volume RedWall_Part_27
RedWall_Part_27.Material Aluminium
RedWall_Part_27.Shape BRIK 0.1 3.7 0.19
RedWall_Part_27.Position -2.1 -1.55 0.21
RedWall_Part_27.Color 1
RedWall_Part_27.Visibility 1
RedWall_Part_27.Mother RedWall_Volume

// (28)
Volume RedWall_Part_28
RedWall_Part_28.Material Aluminium
RedWall_Part_28.Shape BRIK 0.1 3.7 0.19
RedWall_Part_28.Position 5.2 -1.55 0.21
RedWall_Part_28.Color 1
RedWall_Part_28.Visibility 1
RedWall_Part_28.Mother RedWall_Volume

// (29)
Volume RedWall_Part_29
RedWall_Part_29.Material Aluminium
RedWall_Part_29.Shape BRIK 0.2 3.7 0.19
RedWall_Part_29.Position 13.075 -1.55 0.21
RedWall_Part_29.Color 1
RedWall_Part_29.Visibility 1
RedWall_Part_29.Mother RedWall_Volume

// (30)
Volume RedWall_Part_30
RedWall_Part_30.Material Aluminium
RedWall_Part_30.Shape BRIK 0.1 3.7 0.19
RedWall_Part_30.Position 15.675 -1.55 0.21
RedWall_Part_30.Color 1
RedWall_Part_30.Visibility 1
RedWall_Part_30.Mother RedWall_Volume


// (31)
Volume RedWall_Part_31
RedWall_Part_31.Material Aluminium
RedWall_Part_31.Shape BRIK 0.1 3.725 0.19
RedWall_Part_31.Position -12.15 -9.175 0.21
RedWall_Part_31.Color 1
RedWall_Part_31.Visibility 1
RedWall_Part_31.Mother RedWall_Volume

// (32)
Volume RedWall_Part_32
RedWall_Part_32.Material Aluminium
RedWall_Part_32.Shape BRIK 0.1 3.725 0.19
RedWall_Part_32.Position -2.1 -9.175 0.21
RedWall_Part_32.Color 1
RedWall_Part_32.Visibility 1
RedWall_Part_32.Mother RedWall_Volume

// (33)
Volume RedWall_Part_33
RedWall_Part_33.Material Aluminium
RedWall_Part_33.Shape BRIK 0.1 3.725 0.19
RedWall_Part_33.Position 5.2 -9.175 0.21
RedWall_Part_33.Color 1
RedWall_Part_33.Visibility 1
RedWall_Part_33.Mother RedWall_Volume

// (34)
Volume RedWall_Part_34
RedWall_Part_34.Material Aluminium
RedWall_Part_34.Shape BRIK 0.2 3.725 0.19
RedWall_Part_34.Position 13.075 -9.175 0.21
RedWall_Part_34.Color 1
RedWall_Part_34.Visibility 1
RedWall_Part_34.Mother RedWall_Volume

// (35)
Volume RedWall_Part_35
RedWall_Part_35.Material Aluminium
RedWall_Part_35.Shape BRIK 0.1 3.725 0.19
RedWall_Part_35.Position 15.675 -9.175 0.21
RedWall_Part_35.Color 1
RedWall_Part_35.Visibility 1
RedWall_Part_35.Mother RedWall_Volume




// (36)
Volume RedWall_Part_36
RedWall_Part_36.Material Aluminium
RedWall_Part_36.Shape BRIK 11.3375 0.2 0.19
RedWall_Part_36.Position 1.5375 -13.1 0.21
RedWall_Part_36.Color 1
RedWall_Part_36.Visibility 1
RedWall_Part_36.Mother RedWall_Volume

// (37)
Volume RedWall_Part_37
RedWall_Part_37.Material Aluminium
RedWall_Part_37.Shape BRIK 0.1 1.05 0.19
RedWall_Part_37.Position -2.1 -14.35 0.21
RedWall_Part_37.Color 1
RedWall_Part_37.Visibility 1
RedWall_Part_37.Mother RedWall_Volume

// (38)
Volume RedWall_Part_38
RedWall_Part_38.Material Aluminium
RedWall_Part_38.Shape BRIK 0.1 0.875 0.19
RedWall_Part_38.Position -2.1 -16.925 0.21
RedWall_Part_38.Color 1
RedWall_Part_38.Visibility 1
RedWall_Part_38.Mother RedWall_Volume

// (39)
Volume RedWall_Part_39
RedWall_Part_39.Material Aluminium
RedWall_Part_39.Shape BRIK 0.3 0.325 0.19
RedWall_Part_39.Position -2.1 -15.725 0.21
RedWall_Part_39.Color 1
RedWall_Part_39.Visibility 1
RedWall_Part_39.Mother RedWall_Volume

// (40)
Volume RedWall_Part_40
RedWall_Part_40.Material Aluminium
RedWall_Part_40.Shape BRIK 0.1 1.05 0.19
RedWall_Part_40.Position 5.2 -14.35 0.21
RedWall_Part_40.Color 1
RedWall_Part_40.Visibility 1
RedWall_Part_40.Mother RedWall_Volume

// (41)
Volume RedWall_Part_41
RedWall_Part_41.Material Aluminium
RedWall_Part_41.Shape BRIK 0.1 0.875 0.19
RedWall_Part_41.Position 5.2 -16.925 0.21
RedWall_Part_41.Color 1
RedWall_Part_41.Visibility 1
RedWall_Part_41.Mother RedWall_Volume

// (42)
Volume RedWall_Part_42
RedWall_Part_42.Material Aluminium
RedWall_Part_42.Shape BRIK 0.3 0.325 0.19
RedWall_Part_42.Position 5.2 -15.725 0.21
RedWall_Part_42.Color 1
RedWall_Part_42.Visibility 1
RedWall_Part_42.Mother RedWall_Volume


// (43)
Volume RedWall_Part_43
RedWall_Part_43.Material Aluminium
RedWall_Part_43.Shape BRIK 0.1 3.0 0.19
RedWall_Part_43.Position -2.1 14.8 0.21
RedWall_Part_43.Color 1
RedWall_Part_43.Visibility 1
RedWall_Part_43.Mother RedWall_Volume

// (44)
Volume RedWall_Part_44
RedWall_Part_44.Material Aluminium
RedWall_Part_44.Shape BRIK 0.1 3.0 0.19
RedWall_Part_44.Position 5.2 14.8 0.21
RedWall_Part_44.Color 1
RedWall_Part_44.Visibility 1
RedWall_Part_44.Mother RedWall_Volume

// (45)
Volume RedWall_Part_45
RedWall_Part_45.Material Aluminium
RedWall_Part_45.Shape BRIK 0.3 0.325 0.19
RedWall_Part_45.Position -2.1 11.475 0.21
RedWall_Part_45.Color 1
RedWall_Part_45.Visibility 1
RedWall_Part_45.Mother RedWall_Volume

// (46)
Volume RedWall_Part_46
RedWall_Part_46.Material Aluminium
RedWall_Part_46.Shape BRIK 0.3 0.325 0.19
RedWall_Part_46.Position 5.2 11.475 0.21
RedWall_Part_46.Color 1
RedWall_Part_46.Visibility 1
RedWall_Part_46.Mother RedWall_Volume

// (47)
Volume RedWall_Part_47
RedWall_Part_47.Material Aluminium
RedWall_Part_47.Shape BRIK 0.1 0.475 0.19
RedWall_Part_47.Position -2.1 10.675 0.21
RedWall_Part_47.Color 1
RedWall_Part_47.Visibility 1
RedWall_Part_47.Mother RedWall_Volume

// (48)
Volume RedWall_Part_48
RedWall_Part_48.Material Aluminium
RedWall_Part_48.Shape BRIK 0.1 0.475 0.19
RedWall_Part_48.Position 5.2 10.675 0.21
RedWall_Part_48.Color 1
RedWall_Part_48.Visibility 1
RedWall_Part_48.Mother RedWall_Volume


// (49)
Volume RedWall_Part_49
RedWall_Part_49.Material Aluminium
RedWall_Part_49.Shape BRIK 0.35 0.8 0.19
RedWall_Part_49.Position -10.55 11.0 0.21
RedWall_Part_49.Color 1
RedWall_Part_49.Visibility 1
RedWall_Part_49.Mother RedWall_Volume

// (50)
Volume RedWall_Part_50
RedWall_Part_50.Material Aluminium
RedWall_Part_50.Shape BRIK 0.55 0.3 0.19
RedWall_Part_50.Position 13.825 11.5 0.21
RedWall_Part_50.Color 1
RedWall_Part_50.Visibility 1
RedWall_Part_50.Mother RedWall_Volume

// (51)
Volume RedWall_Part_51
RedWall_Part_51.Material Aluminium
RedWall_Part_51.Shape BRIK 0.4 0.3 0.19
RedWall_Part_51.Position 0.0 17.5 0.21
RedWall_Part_51.Color 1
RedWall_Part_51.Visibility 1
RedWall_Part_51.Mother RedWall_Volume

// (52)
Volume RedWall_Part_52
RedWall_Part_52.Material Aluminium
RedWall_Part_52.Shape BRIK 1.175 0.25 0.19
RedWall_Part_52.Position -11.375 9.55 0.21
RedWall_Part_52.Color 1
RedWall_Part_52.Visibility 1
RedWall_Part_52.Mother RedWall_Volume

// (53)
Volume RedWall_Part_53
RedWall_Part_53.Material Aluminium
RedWall_Part_53.Shape BRIK 0.3 0.4 0.19
RedWall_Part_53.Position 17.5 0.0 0.21
RedWall_Part_53.Color 1
RedWall_Part_53.Visibility 1
RedWall_Part_53.Mother RedWall_Volume

// (54)
Volume RedWall_Part_54
RedWall_Part_54.Material Aluminium
RedWall_Part_54.Shape BRIK 0.3 0.4 0.19
RedWall_Part_54.Position -17.5 0.0 0.21
RedWall_Part_54.Color 1
RedWall_Part_54.Visibility 1
RedWall_Part_54.Mother RedWall_Volume

// (55)
Volume RedWall_Part_55
RedWall_Part_55.Material Aluminium
RedWall_Part_55.Shape BRIK 0.4 0.3 0.19
RedWall_Part_55.Position 0.0 -17.5 0.21
RedWall_Part_55.Color 1
RedWall_Part_55.Visibility 1
RedWall_Part_55.Mother RedWall_Volume

// (56)
Volume RedWall_Part_56
RedWall_Part_56.Material Aluminium
RedWall_Part_56.Shape BRIK 0.3 0.25 0.19
RedWall_Part_56.Position 15.675 9.55 0.21
RedWall_Part_56.Color 1
RedWall_Part_56.Visibility 1
RedWall_Part_56.Mother RedWall_Volume

// (57)
Volume RedWall_Part_57
RedWall_Part_57.Material Aluminium
RedWall_Part_57.Shape BRIK 0.35 0.625 0.19
RedWall_Part_57.Position -12.15 -13.925 0.21
RedWall_Part_57.Color 1
RedWall_Part_57.Visibility 1
RedWall_Part_57.Mother RedWall_Volume

// (58)
Volume RedWall_Part_58
RedWall_Part_58.Material Aluminium
RedWall_Part_58.Shape BRIK 0.35 0.35 0.19
RedWall_Part_58.Position -10.55 -15.75 0.21
RedWall_Part_58.Color 1
RedWall_Part_58.Visibility 1
RedWall_Part_58.Mother RedWall_Volume

// (59)
Volume RedWall_Part_59
RedWall_Part_59.Material Aluminium
RedWall_Part_59.Shape BRIK 0.1 1.625 0.19
RedWall_Part_59.Position -12.15 -16.175 0.21
RedWall_Part_59.Color 1
RedWall_Part_59.Visibility 1
RedWall_Part_59.Mother RedWall_Volume

// (60)
Volume RedWall_Part_60
RedWall_Part_60.Material Aluminium
RedWall_Part_60.Shape BRIK 0.575 0.1 0.19
RedWall_Part_60.Position -11.475 -15.75 0.21
RedWall_Part_60.Color 1
RedWall_Part_60.Visibility 1
RedWall_Part_60.Mother RedWall_Volume


// From here on, dimension and position of the parts are approximated for technical reasons:


// Teil 61 noch kürzen!!!
// (61)
Volume RedWall_Part_61
RedWall_Part_61.Material Aluminium
//RedWall_Part_61.Shape BRIK 0.2 3.85 0.19
RedWall_Part_61.Shape BRIK 0.2 3.44 0.19
//RedWall_Part_61.Position 13.075 14.04 0.21
RedWall_Part_61.Position 13.075 13.64 0.21
RedWall_Part_61.Color 1
RedWall_Part_61.Visibility 1
RedWall_Part_61.Mother RedWall_Volume

// (62)
Volume RedWall_Part_62
RedWall_Part_62.Material Aluminium
RedWall_Part_62.Shape TRD1 3.82 3.42 0.19 0.2		// 3.853725 3.453725 0.19 0.2
RedWall_Part_62.Rotation -90.0 0.0 135.0
RedWall_Part_62.Position 15.32 15.33 0.21
RedWall_Part_62.Color 1
RedWall_Part_62.Visibility 1
RedWall_Part_62.Mother RedWall_Volume

// (63)
// NOT EXACT, APPROXIMATED
Volume RedWall_Part_63
RedWall_Part_63.Material Aluminium
RedWall_Part_63.Shape BRIK 0.3 0.5 0.19
RedWall_Part_63.Position 17.5 12.25 0.21
RedWall_Part_63.Color 1
RedWall_Part_63.Visibility 1
RedWall_Part_63.Mother RedWall_Volume

// (65)
// NOT EXACT, APPROXIMATED
Volume RedWall_Part_65
RedWall_Part_65.Material Aluminium
RedWall_Part_65.Shape BRIK 0.3 0.5 0.19
RedWall_Part_65.Position -17.5 12.25 0.21
RedWall_Part_65.Color 1
RedWall_Part_65.Visibility 1
RedWall_Part_65.Mother RedWall_Volume

// (64)
Volume RedWall_Part_64
RedWall_Part_64.Material Aluminium
//RedWall_Part_64.Shape TRD1 3.85 3.45 0.19 0.2
RedWall_Part_64.Shape TRD1 3.853725 3.453725 0.19 0.2
RedWall_Part_64.Rotation -90.0 0.0 225.0
//RedWall_Part_64.Position -15.32 15.33 0.21
RedWall_Part_64.Position -15.33358 15.33358 0.21
RedWall_Part_64.Color 1
RedWall_Part_64.Visibility 1
RedWall_Part_64.Mother RedWall_Volume

// (66)
Volume RedWall_Part_66
RedWall_Part_66.Material Aluminium
//RedWall_Part_66.Shape TRD1 3.853725 3.453725 0.19 0.2
RedWall_Part_66.Shape TRD1 3.7 0.0 0.19 1.85 // x = 3.853725,  z = 1.9268625
RedWall_Part_66.Rotation -90.0 0.0 45.0
//RedWall_Part_66.Position 15.33358 -15.33358 0.21
RedWall_Part_66.Position 14.185 -14.21 0.21
RedWall_Part_66.Color 1
RedWall_Part_66.Visibility 1
RedWall_Part_66.Mother RedWall_Volume

// (67)
Volume RedWall_Part_67
RedWall_Part_67.Material Aluminium
RedWall_Part_67.Shape TRD1 3.853725 3.453725 0.19 0.2
RedWall_Part_67.Rotation -90.0 0.0 315.0
RedWall_Part_67.Position -15.33358 -15.33358 0.21
RedWall_Part_67.Color 1
RedWall_Part_67.Visibility 1
RedWall_Part_67.Mother RedWall_Volume



// (70)
Volume RedWall_Part_70
RedWall_Part_70.Material Aluminium
RedWall_Part_70.Shape BRIK 3.4 0.2 0.19
RedWall_Part_70.Position -13.6 -13.1 0.21
RedWall_Part_70.Color 1
RedWall_Part_70.Visibility 1
RedWall_Part_70.Mother RedWall_Volume



// MIWorks reports intersections of part (71) and (66), but if you have a closer view on it,
// The Canvas Viewer does not show any intersections.


// (71)
Volume RedWall_Part_71
RedWall_Part_71.Material Aluminium
RedWall_Part_71.Shape BRIK 1.05 0.19 1.05
RedWall_Part_71.Position 0.0 0.0 -0.21 // z = 1.1663 x = -1.1663
RedWall_Part_71.Rotation 0. 45. 0.
RedWall_Part_71.Color 6
RedWall_Part_71.Visibility 1
RedWall_Part_71.Mother RedWall_Part_66





// Borings:

// The following borings are exactly dimensioned and positioned:

// (80)
Volume RedWall_Part_80
RedWall_Part_80.Material Steel
RedWall_Part_80.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_80.Position 0.0 0.0 0.0
RedWall_Part_80.Color 7
RedWall_Part_80.Visibility 1
RedWall_Part_80.Mother RedWall_Part_45

// (81)
Volume RedWall_Part_81
RedWall_Part_81.Material Steel
RedWall_Part_81.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_81.Position 0.0 0.0 0.0
RedWall_Part_81.Color 7
RedWall_Part_81.Visibility 1
RedWall_Part_81.Mother RedWall_Part_46

// (82)
Volume RedWall_Part_82
RedWall_Part_82.Material Steel
RedWall_Part_82.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_82.Position 0.0 0.0 0.0
RedWall_Part_82.Color 7
RedWall_Part_82.Visibility 1
RedWall_Part_82.Mother RedWall_Part_39

// (83)
Volume RedWall_Part_83
RedWall_Part_83.Material Steel
RedWall_Part_83.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_83.Position 0.0 0.0 0.0
RedWall_Part_83.Color 7
RedWall_Part_83.Visibility 1
RedWall_Part_83.Mother RedWall_Part_42

// The borings with extention "a" lie beneath the ones without the extention in the thin wall.

// (80a)
Volume RedWall_Part_80a
RedWall_Part_80a.Material Steel
RedWall_Part_80a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_45.Position -2.1 11.475 0.21
RedWall_Part_80a.Position -2.1 11.475 0.0
RedWall_Part_80a.Color 7
RedWall_Part_80a.Visibility 1
RedWall_Part_80a.Mother RedWall_ThinWall_1

// (81a)
Volume RedWall_Part_81a
RedWall_Part_81a.Material Steel
RedWall_Part_81a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_46.Position 5.2 11.475 0.21
RedWall_Part_81a.Position 5.2 11.475 0.0
RedWall_Part_81a.Color 7
RedWall_Part_81a.Visibility 1
RedWall_Part_81a.Mother RedWall_ThinWall_1

// (82a)
Volume RedWall_Part_82a
RedWall_Part_82a.Material Steel
RedWall_Part_82a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_39.Position -2.1 -15.725 0.21
RedWall_Part_82a.Position -2.1 -0.25 0.0
RedWall_Part_82a.Color 7
RedWall_Part_82a.Visibility 1
RedWall_Part_82a.Mother RedWall_ThinWall_3

// (83a)
Volume RedWall_Part_83a
RedWall_Part_83a.Material Steel
RedWall_Part_83a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_42.Position 5.2 -15.725 0.21
RedWall_Part_83a.Position 5.2 -0.25 0.0
RedWall_Part_83a.Color 7
RedWall_Part_83a.Visibility 1
RedWall_Part_83a.Mother RedWall_ThinWall_3


// (84)
Volume RedWall_Part_84
RedWall_Part_84.Material Steel
RedWall_Part_84.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_84.Position 0.025 0.0 0.0
RedWall_Part_84.Color 7
RedWall_Part_84.Visibility 1
RedWall_Part_84.Mother RedWall_Part_58

// (84a)
Volume RedWall_Part_84a
RedWall_Part_84a.Material Steel
RedWall_Part_84a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_58.Position -10.55 -15.75 0.21
RedWall_Part_84a.Position -10.525 -0.275 0.0
RedWall_Part_84a.Color 7
RedWall_Part_84a.Visibility 1
RedWall_Part_84a.Mother RedWall_ThinWall_3


// (85)
Volume RedWall_Part_85
RedWall_Part_85.Material Steel
RedWall_Part_85.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_85.Position 0.0 -0.25 0.0
RedWall_Part_85.Color 7
RedWall_Part_85.Visibility 1
RedWall_Part_85.Mother RedWall_Part_57

// (85a)
Volume RedWall_Part_85a
RedWall_Part_85a.Material Steel
RedWall_Part_85a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_57.Position -12.15 -13.925 0.21
RedWall_Part_85a.Position -12.15 1.3 0.0
RedWall_Part_85a.Color 7
RedWall_Part_85a.Visibility 1
RedWall_Part_85a.Mother RedWall_ThinWall_3


// (86)
Volume RedWall_Part_86
RedWall_Part_86.Material Steel
RedWall_Part_86.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_86.Position 0.0 0.5 0.0
RedWall_Part_86.Color 7
RedWall_Part_86.Visibility 1
RedWall_Part_86.Mother RedWall_Part_49

// (86a)
Volume RedWall_Part_86a
RedWall_Part_86a.Material Steel
RedWall_Part_86a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_49.Position -10.55 11.0 0.21
RedWall_Part_86a.Position -10.55 11.5 0.0
RedWall_Part_86a.Color 7
RedWall_Part_86a.Visibility 1
RedWall_Part_86a.Mother RedWall_ThinWall_1

// (87)
Volume RedWall_Part_87
RedWall_Part_87.Material Steel
RedWall_Part_87.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_87.Position 0.25 0.0 0.0
RedWall_Part_87.Color 7
RedWall_Part_87.Visibility 1
RedWall_Part_87.Mother RedWall_Part_50

// (87a)
Volume RedWall_Part_87a
RedWall_Part_87a.Material Steel
RedWall_Part_87a.Shape TUBS 0.0 0.15 0.01 0. 360.
//RedWall_Part_50.Position 13.825 11.5 0.21
RedWall_Part_87a.Position 14.075 11.5 0.0
RedWall_Part_87a.Color 7
RedWall_Part_87a.Visibility 1
RedWall_Part_87a.Mother RedWall_ThinWall_1


// (88)
Volume RedWall_Part_88
RedWall_Part_88.Material Steel
RedWall_Part_88.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_88.Position 0.0 0.1 0.0
RedWall_Part_88.Color 7
RedWall_Part_88.Visibility 1
RedWall_Part_88.Mother RedWall_Part_51

// (88a)
Volume RedWall_Part_88a
RedWall_Part_88a.Material Steel
RedWall_Part_88a.Shape TUBS 0.0 0.2 0.01 0. 360.
//RedWall_Part_51.Position 0.0 17.5 0.21
RedWall_Part_88a.Position 0.0 2.125 0.0
RedWall_Part_88a.Color 7
RedWall_Part_88a.Visibility 1
RedWall_Part_88a.Mother RedWall_ThinWall_2


// (89)
Volume RedWall_Part_89
RedWall_Part_89.Material Steel
RedWall_Part_89.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_89.Position 0.0 -0.1 0.0
RedWall_Part_89.Color 7
RedWall_Part_89.Visibility 1
RedWall_Part_89.Mother RedWall_Part_55

// (89a)
Volume RedWall_Part_89a
RedWall_Part_89a.Material Steel
RedWall_Part_89a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_89a.Position 0.0 -2.125 0.0
RedWall_Part_89a.Color 7
RedWall_Part_89a.Visibility 1
RedWall_Part_89a.Mother RedWall_ThinWall_3


// (90)
Volume RedWall_Part_90
RedWall_Part_90.Material Steel
RedWall_Part_90.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_90.Position +0.1 0.0 0.0
RedWall_Part_90.Color 7
RedWall_Part_90.Visibility 1
RedWall_Part_90.Mother RedWall_Part_53

// (90a)
Volume RedWall_Part_90a
RedWall_Part_90a.Material Steel
RedWall_Part_90a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_90a.Position -17.6 0.0 0.0
RedWall_Part_90a.Color 7
RedWall_Part_90a.Visibility 1
RedWall_Part_90a.Mother RedWall_ThinWall_1


// (91)
Volume RedWall_Part_91
RedWall_Part_91.Material Steel
RedWall_Part_91.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_91.Position -0.1 0.0 0.0
RedWall_Part_91.Color 7
RedWall_Part_91.Visibility 1
RedWall_Part_91.Mother RedWall_Part_54

// (91a)
Volume RedWall_Part_91a
RedWall_Part_91a.Material Steel
RedWall_Part_91a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_91a.Position 17.6 0.0  0.0
RedWall_Part_91a.Color 7
RedWall_Part_91a.Visibility 1
RedWall_Part_91a.Mother RedWall_ThinWall_1


// (92)
Volume RedWall_Part_92
RedWall_Part_92.Material Steel
RedWall_Part_92.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_92.Position 0.0 10.0 0.0
RedWall_Part_92.Color 7
RedWall_Part_92.Visibility 1
RedWall_Part_92.Mother RedWall_Part_10

// (92a)
Volume RedWall_Part_92a
RedWall_Part_92a.Material Steel
RedWall_Part_92a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_92a.Position -10.0 10.0 0.0
RedWall_Part_92a.Color 7
RedWall_Part_92a.Visibility 1
RedWall_Part_92a.Mother RedWall_ThinWall_1


// (93)
Volume RedWall_Part_93
RedWall_Part_93.Material Steel
RedWall_Part_93.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_93.Position 0.0 -13.1 0.0
RedWall_Part_93.Color 7
RedWall_Part_93.Visibility 1
RedWall_Part_93.Mother RedWall_Part_10

// (93a)
Volume RedWall_Part_93a
RedWall_Part_93a.Material Steel
RedWall_Part_93a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_93a.Position -10.0 2.375 0.0
RedWall_Part_93a.Color 7
RedWall_Part_93a.Visibility 1
RedWall_Part_93a.Mother RedWall_ThinWall_3


// (94)
Volume RedWall_Part_94
RedWall_Part_94.Material Steel
RedWall_Part_94.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_94.Position 9.075 0.0 0.0
RedWall_Part_94.Color 7
RedWall_Part_94.Visibility 1
RedWall_Part_94.Mother RedWall_Part_16

// (94a)
Volume RedWall_Part_94a
RedWall_Part_94a.Material Steel
RedWall_Part_94a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_94a.Position 13.075 10.0 0.0
RedWall_Part_94a.Color 7
RedWall_Part_94a.Visibility 1
RedWall_Part_94a.Mother RedWall_ThinWall_1


// (95)
Volume RedWall_Part_95
RedWall_Part_95.Material Steel
RedWall_Part_95.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_95.Position 0.0 0.1 0.0
RedWall_Part_95.Color 7
RedWall_Part_95.Visibility 1
RedWall_Part_95.Mother RedWall_Part_56

// (95a)
Volume RedWall_Part_95a
RedWall_Part_95a.Material Steel
RedWall_Part_95a.Shape TUBS 0.0 0.15 0.01 0. 360.
RedWall_Part_95a.Position 15.675 9.65 0.0
RedWall_Part_95a.Color 7
RedWall_Part_95a.Visibility 1
RedWall_Part_95a.Mother RedWall_ThinWall_1


// (96)
Volume RedWall_Part_96
RedWall_Part_96.Material Steel
RedWall_Part_96.Shape TUBS 0.0 0.15 0.19 0. 360.
RedWall_Part_96.Position -0.775 0.1 0.0
RedWall_Part_96.Color 7
RedWall_Part_96.Visibility 1
RedWall_Part_96.Mother RedWall_Part_52

// (96a)
Volume RedWall_Part_96a
RedWall_Part_96a.Material Steel
RedWall_Part_96a.Shape TUBS 0.0 0.15 0.01 0. 360.
RedWall_Part_96a.Position -12.15 9.65 0.0
RedWall_Part_96a.Color 7
RedWall_Part_96a.Visibility 1
RedWall_Part_96a.Mother RedWall_ThinWall_1




// The now following boringsa are NOT exactly dimensioned, positioned or both!!!

// Screws (100) and (101) have right dimensions, but are slightly shifted inside
// (100)
Volume RedWall_Part_100
RedWall_Part_100.Material Steel
RedWall_Part_100.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_100.Position -0.1 0.3 0.0
RedWall_Part_100.Color 7
RedWall_Part_100.Visibility 1
RedWall_Part_100.Mother RedWall_Part_65

// (100a)
Volume RedWall_Part_100a
RedWall_Part_100a.Material Steel
RedWall_Part_100a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_100a.Position 17.6 12.55 0.0
RedWall_Part_100a.Color 7
RedWall_Part_100a.Visibility 1
RedWall_Part_100a.Mother RedWall_ThinWall_1


// (101)
Volume RedWall_Part_101
RedWall_Part_101.Material Steel
RedWall_Part_101.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_101.Position 0.1 0.3 0.0
RedWall_Part_101.Color 7
RedWall_Part_101.Visibility 1
RedWall_Part_101.Mother RedWall_Part_63

// (101a)
Volume RedWall_Part_101a
RedWall_Part_101a.Material Steel
RedWall_Part_101a.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_101a.Position -17.6 12.55 0.0
RedWall_Part_101a.Color 7
RedWall_Part_101a.Visibility 1
RedWall_Part_101a.Mother RedWall_ThinWall_1


// Screws (102) to (105) are right positioned, but their diameter is reduced from 3mm to 2mm:

// (102)
Volume RedWall_Part_102
RedWall_Part_102.Material Steel
RedWall_Part_102.Shape TUBS 0.0 0.1 0.19 0. 360.
RedWall_Part_102.Position 1.85 0.0 0.0
RedWall_Part_102.Color 7
RedWall_Part_102.Visibility 1
RedWall_Part_102.Mother RedWall_Part_19

// (102a)
Volume RedWall_Part_102a
RedWall_Part_102a.Material Steel
RedWall_Part_102a.Shape TUBS 0.0 0.1 0.01 0. 360.
RedWall_Part_102a.Position -12.15 2.25 0.0
RedWall_Part_102a.Color 7
RedWall_Part_102a.Visibility 1
RedWall_Part_102a.Mother RedWall_ThinWall_1


// (103)
Volume RedWall_Part_103
RedWall_Part_103.Material Steel
RedWall_Part_103.Shape TUBS 0.0 0.1 0.19 0. 360.
RedWall_Part_103.Position 1.85 0.0 0.0
RedWall_Part_103.Color 7
RedWall_Part_103.Visibility 1
RedWall_Part_103.Mother RedWall_Part_20

// (103a)
Volume RedWall_Part_103a
RedWall_Part_103a.Material Steel
RedWall_Part_103a.Shape TUBS 0.0 0.1 0.01 0. 360.
RedWall_Part_103a.Position -12.15 -5.35 0.0
RedWall_Part_103a.Color 7
RedWall_Part_103a.Visibility 1
RedWall_Part_103a.Mother RedWall_ThinWall_1


// (104)
Volume RedWall_Part_104
RedWall_Part_104.Material Steel
RedWall_Part_104.Shape TUBS 0.0 0.1 0.19 0. 360.
RedWall_Part_104.Position 11.675 0.0 0.0
RedWall_Part_104.Color 7
RedWall_Part_104.Visibility 1
RedWall_Part_104.Mother RedWall_Part_17

// (104a)
Volume RedWall_Part_104a
RedWall_Part_104a.Material Steel
RedWall_Part_104a.Shape TUBS 0.0 0.1 0.01 0. 360.
RedWall_Part_104a.Position 15.675 2.25 0.0
RedWall_Part_104a.Color 7
RedWall_Part_104a.Visibility 1
RedWall_Part_104a.Mother RedWall_ThinWall_1


// (105)
Volume RedWall_Part_105
RedWall_Part_105.Material Steel
RedWall_Part_105.Shape TUBS 0.0 0.1 0.19 0. 360.
RedWall_Part_105.Position 11.675 0.0 0.0
RedWall_Part_105.Color 7
RedWall_Part_105.Visibility 1
RedWall_Part_105.Mother RedWall_Part_18

// (105a)
Volume RedWall_Part_105a
RedWall_Part_105a.Material Steel
RedWall_Part_105a.Shape TUBS 0.0 0.1 0.01 0. 360.
RedWall_Part_105a.Position 15.675 -5.35 0.0
RedWall_Part_105a.Color 7
RedWall_Part_105a.Visibility 1
RedWall_Part_105a.Mother RedWall_ThinWall_1


// Three screws within the triangle at the right front:
// the screws are not going through the thin layer on the bottom!
// their positions are optically approximanted and not totally exact!

// (106)
Volume RedWall_Part_106
RedWall_Part_106.Material Steel
RedWall_Part_106.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_106.Position 1.0 0.0 -1.2
RedWall_Part_106.Rotation 90. 0. 0.
RedWall_Part_106.Color 6
RedWall_Part_106.Visibility 1
RedWall_Part_106.Mother RedWall_Part_66

// (107)
Volume RedWall_Part_107
RedWall_Part_107.Material Steel
RedWall_Part_107.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_107.Position -1.0 0.0 -1.2
RedWall_Part_107.Rotation 90. 0. 0.
RedWall_Part_107.Color 8
RedWall_Part_107.Visibility 1
RedWall_Part_107.Mother RedWall_Part_66

// (108)
Volume RedWall_Part_108
RedWall_Part_108.Material Steel
RedWall_Part_108.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_108.Position 0.0 0.0 1.55
RedWall_Part_108.Rotation 90. 0. 0.
RedWall_Part_108.Color 7
RedWall_Part_108.Visibility 1
RedWall_Part_108.Mother RedWall_Part_66





// The area around screw 110 is changed from a diamond to a rectangular shape.
// the screw is right dimensioned, but shifted about a millimeter to the right and front (to the center).
// (110):
Volume RedWall_Part_110
RedWall_Part_110.Material Aluminium
RedWall_Part_110.Shape BRIK 0.35 0.3 0.19
RedWall_Part_110.Position -12.4 17.5 0.21
RedWall_Part_110.Color 1
RedWall_Part_110.Visibility 1
RedWall_Part_110.Mother RedWall_Volume

Volume RedWall_Part_110a
RedWall_Part_110a.Material Steel
RedWall_Part_110a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_110a.Position -0.15 0.1 0.0
RedWall_Part_110a.Color 7
RedWall_Part_110a.Visibility 1
RedWall_Part_110a.Mother RedWall_Part_110

Volume RedWall_Part_110b
RedWall_Part_110b.Material Steel
RedWall_Part_110b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_110b.Position -12.55 2.125 0.0
RedWall_Part_110b.Color 7
RedWall_Part_110b.Visibility 1
RedWall_Part_110b.Mother RedWall_ThinWall_2

// Shape of surrounding area shifted from triangular to rectangular.
// screw right dimensioned, position shifted to center.
// (111):
Volume RedWall_Part_111
RedWall_Part_111.Material Aluminium
RedWall_Part_111.Shape BRIK 0.35 0.3 0.19
RedWall_Part_111.Position 12.4 17.5 0.21
RedWall_Part_111.Color 1
RedWall_Part_111.Visibility 1
RedWall_Part_111.Mother RedWall_Volume

Volume RedWall_Part_111a
RedWall_Part_111a.Material Steel
RedWall_Part_111a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_111a.Position 0.15 0.1 0.0
RedWall_Part_111a.Color 7
RedWall_Part_111a.Visibility 1
RedWall_Part_111a.Mother RedWall_Part_111

Volume RedWall_Part_111b
RedWall_Part_111b.Material Steel
RedWall_Part_111b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_111b.Position 12.55 2.125 0.0
RedWall_Part_111b.Color 7
RedWall_Part_111b.Visibility 1
RedWall_Part_111b.Mother RedWall_ThinWall_2


// The shape of the area around screw 112 is changed from tiangular to rectangular,
// the dimension of the screw is right, its position shifted about 1 mm to the center.
// (112):
Volume RedWall_Part_112
RedWall_Part_112.Material Aluminium
RedWall_Part_112.Shape BRIK 0.3 0.3 0.19
RedWall_Part_112.Position -17.5 -12.45 0.21
RedWall_Part_112.Color 1
RedWall_Part_112.Visibility 1
RedWall_Part_112.Mother RedWall_Volume

Volume RedWall_Part_112a
RedWall_Part_112a.Material Steel
RedWall_Part_112a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_112a.Position -0.1 -0.1 0.0
RedWall_Part_112a.Color 7
RedWall_Part_112a.Visibility 1
RedWall_Part_112a.Mother RedWall_Part_112

Volume RedWall_Part_112b
RedWall_Part_112b.Material Steel
RedWall_Part_112b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_112b.Position -17.6 -12.55 0.0
RedWall_Part_112b.Color 7
RedWall_Part_112b.Visibility 1
RedWall_Part_112b.Mother RedWall_ThinWall_1

// position shifted to the right.
// (113):
Volume RedWall_Part_113
RedWall_Part_113.Material Aluminium
RedWall_Part_113.Shape BRIK 0.25 0.3 0.19
RedWall_Part_113.Position -12.5 -17.5 0.21
RedWall_Part_113.Color 1
RedWall_Part_113.Visibility 1
RedWall_Part_113.Mother RedWall_Volume

Volume RedWall_Part_113a
RedWall_Part_113a.Material Steel
RedWall_Part_113a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_113a.Position -0.05 -0.1 0.0
RedWall_Part_113a.Color 7
RedWall_Part_113a.Visibility 1
RedWall_Part_113a.Mother RedWall_Part_113

Volume RedWall_Part_113b
RedWall_Part_113b.Material Steel
RedWall_Part_113b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_113b.Position -12.55 -2.125 0.0
RedWall_Part_113b.Color 7
RedWall_Part_113b.Visibility 1
RedWall_Part_113b.Mother RedWall_ThinWall_3

// same changes as (111) and (112):
// (114):
Volume RedWall_Part_114
RedWall_Part_114.Material Aluminium
RedWall_Part_114.Shape BRIK 0.35 0.3 0.19
RedWall_Part_114.Position 12.4 -17.5 0.21
RedWall_Part_114.Color 1
RedWall_Part_114.Visibility 1
RedWall_Part_114.Mother RedWall_Volume

Volume RedWall_Part_114a
RedWall_Part_114a.Material Steel
RedWall_Part_114a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_114a.Position 0.15 -0.1 0.0
RedWall_Part_114a.Color 7
RedWall_Part_114a.Visibility 1
RedWall_Part_114a.Mother RedWall_Part_114

Volume RedWall_Part_114b
RedWall_Part_114b.Material Steel
RedWall_Part_114b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_114b.Position 12.55 -2.125 0.0
RedWall_Part_114b.Color 7
RedWall_Part_114b.Visibility 1
RedWall_Part_114b.Mother RedWall_ThinWall_3



// The shape of the area around screw 112 is changed from tiangular to rectangular,
// the dimension of the screw is right, its position shifted about 1 mm to the center.
// (115):
Volume RedWall_Part_115
RedWall_Part_115.Material Aluminium
RedWall_Part_115.Shape BRIK 0.3 0.3 0.19
RedWall_Part_115.Position 17.5 -12.45 0.21
RedWall_Part_115.Color 1
RedWall_Part_115.Visibility 1
RedWall_Part_115.Mother RedWall_Volume

Volume RedWall_Part_115a
RedWall_Part_115a.Material Steel
RedWall_Part_115a.Shape TUBS 0.0 0.2 0.19 0. 360.
RedWall_Part_115a.Position 0.1 -0.1 0.0
RedWall_Part_115a.Color 7
RedWall_Part_115a.Visibility 1
RedWall_Part_115a.Mother RedWall_Part_115

Volume RedWall_Part_115b
RedWall_Part_115b.Material Steel
RedWall_Part_115b.Shape TUBS 0.0 0.2 0.01 0. 360.
RedWall_Part_115b.Position 17.6 -12.55 0.0
RedWall_Part_115b.Color 7
RedWall_Part_115b.Visibility 1
RedWall_Part_115b.Mother RedWall_ThinWall_1