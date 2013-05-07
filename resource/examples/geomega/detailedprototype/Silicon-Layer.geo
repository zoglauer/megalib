// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo



// Basic Silicon Wafer:


Volume SL_Basic_Silicon_Wafer
SL_Basic_Silicon_Wafer.Material Silicon
SL_Basic_Silicon_Wafer.Shape BRIK 3.14 3.15 0.025
SL_Basic_Silicon_Wafer.Color 8

Volume SL_WaferLayer_Volume
SL_WaferLayer_Volume.Material Air
SL_WaferLayer_Volume.Shape BRIK 100. 100. 100.
SL_WaferLayer_Volume.Virtual true
//SL_WaferLayer_Volume.Visibility 0


// (126):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_126
SL_Wafer_126.Position -6.45 -6.46 0.525
SL_Wafer_126.Visibility 1
SL_Wafer_126.Mother SL_WaferLayer_Volume

// (127):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_127
SL_Wafer_127.Position 0.0 -6.46 0.525
SL_Wafer_127.Visibility 1
SL_Wafer_127.Mother SL_WaferLayer_Volume

// (128):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_128
SL_Wafer_128.Position 6.45 -6.46 0.525
SL_Wafer_128.Visibility 1
SL_Wafer_128.Mother SL_WaferLayer_Volume

// (129):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_129
SL_Wafer_129.Position -6.45 0.0 0.525
SL_Wafer_129.Visibility 1
SL_Wafer_129.Mother SL_WaferLayer_Volume

// (130):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_130
SL_Wafer_130.Position 0.0 0.0 0.525
SL_Wafer_130.Visibility 1
SL_Wafer_130.Mother SL_WaferLayer_Volume

// (131):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_131
SL_Wafer_131.Position 6.45 0.0 0.525
SL_Wafer_131.Visibility 1
SL_Wafer_131.Mother SL_WaferLayer_Volume

// (132):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_132
SL_Wafer_132.Position -6.45 6.46 0.525
SL_Wafer_132.Visibility 1
SL_Wafer_132.Mother SL_WaferLayer_Volume

// (133):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_133
SL_Wafer_133.Position 0.0 6.46 0.525
SL_Wafer_133.Visibility 1
SL_Wafer_133.Mother SL_WaferLayer_Volume

// (134):
SL_Basic_Silicon_Wafer.Copy SL_Wafer_134
SL_Wafer_134.Position 6.45 6.46 0.525
SL_Wafer_134.Visibility 1
SL_Wafer_134.Mother SL_WaferLayer_Volume