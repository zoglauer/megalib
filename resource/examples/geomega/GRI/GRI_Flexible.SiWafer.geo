
Include GRI_Flexible.Intro.geo


Constant Si_Layer_XHDim  { (3+Si_Layer_NX)*Si_Peek_WidthHDim + 2*Si_Electronics_WidthHDim + Si_Layer_NX*Si_Wafer_XYHDim }
Constant Si_Layer_YHDim  { (3+Si_Layer_NY)*Si_Peek_WidthHDim + 2*Si_Electronics_WidthHDim + Si_Layer_NY*Si_Wafer_XYHDim }
Constant Si_Layer_ZHDim  Si_Wafer_ZHDim

# One basic Si layer
Volume SiLayer
SiLayer.Material Peek
SiLayer.Visibility 0
SiLayer.Color 5
SiLayer.Shape BRIK Si_Layer_XHDim Si_Layer_YHDim Si_Layer_ZHDim
#SiLayer.Mother VAC0
#SiLayer.Position  0.0  0.0  0.0

# One basic Si wafer
Volume SiWafer
SiWafer.Material active_si_recoil
SiWafer.Visibility 1
SiWafer.Color 5
SiWafer.Shape BRIK Si_Wafer_XYHDim Si_Wafer_XYHDim Si_Wafer_ZHDim

For X  Si_Layer_NX  {-(Si_Layer_NX-1)*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)}  {2*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)} 
  For Y  Si_Layer_NY  {-(Si_Layer_NY-1)*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)}  {2*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)}
    SiWafer.Copy SiWafer_Copy%X%Y
    SiWafer_Copy%X%Y.Mother SiLayer
    SiWafer_Copy%X%Y.Position $X $Y 0.0
  Done
Done

# One basic electronics board:
Volume SiBoard
SiBoard.Material vacuum
SiBoard.Visibility 1
SiBoard.Color 6
SiBoard.Shape BRIK Si_Wafer_XYHDim Si_Electronics_WidthHDim Si_Layer_ZHDim

Volume SiBoardElectronics
SiBoardElectronics.Material electr
SiBoardElectronics.Visibility 1
SiBoardElectronics.Color 4
SiBoardElectronics.Shape BRIK Si_Wafer_XYHDim Si_Electronics_WidthHDim Si_Electronics_ZHDim
SiBoardElectronics.Position 0.0 0.0 {Si_Layer_ZHDim - Si_Electronics_ZHDim } 
SiBoardElectronics.Mother SiBoard

# x-side
For X  2  {-Si_Layer_XHDim + 2*Si_Peek_WidthHDim + Si_Electronics_WidthHDim}  {2*(Si_Layer_XHDim - 2*Si_Peek_WidthHDim - Si_Electronics_WidthHDim)} 
  For Y  Si_Layer_NY  {-(Si_Layer_NY-1)*(Si_Wafer_XYHDim+Si_Peek_WidthHDim) }  {2*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)}
    SiBoard.Copy SiBoard_CopyA%X%Y
    SiBoard_CopyA%X%Y.Mother SiLayer
    SiBoard_CopyA%X%Y.Rotation 0.0 0.0 90.0
    SiBoard_CopyA%X%Y.Position $X $Y 0.0
  Done
Done

# y-side
For X  Si_Layer_NX  {-(Si_Layer_NX-1)*(Si_Wafer_XYHDim+Si_Peek_WidthHDim) }  {2*(Si_Wafer_XYHDim+Si_Peek_WidthHDim)}
  For Y  2  {-Si_Layer_YHDim + 2*Si_Peek_WidthHDim + Si_Electronics_WidthHDim}  {2*(Si_Layer_YHDim - 2*Si_Peek_WidthHDim - Si_Electronics_WidthHDim)} 
    SiBoard.Copy SiBoard_CopyB%X%Y
    SiBoard_CopyB%X%Y.Mother SiLayer
    SiBoard_CopyB%X%Y.Position $X $Y 0.0
  Done
Done
