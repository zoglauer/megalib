
Name GRI_Flexible
Version 1.0

SurroundingSphere 110  0.0  0.0  0.0  110.0

DetectorSearchTolerance 0.00001

Include GRI_Flexible.Materials.geo


# World volume
Volume VAC0
VAC0.Visibility 1
VAC0.Material vacuum
VAC0.Shape TUBS 0 300 400 0 360 
VAC0.Mother 0

# Si Wafer constants
Constant Si_Wafer_XYHDim 5.15
Constant Si_Wafer_ZHDim  0.075

# Si Layer
Constant Si_Electronics_WidthHDim  0.5
Constant Si_Electronics_ZHDim  0.05
Constant Si_Peek_WidthHDim  0.1
Constant Si_Layer_NX 1
# Numbers have to be equal for the moment!
Constant Si_Layer_NY Si_Layer_NX


# CZT Wafer constants
Constant CZT_Wafer_XYHDim 1.0 
Constant CZT_Wafer_ZHDim  0.35 


# CZT Bottom layer 
Constant CZT_BLayer_NX 6
# Numbers have to be equal for the moment!
Constant CZT_BLayer_NY CZT_BLayer_NX
Constant CZT_BLayer_Gap_HDim 0.0125 
Constant CZT_BLayer_Alumina_ZHDim 0.0375
Constant CZT_BLayer_Electronics_ZHDim 0.025 


# CZT Side layer (Y is height!)
Constant CZT_SLayer_NX 6
Constant CZT_SLayer_NY 16
Constant CZT_SLayer_Gap_HDim           CZT_BLayer_Gap_HDim
Constant CZT_SLayer_Alumina_ZHDim      CZT_BLayer_Alumina_ZHDim
Constant CZT_SLayer_Electronics_ZHDim  CZT_BLayer_Electronics_ZHDim    


#
Constant Si_Layers    35
Constant Si_LayerDistance 0.4

Constant CZT_Layers_Z 10
Constant CZT_Layers_XY 4
Constant CZT_LayerDistance 1.2
Constant CZT_SLayers_BottomDistance {3*CZT_LayerDistance}


# Shields:
Constant Shield_To_CZT_Distance  0.5

# Bottom:
Constant ShieldBottom_ActiveMaterial  bgo
Constant ShieldBottom_CoverMaterial   c_epoxy
Constant ShieldBottom_ThickHDim       2.5
Constant ShieldBottom_CoverThickness  0.05


# Side
Constant ShieldSide_ActiveMaterial  bgo
Constant ShieldSide_CoverMaterial   c_epoxy
Constant ShieldSide_HeightHDim      23.0
Constant ShieldSide_ThickHDim       1.0
Constant ShieldSide_CoverThickness  0.05


# Top
Constant ShieldTop_ActiveMaterial  plastic
Constant ShieldTop_CoverMaterial   INST_ACsupport
Constant ShieldTop_ThickHDim       0.375
Constant ShieldTop_CoverThickness  0.05


