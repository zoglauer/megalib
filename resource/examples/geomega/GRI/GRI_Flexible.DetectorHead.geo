
Include GRI_Flexible.Intro.geo

Constant DetectorHead_XYDim { ShieldBottom_XYHDim + ShieldBottom_CoverThickness }
Constant DetectorHead_ZDim { 2*ShieldBottom_CoverThickness + ShieldBottom_ThickHDim + ShieldSide_CorrectedHeightHDim + ShieldTop_ThickHDim + 2*ShieldTop_CoverThickness}

Constant ShieldBottom_XYHDim  { ID_XYHDIM + 2*ShieldSide_ThickHDim + 2*ShieldSide_CoverThickness + Shield_To_CZT_Distance  }
Constant ShieldSide_CorrectedHeightHDim { TMath::Max(ShieldSide_HeightHDim, Shield_To_CZT_Distance + ID_ZHDIM) }


# Container for the whole detector

Volume DetectorHead
DetectorHead.Material vacuum
DetectorHead.Visibility 2
DetectorHead.Color 3
DetectorHead.Shape BRIK DetectorHead_XYDim  DetectorHead_XYDim  DetectorHead_ZDim 
DetectorHead.Position 0.0 0.0 0.0
DetectorHead.Mother VAC0


# Add the inner detector

Include GRI_Flexible.InnerDetector.geo
InnerDetector.Position 0.0 0.0 { -DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldBottom_CoverThickness) + ID_ZHDIM + Shield_To_CZT_Distance  }
InnerDetector.Mother DetectorHead



# Bottom shield

Volume ShieldBottom
ShieldBottom.Material ShieldBottom_CoverMaterial
ShieldBottom.Visibility 1
ShieldBottom.Color 3
ShieldBottom.Shape BRIK { ShieldBottom_XYHDim + ShieldBottom_CoverThickness } { ShieldBottom_XYHDim + ShieldBottom_CoverThickness} { ShieldBottom_ThickHDim + ShieldBottom_CoverThickness }
ShieldBottom.Mother DetectorHead
ShieldBottom.Position 0 0 { -DetectorHead_ZDim + ShieldBottom_ThickHDim + ShieldBottom_CoverThickness  } 

Volume ShieldBottomActive
ShieldBottomActive.Material ShieldBottom_ActiveMaterial
ShieldBottomActive.Visibility 1
ShieldBottomActive.Color 3
ShieldBottomActive.Shape BRIK ShieldBottom_XYHDim ShieldBottom_XYHDim ShieldBottom_ThickHDim 
ShieldBottomActive.Mother ShieldBottom
ShieldBottomActive.Position 0 0 0



# Side shield:

Volume ShieldSide
ShieldSide.Material ShieldSide_CoverMaterial
ShieldSide.Visibility 1
ShieldSide.Color 3
ShieldSide.Shape BRIK { ShieldSide_ThickHDim + ShieldSide_CoverThickness } { ShieldBottom_XYHDim - ShieldSide_ThickHDim } { ShieldSide_CorrectedHeightHDim + ShieldSide_CoverThickness }

Volume ShieldSideActive
ShieldSideActive.Material ShieldSide_ActiveMaterial
ShieldSideActive.Visibility 1
ShieldSideActive.Color 5
ShieldSideActive.Shape BRIK ShieldSide_ThickHDim { ShieldBottom_XYHDim - ShieldSide_ThickHDim - ShieldSide_CoverThickness } ShieldSide_CorrectedHeightHDim
ShieldSideActive.Mother ShieldSide
ShieldSideActive.Position 0 0 0


ShieldSide.Copy ShieldSide_Copy1
ShieldSide_Copy1.Mother DetectorHead
ShieldSide_Copy1.Position { -ShieldBottom_XYHDim + ShieldSide_ThickHDim } { -ShieldSide_ThickHDim-ShieldSide_CoverThickness } { -DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldSide_CoverThickness) + ShieldSide_CorrectedHeightHDim + ShieldSide_CoverThickness } 

ShieldSide.Copy ShieldSide_Copy2
ShieldSide_Copy2.Mother DetectorHead
ShieldSide_Copy2.Position { ShieldBottom_XYHDim - ShieldSide_ThickHDim } { ShieldSide_ThickHDim+ShieldSide_CoverThickness } { -DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldSide_CoverThickness) + ShieldSide_CorrectedHeightHDim + ShieldSide_CoverThickness } 

ShieldSide.Copy ShieldSide_Copy3
ShieldSide_Copy3.Mother DetectorHead
ShieldSide_Copy3.Rotation 0.0  0.0  90.0
ShieldSide_Copy3.Position { -ShieldSide_ThickHDim-ShieldSide_CoverThickness } { ShieldBottom_XYHDim - ShieldSide_ThickHDim } { -DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldSide_CoverThickness) + ShieldSide_CorrectedHeightHDim + ShieldSide_CoverThickness } 

ShieldSide.Copy ShieldSide_Copy4
ShieldSide_Copy4.Mother DetectorHead
ShieldSide_Copy4.Rotation 0.0  0.0  90.0
ShieldSide_Copy4.Position { ShieldSide_ThickHDim+ShieldSide_CoverThickness } { -ShieldBottom_XYHDim + ShieldSide_ThickHDim } { -DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldSide_CoverThickness) + ShieldSide_CorrectedHeightHDim + ShieldSide_CoverThickness } 




# Add the top shield

Volume ShieldTop
ShieldTop.Material ShieldTop_CoverMaterial
ShieldTop.Color 7
ShieldTop.Visibility 1
ShieldTop.Shape BRIK { ShieldBottom_XYHDim + ShieldBottom_CoverThickness }  { ShieldBottom_XYHDim + ShieldBottom_CoverThickness }  { ShieldTop_ThickHDim + ShieldTop_CoverThickness }
ShieldTop.Position 0.0  0.0 {-DetectorHead_ZDim + 2*(ShieldBottom_ThickHDim+ShieldBottom_CoverThickness) + 2*(ShieldSide_CorrectedHeightHDim+ShieldSide_CoverThickness) + ShieldTop_ThickHDim + ShieldTop_CoverThickness }
ShieldTop.Mother DetectorHead


Volume ShieldTopActive
ShieldTopActive.Material ShieldTop_ActiveMaterial
ShieldTopActive.Visibility 1
ShieldTopActive.Color 3
ShieldTopActive.Shape BRIK { ShieldBottom_XYHDim + ShieldBottom_CoverThickness -  ShieldTop_CoverThickness } { ShieldBottom_XYHDim + ShieldBottom_CoverThickness -  ShieldTop_CoverThickness } { ShieldTop_ThickHDim }
ShieldTopActive.Position 0.0 0.0 0.0
ShieldTopActive.Mother ShieldTop






