// Setup file for the MEGA prototype in the Duke configuration

Name MEGAPrototype_Duke_v3.0
Version 3.0

SurroundingSphere 200  0.0  0.0  20.0  200.0



// Include section

// NEEDS THIS LINE TO VIEW ALONE:
// Include Materials.geo


// World volume section

// Volume WorldVolume             
// WorldVolume.Material Air
// WorldVolume.Visibility 0
// WorldVolume.Shape BRIK 1000. 1000. 1000.
// WorldVolume.Mother 0

Volume 8cm_CsICore
8cm_CsICore.Material Air
8cm_CsICore.Visibility 2
8cm_CsICore.Shape BRIK 2.885 3.455. 4.
//8cm_CsICore.Virtual 1
// NEEDS THIS LINE TO VIEW ALONE:
// 8cm_CsICore.Mother 0



// Basic Volume Types:

// CsI Crystal Cell:
Volume 8cm_CsICrystal
8cm_CsICrystal.Material CsI
8cm_CsICrystal.Visibility 1
8cm_CsICrystal.Color 4
8cm_CsICrystal.Shape BRIK 0.25 0.25 4.0

// CsI Crystal Block of 10 cells:
Volume 8cm_CsICrystalBlock
8cm_CsICrystalBlock.Material Millipore
8cm_CsICrystalBlock.Visibility 1  // 1
8cm_CsICrystalBlock.Shape BRIK 2.885 0.285 4.
8cm_CsICrystalBlock.Color 3


// Millipore Layer:
Volume 8cm_MilliPoreLayer
8cm_MilliPoreLayer.Material Millipore
8cm_MilliPoreLayer.Visibility 1  // 1
8cm_MilliPoreLayer.Color 3
8cm_MilliPoreLayer.Shape BRIK 2.885 0.0175 4.0




// define a CsI Crystal Volume row consisting of 10 CsI cells:

8cm_CsICrystal.Copy 8cm_CsICrystalN04
8cm_CsICrystalN04.Position 0.285 0. 0.
8cm_CsICrystalN04.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN03
8cm_CsICrystalN03.Position 0.855  0. 0.
8cm_CsICrystalN03.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN02
8cm_CsICrystalN02.Position 1.425  0. 0.
8cm_CsICrystalN02.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN01
8cm_CsICrystalN01.Position 1.995  0. 0.
8cm_CsICrystalN01.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN00
8cm_CsICrystalN00.Position 2.565  0. 0.
8cm_CsICrystalN00.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN05
8cm_CsICrystalN05.Position -0.285 0. 0.
8cm_CsICrystalN05.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN06
8cm_CsICrystalN06.Position -0.855  0. 0.
8cm_CsICrystalN06.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN07
8cm_CsICrystalN07.Position -1.425  0. 0.
8cm_CsICrystalN07.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN08
8cm_CsICrystalN08.Position -1.995  0. 0.
8cm_CsICrystalN08.Mother 8cm_CsICrystalBlock

8cm_CsICrystal.Copy 8cm_CsICrystalN09
8cm_CsICrystalN09.Position -2.565  0. 0.
8cm_CsICrystalN09.Mother 8cm_CsICrystalBlock


// 10 rows:

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN0
8cm_CsICrystalBlockN0.Position 0. 0.285 0.
8cm_CsICrystalBlockN0.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN1
8cm_CsICrystalBlockN1.Position 0. -0.285 0.
8cm_CsICrystalBlockN1.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN2
8cm_CsICrystalBlockN2.Position 0. 0.855 0.
8cm_CsICrystalBlockN2.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN3
8cm_CsICrystalBlockN3.Position 0. -0.855 0.
8cm_CsICrystalBlockN3.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN4
8cm_CsICrystalBlockN4.Position 0. 1.425 0.
8cm_CsICrystalBlockN4.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN5
8cm_CsICrystalBlockN5.Position 0. -1.425 0.
8cm_CsICrystalBlockN5.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN6
8cm_CsICrystalBlockN6.Position 0. 1.995 0.
8cm_CsICrystalBlockN6.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN7
8cm_CsICrystalBlockN7.Position 0. -1.995 0.
8cm_CsICrystalBlockN7.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN8
8cm_CsICrystalBlockN8.Position 0. 2.565 0.
8cm_CsICrystalBlockN8.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN9
8cm_CsICrystalBlockN9.Position 0. -2.565 0.
8cm_CsICrystalBlockN9.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN10
8cm_CsICrystalBlockN10.Position 0. 3.135 0.
8cm_CsICrystalBlockN10.Mother 8cm_CsICore

8cm_CsICrystalBlock.Copy 8cm_CsICrystalBlockN11
8cm_CsICrystalBlockN11.Position 0. -3.135 0.
8cm_CsICrystalBlockN11.Mother 8cm_CsICore


// plus two layers of millipore on each end of the rows:

8cm_MilliPoreLayer.Copy 8cm_MilliPoreLayerA
8cm_MilliPoreLayerA.Position 0. -3.4375 0.
8cm_MilliPoreLayerA.Mother 8cm_CsICore

8cm_MilliPoreLayer.Copy 8cm_MilliPoreLayerB
8cm_MilliPoreLayerB.Position 0. 3.4375 0.
8cm_MilliPoreLayerB.Mother 8cm_CsICore



