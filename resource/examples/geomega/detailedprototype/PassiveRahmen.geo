// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// The Mother Volume of the Detectot Configuration:
Volume PassiveFrame_Volume
PassiveFrame_Volume.Material Air
PassiveFrame_Volume.Visibility 0
PassiveFrame_Volume.Virtual true
PassiveFrame_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// PassiveFrame_Volume.Mother 0



// One big block from whish parts are cut away later (1)
Volume PassiveFrame_BigBlock_1
PassiveFrame_BigBlock_1.Material Peek
PassiveFrame_BigBlock_1.Shape BRIK 10.13 10.15 0.225
PassiveFrame_BigBlock_1.Position -0.32 0.3 0.225
PassiveFrame_BigBlock_1.Color 1
PassiveFrame_BigBlock_1.Visibility 1
PassiveFrame_BigBlock_1.Mother PassiveFrame_Volume



// 9 Holes directly under the Detectors:

// (2)
Volume PassiveFrame_Hole_2
PassiveFrame_Hole_2.Material Air
PassiveFrame_Hole_2.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_2.Position -6.13 6.16 0.0
PassiveFrame_Hole_2.Color 1
PassiveFrame_Hole_2.Visibility 1
PassiveFrame_Hole_2.Mother PassiveFrame_BigBlock_1

// (3)
Volume PassiveFrame_Hole_3
PassiveFrame_Hole_3.Material Air
PassiveFrame_Hole_3.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_3.Position 0.32 6.16 0.0
PassiveFrame_Hole_3.Color 1
PassiveFrame_Hole_3.Visibility 1
PassiveFrame_Hole_3.Mother PassiveFrame_BigBlock_1

// (4)
Volume PassiveFrame_Hole_4
PassiveFrame_Hole_4.Material Air
PassiveFrame_Hole_4.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_4.Position 6.77 6.16 0.0
PassiveFrame_Hole_4.Color 1
PassiveFrame_Hole_4.Visibility 1
PassiveFrame_Hole_4.Mother PassiveFrame_BigBlock_1

// (5)
Volume PassiveFrame_Hole_5
PassiveFrame_Hole_5.Material Air
PassiveFrame_Hole_5.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_5.Position -6.13 -0.3 0.0
PassiveFrame_Hole_5.Color 1
PassiveFrame_Hole_5.Visibility 1
PassiveFrame_Hole_5.Mother PassiveFrame_BigBlock_1

// (6)
Volume PassiveFrame_Hole_6
PassiveFrame_Hole_6.Material Air
PassiveFrame_Hole_6.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_6.Position 0.32 -0.3 0.0
PassiveFrame_Hole_6.Color 1
PassiveFrame_Hole_6.Visibility 1
PassiveFrame_Hole_6.Mother PassiveFrame_BigBlock_1

// (7)
Volume PassiveFrame_Hole_7
PassiveFrame_Hole_7.Material Air
PassiveFrame_Hole_7.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_7.Position 6.77 -0.3 0.0
PassiveFrame_Hole_7.Color 1
PassiveFrame_Hole_7.Visibility 1
PassiveFrame_Hole_7.Mother PassiveFrame_BigBlock_1

// (8)
Volume PassiveFrame_Hole_8
PassiveFrame_Hole_8.Material Air
PassiveFrame_Hole_8.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_8.Position -6.13 -6.76 0.0
PassiveFrame_Hole_8.Color 1
PassiveFrame_Hole_8.Visibility 1
PassiveFrame_Hole_8.Mother PassiveFrame_BigBlock_1

// (9)
Volume PassiveFrame_Hole_9
PassiveFrame_Hole_9.Material Air
PassiveFrame_Hole_9.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_9.Position 0.32 -6.76 0.0
PassiveFrame_Hole_9.Color 1
PassiveFrame_Hole_9.Visibility 1
PassiveFrame_Hole_9.Mother PassiveFrame_BigBlock_1

// (10)
Volume PassiveFrame_Hole_10
PassiveFrame_Hole_10.Material Air
PassiveFrame_Hole_10.Shape BRIK 3.09 3.15 0.225
PassiveFrame_Hole_10.Position 6.77 -6.76 0.0
PassiveFrame_Hole_10.Color 1
PassiveFrame_Hole_10.Visibility 1
PassiveFrame_Hole_10.Mother PassiveFrame_BigBlock_1




// (11):
Volume PassiveFrame_Part_11
PassiveFrame_Part_11.Material Peek
PassiveFrame_Part_11.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_11.Position -3.225 6.46 0.475
PassiveFrame_Part_11.Color 1
PassiveFrame_Part_11.Visibility 1
PassiveFrame_Part_11.Mother PassiveFrame_Volume

// (12):
Volume PassiveFrame_Part_12
PassiveFrame_Part_12.Material Peek
PassiveFrame_Part_12.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_12.Position 3.225 6.46 0.475
PassiveFrame_Part_12.Color 1
PassiveFrame_Part_12.Visibility 1
PassiveFrame_Part_12.Mother PassiveFrame_Volume

// (13):
Volume PassiveFrame_Part_13
PassiveFrame_Part_13.Material Peek
PassiveFrame_Part_13.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_13.Position 9.675 6.46 0.475
PassiveFrame_Part_13.Color 1
PassiveFrame_Part_13.Visibility 1
PassiveFrame_Part_13.Mother PassiveFrame_Volume

// (14):
Volume PassiveFrame_Part_14
PassiveFrame_Part_14.Material Peek
PassiveFrame_Part_14.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_14.Position -3.225 0.0 0.475
PassiveFrame_Part_14.Color 1
PassiveFrame_Part_14.Visibility 1
PassiveFrame_Part_14.Mother PassiveFrame_Volume

// (15):
Volume PassiveFrame_Part_15
PassiveFrame_Part_15.Material Peek
PassiveFrame_Part_15.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_15.Position 3.225 0.0 0.475
PassiveFrame_Part_15.Color 1
PassiveFrame_Part_15.Visibility 1
PassiveFrame_Part_15.Mother PassiveFrame_Volume

// (16):
Volume PassiveFrame_Part_16
PassiveFrame_Part_16.Material Peek
PassiveFrame_Part_16.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_16.Position 9.675 0.0 0.475
PassiveFrame_Part_16.Color 1
PassiveFrame_Part_16.Visibility 1
PassiveFrame_Part_16.Mother PassiveFrame_Volume

// (17):
Volume PassiveFrame_Part_17
PassiveFrame_Part_17.Material Peek
PassiveFrame_Part_17.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_17.Position -3.225 -6.46 0.475
PassiveFrame_Part_17.Color 1
PassiveFrame_Part_17.Visibility 1
PassiveFrame_Part_17.Mother PassiveFrame_Volume

// (18):
Volume PassiveFrame_Part_18
PassiveFrame_Part_18.Material Peek
PassiveFrame_Part_18.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_18.Position 3.225 -6.46 0.475
PassiveFrame_Part_18.Color 1
PassiveFrame_Part_18.Visibility 1
PassiveFrame_Part_18.Mother PassiveFrame_Volume

// (19):
Volume PassiveFrame_Part_19
PassiveFrame_Part_19.Material Peek
PassiveFrame_Part_19.Shape BRIK 0.135 3.15 0.025
PassiveFrame_Part_19.Position 9.675 -6.46 0.475
PassiveFrame_Part_19.Color 1
PassiveFrame_Part_19.Visibility 1
PassiveFrame_Part_19.Mother PassiveFrame_Volume





// (20):
Volume PassiveFrame_Part_20
PassiveFrame_Part_20.Material Peek
PassiveFrame_Part_20.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_20.Position -3.225 6.46 0.525
PassiveFrame_Part_20.Color 1
PassiveFrame_Part_20.Visibility 1
PassiveFrame_Part_20.Mother PassiveFrame_Volume

// (21):
Volume PassiveFrame_Part_21
PassiveFrame_Part_21.Material Peek
PassiveFrame_Part_21.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_21.Position 3.225 6.46 0.525
PassiveFrame_Part_21.Color 1
PassiveFrame_Part_21.Visibility 1
PassiveFrame_Part_21.Mother PassiveFrame_Volume

// (22):
Volume PassiveFrame_Part_22
PassiveFrame_Part_22.Material Peek
PassiveFrame_Part_22.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_22.Position 9.675 6.46 0.525
PassiveFrame_Part_22.Color 1
PassiveFrame_Part_22.Visibility 1
PassiveFrame_Part_22.Mother PassiveFrame_Volume

// (23):
Volume PassiveFrame_Part_23
PassiveFrame_Part_23.Material Peek
PassiveFrame_Part_23.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_23.Position -3.225 0.0 0.525
PassiveFrame_Part_23.Color 1
PassiveFrame_Part_23.Visibility 1
PassiveFrame_Part_23.Mother PassiveFrame_Volume

// (24):
Volume PassiveFrame_Part_24
PassiveFrame_Part_24.Material Peek
PassiveFrame_Part_24.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_24.Position 3.225 0.0 0.525
PassiveFrame_Part_24.Color 1
PassiveFrame_Part_24.Visibility 1
PassiveFrame_Part_24.Mother PassiveFrame_Volume

// (25):
Volume PassiveFrame_Part_25
PassiveFrame_Part_25.Material Peek
PassiveFrame_Part_25.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_25.Position 9.675 0.0 0.525
PassiveFrame_Part_25.Color 1
PassiveFrame_Part_25.Visibility 1
PassiveFrame_Part_25.Mother PassiveFrame_Volume

// (26):
Volume PassiveFrame_Part_26
PassiveFrame_Part_26.Material Peek
PassiveFrame_Part_26.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_26.Position -3.225 -6.46 0.525
PassiveFrame_Part_26.Color 1
PassiveFrame_Part_26.Visibility 1
PassiveFrame_Part_26.Mother PassiveFrame_Volume

// (27):
Volume PassiveFrame_Part_27
PassiveFrame_Part_27.Material Peek
PassiveFrame_Part_27.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_27.Position 3.225 -6.46 0.525
PassiveFrame_Part_27.Color 1
PassiveFrame_Part_27.Visibility 1
PassiveFrame_Part_27.Mother PassiveFrame_Volume

// (28):
Volume PassiveFrame_Part_28
PassiveFrame_Part_28.Material Peek
PassiveFrame_Part_28.Shape BRIK 0.08 2.75 0.025
PassiveFrame_Part_28.Position 9.675 -6.46 0.525
PassiveFrame_Part_28.Color 1
PassiveFrame_Part_28.Visibility 1
PassiveFrame_Part_28.Mother PassiveFrame_Volume





// (29):
Volume PassiveFrame_Part_29
PassiveFrame_Part_29.Material Peek
PassiveFrame_Part_29.Shape BRIK 10.1025 0.08 0.025
PassiveFrame_Part_29.Position -0.3475 3.23 0.525
PassiveFrame_Part_29.Color 1
PassiveFrame_Part_29.Visibility 1
PassiveFrame_Part_29.Mother PassiveFrame_Volume

// (30):
Volume PassiveFrame_Part_30
PassiveFrame_Part_30.Material Peek
PassiveFrame_Part_30.Shape BRIK 9.8025 0.08 0.025
PassiveFrame_Part_30.Position -0.0475 -3.23 0.525
PassiveFrame_Part_30.Color 1
PassiveFrame_Part_30.Visibility 1
PassiveFrame_Part_30.Mother PassiveFrame_Volume

// (31):
Volume PassiveFrame_Part_31
PassiveFrame_Part_31.Material Peek
PassiveFrame_Part_31.Shape BRIK 10.1025 0.08 0.025
PassiveFrame_Part_31.Position -0.3475 -9.69 0.525
PassiveFrame_Part_31.Color 1
PassiveFrame_Part_31.Visibility 1
PassiveFrame_Part_31.Mother PassiveFrame_Volume



// (32):
Volume PassiveFrame_Part_32
PassiveFrame_Part_32.Material Peek
PassiveFrame_Part_32.Shape BRIK 9.54 0.125 0.025
PassiveFrame_Part_32.Position 0.0 -9.735 0.475
PassiveFrame_Part_32.Color 1
PassiveFrame_Part_32.Visibility 1
PassiveFrame_Part_32.Mother PassiveFrame_Volume





// (35):
Volume PassiveFrame_Part_35
PassiveFrame_Part_35.Material Peek
PassiveFrame_Part_35.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_35.Position -6.45 3.23 0.475
PassiveFrame_Part_35.Color 1
PassiveFrame_Part_35.Visibility 1
PassiveFrame_Part_35.Mother PassiveFrame_Volume

// (36):
Volume PassiveFrame_Part_36
PassiveFrame_Part_36.Material Peek
PassiveFrame_Part_36.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_36.Position 0.0 3.23 0.475
PassiveFrame_Part_36.Color 1
PassiveFrame_Part_36.Visibility 1
PassiveFrame_Part_36.Mother PassiveFrame_Volume

// (37):
Volume PassiveFrame_Part_37
PassiveFrame_Part_37.Material Peek
PassiveFrame_Part_37.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_37.Position 6.45 3.23 0.475
PassiveFrame_Part_37.Color 1
PassiveFrame_Part_37.Visibility 1
PassiveFrame_Part_37.Mother PassiveFrame_Volume

// (38):
Volume PassiveFrame_Part_38
PassiveFrame_Part_38.Material Peek
PassiveFrame_Part_38.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_38.Position -6.45 -3.23 0.475
PassiveFrame_Part_38.Color 1
PassiveFrame_Part_38.Visibility 1
PassiveFrame_Part_38.Mother PassiveFrame_Volume

// (39):
Volume PassiveFrame_Part_39
PassiveFrame_Part_39.Material Peek
PassiveFrame_Part_39.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_39.Position 0.0 -3.23 0.475
PassiveFrame_Part_39.Color 1
PassiveFrame_Part_39.Visibility 1
PassiveFrame_Part_39.Mother PassiveFrame_Volume

// (40):
Volume PassiveFrame_Part_40
PassiveFrame_Part_40.Material Peek
PassiveFrame_Part_40.Shape BRIK 2.875 0.175 0.025
PassiveFrame_Part_40.Position +6.45 -3.23 0.475
PassiveFrame_Part_40.Color 1
PassiveFrame_Part_40.Visibility 1
PassiveFrame_Part_40.Mother PassiveFrame_Volume




// (41):
Volume PassiveFrame_Part_41
PassiveFrame_Part_41.Material Peek
PassiveFrame_Part_41.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_41.Position -6.45 -9.56 0.475
PassiveFrame_Part_41.Color 1
PassiveFrame_Part_41.Visibility 1
PassiveFrame_Part_41.Mother PassiveFrame_Volume

// (42):
Volume PassiveFrame_Part_42
PassiveFrame_Part_42.Material Peek
PassiveFrame_Part_42.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_42.Position 0.0 -9.56 0.475
PassiveFrame_Part_42.Color 1
PassiveFrame_Part_42.Visibility 1
PassiveFrame_Part_42.Mother PassiveFrame_Volume

// (43):
Volume PassiveFrame_Part_43
PassiveFrame_Part_43.Material Peek
PassiveFrame_Part_43.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_43.Position +6.45 -9.56 0.475
PassiveFrame_Part_43.Color 1
PassiveFrame_Part_43.Visibility 1
PassiveFrame_Part_43.Mother PassiveFrame_Volume




// (44):
Volume PassiveFrame_Part_44
PassiveFrame_Part_44.Material Peek
PassiveFrame_Part_44.Shape BRIK 0.425 2.75 0.025
PassiveFrame_Part_44.Position -10.025 6.46 0.525
PassiveFrame_Part_44.Color 1
PassiveFrame_Part_44.Visibility 1
PassiveFrame_Part_44.Mother PassiveFrame_Volume

// (45):
Volume PassiveFrame_Part_45
PassiveFrame_Part_45.Material Peek
PassiveFrame_Part_45.Shape BRIK 0.425 2.75 0.025
PassiveFrame_Part_45.Position -10.025 0.0 0.525
PassiveFrame_Part_45.Color 1
PassiveFrame_Part_45.Visibility 1
PassiveFrame_Part_45.Mother PassiveFrame_Volume

// (46):
Volume PassiveFrame_Part_46
PassiveFrame_Part_46.Material Peek
PassiveFrame_Part_46.Shape BRIK 0.425 2.75 0.025
PassiveFrame_Part_46.Position -10.025 -6.46 0.525
PassiveFrame_Part_46.Color 1
PassiveFrame_Part_46.Visibility 1
PassiveFrame_Part_46.Mother PassiveFrame_Volume




// (47):
Volume PassiveFrame_Part_47
PassiveFrame_Part_47.Material Peek
PassiveFrame_Part_47.Shape BRIK 0.3 0.2 0.025
PassiveFrame_Part_47.Position  -10.15 2.95 0.525
PassiveFrame_Part_47.Color 1
PassiveFrame_Part_47.Visibility 1
PassiveFrame_Part_47.Mother PassiveFrame_Volume

// (48):
//Volume PassiveFrame_Part_48
//PassiveFrame_Part_48.Material Peek
//PassiveFrame_Part_48.Shape BRIK 0.3 0.2 0.025
//PassiveFrame_Part_48.Position  -10.15 -2.95 0.525
//PassiveFrame_Part_48.Color 1
//PassiveFrame_Part_48.Visibility 1
//PassiveFrame_Part_48.Mother PassiveFrame_Volume

// (49):
Volume PassiveFrame_Part_49
PassiveFrame_Part_49.Material Peek
PassiveFrame_Part_49.Shape BRIK 0.3 0.2 0.025
PassiveFrame_Part_49.Position  -10.15 3.51 0.525
PassiveFrame_Part_49.Color 1
PassiveFrame_Part_49.Visibility 1
PassiveFrame_Part_49.Mother PassiveFrame_Volume


// Joined Part 50, Part 51 and the left and of Part 30 afterwards, for getting the screw-hole done:
// (50):
Volume PassiveFrame_Part_50
PassiveFrame_Part_50.Material Peek
PassiveFrame_Part_50.Shape BRIK 0.3 0.48 0.025
PassiveFrame_Part_50.Position  -10.15 -3.23 0.525
PassiveFrame_Part_50.Color 1
PassiveFrame_Part_50.Visibility 1
PassiveFrame_Part_50.Mother PassiveFrame_Volume

// (51):
Volume PassiveFrame_Part_51
PassiveFrame_Part_51.Material Peek
PassiveFrame_Part_51.Shape BRIK 0.3 0.2 0.025
PassiveFrame_Part_51.Position  -10.15 9.41 0.525
PassiveFrame_Part_51.Color 1
PassiveFrame_Part_51.Visibility 1
PassiveFrame_Part_51.Mother PassiveFrame_Volume

// (52):
Volume PassiveFrame_Part_52
PassiveFrame_Part_52.Material Peek
PassiveFrame_Part_52.Shape BRIK 0.3 0.2 0.025
PassiveFrame_Part_52.Position  -10.15 -9.41 0.525
PassiveFrame_Part_52.Color 1
PassiveFrame_Part_52.Visibility 1
PassiveFrame_Part_52.Mother PassiveFrame_Volume

// (53):
Volume PassiveFrame_Part_53
PassiveFrame_Part_53.Material Peek
PassiveFrame_Part_53.Shape BRIK 0.3 0.2 0.025
PassiveFrame_Part_53.Position  -10.15 -9.97 0.525
PassiveFrame_Part_53.Color 1
PassiveFrame_Part_53.Visibility 1
PassiveFrame_Part_53.Mother PassiveFrame_Volume


// (54):
Volume PassiveFrame_Part_54
PassiveFrame_Part_54.Material Peek
PassiveFrame_Part_54.Shape BRIK 0.425 1.69 0.025
PassiveFrame_Part_54.Position  -10.025 -11.86 0.525
PassiveFrame_Part_54.Color 2
PassiveFrame_Part_54.Visibility 1
PassiveFrame_Part_54.Mother PassiveFrame_Volume



// (55):
Volume PassiveFrame_Part_55
PassiveFrame_Part_55.Material Peek
PassiveFrame_Part_55.Shape BRIK 0.45 12.0 0.025
PassiveFrame_Part_55.Position  -10.0 -1.55 0.475
PassiveFrame_Part_55.Color 1
PassiveFrame_Part_55.Visibility 1
PassiveFrame_Part_55.Mother PassiveFrame_Volume

// (56):
Volume PassiveFrame_Part_56
PassiveFrame_Part_56.Material Peek
PassiveFrame_Part_56.Shape BRIK 11.55 0.42 0.05
PassiveFrame_Part_56.Position 2.0 10.03 0.5
PassiveFrame_Part_56.Color 1
PassiveFrame_Part_56.Visibility 1
PassiveFrame_Part_56.Mother PassiveFrame_Volume

// (57):
Volume PassiveFrame_Part_57
PassiveFrame_Part_57.Material Peek
PassiveFrame_Part_57.Shape BRIK 0.45 0.42 0.025
PassiveFrame_Part_57.Position -10.0 10.03 0.525
PassiveFrame_Part_57.Color 1
PassiveFrame_Part_57.Visibility 1
PassiveFrame_Part_57.Mother PassiveFrame_Volume



// (58):
Volume PassiveFrame_Part_58
PassiveFrame_Part_58.Material Peek
PassiveFrame_Part_58.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_58.Position -6.45 9.56 0.475
PassiveFrame_Part_58.Color 1
PassiveFrame_Part_58.Visibility 1
PassiveFrame_Part_58.Mother PassiveFrame_Volume

// (59):
Volume PassiveFrame_Part_59
PassiveFrame_Part_59.Material Peek
PassiveFrame_Part_59.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_59.Position 0.0 9.56 0.475
PassiveFrame_Part_59.Color 1
PassiveFrame_Part_59.Visibility 1
PassiveFrame_Part_59.Mother PassiveFrame_Volume

// (60):
Volume PassiveFrame_Part_60
PassiveFrame_Part_60.Material Peek
PassiveFrame_Part_60.Shape BRIK 2.875 0.05 0.025
PassiveFrame_Part_60.Position 6.45 9.56 0.475
PassiveFrame_Part_60.Color 1
PassiveFrame_Part_60.Visibility 1
PassiveFrame_Part_60.Mother PassiveFrame_Volume



// Long hole on sides:
// (61):
Volume PassiveFrame_Part_61
PassiveFrame_Part_61.Material Peek
PassiveFrame_Part_61.Shape BRIK 2.95 0.25 0.225
PassiveFrame_Part_61.Position -6.13 9.75 0.0
PassiveFrame_Part_61.Color 1
PassiveFrame_Part_61.Visibility 1
PassiveFrame_Part_61.Mother PassiveFrame_BigBlock_1

// (62):
Volume PassiveFrame_Part_62
PassiveFrame_Part_62.Material Peek
PassiveFrame_Part_62.Shape BRIK 2.95 0.25 0.225
PassiveFrame_Part_62.Position 0.32 9.75 0.0
PassiveFrame_Part_62.Color 1
PassiveFrame_Part_62.Visibility 1
PassiveFrame_Part_62.Mother PassiveFrame_BigBlock_1

// (63):
Volume PassiveFrame_Part_63
PassiveFrame_Part_63.Material Peek
PassiveFrame_Part_63.Shape BRIK 2.95 0.25 0.225
PassiveFrame_Part_63.Position 6.77 9.75 0.0
PassiveFrame_Part_63.Color 1
PassiveFrame_Part_63.Visibility 1
PassiveFrame_Part_63.Mother PassiveFrame_BigBlock_1

// (64):
Volume PassiveFrame_Part_64
PassiveFrame_Part_64.Material Peek
PassiveFrame_Part_64.Shape BRIK 0.25 2.95 0.225
PassiveFrame_Part_64.Position -9.73 6.15 0.0
PassiveFrame_Part_64.Color 1
PassiveFrame_Part_64.Visibility 1
PassiveFrame_Part_64.Mother PassiveFrame_BigBlock_1

// (65):
Volume PassiveFrame_Part_65
PassiveFrame_Part_65.Material Peek
PassiveFrame_Part_65.Shape BRIK 0.25 2.95 0.225
PassiveFrame_Part_65.Position -9.73 -0.3 0.0
PassiveFrame_Part_65.Color 1
PassiveFrame_Part_65.Visibility 1
PassiveFrame_Part_65.Mother PassiveFrame_BigBlock_1

// (66):
Volume PassiveFrame_Part_66
PassiveFrame_Part_66.Material Peek
PassiveFrame_Part_66.Shape BRIK 0.25 2.95 0.225
PassiveFrame_Part_66.Position -9.73 -6.75 0.0
PassiveFrame_Part_66.Color 1
PassiveFrame_Part_66.Visibility 1
PassiveFrame_Part_66.Mother PassiveFrame_BigBlock_1


// (67):
Volume PassiveFrame_Part_67
PassiveFrame_Part_67.Material Peek
PassiveFrame_Part_67.Shape BRIK 1.87 0.42 0.225
PassiveFrame_Part_67.Position 11.68 10.03 0.225
PassiveFrame_Part_67.Color 1
PassiveFrame_Part_67.Visibility 1
PassiveFrame_Part_67.Mother PassiveFrame_Volume

// (68):
Volume PassiveFrame_Part_68
PassiveFrame_Part_68.Material Peek
PassiveFrame_Part_68.Shape BRIK 1.3 0.25 0.225
PassiveFrame_Part_68.Position -0.33 0.0 0.0
PassiveFrame_Part_68.Color 1
PassiveFrame_Part_68.Visibility 1
PassiveFrame_Part_68.Mother PassiveFrame_Part_67


// (69):
Volume PassiveFrame_Part_69
PassiveFrame_Part_69.Material Peek
PassiveFrame_Part_69.Shape BRIK 0.4 1.85 0.225
PassiveFrame_Part_69.Position -10.05 -11.7 0.225
PassiveFrame_Part_69.Color 1
PassiveFrame_Part_69.Visibility 1
PassiveFrame_Part_69.Mother PassiveFrame_Volume

// (70):
Volume PassiveFrame_Part_70
PassiveFrame_Part_70.Material Peek
PassiveFrame_Part_70.Shape BRIK 0.25 1.3 0.225
PassiveFrame_Part_70.Position 0.0 0.3 0.0
PassiveFrame_Part_70.Color 1
PassiveFrame_Part_70.Visibility 1
PassiveFrame_Part_70.Mother PassiveFrame_Part_69


// (71):
Volume PassiveFrame_Part_71
PassiveFrame_Part_71.Material Peek
PassiveFrame_Part_71.Shape BRIK 1.675 0.05 0.025
PassiveFrame_Part_71.Position 11.485 9.56 0.475
PassiveFrame_Part_71.Color 1
PassiveFrame_Part_71.Visibility 1
PassiveFrame_Part_71.Mother PassiveFrame_Volume


// (72):
Volume PassiveFrame_Part_72
PassiveFrame_Part_72.Material Peek
PassiveFrame_Part_72.Shape BRIK 0.075 8.7 0.275
PassiveFrame_Part_72.Position 13.475 0.91 0.275
PassiveFrame_Part_72.Color 1
PassiveFrame_Part_72.Visibility 1
PassiveFrame_Part_72.Mother PassiveFrame_Volume

// (73):
Volume PassiveFrame_Part_73
PassiveFrame_Part_73.Material Peek
PassiveFrame_Part_73.Shape BRIK 0.075 8.7 0.05
PassiveFrame_Part_73.Position 13.325 0.91 0.5
PassiveFrame_Part_73.Color 1
PassiveFrame_Part_73.Visibility 1
PassiveFrame_Part_73.Mother PassiveFrame_Volume

// (74):
Volume PassiveFrame_Part_74
PassiveFrame_Part_74.Material Peek
PassiveFrame_Part_74.Shape BRIK 0.045 8.7 0.025
PassiveFrame_Part_74.Position 13.205 0.91 0.475
PassiveFrame_Part_74.Color 1
PassiveFrame_Part_74.Visibility 1
PassiveFrame_Part_74.Mother PassiveFrame_Volume

// (75):
Volume PassiveFrame_Part_75
PassiveFrame_Part_75.Material Peek
PassiveFrame_Part_75.Shape BRIK 0.15 0.15 0.25
PassiveFrame_Part_75.Position 11.5 9.36 0.25
PassiveFrame_Part_75.Color 1
PassiveFrame_Part_75.Visibility 1
PassiveFrame_Part_75.Mother PassiveFrame_Volume


// (76):
Volume PassiveFrame_Part_76
PassiveFrame_Part_76.Material Peek
PassiveFrame_Part_76.Shape BRIK 0.4 0.075 0.25
PassiveFrame_Part_76.Position 10.21 3.23 0.25
PassiveFrame_Part_76.Color 1
PassiveFrame_Part_76.Visibility 1
PassiveFrame_Part_76.Mother PassiveFrame_Volume

// (77):
Volume PassiveFrame_Part_77
PassiveFrame_Part_77.Material Peek
PassiveFrame_Part_77.Shape BRIK 0.175 0.175 0.25
PassiveFrame_Part_77.Position 10.785 3.23 0.25
PassiveFrame_Part_77.Color 1
PassiveFrame_Part_77.Visibility 1
PassiveFrame_Part_77.Mother PassiveFrame_Volume

// (78):
Volume PassiveFrame_Part_78
PassiveFrame_Part_78.Material Peek
PassiveFrame_Part_78.Shape BRIK 0.975 0.175 0.2
PassiveFrame_Part_78.Position 11.935 3.23 0.2
PassiveFrame_Part_78.Color 1
PassiveFrame_Part_78.Visibility 1
PassiveFrame_Part_78.Mother PassiveFrame_Volume

// (79):
Volume PassiveFrame_Part_79
PassiveFrame_Part_79.Material Peek
PassiveFrame_Part_79.Shape BRIK 0.125 0.175 0.25
PassiveFrame_Part_79.Position 13.035 3.23 0.25
PassiveFrame_Part_79.Color 1
PassiveFrame_Part_79.Visibility 1
PassiveFrame_Part_79.Mother PassiveFrame_Volume


// (80):
Volume PassiveFrame_Part_80
PassiveFrame_Part_80.Material Peek
PassiveFrame_Part_80.Shape BRIK 0.4 0.075 0.25
PassiveFrame_Part_80.Position 10.21 -3.23 0.25
PassiveFrame_Part_80.Color 1
PassiveFrame_Part_80.Visibility 1
PassiveFrame_Part_80.Mother PassiveFrame_Volume

// (81):
Volume PassiveFrame_Part_81
PassiveFrame_Part_81.Material Peek
PassiveFrame_Part_81.Shape BRIK 0.175 0.175 0.25
PassiveFrame_Part_81.Position 10.785 -3.23 0.25
PassiveFrame_Part_81.Color 1
PassiveFrame_Part_81.Visibility 1
PassiveFrame_Part_81.Mother PassiveFrame_Volume

// (82):
Volume PassiveFrame_Part_82
PassiveFrame_Part_82.Material Peek
PassiveFrame_Part_82.Shape BRIK 0.975 0.175 0.2
PassiveFrame_Part_82.Position 11.935 -3.23 0.2
PassiveFrame_Part_82.Color 1
PassiveFrame_Part_82.Visibility 1
PassiveFrame_Part_82.Mother PassiveFrame_Volume

// (83):
Volume PassiveFrame_Part_83
PassiveFrame_Part_83.Material Peek
PassiveFrame_Part_83.Shape BRIK 0.125 0.175 0.25
PassiveFrame_Part_83.Position 13.035 -3.23 0.25
PassiveFrame_Part_83.Color 1
PassiveFrame_Part_83.Visibility 1
PassiveFrame_Part_83.Mother PassiveFrame_Volume


// (84):
Volume PassiveFrame_Part_84
PassiveFrame_Part_84.Material Peek
PassiveFrame_Part_84.Shape BRIK 0.12 0.175 0.225
PassiveFrame_Part_84.Position 13.28 3.23 0.225
PassiveFrame_Part_84.Color 1
PassiveFrame_Part_84.Visibility 1
PassiveFrame_Part_84.Mother PassiveFrame_Volume

// (85):
Volume PassiveFrame_Part_85
PassiveFrame_Part_85.Material Peek
PassiveFrame_Part_85.Shape BRIK 0.12 0.175 0.225
PassiveFrame_Part_85.Position 13.28 -3.23 0.225
PassiveFrame_Part_85.Color 1
PassiveFrame_Part_85.Visibility 1
PassiveFrame_Part_85.Mother PassiveFrame_Volume


// (86):
Volume PassiveFrame_Part_86
PassiveFrame_Part_86.Material Peek
PassiveFrame_Part_86.Shape BRIK 0.15 0.15 0.25
PassiveFrame_Part_86.Position -9.4 -11.5 0.25
PassiveFrame_Part_86.Color 1
PassiveFrame_Part_86.Visibility 1
PassiveFrame_Part_86.Mother PassiveFrame_Volume



// (87):
Volume PassiveFrame_Part_87
PassiveFrame_Part_87.Material Peek
PassiveFrame_Part_87.Shape BRIK 11.1 0.075 0.275
PassiveFrame_Part_87.Position 1.55 -13.475 0.275
PassiveFrame_Part_87.Color 1
PassiveFrame_Part_87.Visibility 1
PassiveFrame_Part_87.Mother PassiveFrame_Volume

// (88):
Volume PassiveFrame_Part_88
PassiveFrame_Part_88.Material Peek
PassiveFrame_Part_88.Shape BRIK 0.05 0.075 0.225
PassiveFrame_Part_88.Position -9.6 -13.475 0.225
PassiveFrame_Part_88.Color 1
PassiveFrame_Part_88.Visibility 1
PassiveFrame_Part_88.Mother PassiveFrame_Volume


// (89):
Volume PassiveFrame_Part_89
PassiveFrame_Part_89.Material Peek
PassiveFrame_Part_89.Shape BRIK 10.725 0.075 0.025
PassiveFrame_Part_89.Position 1.525 -13.325 0.525
PassiveFrame_Part_89.Color 1
PassiveFrame_Part_89.Visibility 1
PassiveFrame_Part_89.Mother PassiveFrame_Volume

// (90):
Volume PassiveFrame_Part_90
PassiveFrame_Part_90.Material Peek
PassiveFrame_Part_90.Shape BRIK 11.1 0.125 0.025
PassiveFrame_Part_90.Position 1.55 -13.275 0.475
PassiveFrame_Part_90.Color 1
PassiveFrame_Part_90.Visibility 1
PassiveFrame_Part_90.Mother PassiveFrame_Volume



// (91):
Volume PassiveFrame_Part_91
PassiveFrame_Part_91.Material Peek
PassiveFrame_Part_91.Shape BRIK 0.175 0.525 0.25
PassiveFrame_Part_91.Position -3.225 -10.385 0.25
PassiveFrame_Part_91.Color 1
PassiveFrame_Part_91.Visibility 1
PassiveFrame_Part_91.Mother PassiveFrame_Volume

// (92):
Volume PassiveFrame_Part_92
PassiveFrame_Part_92.Material Peek
PassiveFrame_Part_92.Shape BRIK 0.175 0.975 0.2
PassiveFrame_Part_92.Position -3.225 -11.885 0.2
PassiveFrame_Part_92.Color 1
PassiveFrame_Part_92.Visibility 1
PassiveFrame_Part_92.Mother PassiveFrame_Volume

// (93):
Volume PassiveFrame_Part_93
PassiveFrame_Part_93.Material Peek
PassiveFrame_Part_93.Shape BRIK 0.175 0.145 0.25
PassiveFrame_Part_93.Position -3.225 -13.005 0.25
PassiveFrame_Part_93.Color 1
PassiveFrame_Part_93.Visibility 1
PassiveFrame_Part_93.Mother PassiveFrame_Volume


// (94):
Volume PassiveFrame_Part_94
PassiveFrame_Part_94.Material Peek
PassiveFrame_Part_94.Shape BRIK 0.175 0.525 0.25
PassiveFrame_Part_94.Position 3.225 -10.385 0.25
PassiveFrame_Part_94.Color 1
PassiveFrame_Part_94.Visibility 1
PassiveFrame_Part_94.Mother PassiveFrame_Volume

// (95):
Volume PassiveFrame_Part_95
PassiveFrame_Part_95.Material Peek
PassiveFrame_Part_95.Shape BRIK 0.175 0.975 0.2
PassiveFrame_Part_95.Position 3.225 -11.885 0.2
PassiveFrame_Part_95.Color 1
PassiveFrame_Part_95.Visibility 1
PassiveFrame_Part_95.Mother PassiveFrame_Volume

// (96):
Volume PassiveFrame_Part_96
PassiveFrame_Part_96.Material Peek
PassiveFrame_Part_96.Shape BRIK 0.175 0.145 0.25
PassiveFrame_Part_96.Position 3.225 -13.005 0.25
PassiveFrame_Part_96.Color 1
PassiveFrame_Part_96.Visibility 1
PassiveFrame_Part_96.Mother PassiveFrame_Volume


// (97):
Volume PassiveFrame_Part_97
PassiveFrame_Part_97.Material Peek
PassiveFrame_Part_97.Shape BRIK 0.45 0.45 0.275
PassiveFrame_Part_97.Position 13.1 -13.1 0.275
PassiveFrame_Part_97.Color 1
PassiveFrame_Part_97.Visibility 1
PassiveFrame_Part_97.Mother PassiveFrame_Volume



// (98):
Volume PassiveFrame_Part_98
PassiveFrame_Part_98.Material Peek
PassiveFrame_Part_98.Shape BRIK 0.4 1.97 0.25
PassiveFrame_Part_98.Position 11.23 -11.23 0.25
PassiveFrame_Part_98.Rotation 0.0 0.0 45.0
PassiveFrame_Part_98.Color 1
PassiveFrame_Part_98.Visibility 1
PassiveFrame_Part_98.Mother PassiveFrame_Volume

// (99):
Volume PassiveFrame_Part_99
PassiveFrame_Part_99.Material Peek
PassiveFrame_Part_99.Shape BRIK 0.1 1.7 0.25
PassiveFrame_Part_99.Position 12.06 -9.08 0.25
PassiveFrame_Part_99.Rotation 0.0 0.0 -45.0
PassiveFrame_Part_99.Color 1
PassiveFrame_Part_99.Visibility 1
PassiveFrame_Part_99.Mother PassiveFrame_Volume



// Now come the cylinders for the big rods:

// rod in upper right corner:
// (100):
Volume PassiveFrame_Part_100
PassiveFrame_Part_100.Material Steel
PassiveFrame_Part_100.Shape TUBS 0.0 0.225 0.225 0.0 360.0
PassiveFrame_Part_100.Position 1.42 0.0 0.0
PassiveFrame_Part_100.Color 4
PassiveFrame_Part_100.Visibility 1
PassiveFrame_Part_100.Mother PassiveFrame_Part_67

// (101):
Volume PassiveFrame_Part_101
PassiveFrame_Part_101.Material Steel
PassiveFrame_Part_101.Shape TUBS 0.0 0.225 0.05 0.0 360.0
PassiveFrame_Part_101.Position 11.1 0.0 0.0
PassiveFrame_Part_101.Color 4
PassiveFrame_Part_101.Visibility 1
PassiveFrame_Part_101.Mother PassiveFrame_Part_56


// rod in upper left corner:
// (102):
Volume PassiveFrame_Part_102
PassiveFrame_Part_102.Material Steel
PassiveFrame_Part_102.Shape TUBS 0.0 0.225 0.225 0.0 360.0
PassiveFrame_Part_102.Position -9.68 9.7  0.0
PassiveFrame_Part_102.Color 4
PassiveFrame_Part_102.Visibility 1
PassiveFrame_Part_102.Mother PassiveFrame_BigBlock_1

// (103):
Volume PassiveFrame_Part_103
PassiveFrame_Part_103.Material Steel
PassiveFrame_Part_103.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_103.Position 0.0 11.55 0.0
PassiveFrame_Part_103.Color 4
PassiveFrame_Part_103.Visibility 1
PassiveFrame_Part_103.Mother PassiveFrame_Part_55

// (104):
Volume PassiveFrame_Part_104
PassiveFrame_Part_104.Material Steel
PassiveFrame_Part_104.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_104.Position 0.0 -0.03 0.0
PassiveFrame_Part_104.Color 4
PassiveFrame_Part_104.Visibility 1
PassiveFrame_Part_104.Mother PassiveFrame_Part_57


// lower left rod:
// (105):
Volume PassiveFrame_Part_105
PassiveFrame_Part_105.Material Steel
PassiveFrame_Part_105.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_105.Position 0.025 -1.24 0.0
PassiveFrame_Part_105.Color 4
PassiveFrame_Part_105.Visibility 1
PassiveFrame_Part_105.Mother PassiveFrame_Part_54

// (106):
Volume PassiveFrame_Part_106
PassiveFrame_Part_106.Material Steel
PassiveFrame_Part_106.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_106.Position 0.0 -11.55 0.0
PassiveFrame_Part_106.Color 4
PassiveFrame_Part_106.Visibility 1
PassiveFrame_Part_106.Mother PassiveFrame_Part_55

// (107):
Volume PassiveFrame_Part_107
PassiveFrame_Part_107.Material Steel
PassiveFrame_Part_107.Shape TUBS 0.0 0.225 0.225 0.0 360.0
PassiveFrame_Part_107.Position 0.05 -1.4 0.0
PassiveFrame_Part_107.Color 4
PassiveFrame_Part_107.Visibility 1
PassiveFrame_Part_107.Mother PassiveFrame_Part_69


// lower right rod:
// (108):
Volume PassiveFrame_Part_108
PassiveFrame_Part_108.Material Steel
PassiveFrame_Part_108.Shape TUBS 0.0 0.225 0.275 0.0 360.0
PassiveFrame_Part_108.Position 0.0 0.0 0.0
PassiveFrame_Part_108.Color 4
PassiveFrame_Part_108.Visibility 1
PassiveFrame_Part_108.Mother PassiveFrame_Part_97

// Now empty rod-holes:

// upper rod-hole:
// (109):
Volume PassiveFrame_Part_109
PassiveFrame_Part_109.Material Air
PassiveFrame_Part_109.Shape TUBS 0.0 0.225 0.05 0.0 360.0
PassiveFrame_Part_109.Position 1.225 0.0 0.0
PassiveFrame_Part_109.Color 6
PassiveFrame_Part_109.Visibility 1
PassiveFrame_Part_109.Mother PassiveFrame_Part_56

// (110):
Volume PassiveFrame_Part_110
PassiveFrame_Part_110.Material Air
PassiveFrame_Part_110.Shape TUBS 0.0 0.225 0.225 0.0 360.0
PassiveFrame_Part_110.Position 3.545 9.73 0.0
PassiveFrame_Part_110.Color 6
PassiveFrame_Part_110.Visibility 1
PassiveFrame_Part_110.Mother PassiveFrame_BigBlock_1


// left rod-hole:
// note that this one is shifted slightly to the left (ca. 1 mm) from the real position,
// because that was easier to realize:
// (111):
Volume PassiveFrame_Part_111
PassiveFrame_Part_111.Material Air
PassiveFrame_Part_111.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_111.Position 0.0 0.0 0.0
PassiveFrame_Part_111.Color 6
PassiveFrame_Part_111.Visibility 1
PassiveFrame_Part_111.Mother PassiveFrame_Part_50

// (112):
Volume PassiveFrame_Part_112
PassiveFrame_Part_112.Material Air
PassiveFrame_Part_112.Shape TUBS 0.0 0.225 0.025 0.0 360.0
PassiveFrame_Part_112.Position -0.15 -1.68 0.0
PassiveFrame_Part_112.Color 6
PassiveFrame_Part_112.Visibility 1
PassiveFrame_Part_112.Mother PassiveFrame_Part_55

// (113):
Volume PassiveFrame_Part_113
PassiveFrame_Part_113.Material Air
PassiveFrame_Part_113.Shape TUBS 0.0 0.225 0.225 0.0 360.0
PassiveFrame_Part_113.Position -9.83 -3.53 0.0
PassiveFrame_Part_113.Color 6
PassiveFrame_Part_113.Visibility 1
PassiveFrame_Part_113.Mother PassiveFrame_BigBlock_1



// Now the little screws along right and lower side:

// (114):
Volume PassiveFrame_Part_114
PassiveFrame_Part_114.Material Air
PassiveFrame_Part_114.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_114.Position 0.0 0.0 0.0
PassiveFrame_Part_114.Color 4
PassiveFrame_Part_114.Visibility 1
PassiveFrame_Part_114.Mother PassiveFrame_Part_75

// (115):
Volume PassiveFrame_Part_115
PassiveFrame_Part_115.Material Air
PassiveFrame_Part_115.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_115.Position 0.0 0.0 0.0
PassiveFrame_Part_115.Color 4
PassiveFrame_Part_115.Visibility 1
PassiveFrame_Part_115.Mother PassiveFrame_Part_79

// (116):
Volume PassiveFrame_Part_116
PassiveFrame_Part_116.Material Air
PassiveFrame_Part_116.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_116.Position 0.0 0.0 0.0
PassiveFrame_Part_116.Color 4
PassiveFrame_Part_116.Visibility 1
PassiveFrame_Part_116.Mother PassiveFrame_Part_83

// (117):
Volume PassiveFrame_Part_117
PassiveFrame_Part_117.Material Air
PassiveFrame_Part_117.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_117.Position 0.0 0.0 0.0
PassiveFrame_Part_117.Color 4
PassiveFrame_Part_117.Visibility 1
PassiveFrame_Part_117.Mother PassiveFrame_Part_86

// (118):
Volume PassiveFrame_Part_118
PassiveFrame_Part_118.Material Air
PassiveFrame_Part_118.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_118.Position 0.0 0.0 0.0
PassiveFrame_Part_118.Color 4
PassiveFrame_Part_118.Visibility 1
PassiveFrame_Part_118.Mother PassiveFrame_Part_93

// (119):
Volume PassiveFrame_Part_119
PassiveFrame_Part_119.Material Air
PassiveFrame_Part_119.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_119.Position 0.0 0.0 0.0
PassiveFrame_Part_119.Color 4
PassiveFrame_Part_119.Visibility 1
PassiveFrame_Part_119.Mother PassiveFrame_Part_96


// (120):
Volume PassiveFrame_Part_120
PassiveFrame_Part_120.Material Air
PassiveFrame_Part_120.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_120.Position 0.025 0.0 0.0
PassiveFrame_Part_120.Color 4
PassiveFrame_Part_120.Visibility 1
PassiveFrame_Part_120.Mother PassiveFrame_Part_77

// (121):
Volume PassiveFrame_Part_121
PassiveFrame_Part_121.Material Air
PassiveFrame_Part_121.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_121.Position 0.025 0.0 0.0
PassiveFrame_Part_121.Color 4
PassiveFrame_Part_121.Visibility 1
PassiveFrame_Part_121.Mother PassiveFrame_Part_81

// (122):
Volume PassiveFrame_Part_122
PassiveFrame_Part_122.Material Air
PassiveFrame_Part_122.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_122.Position 0.0 -0.375 0.0
PassiveFrame_Part_122.Color 4
PassiveFrame_Part_122.Visibility 1
PassiveFrame_Part_122.Mother PassiveFrame_Part_91

// (123):
Volume PassiveFrame_Part_123
PassiveFrame_Part_123.Material Air
PassiveFrame_Part_123.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_123.Position 0.0 -0.375 0.0
PassiveFrame_Part_123.Color 4
PassiveFrame_Part_123.Visibility 1
PassiveFrame_Part_123.Mother PassiveFrame_Part_94


// (124):
Volume PassiveFrame_Part_124
PassiveFrame_Part_124.Material Air
PassiveFrame_Part_124.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_124.Position -0.25 0.0 0.0
PassiveFrame_Part_124.Color 4
PassiveFrame_Part_124.Visibility 1
PassiveFrame_Part_124.Mother PassiveFrame_Part_98

// (125):
Volume PassiveFrame_Part_125
PassiveFrame_Part_125.Material Air
PassiveFrame_Part_125.Shape TUBS 0.0 0.05 0.25 0.0 360.0
PassiveFrame_Part_125.Position 0.25 0.0 0.0
PassiveFrame_Part_125.Color 4
PassiveFrame_Part_125.Visibility 1
PassiveFrame_Part_125.Mother PassiveFrame_Part_98


// Now come the Siliconlayers:
// check the exact dimensions of the layers, their positions are OK:

// Basic Silicon PassiveWafer (226):
Volume Basic_Silicon_PassiveWafer
Basic_Silicon_PassiveWafer.Material SiliconPIN
Basic_Silicon_PassiveWafer.Shape BRIK 3.14 3.15 0.025
Basic_Silicon_PassiveWafer.Color 5


// (126):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_126
PassiveWafer_126.Position -6.45 -6.46 0.525
PassiveWafer_126.Visibility 1
PassiveWafer_126.Mother PassiveFrame_Volume

// (127):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_127
PassiveWafer_127.Position 0.0 -6.46 0.525
PassiveWafer_127.Visibility 1
PassiveWafer_127.Mother PassiveFrame_Volume

// (128):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_128
PassiveWafer_128.Position +6.45 -6.46 0.525
PassiveWafer_128.Visibility 1
PassiveWafer_128.Mother PassiveFrame_Volume

// (129):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_129
PassiveWafer_129.Position -6.45 0.0 0.525
PassiveWafer_129.Visibility 1
PassiveWafer_129.Mother PassiveFrame_Volume

// (130):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_130
PassiveWafer_130.Position 0.0 0.0 0.525
PassiveWafer_130.Visibility 1
PassiveWafer_130.Mother PassiveFrame_Volume

// (131):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_131
PassiveWafer_131.Position +6.45 0.0 0.525
PassiveWafer_131.Visibility 1
PassiveWafer_131.Mother PassiveFrame_Volume

// (132):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_132
PassiveWafer_132.Position -6.45 6.46 0.525
PassiveWafer_132.Visibility 1
PassiveWafer_132.Mother PassiveFrame_Volume

// (133):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_133
PassiveWafer_133.Position 0.0 6.46 0.525
PassiveWafer_133.Visibility 1
PassiveWafer_133.Mother PassiveFrame_Volume

// (134):
Basic_Silicon_PassiveWafer.Copy PassiveWafer_134
PassiveWafer_134.Position +6.45 6.46 0.525
PassiveWafer_134.Visibility 1
PassiveWafer_134.Mother PassiveFrame_Volume



// The Boards on the right und lower side:

// right board:
// (135):
Volume PassiveFrame_Part_135
PassiveFrame_Part_135.Material PCB
PassiveFrame_Part_135.Shape BRIK 1.7475 11.1245 0.025
PassiveFrame_Part_135.Position 11.5025 -1.5245 0.525
PassiveFrame_Part_135.Color 3
PassiveFrame_Part_135.Visibility 1
PassiveFrame_Part_135.Mother PassiveFrame_Volume

// lower board:
// (136):
Volume PassiveFrame_Part_136
PassiveFrame_Part_136.Material PCB
PassiveFrame_Part_136.Shape BRIK 9.6775 1.74 0.025
PassiveFrame_Part_136.Position 0.0775 -11.51 0.525
PassiveFrame_Part_136.Color 3
PassiveFrame_Part_136.Visibility 1
PassiveFrame_Part_136.Mother PassiveFrame_Volume

// some space still to fill:
// (137):
Volume PassiveFrame_Part_137
PassiveFrame_Part_137.Material PCB
PassiveFrame_Part_137.Shape BRIK 1.4375 0.3 0.025
PassiveFrame_Part_137.Position 11.1925 -12.95 0.525
PassiveFrame_Part_137.Color 3
PassiveFrame_Part_137.Visibility 1
PassiveFrame_Part_137.Mother PassiveFrame_Volume

