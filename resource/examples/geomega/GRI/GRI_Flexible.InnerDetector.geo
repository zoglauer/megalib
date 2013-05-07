
Include GRI_Flexible.Intro.geo
Include GRI_Flexible.CZTWafer.geo
Include GRI_Flexible.SiWafer.geo


Constant ID_XYHDIM {TMath::Max(CZT_SLayers_XPos,CZT_SLayers_YPos) + CZT_SLayer_ZHDim + (CZT_Layers_XY-1)*CZT_LayerDistance}
Constant ID_ZHDIM {0.5*TMath::Max(CZT_SLayers_BottomDistance+2*CZT_SLayer_YHDim, CZT_Layers_Z*CZT_BLayer_ZHDim + (CZT_Layers_Z-1)*CZT_LayerDistance)}


Volume InnerDetector
InnerDetector.Material vacuum
InnerDetector.Visibility 0
InnerDetector.Color 3
InnerDetector.Shape BRIK ID_XYHDIM ID_XYHDIM ID_ZHDIM
InnerDetector.Position 0 0 0.0
InnerDetector.Mother VAC0


# Arrange Bottom CZT Layers
For Z  CZT_Layers_Z  {-ID_ZHDIM + CZT_BLayer_ZHDim}  CZT_LayerDistance
   CZT_BLayer.Copy CZT_BLayer_Copy%Z
   CZT_BLayer_Copy%Z.Mother InnerDetector
   CZT_BLayer_Copy%Z.Position 0 0 $Z
Done


# Arrange Si Layers
For Z  Si_Layers  {-ID_ZHDIM + CZT_Layers_Z*CZT_LayerDistance + Si_Layer_ZHDim }  Si_LayerDistance
   SiLayer.Copy SiLayer_Copy%Z
   SiLayer_Copy%Z.Mother InnerDetector
   SiLayer_Copy%Z.Position 0 0 $Z
Done

# Arrange Side layers
Constant CZT_SLayers_XPos { TMath::Max(CZT_BLayer_XHDim,Si_Layer_XHDim) + 0.5*CZT_LayerDistance }
Constant CZT_SLayers_YPos { TMath::Max(CZT_BLayer_YHDim,Si_Layer_YHDim) + 0.5*CZT_LayerDistance }
Constant CZT_SLayers_BottomPos { -ID_ZHDIM + CZT_SLayers_BottomDistance+CZT_SLayer_YHDim }


For X  2  {-CZT_SLayers_XPos}  {2*CZT_SLayers_XPos}
  For D  CZT_Layers_XY  1  1
    CZT_SLayer.Copy CZT_SLayer_CopyY%X%D
    CZT_SLayer_CopyY%X%D.Mother InnerDetector
    CZT_SLayer_CopyY%X%D.Rotation 90.0  0.0  {-TMath::Sign(1,$X)*90.0}
    CZT_SLayer_CopyY%X%D.Position {TMath::Sign(1,$X)*($D-1)*CZT_LayerDistance + $X}  0.0 CZT_SLayers_BottomPos
  Done
Done

For Y  2  {-CZT_SLayers_YPos}  {2*CZT_SLayers_YPos}
  For D  CZT_Layers_XY  1  1
    CZT_SLayer.Copy CZT_SLayer_CopyX%Y%D
    CZT_SLayer_CopyX%Y%D.Mother InnerDetector
    CZT_SLayer_CopyX%Y%D.Rotation {TMath::Sign(1,$Y)*90.0}  0.0 0.0
    CZT_SLayer_CopyX%Y%D.Position 0.0  {TMath::Sign(1,$Y)*($D-1)*CZT_LayerDistance + $Y}  CZT_SLayers_BottomPos
  Done
Done


# Add some cables and mounting dummies:

Constant Mounting_HWidth 0.2

Volume Mounting
Mounting.Material electr
Mounting.Visibility 1
Mounting.Color 3
Mounting.Shape BRIK CZT_BLayer_ZHDim Mounting_HWidth ID_ZHDIM

Volume MountingStructure
MountingStructure.Material al6061
MountingStructure.Visibility 1
MountingStructure.Color 3
MountingStructure.Shape BOX CZT_BLayer_ZHDim {0.5*Mounting_HWidth} ID_ZHDIM
MountingStructure.Position 0  {0.5*Mounting_HWidth}  0.0
MountingStructure.Mother Mounting

Constant Mounting_Pos {CZT_SLayer_XHDim + Mounting_HWidth}

#
For S 2 1 -2
  For D  CZT_Layers_XY  -CZT_SLayers_XPos  -CZT_LayerDistance
    For Y  2  {-Mounting_Pos}  {2*Mounting_Pos}
      Mounting.Copy MountingA%D%Y%S
      MountingA%D%Y%S.Mother InnerDetector
      MountingA%D%Y%S.Position { $S*$D } $Y 0.0
    Done
  Done
Done

For S 2 1 -2
  For D  CZT_Layers_XY  -CZT_SLayers_YPos  -CZT_LayerDistance
    For X  2  {-Mounting_Pos}  {2*Mounting_Pos}
      Mounting.Copy MountingB%D%X%S
      MountingB%D%X%S.Mother InnerDetector
      MountingB%D%X%S.Rotation 0.0 0.0 { $S*90.0 }
      MountingB%D%X%S.Position $X { $S*$D } 0.0
    Done
  Done
Done

