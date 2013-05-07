// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// The Mother Volume of the Detectot Configuration:
Volume Frame_Volume
Frame_Volume.Material Air
Frame_Volume.Visibility 0
Frame_Volume.Virtual true
Frame_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// Frame_Volume.Mother 0



// One big block from whish parts are cut away later (1)
Volume Frame_BigBlock_1
Frame_BigBlock_1.Material Peek
Frame_BigBlock_1.Shape BRIK 10.13 10.15 0.225
Frame_BigBlock_1.Position -0.32 0.3 0.225
Frame_BigBlock_1.Color 1
Frame_BigBlock_1.Visibility 1
Frame_BigBlock_1.Mother Frame_Volume



// 9 Holes directly under the Detectors:

// (2)
Volume Frame_Hole_2
Frame_Hole_2.Material Air
Frame_Hole_2.Shape BRIK 3.09 3.15 0.225
Frame_Hole_2.Position -6.13 6.16 0.0
Frame_Hole_2.Color 1
Frame_Hole_2.Visibility 1
Frame_Hole_2.Mother Frame_BigBlock_1

// (3)
Volume Frame_Hole_3
Frame_Hole_3.Material Air
Frame_Hole_3.Shape BRIK 3.09 3.15 0.225
Frame_Hole_3.Position 0.32 6.16 0.0
Frame_Hole_3.Color 1
Frame_Hole_3.Visibility 1
Frame_Hole_3.Mother Frame_BigBlock_1

// (4)
Volume Frame_Hole_4
Frame_Hole_4.Material Air
Frame_Hole_4.Shape BRIK 3.09 3.15 0.225
Frame_Hole_4.Position 6.77 6.16 0.0
Frame_Hole_4.Color 1
Frame_Hole_4.Visibility 1
Frame_Hole_4.Mother Frame_BigBlock_1

// (5)
Volume Frame_Hole_5
Frame_Hole_5.Material Air
Frame_Hole_5.Shape BRIK 3.09 3.15 0.225
Frame_Hole_5.Position -6.13 -0.3 0.0
Frame_Hole_5.Color 1
Frame_Hole_5.Visibility 1
Frame_Hole_5.Mother Frame_BigBlock_1

// (6)
Volume Frame_Hole_6
Frame_Hole_6.Material Air
Frame_Hole_6.Shape BRIK 3.09 3.15 0.225
Frame_Hole_6.Position 0.32 -0.3 0.0
Frame_Hole_6.Color 1
Frame_Hole_6.Visibility 1
Frame_Hole_6.Mother Frame_BigBlock_1

// (7)
Volume Frame_Hole_7
Frame_Hole_7.Material Air
Frame_Hole_7.Shape BRIK 3.09 3.15 0.225
Frame_Hole_7.Position 6.77 -0.3 0.0
Frame_Hole_7.Color 1
Frame_Hole_7.Visibility 1
Frame_Hole_7.Mother Frame_BigBlock_1

// (8)
Volume Frame_Hole_8
Frame_Hole_8.Material Air
Frame_Hole_8.Shape BRIK 3.09 3.15 0.225
Frame_Hole_8.Position -6.13 -6.76 0.0
Frame_Hole_8.Color 1
Frame_Hole_8.Visibility 1
Frame_Hole_8.Mother Frame_BigBlock_1

// (9)
Volume Frame_Hole_9
Frame_Hole_9.Material Air
Frame_Hole_9.Shape BRIK 3.09 3.15 0.225
Frame_Hole_9.Position 0.32 -6.76 0.0
Frame_Hole_9.Color 1
Frame_Hole_9.Visibility 1
Frame_Hole_9.Mother Frame_BigBlock_1

// (10)
Volume Frame_Hole_10
Frame_Hole_10.Material Air
Frame_Hole_10.Shape BRIK 3.09 3.15 0.225
Frame_Hole_10.Position 6.77 -6.76 0.0
Frame_Hole_10.Color 1
Frame_Hole_10.Visibility 1
Frame_Hole_10.Mother Frame_BigBlock_1




// (11):
Volume Frame_Part_11
Frame_Part_11.Material Peek
Frame_Part_11.Shape BRIK 0.135 3.15 0.025
Frame_Part_11.Position -3.225 6.46 0.475
Frame_Part_11.Color 1
Frame_Part_11.Visibility 1
Frame_Part_11.Mother Frame_Volume

// (12):
Volume Frame_Part_12
Frame_Part_12.Material Peek
Frame_Part_12.Shape BRIK 0.135 3.15 0.025
Frame_Part_12.Position 3.225 6.46 0.475
Frame_Part_12.Color 1
Frame_Part_12.Visibility 1
Frame_Part_12.Mother Frame_Volume

// (13):
Volume Frame_Part_13
Frame_Part_13.Material Peek
Frame_Part_13.Shape BRIK 0.135 3.15 0.025
Frame_Part_13.Position 9.675 6.46 0.475
Frame_Part_13.Color 1
Frame_Part_13.Visibility 1
Frame_Part_13.Mother Frame_Volume

// (14):
Volume Frame_Part_14
Frame_Part_14.Material Peek
Frame_Part_14.Shape BRIK 0.135 3.15 0.025
Frame_Part_14.Position -3.225 0.0 0.475
Frame_Part_14.Color 1
Frame_Part_14.Visibility 1
Frame_Part_14.Mother Frame_Volume

// (15):
Volume Frame_Part_15
Frame_Part_15.Material Peek
Frame_Part_15.Shape BRIK 0.135 3.15 0.025
Frame_Part_15.Position 3.225 0.0 0.475
Frame_Part_15.Color 1
Frame_Part_15.Visibility 1
Frame_Part_15.Mother Frame_Volume

// (16):
Volume Frame_Part_16
Frame_Part_16.Material Peek
Frame_Part_16.Shape BRIK 0.135 3.15 0.025
Frame_Part_16.Position 9.675 0.0 0.475
Frame_Part_16.Color 1
Frame_Part_16.Visibility 1
Frame_Part_16.Mother Frame_Volume

// (17):
Volume Frame_Part_17
Frame_Part_17.Material Peek
Frame_Part_17.Shape BRIK 0.135 3.15 0.025
Frame_Part_17.Position -3.225 -6.46 0.475
Frame_Part_17.Color 1
Frame_Part_17.Visibility 1
Frame_Part_17.Mother Frame_Volume

// (18):
Volume Frame_Part_18
Frame_Part_18.Material Peek
Frame_Part_18.Shape BRIK 0.135 3.15 0.025
Frame_Part_18.Position 3.225 -6.46 0.475
Frame_Part_18.Color 1
Frame_Part_18.Visibility 1
Frame_Part_18.Mother Frame_Volume

// (19):
Volume Frame_Part_19
Frame_Part_19.Material Peek
Frame_Part_19.Shape BRIK 0.135 3.15 0.025
Frame_Part_19.Position 9.675 -6.46 0.475
Frame_Part_19.Color 1
Frame_Part_19.Visibility 1
Frame_Part_19.Mother Frame_Volume





// (20):
Volume Frame_Part_20
Frame_Part_20.Material Peek
Frame_Part_20.Shape BRIK 0.08 2.75 0.025
Frame_Part_20.Position -3.225 6.46 0.525
Frame_Part_20.Color 1
Frame_Part_20.Visibility 1
Frame_Part_20.Mother Frame_Volume

// (21):
Volume Frame_Part_21
Frame_Part_21.Material Peek
Frame_Part_21.Shape BRIK 0.08 2.75 0.025
Frame_Part_21.Position 3.225 6.46 0.525
Frame_Part_21.Color 1
Frame_Part_21.Visibility 1
Frame_Part_21.Mother Frame_Volume

// (22):
Volume Frame_Part_22
Frame_Part_22.Material Peek
Frame_Part_22.Shape BRIK 0.08 2.75 0.025
Frame_Part_22.Position 9.675 6.46 0.525
Frame_Part_22.Color 1
Frame_Part_22.Visibility 1
Frame_Part_22.Mother Frame_Volume

// (23):
Volume Frame_Part_23
Frame_Part_23.Material Peek
Frame_Part_23.Shape BRIK 0.08 2.75 0.025
Frame_Part_23.Position -3.225 0.0 0.525
Frame_Part_23.Color 1
Frame_Part_23.Visibility 1
Frame_Part_23.Mother Frame_Volume

// (24):
Volume Frame_Part_24
Frame_Part_24.Material Peek
Frame_Part_24.Shape BRIK 0.08 2.75 0.025
Frame_Part_24.Position 3.225 0.0 0.525
Frame_Part_24.Color 1
Frame_Part_24.Visibility 1
Frame_Part_24.Mother Frame_Volume

// (25):
Volume Frame_Part_25
Frame_Part_25.Material Peek
Frame_Part_25.Shape BRIK 0.08 2.75 0.025
Frame_Part_25.Position 9.675 0.0 0.525
Frame_Part_25.Color 1
Frame_Part_25.Visibility 1
Frame_Part_25.Mother Frame_Volume

// (26):
Volume Frame_Part_26
Frame_Part_26.Material Peek
Frame_Part_26.Shape BRIK 0.08 2.75 0.025
Frame_Part_26.Position -3.225 -6.46 0.525
Frame_Part_26.Color 1
Frame_Part_26.Visibility 1
Frame_Part_26.Mother Frame_Volume

// (27):
Volume Frame_Part_27
Frame_Part_27.Material Peek
Frame_Part_27.Shape BRIK 0.08 2.75 0.025
Frame_Part_27.Position 3.225 -6.46 0.525
Frame_Part_27.Color 1
Frame_Part_27.Visibility 1
Frame_Part_27.Mother Frame_Volume

// (28):
Volume Frame_Part_28
Frame_Part_28.Material Peek
Frame_Part_28.Shape BRIK 0.08 2.75 0.025
Frame_Part_28.Position 9.675 -6.46 0.525
Frame_Part_28.Color 1
Frame_Part_28.Visibility 1
Frame_Part_28.Mother Frame_Volume





// (29):
Volume Frame_Part_29
Frame_Part_29.Material Peek
Frame_Part_29.Shape BRIK 10.1025 0.08 0.025
Frame_Part_29.Position -0.3475 3.23 0.525
Frame_Part_29.Color 1
Frame_Part_29.Visibility 1
Frame_Part_29.Mother Frame_Volume

// (30):
Volume Frame_Part_30
Frame_Part_30.Material Peek
Frame_Part_30.Shape BRIK 9.8025 0.08 0.025
Frame_Part_30.Position -0.0475 -3.23 0.525
Frame_Part_30.Color 1
Frame_Part_30.Visibility 1
Frame_Part_30.Mother Frame_Volume

// (31):
Volume Frame_Part_31
Frame_Part_31.Material Peek
Frame_Part_31.Shape BRIK 10.1025 0.08 0.025
Frame_Part_31.Position -0.3475 -9.69 0.525
Frame_Part_31.Color 1
Frame_Part_31.Visibility 1
Frame_Part_31.Mother Frame_Volume



// (32):
Volume Frame_Part_32
Frame_Part_32.Material Peek
Frame_Part_32.Shape BRIK 9.54 0.125 0.025
Frame_Part_32.Position 0.0 -9.735 0.475
Frame_Part_32.Color 1
Frame_Part_32.Visibility 1
Frame_Part_32.Mother Frame_Volume





// (35):
Volume Frame_Part_35
Frame_Part_35.Material Peek
Frame_Part_35.Shape BRIK 2.875 0.175 0.025
Frame_Part_35.Position -6.45 3.23 0.475
Frame_Part_35.Color 1
Frame_Part_35.Visibility 1
Frame_Part_35.Mother Frame_Volume

// (36):
Volume Frame_Part_36
Frame_Part_36.Material Peek
Frame_Part_36.Shape BRIK 2.875 0.175 0.025
Frame_Part_36.Position 0.0 3.23 0.475
Frame_Part_36.Color 1
Frame_Part_36.Visibility 1
Frame_Part_36.Mother Frame_Volume

// (37):
Volume Frame_Part_37
Frame_Part_37.Material Peek
Frame_Part_37.Shape BRIK 2.875 0.175 0.025
Frame_Part_37.Position 6.45 3.23 0.475
Frame_Part_37.Color 1
Frame_Part_37.Visibility 1
Frame_Part_37.Mother Frame_Volume

// (38):
Volume Frame_Part_38
Frame_Part_38.Material Peek
Frame_Part_38.Shape BRIK 2.875 0.175 0.025
Frame_Part_38.Position -6.45 -3.23 0.475
Frame_Part_38.Color 1
Frame_Part_38.Visibility 1
Frame_Part_38.Mother Frame_Volume

// (39):
Volume Frame_Part_39
Frame_Part_39.Material Peek
Frame_Part_39.Shape BRIK 2.875 0.175 0.025
Frame_Part_39.Position 0.0 -3.23 0.475
Frame_Part_39.Color 1
Frame_Part_39.Visibility 1
Frame_Part_39.Mother Frame_Volume

// (40):
Volume Frame_Part_40
Frame_Part_40.Material Peek
Frame_Part_40.Shape BRIK 2.875 0.175 0.025
Frame_Part_40.Position +6.45 -3.23 0.475
Frame_Part_40.Color 1
Frame_Part_40.Visibility 1
Frame_Part_40.Mother Frame_Volume




// (41):
Volume Frame_Part_41
Frame_Part_41.Material Peek
Frame_Part_41.Shape BRIK 2.875 0.05 0.025
Frame_Part_41.Position -6.45 -9.56 0.475
Frame_Part_41.Color 1
Frame_Part_41.Visibility 1
Frame_Part_41.Mother Frame_Volume

// (42):
Volume Frame_Part_42
Frame_Part_42.Material Peek
Frame_Part_42.Shape BRIK 2.875 0.05 0.025
Frame_Part_42.Position 0.0 -9.56 0.475
Frame_Part_42.Color 1
Frame_Part_42.Visibility 1
Frame_Part_42.Mother Frame_Volume

// (43):
Volume Frame_Part_43
Frame_Part_43.Material Peek
Frame_Part_43.Shape BRIK 2.875 0.05 0.025
Frame_Part_43.Position +6.45 -9.56 0.475
Frame_Part_43.Color 1
Frame_Part_43.Visibility 1
Frame_Part_43.Mother Frame_Volume




// (44):
Volume Frame_Part_44
Frame_Part_44.Material Peek
Frame_Part_44.Shape BRIK 0.425 2.75 0.025
Frame_Part_44.Position -10.025 6.46 0.525
Frame_Part_44.Color 1
Frame_Part_44.Visibility 1
Frame_Part_44.Mother Frame_Volume

// (45):
Volume Frame_Part_45
Frame_Part_45.Material Peek
Frame_Part_45.Shape BRIK 0.425 2.75 0.025
Frame_Part_45.Position -10.025 0.0 0.525
Frame_Part_45.Color 1
Frame_Part_45.Visibility 1
Frame_Part_45.Mother Frame_Volume

// (46):
Volume Frame_Part_46
Frame_Part_46.Material Peek
Frame_Part_46.Shape BRIK 0.425 2.75 0.025
Frame_Part_46.Position -10.025 -6.46 0.525
Frame_Part_46.Color 1
Frame_Part_46.Visibility 1
Frame_Part_46.Mother Frame_Volume




// (47):
Volume Frame_Part_47
Frame_Part_47.Material Peek
Frame_Part_47.Shape BRIK 0.3 0.2 0.025
Frame_Part_47.Position  -10.15 2.95 0.525
Frame_Part_47.Color 1
Frame_Part_47.Visibility 1
Frame_Part_47.Mother Frame_Volume

// (48):
//Volume Frame_Part_48
//Frame_Part_48.Material Peek
//Frame_Part_48.Shape BRIK 0.3 0.2 0.025
//Frame_Part_48.Position  -10.15 -2.95 0.525
//Frame_Part_48.Color 1
//Frame_Part_48.Visibility 1
//Frame_Part_48.Mother Frame_Volume

// (49):
Volume Frame_Part_49
Frame_Part_49.Material Peek
Frame_Part_49.Shape BRIK 0.3 0.2 0.025
Frame_Part_49.Position  -10.15 3.51 0.525
Frame_Part_49.Color 1
Frame_Part_49.Visibility 1
Frame_Part_49.Mother Frame_Volume


// Joined Part 50, Part 51 and the left and of Part 30 afterwards, for getting the screw-hole done:
// (50):
Volume Frame_Part_50
Frame_Part_50.Material Peek
Frame_Part_50.Shape BRIK 0.3 0.48 0.025
Frame_Part_50.Position  -10.15 -3.23 0.525
Frame_Part_50.Color 1
Frame_Part_50.Visibility 1
Frame_Part_50.Mother Frame_Volume

// (51):
Volume Frame_Part_51
Frame_Part_51.Material Peek
Frame_Part_51.Shape BRIK 0.3 0.2 0.025
Frame_Part_51.Position  -10.15 9.41 0.525
Frame_Part_51.Color 1
Frame_Part_51.Visibility 1
Frame_Part_51.Mother Frame_Volume

// (52):
Volume Frame_Part_52
Frame_Part_52.Material Peek
Frame_Part_52.Shape BRIK 0.3 0.2 0.025
Frame_Part_52.Position  -10.15 -9.41 0.525
Frame_Part_52.Color 1
Frame_Part_52.Visibility 1
Frame_Part_52.Mother Frame_Volume

// (53):
Volume Frame_Part_53
Frame_Part_53.Material Peek
Frame_Part_53.Shape BRIK 0.3 0.2 0.025
Frame_Part_53.Position  -10.15 -9.97 0.525
Frame_Part_53.Color 1
Frame_Part_53.Visibility 1
Frame_Part_53.Mother Frame_Volume


// (54):
Volume Frame_Part_54
Frame_Part_54.Material Peek
Frame_Part_54.Shape BRIK 0.425 1.69 0.025
Frame_Part_54.Position  -10.025 -11.86 0.525
Frame_Part_54.Color 2
Frame_Part_54.Visibility 1
Frame_Part_54.Mother Frame_Volume



// (55):
Volume Frame_Part_55
Frame_Part_55.Material Peek
Frame_Part_55.Shape BRIK 0.45 12.0 0.025
Frame_Part_55.Position  -10.0 -1.55 0.475
Frame_Part_55.Color 1
Frame_Part_55.Visibility 1
Frame_Part_55.Mother Frame_Volume

// (56):
Volume Frame_Part_56
Frame_Part_56.Material Peek
Frame_Part_56.Shape BRIK 11.55 0.42 0.05
Frame_Part_56.Position 2.0 10.03 0.5
Frame_Part_56.Color 1
Frame_Part_56.Visibility 1
Frame_Part_56.Mother Frame_Volume

// (57):
Volume Frame_Part_57
Frame_Part_57.Material Peek
Frame_Part_57.Shape BRIK 0.45 0.42 0.025
Frame_Part_57.Position -10.0 10.03 0.525
Frame_Part_57.Color 1
Frame_Part_57.Visibility 1
Frame_Part_57.Mother Frame_Volume



// (58):
Volume Frame_Part_58
Frame_Part_58.Material Peek
Frame_Part_58.Shape BRIK 2.875 0.05 0.025
Frame_Part_58.Position -6.45 9.56 0.475
Frame_Part_58.Color 1
Frame_Part_58.Visibility 1
Frame_Part_58.Mother Frame_Volume

// (59):
Volume Frame_Part_59
Frame_Part_59.Material Peek
Frame_Part_59.Shape BRIK 2.875 0.05 0.025
Frame_Part_59.Position 0.0 9.56 0.475
Frame_Part_59.Color 1
Frame_Part_59.Visibility 1
Frame_Part_59.Mother Frame_Volume

// (60):
Volume Frame_Part_60
Frame_Part_60.Material Peek
Frame_Part_60.Shape BRIK 2.875 0.05 0.025
Frame_Part_60.Position 6.45 9.56 0.475
Frame_Part_60.Color 1
Frame_Part_60.Visibility 1
Frame_Part_60.Mother Frame_Volume



// Long hole on sides:
// (61):
Volume Frame_Part_61
Frame_Part_61.Material Peek
Frame_Part_61.Shape BRIK 2.95 0.25 0.225
Frame_Part_61.Position -6.13 9.75 0.0
Frame_Part_61.Color 1
Frame_Part_61.Visibility 1
Frame_Part_61.Mother Frame_BigBlock_1

// (62):
Volume Frame_Part_62
Frame_Part_62.Material Peek
Frame_Part_62.Shape BRIK 2.95 0.25 0.225
Frame_Part_62.Position 0.32 9.75 0.0
Frame_Part_62.Color 1
Frame_Part_62.Visibility 1
Frame_Part_62.Mother Frame_BigBlock_1

// (63):
Volume Frame_Part_63
Frame_Part_63.Material Peek
Frame_Part_63.Shape BRIK 2.95 0.25 0.225
Frame_Part_63.Position 6.77 9.75 0.0
Frame_Part_63.Color 1
Frame_Part_63.Visibility 1
Frame_Part_63.Mother Frame_BigBlock_1

// (64):
Volume Frame_Part_64
Frame_Part_64.Material Peek
Frame_Part_64.Shape BRIK 0.25 2.95 0.225
Frame_Part_64.Position -9.73 6.15 0.0
Frame_Part_64.Color 1
Frame_Part_64.Visibility 1
Frame_Part_64.Mother Frame_BigBlock_1

// (65):
Volume Frame_Part_65
Frame_Part_65.Material Peek
Frame_Part_65.Shape BRIK 0.25 2.95 0.225
Frame_Part_65.Position -9.73 -0.3 0.0
Frame_Part_65.Color 1
Frame_Part_65.Visibility 1
Frame_Part_65.Mother Frame_BigBlock_1

// (66):
Volume Frame_Part_66
Frame_Part_66.Material Peek
Frame_Part_66.Shape BRIK 0.25 2.95 0.225
Frame_Part_66.Position -9.73 -6.75 0.0
Frame_Part_66.Color 1
Frame_Part_66.Visibility 1
Frame_Part_66.Mother Frame_BigBlock_1


// (67):
Volume Frame_Part_67
Frame_Part_67.Material Peek
Frame_Part_67.Shape BRIK 1.87 0.42 0.225
Frame_Part_67.Position 11.68 10.03 0.225
Frame_Part_67.Color 1
Frame_Part_67.Visibility 1
Frame_Part_67.Mother Frame_Volume

// (68):
Volume Frame_Part_68
Frame_Part_68.Material Peek
Frame_Part_68.Shape BRIK 1.3 0.25 0.225
Frame_Part_68.Position -0.33 0.0 0.0
Frame_Part_68.Color 1
Frame_Part_68.Visibility 1
Frame_Part_68.Mother Frame_Part_67


// (69):
Volume Frame_Part_69
Frame_Part_69.Material Peek
Frame_Part_69.Shape BRIK 0.4 1.85 0.225
Frame_Part_69.Position -10.05 -11.7 0.225
Frame_Part_69.Color 1
Frame_Part_69.Visibility 1
Frame_Part_69.Mother Frame_Volume

// (70):
Volume Frame_Part_70
Frame_Part_70.Material Peek
Frame_Part_70.Shape BRIK 0.25 1.3 0.225
Frame_Part_70.Position 0.0 0.3 0.0
Frame_Part_70.Color 1
Frame_Part_70.Visibility 1
Frame_Part_70.Mother Frame_Part_69


// (71):
Volume Frame_Part_71
Frame_Part_71.Material Peek
Frame_Part_71.Shape BRIK 1.675 0.05 0.025
Frame_Part_71.Position 11.485 9.56 0.475
Frame_Part_71.Color 1
Frame_Part_71.Visibility 1
Frame_Part_71.Mother Frame_Volume


// (72):
Volume Frame_Part_72
Frame_Part_72.Material Peek
Frame_Part_72.Shape BRIK 0.075 8.7 0.275
Frame_Part_72.Position 13.475 0.91 0.275
Frame_Part_72.Color 1
Frame_Part_72.Visibility 1
Frame_Part_72.Mother Frame_Volume

// (73):
Volume Frame_Part_73
Frame_Part_73.Material Peek
Frame_Part_73.Shape BRIK 0.075 8.7 0.05
Frame_Part_73.Position 13.325 0.91 0.5
Frame_Part_73.Color 1
Frame_Part_73.Visibility 1
Frame_Part_73.Mother Frame_Volume

// (74):
Volume Frame_Part_74
Frame_Part_74.Material Peek
Frame_Part_74.Shape BRIK 0.045 8.7 0.025
Frame_Part_74.Position 13.205 0.91 0.475
Frame_Part_74.Color 1
Frame_Part_74.Visibility 1
Frame_Part_74.Mother Frame_Volume

// (75):
Volume Frame_Part_75
Frame_Part_75.Material Peek
Frame_Part_75.Shape BRIK 0.15 0.15 0.25
Frame_Part_75.Position 11.5 9.36 0.25
Frame_Part_75.Color 1
Frame_Part_75.Visibility 1
Frame_Part_75.Mother Frame_Volume


// (76):
Volume Frame_Part_76
Frame_Part_76.Material Peek
Frame_Part_76.Shape BRIK 0.4 0.075 0.25
Frame_Part_76.Position 10.21 3.23 0.25
Frame_Part_76.Color 1
Frame_Part_76.Visibility 1
Frame_Part_76.Mother Frame_Volume

// (77):
Volume Frame_Part_77
Frame_Part_77.Material Peek
Frame_Part_77.Shape BRIK 0.175 0.175 0.25
Frame_Part_77.Position 10.785 3.23 0.25
Frame_Part_77.Color 1
Frame_Part_77.Visibility 1
Frame_Part_77.Mother Frame_Volume

// (78):
Volume Frame_Part_78
Frame_Part_78.Material Peek
Frame_Part_78.Shape BRIK 0.975 0.175 0.2
Frame_Part_78.Position 11.935 3.23 0.2
Frame_Part_78.Color 1
Frame_Part_78.Visibility 1
Frame_Part_78.Mother Frame_Volume

// (79):
Volume Frame_Part_79
Frame_Part_79.Material Peek
Frame_Part_79.Shape BRIK 0.125 0.175 0.25
Frame_Part_79.Position 13.035 3.23 0.25
Frame_Part_79.Color 1
Frame_Part_79.Visibility 1
Frame_Part_79.Mother Frame_Volume


// (80):
Volume Frame_Part_80
Frame_Part_80.Material Peek
Frame_Part_80.Shape BRIK 0.4 0.075 0.25
Frame_Part_80.Position 10.21 -3.23 0.25
Frame_Part_80.Color 1
Frame_Part_80.Visibility 1
Frame_Part_80.Mother Frame_Volume

// (81):
Volume Frame_Part_81
Frame_Part_81.Material Peek
Frame_Part_81.Shape BRIK 0.175 0.175 0.25
Frame_Part_81.Position 10.785 -3.23 0.25
Frame_Part_81.Color 1
Frame_Part_81.Visibility 1
Frame_Part_81.Mother Frame_Volume

// (82):
Volume Frame_Part_82
Frame_Part_82.Material Peek
Frame_Part_82.Shape BRIK 0.975 0.175 0.2
Frame_Part_82.Position 11.935 -3.23 0.2
Frame_Part_82.Color 1
Frame_Part_82.Visibility 1
Frame_Part_82.Mother Frame_Volume

// (83):
Volume Frame_Part_83
Frame_Part_83.Material Peek
Frame_Part_83.Shape BRIK 0.125 0.175 0.25
Frame_Part_83.Position 13.035 -3.23 0.25
Frame_Part_83.Color 1
Frame_Part_83.Visibility 1
Frame_Part_83.Mother Frame_Volume


// (84):
Volume Frame_Part_84
Frame_Part_84.Material Peek
Frame_Part_84.Shape BRIK 0.12 0.175 0.225
Frame_Part_84.Position 13.28 3.23 0.225
Frame_Part_84.Color 1
Frame_Part_84.Visibility 1
Frame_Part_84.Mother Frame_Volume

// (85):
Volume Frame_Part_85
Frame_Part_85.Material Peek
Frame_Part_85.Shape BRIK 0.12 0.175 0.225
Frame_Part_85.Position 13.28 -3.23 0.225
Frame_Part_85.Color 1
Frame_Part_85.Visibility 1
Frame_Part_85.Mother Frame_Volume


// (86):
Volume Frame_Part_86
Frame_Part_86.Material Peek
Frame_Part_86.Shape BRIK 0.15 0.15 0.25
Frame_Part_86.Position -9.4 -11.5 0.25
Frame_Part_86.Color 1
Frame_Part_86.Visibility 1
Frame_Part_86.Mother Frame_Volume



// (87):
Volume Frame_Part_87
Frame_Part_87.Material Peek
Frame_Part_87.Shape BRIK 11.1 0.075 0.275
Frame_Part_87.Position 1.55 -13.475 0.275
Frame_Part_87.Color 1
Frame_Part_87.Visibility 1
Frame_Part_87.Mother Frame_Volume

// (88):
Volume Frame_Part_88
Frame_Part_88.Material Peek
Frame_Part_88.Shape BRIK 0.05 0.075 0.225
Frame_Part_88.Position -9.6 -13.475 0.225
Frame_Part_88.Color 1
Frame_Part_88.Visibility 1
Frame_Part_88.Mother Frame_Volume


// (89):
Volume Frame_Part_89
Frame_Part_89.Material Peek
Frame_Part_89.Shape BRIK 10.725 0.075 0.025
Frame_Part_89.Position 1.525 -13.325 0.525
Frame_Part_89.Color 1
Frame_Part_89.Visibility 1
Frame_Part_89.Mother Frame_Volume

// (90):
Volume Frame_Part_90
Frame_Part_90.Material Peek
Frame_Part_90.Shape BRIK 11.1 0.125 0.025
Frame_Part_90.Position 1.55 -13.275 0.475
Frame_Part_90.Color 1
Frame_Part_90.Visibility 1
Frame_Part_90.Mother Frame_Volume



// (91):
Volume Frame_Part_91
Frame_Part_91.Material Peek
Frame_Part_91.Shape BRIK 0.175 0.525 0.25
Frame_Part_91.Position -3.225 -10.385 0.25
Frame_Part_91.Color 1
Frame_Part_91.Visibility 1
Frame_Part_91.Mother Frame_Volume

// (92):
Volume Frame_Part_92
Frame_Part_92.Material Peek
Frame_Part_92.Shape BRIK 0.175 0.975 0.2
Frame_Part_92.Position -3.225 -11.885 0.2
Frame_Part_92.Color 1
Frame_Part_92.Visibility 1
Frame_Part_92.Mother Frame_Volume

// (93):
Volume Frame_Part_93
Frame_Part_93.Material Peek
Frame_Part_93.Shape BRIK 0.175 0.145 0.25
Frame_Part_93.Position -3.225 -13.005 0.25
Frame_Part_93.Color 1
Frame_Part_93.Visibility 1
Frame_Part_93.Mother Frame_Volume


// (94):
Volume Frame_Part_94
Frame_Part_94.Material Peek
Frame_Part_94.Shape BRIK 0.175 0.525 0.25
Frame_Part_94.Position 3.225 -10.385 0.25
Frame_Part_94.Color 1
Frame_Part_94.Visibility 1
Frame_Part_94.Mother Frame_Volume

// (95):
Volume Frame_Part_95
Frame_Part_95.Material Peek
Frame_Part_95.Shape BRIK 0.175 0.975 0.2
Frame_Part_95.Position 3.225 -11.885 0.2
Frame_Part_95.Color 1
Frame_Part_95.Visibility 1
Frame_Part_95.Mother Frame_Volume

// (96):
Volume Frame_Part_96
Frame_Part_96.Material Peek
Frame_Part_96.Shape BRIK 0.175 0.145 0.25
Frame_Part_96.Position 3.225 -13.005 0.25
Frame_Part_96.Color 1
Frame_Part_96.Visibility 1
Frame_Part_96.Mother Frame_Volume


// (97):
Volume Frame_Part_97
Frame_Part_97.Material Peek
Frame_Part_97.Shape BRIK 0.45 0.45 0.275
Frame_Part_97.Position 13.1 -13.1 0.275
Frame_Part_97.Color 1
Frame_Part_97.Visibility 1
Frame_Part_97.Mother Frame_Volume



// (98):
Volume Frame_Part_98
Frame_Part_98.Material Peek
Frame_Part_98.Shape BRIK 0.4 1.97 0.25
Frame_Part_98.Position 11.23 -11.23 0.25
Frame_Part_98.Rotation 0.0 0.0 45.0
Frame_Part_98.Color 1
Frame_Part_98.Visibility 1
Frame_Part_98.Mother Frame_Volume

// (99):
Volume Frame_Part_99
Frame_Part_99.Material Peek
Frame_Part_99.Shape BRIK 0.1 1.7 0.25
Frame_Part_99.Position 12.06 -9.08 0.25
Frame_Part_99.Rotation 0.0 0.0 -45.0
Frame_Part_99.Color 1
Frame_Part_99.Visibility 1
Frame_Part_99.Mother Frame_Volume



// Now come the cylinders for the big rods:

// rod in upper right corner:
// (100):
Volume Frame_Part_100
Frame_Part_100.Material Steel
Frame_Part_100.Shape TUBS 0.0 0.225 0.225 0.0 360.0
Frame_Part_100.Position 1.42 0.0 0.0
Frame_Part_100.Color 4
Frame_Part_100.Visibility 1
Frame_Part_100.Mother Frame_Part_67

// (101):
Volume Frame_Part_101
Frame_Part_101.Material Steel
Frame_Part_101.Shape TUBS 0.0 0.225 0.05 0.0 360.0
Frame_Part_101.Position 11.1 0.0 0.0
Frame_Part_101.Color 4
Frame_Part_101.Visibility 1
Frame_Part_101.Mother Frame_Part_56


// rod in upper left corner:
// (102):
Volume Frame_Part_102
Frame_Part_102.Material Steel
Frame_Part_102.Shape TUBS 0.0 0.225 0.225 0.0 360.0
Frame_Part_102.Position -9.68 9.7  0.0
Frame_Part_102.Color 4
Frame_Part_102.Visibility 1
Frame_Part_102.Mother Frame_BigBlock_1

// (103):
Volume Frame_Part_103
Frame_Part_103.Material Steel
Frame_Part_103.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_103.Position 0.0 11.55 0.0
Frame_Part_103.Color 4
Frame_Part_103.Visibility 1
Frame_Part_103.Mother Frame_Part_55

// (104):
Volume Frame_Part_104
Frame_Part_104.Material Steel
Frame_Part_104.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_104.Position 0.0 -0.03 0.0
Frame_Part_104.Color 4
Frame_Part_104.Visibility 1
Frame_Part_104.Mother Frame_Part_57


// lower left rod:
// (105):
Volume Frame_Part_105
Frame_Part_105.Material Steel
Frame_Part_105.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_105.Position 0.025 -1.24 0.0
Frame_Part_105.Color 4
Frame_Part_105.Visibility 1
Frame_Part_105.Mother Frame_Part_54

// (106):
Volume Frame_Part_106
Frame_Part_106.Material Steel
Frame_Part_106.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_106.Position 0.0 -11.55 0.0
Frame_Part_106.Color 4
Frame_Part_106.Visibility 1
Frame_Part_106.Mother Frame_Part_55

// (107):
Volume Frame_Part_107
Frame_Part_107.Material Steel
Frame_Part_107.Shape TUBS 0.0 0.225 0.225 0.0 360.0
Frame_Part_107.Position 0.05 -1.4 0.0
Frame_Part_107.Color 4
Frame_Part_107.Visibility 1
Frame_Part_107.Mother Frame_Part_69


// lower right rod:
// (108):
Volume Frame_Part_108
Frame_Part_108.Material Steel
Frame_Part_108.Shape TUBS 0.0 0.225 0.275 0.0 360.0
Frame_Part_108.Position 0.0 0.0 0.0
Frame_Part_108.Color 4
Frame_Part_108.Visibility 1
Frame_Part_108.Mother Frame_Part_97

// Now empty rod-holes:

// upper rod-hole:
// (109):
Volume Frame_Part_109
Frame_Part_109.Material Air
Frame_Part_109.Shape TUBS 0.0 0.225 0.05 0.0 360.0
Frame_Part_109.Position 1.225 0.0 0.0
Frame_Part_109.Color 6
Frame_Part_109.Visibility 1
Frame_Part_109.Mother Frame_Part_56

// (110):
Volume Frame_Part_110
Frame_Part_110.Material Air
Frame_Part_110.Shape TUBS 0.0 0.225 0.225 0.0 360.0
Frame_Part_110.Position 3.545 9.73 0.0
Frame_Part_110.Color 6
Frame_Part_110.Visibility 1
Frame_Part_110.Mother Frame_BigBlock_1


// left rod-hole:
// note that this one is shifted slightly to the left (ca. 1 mm) from the real position,
// because that was easier to realize:
// (111):
Volume Frame_Part_111
Frame_Part_111.Material Air
Frame_Part_111.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_111.Position 0.0 0.0 0.0
Frame_Part_111.Color 6
Frame_Part_111.Visibility 1
Frame_Part_111.Mother Frame_Part_50

// (112):
Volume Frame_Part_112
Frame_Part_112.Material Air
Frame_Part_112.Shape TUBS 0.0 0.225 0.025 0.0 360.0
Frame_Part_112.Position -0.15 -1.68 0.0
Frame_Part_112.Color 6
Frame_Part_112.Visibility 1
Frame_Part_112.Mother Frame_Part_55

// (113):
Volume Frame_Part_113
Frame_Part_113.Material Air
Frame_Part_113.Shape TUBS 0.0 0.225 0.225 0.0 360.0
Frame_Part_113.Position -9.83 -3.53 0.0
Frame_Part_113.Color 6
Frame_Part_113.Visibility 1
Frame_Part_113.Mother Frame_BigBlock_1



// Now the little screws along right and lower side:

// (114):
Volume Frame_Part_114
Frame_Part_114.Material Air
Frame_Part_114.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_114.Position 0.0 0.0 0.0
Frame_Part_114.Color 4
Frame_Part_114.Visibility 1
Frame_Part_114.Mother Frame_Part_75

// (115):
Volume Frame_Part_115
Frame_Part_115.Material Air
Frame_Part_115.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_115.Position 0.0 0.0 0.0
Frame_Part_115.Color 4
Frame_Part_115.Visibility 1
Frame_Part_115.Mother Frame_Part_79

// (116):
Volume Frame_Part_116
Frame_Part_116.Material Air
Frame_Part_116.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_116.Position 0.0 0.0 0.0
Frame_Part_116.Color 4
Frame_Part_116.Visibility 1
Frame_Part_116.Mother Frame_Part_83

// (117):
Volume Frame_Part_117
Frame_Part_117.Material Air
Frame_Part_117.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_117.Position 0.0 0.0 0.0
Frame_Part_117.Color 4
Frame_Part_117.Visibility 1
Frame_Part_117.Mother Frame_Part_86

// (118):
Volume Frame_Part_118
Frame_Part_118.Material Air
Frame_Part_118.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_118.Position 0.0 0.0 0.0
Frame_Part_118.Color 4
Frame_Part_118.Visibility 1
Frame_Part_118.Mother Frame_Part_93

// (119):
Volume Frame_Part_119
Frame_Part_119.Material Air
Frame_Part_119.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_119.Position 0.0 0.0 0.0
Frame_Part_119.Color 4
Frame_Part_119.Visibility 1
Frame_Part_119.Mother Frame_Part_96


// (120):
Volume Frame_Part_120
Frame_Part_120.Material Air
Frame_Part_120.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_120.Position 0.025 0.0 0.0
Frame_Part_120.Color 4
Frame_Part_120.Visibility 1
Frame_Part_120.Mother Frame_Part_77

// (121):
Volume Frame_Part_121
Frame_Part_121.Material Air
Frame_Part_121.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_121.Position 0.025 0.0 0.0
Frame_Part_121.Color 4
Frame_Part_121.Visibility 1
Frame_Part_121.Mother Frame_Part_81

// (122):
Volume Frame_Part_122
Frame_Part_122.Material Air
Frame_Part_122.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_122.Position 0.0 -0.375 0.0
Frame_Part_122.Color 4
Frame_Part_122.Visibility 1
Frame_Part_122.Mother Frame_Part_91

// (123):
Volume Frame_Part_123
Frame_Part_123.Material Air
Frame_Part_123.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_123.Position 0.0 -0.375 0.0
Frame_Part_123.Color 4
Frame_Part_123.Visibility 1
Frame_Part_123.Mother Frame_Part_94


// (124):
Volume Frame_Part_124
Frame_Part_124.Material Air
Frame_Part_124.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_124.Position -0.25 0.0 0.0
Frame_Part_124.Color 4
Frame_Part_124.Visibility 1
Frame_Part_124.Mother Frame_Part_98

// (125):
Volume Frame_Part_125
Frame_Part_125.Material Air
Frame_Part_125.Shape TUBS 0.0 0.05 0.25 0.0 360.0
Frame_Part_125.Position 0.25 0.0 0.0
Frame_Part_125.Color 4
Frame_Part_125.Visibility 1
Frame_Part_125.Mother Frame_Part_98


// Now come the Siliconlayers:
// check the exact dimensions of the layers, their positions are OK:

// Basic Silicon Wafer (226):
Volume Basic_Silicon_Wafer
Basic_Silicon_Wafer.Material Silicon
Basic_Silicon_Wafer.Shape BRIK 3.14 3.15 0.025
Basic_Silicon_Wafer.Color 5


// (126):
Basic_Silicon_Wafer.Copy Wafer_126
Wafer_126.Position -6.45 -6.46 0.525
Wafer_126.Visibility 1
Wafer_126.Mother Frame_Volume

// (127):
Basic_Silicon_Wafer.Copy Wafer_127
Wafer_127.Position 0.0 -6.46 0.525
Wafer_127.Visibility 1
Wafer_127.Mother Frame_Volume

// (128):
Basic_Silicon_Wafer.Copy Wafer_128
Wafer_128.Position +6.45 -6.46 0.525
Wafer_128.Visibility 1
Wafer_128.Mother Frame_Volume

// (129):
Basic_Silicon_Wafer.Copy Wafer_129
Wafer_129.Position -6.45 0.0 0.525
Wafer_129.Visibility 1
Wafer_129.Mother Frame_Volume

// (130):
Basic_Silicon_Wafer.Copy Wafer_130
Wafer_130.Position 0.0 0.0 0.525
Wafer_130.Visibility 1
Wafer_130.Mother Frame_Volume

// (131):
Basic_Silicon_Wafer.Copy Wafer_131
Wafer_131.Position +6.45 0.0 0.525
Wafer_131.Visibility 1
Wafer_131.Mother Frame_Volume

// (132):
Basic_Silicon_Wafer.Copy Wafer_132
Wafer_132.Position -6.45 6.46 0.525
Wafer_132.Visibility 1
Wafer_132.Mother Frame_Volume

// (133):
Basic_Silicon_Wafer.Copy Wafer_133
Wafer_133.Position 0.0 6.46 0.525
Wafer_133.Visibility 1
Wafer_133.Mother Frame_Volume

// (134):
Basic_Silicon_Wafer.Copy Wafer_134
Wafer_134.Position +6.45 6.46 0.525
Wafer_134.Visibility 1
Wafer_134.Mother Frame_Volume



// The Boards on the right und lower side:

// right board:
// (135):
Volume Frame_Part_135
Frame_Part_135.Material PCB
Frame_Part_135.Shape BRIK 1.7475 11.1245 0.025
Frame_Part_135.Position 11.5025 -1.5245 0.525
Frame_Part_135.Color 3
Frame_Part_135.Visibility 1
Frame_Part_135.Mother Frame_Volume

// lower board:
// (136):
Volume Frame_Part_136
Frame_Part_136.Material PCB
Frame_Part_136.Shape BRIK 9.6775 1.74 0.025
Frame_Part_136.Position 0.0775 -11.51 0.525
Frame_Part_136.Color 3
Frame_Part_136.Visibility 1
Frame_Part_136.Mother Frame_Volume

// some space still to fill:
// (137):
Volume Frame_Part_137
Frame_Part_137.Material PCB
Frame_Part_137.Shape BRIK 1.4375 0.3 0.025
Frame_Part_137.Position 11.1925 -12.95 0.525
Frame_Part_137.Color 3
Frame_Part_137.Visibility 1
Frame_Part_137.Mother Frame_Volume

