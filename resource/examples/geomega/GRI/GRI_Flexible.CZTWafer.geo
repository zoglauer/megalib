
Include GRI_Flexible.Intro.geo


# Arrangement of the bottom layer

# One basic wafer
Volume CZTWafer_B
CZTWafer_B.Material active_czt_recoil
CZTWafer_B.Visibility 1
CZTWafer_B.Color 4
CZTWafer_B.Shape BRIK CZT_Wafer_XYHDim CZT_Wafer_XYHDim CZT_Wafer_ZHDim

# Additional 2nd level constants
Constant CZT_BLayer_XHDim {CZT_BLayer_NX*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)}
Constant CZT_BLayer_YHDim {CZT_BLayer_NY*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)}
Constant CZT_BLayer_ZHDim {CZT_Wafer_ZHDim + CZT_BLayer_Alumina_ZHDim + CZT_BLayer_Electronics_ZHDim}

Volume CZT_BLayer
CZT_BLayer.Material vacuum
CZT_BLayer.Visibility 0
CZT_BLayer.Shape BRIK  CZT_BLayer_XHDim CZT_BLayer_YHDim CZT_BLayer_ZHDim
#CZT_BLayer.Mother VAC0
#CZT_BLayer.Position 0.0 0.0 0.0


For X  CZT_BLayer_NX  {-(CZT_BLayer_NX-1)*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)}  {2*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)} 
 For Y  CZT_BLayer_NY  {-(CZT_BLayer_NY-1)*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)}  {2*(CZT_Wafer_XYHDim+CZT_BLayer_Gap_HDim)}
   CZTWafer_B.Copy CZTWafer_BCopy%X%Y
   CZTWafer_BCopy%X%Y.Mother CZT_BLayer
   CZTWafer_BCopy%X%Y.Position $X $Y {CZT_BLayer_ZHDim - CZT_Wafer_ZHDim}
 Done
Done

# Fills the complete base of the detector
Volume CZT_BLayer_Base1
CZT_BLayer_Base1.Material alumina
CZT_BLayer_Base1.Color 5
CZT_BLayer_Base1.Shape BRIK CZT_BLayer_XHDim CZT_BLayer_YHDim CZT_BLayer_Alumina_ZHDim
CZT_BLayer_Base1.Mother CZT_BLayer
CZT_BLayer_Base1.Position 0 0 {-CZT_BLayer_ZHDim + 2*CZT_BLayer_Electronics_ZHDim + CZT_BLayer_Alumina_ZHDim}

Volume CZT_BLayer_Base2
CZT_BLayer_Base2.Material electr
CZT_BLayer_Base2.Color 5
CZT_BLayer_Base2.Shape BRIK CZT_BLayer_XHDim CZT_BLayer_YHDim CZT_BLayer_Electronics_ZHDim
CZT_BLayer_Base2.Mother CZT_BLayer
CZT_BLayer_Base2.Position 0 0 {-CZT_BLayer_ZHDim + CZT_BLayer_Electronics_ZHDim}







# Arrangement of the bottom layer

# One basic wafer
Volume CZTWafer_S
CZTWafer_S.Material active_czt_recoil
CZTWafer_S.Visibility 1
CZTWafer_S.Color 4
CZTWafer_S.Shape BRIK CZT_Wafer_XYHDim CZT_Wafer_XYHDim CZT_Wafer_ZHDim

# Additional 2nd level constants
Constant CZT_SLayer_XHDim {CZT_SLayer_NX*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)}
Constant CZT_SLayer_YHDim {CZT_SLayer_NY*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)}
Constant CZT_SLayer_ZHDim {CZT_Wafer_ZHDim + CZT_SLayer_Alumina_ZHDim + CZT_SLayer_Electronics_ZHDim}

Volume CZT_SLayer
CZT_SLayer.Material vacuum
CZT_SLayer.Visibility 0
CZT_SLayer.Shape BRIK  CZT_SLayer_XHDim CZT_SLayer_YHDim CZT_SLayer_ZHDim
#CZT_SLayer.Mother VAC0
#CZT_SLayer.Position 0.0 0.0 0.0

For X  CZT_SLayer_NX  {-(CZT_SLayer_NX-1)*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)}  {2*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)} 
 For Y  CZT_SLayer_NY  {-(CZT_SLayer_NY-1)*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)}  {2*(CZT_Wafer_XYHDim+CZT_SLayer_Gap_HDim)}
   CZTWafer_S.Copy CZTWafer_SCopy%X%Y
   CZTWafer_SCopy%X%Y.Mother CZT_SLayer
   CZTWafer_SCopy%X%Y.Position $X $Y {CZT_SLayer_ZHDim - CZT_Wafer_ZHDim}
 Done
Done

# Fills the complete base of the detector
Volume CZT_SLayer_Base1
CZT_SLayer_Base1.Material alumina
CZT_SLayer_Base1.Color 5
CZT_SLayer_Base1.Shape BRIK CZT_SLayer_XHDim CZT_SLayer_YHDim CZT_SLayer_Alumina_ZHDim
CZT_SLayer_Base1.Mother CZT_SLayer
CZT_SLayer_Base1.Position 0 0 {-CZT_SLayer_ZHDim + 2*CZT_SLayer_Electronics_ZHDim + CZT_SLayer_Alumina_ZHDim}

Volume CZT_SLayer_Base2
CZT_SLayer_Base2.Material electr
CZT_SLayer_Base2.Color 5
CZT_SLayer_Base2.Shape BRIK CZT_SLayer_XHDim CZT_SLayer_YHDim CZT_SLayer_Electronics_ZHDim
CZT_SLayer_Base2.Mother CZT_SLayer
CZT_SLayer_Base2.Position 0 0 {-CZT_SLayer_ZHDim + CZT_SLayer_Electronics_ZHDim}




