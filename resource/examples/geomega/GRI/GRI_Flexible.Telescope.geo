
Include GRI_Flexible.Intro.geo
Include GRI_Flexible.DetectorHead.geo

Constant Telescope_XYDim { TMath::Max(Adapter_Spacecraft_Size, DetectorHead_XYDim) }
Constant Telescope_ZDim { DetectorHead_ZDim + Adapter_Spacecraft_Height }


Volume Telescope
Telescope.Material vacuum
Telescope.Visibility 0
Telescope.Color 3
Telescope.Shape BRIK Telescope_XYDim  Telescope_XYDim  Telescope_ZDim 
Telescope.Position 0.0 0.0 0.0
Telescope.Mother VAC0

DetectorHead.Position 0.0 0.0 {Telescope_ZDim - DetectorHead_ZDim }
DetectorHead.Mother Telescope

Constant Adapter_Spacecraft_Size  13.0
Constant Adapter_Spacecraft_Height 0.5

! Lower BGO cover
Volume BLC
BLC.Visibility 1
BLC.Material c_epoxy
BLC.Shape TUBS 0.0 Adapter_Spacecraft_Size Adapter_Spacecraft_Height 0.0 360.0 
BLC.Position 0 0 {-Telescope_ZDim+Adapter_Spacecraft_Height}
BLC.Mother Telescope
