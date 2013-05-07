// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


Include Blue_Frame.geo
Include Duke.v2.00.CsI_geh_80.geo
Include Part_B.geo
Include Part_A.geo
Include Part_C.geo






// The MOTHER VOLUME of the Detectot Configuration:
// ------------------------------------------------
Volume 80mm_DC_Volume
80mm_DC_Volume.Material Air
80mm_DC_Volume.Visibility 0
80mm_DC_Volume.Virtual true
80mm_DC_Volume.Shape BRIK 100. 100. 100.
// NEEDS THIS LINE TO VIEW ALONE:
// 80mm_DC_Volume.Mother 0




// The COMPONENTS :
// ----------------

// The Blue Frame on bottom of the Configuration (1):
Part_BF_Volume.Copy DC_BF_1
DC_BF_1.Position 0.0 0.0 -5.58  // -5.58
DC_BF_1.Mother 80mm_DC_Volume

// The four boxed DETECTOR UNITS:

// (2):
Geh_80_Volume.Copy aphrodite
aphrodite.Position 5.1 4.65 0.
aphrodite.Mother 80mm_DC_Volume

// (3):
Geh_80_Volume.Copy athene
athene.Position 5.1 -4.65 0.
athene.Rotation 0. 0. 180.
athene.Mother 80mm_DC_Volume

// (4):
Geh_80_Volume.Copy xantippe
xantippe.Position -5.1 4.65 0.
xantippe.Mother 80mm_DC_Volume

// (5):
Geh_80_Volume.Copy daphne
daphne.Position -5.1 -4.65 0.
daphne.Mother 80mm_DC_Volume
daphne.Rotation 0. 0. 180.



// Parts B:
// (6):
Part_B_Volume.Copy DC_PartB_6
DC_PartB_6.Position 1.1 0.0 0.0
DC_PartB_6.Mother 80mm_DC_Volume
DC_PartB_6.Rotation 90.0 0.0 -90.0

// (7):
Part_B_Volume.Copy DC_PartB_7
DC_PartB_7.Position -1.1 0.0 0.0
DC_PartB_7.Mother 80mm_DC_Volume
DC_PartB_7.Rotation 90.0 0.0 90.0


// Parts A:
// (8):
Part_A_Volume.Copy DC_PartA_8
DC_PartA_8.Position 9.75 4.65 -5.58
DC_PartA_8.Mother 80mm_DC_Volume
DC_PartA_8.Rotation 90.0 0.0 90.0

// (9):
Part_A_Volume.Copy DC_PartA_9
DC_PartA_9.Position 9.75 -4.65 -5.58
DC_PartA_9.Mother 80mm_DC_Volume
DC_PartA_9.Rotation 90.0 0.0 90.0

// (10):
Part_A_Volume.Copy DC_PartA_10
DC_PartA_10.Position -9.75 4.65 -5.58
DC_PartA_10.Mother 80mm_DC_Volume
DC_PartA_10.Rotation 90.0 0.0 -90.0

// (11):
Part_A_Volume.Copy DC_PartA_11
DC_PartA_11.Position -9.75 -4.65 -5.58
DC_PartA_11.Mother 80mm_DC_Volume
DC_PartA_11.Rotation 90.0 0.0 -90.0


// Parts C:

// (12):
Part_C_Volume.Copy DC_PartC_12
DC_PartC_12.Position 0.0 9.3 -5.58
DC_PartC_12.Mother 80mm_DC_Volume
DC_PartC_12.Rotation 90.0 0.0 90.0

// (13):
Part_C_Volume.Copy DC_PartC_13
DC_PartC_13.Position 0.0 -9.3 -5.58
DC_PartC_13.Mother 80mm_DC_Volume
DC_PartC_13.Rotation 90.0 0.0 -90.0



// Shock Absorber:

// (14):
Volume DC_ShockAbsorber_14
DC_ShockAbsorber_14.Material Air
DC_ShockAbsorber_14.Shape BRIK 4.5 4.5 0.1
DC_ShockAbsorber_14.Position 5.1 4.65 -5.48
DC_ShockAbsorber_14.Color 3
DC_ShockAbsorber_14.Visibility 1
DC_ShockAbsorber_14.Mother 80mm_DC_Volume

// (15):
Volume DC_ShockAbsorber_15
DC_ShockAbsorber_15.Material Air
DC_ShockAbsorber_15.Shape BRIK 4.5 4.5 0.1
DC_ShockAbsorber_15.Position 5.1 -4.65 -5.48
DC_ShockAbsorber_15.Color 3
DC_ShockAbsorber_15.Visibility 1
DC_ShockAbsorber_15.Mother 80mm_DC_Volume

// (16):
Volume DC_ShockAbsorber_16
DC_ShockAbsorber_16.Material Air
DC_ShockAbsorber_16.Shape BRIK 4.5 4.5 0.1
DC_ShockAbsorber_16.Position -5.1 4.65 -5.48
DC_ShockAbsorber_16.Color 3
DC_ShockAbsorber_16.Visibility 1
DC_ShockAbsorber_16.Mother 80mm_DC_Volume

// (17):
Volume DC_ShockAbsorber_17
DC_ShockAbsorber_17.Material Air
DC_ShockAbsorber_17.Shape BRIK 4.5 4.5 0.1
DC_ShockAbsorber_17.Position -5.1 -4.65 -5.48
DC_ShockAbsorber_17.Color 3
DC_ShockAbsorber_17.Visibility 1
DC_ShockAbsorber_17.Mother 80mm_DC_Volume