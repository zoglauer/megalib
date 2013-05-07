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

Volume 4cm_CsICore
4cm_CsICore.Material Air
4cm_CsICore.Visibility 2
4cm_CsICore.Shape BRIK 2.885 3.455. 2.
// NEEDS THIS LINE TO VIEW ALONE:
// 4cm_CsICore.Mother 0



// Basic Volume Types:

// CsI Crystal Cell:
Volume 4cm_CsICrystal
4cm_CsICrystal.Material CsI
4cm_CsICrystal.Visibility 1
4cm_CsICrystal.Color 4
4cm_CsICrystal.Shape BRIK 0.25 0.25 2.0

// 4cm_CsI Crystal Block of 10 cells:
Volume 4cm_CsICrystalBlock
4cm_CsICrystalBlock.Material Millipore
4cm_CsICrystalBlock.Visibility 1 //1
4cm_CsICrystalBlock.Shape BRIK 2.885 0.285 2.
4cm_CsICrystalBlock.Color 3


// Millipore Layer:
Volume 4cm_MilliPoreLayer
4cm_MilliPoreLayer.Material Millipore
4cm_MilliPoreLayer.Visibility 1 //1
4cm_MilliPoreLayer.Color 3
4cm_MilliPoreLayer.Shape BRIK 2.885 0.0175 2.0




// define a CsI Crystal Volume row consisting of 10 CsI cells:

4cm_CsICrystal.Copy 4cm_CsICrystalN04
4cm_CsICrystalN04.Position 0.285 0. 0.
4cm_CsICrystalN04.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN03
4cm_CsICrystalN03.Position 0.855  0. 0.
4cm_CsICrystalN03.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN02
4cm_CsICrystalN02.Position 1.425  0. 0.
4cm_CsICrystalN02.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN01
4cm_CsICrystalN01.Position 1.995  0. 0.
4cm_CsICrystalN01.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN00
4cm_CsICrystalN00.Position 2.565  0. 0.
4cm_CsICrystalN00.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN05
4cm_CsICrystalN05.Position -0.285 0. 0.
4cm_CsICrystalN05.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN06
4cm_CsICrystalN06.Position -0.855  0. 0.
4cm_CsICrystalN06.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN07
4cm_CsICrystalN07.Position -1.425  0. 0.
4cm_CsICrystalN07.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN08
4cm_CsICrystalN08.Position -1.995  0. 0.
4cm_CsICrystalN08.Mother 4cm_CsICrystalBlock

4cm_CsICrystal.Copy 4cm_CsICrystalN09
4cm_CsICrystalN09.Position -2.565  0. 0.
4cm_CsICrystalN09.Mother 4cm_CsICrystalBlock


// 10 rows:

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN0
4cm_CsICrystalBlockN0.Position 0. 0.285 0.
4cm_CsICrystalBlockN0.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN1
4cm_CsICrystalBlockN1.Position 0. -0.285 0.
4cm_CsICrystalBlockN1.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN2
4cm_CsICrystalBlockN2.Position 0. 0.855 0.
4cm_CsICrystalBlockN2.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN3
4cm_CsICrystalBlockN3.Position 0. -0.855 0.
4cm_CsICrystalBlockN3.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN4
4cm_CsICrystalBlockN4.Position 0. 1.425 0.
4cm_CsICrystalBlockN4.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN5
4cm_CsICrystalBlockN5.Position 0. -1.425 0.
4cm_CsICrystalBlockN5.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN6
4cm_CsICrystalBlockN6.Position 0. 1.995 0.
4cm_CsICrystalBlockN6.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN7
4cm_CsICrystalBlockN7.Position 0. -1.995 0.
4cm_CsICrystalBlockN7.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN8
4cm_CsICrystalBlockN8.Position 0. 2.565 0.
4cm_CsICrystalBlockN8.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN9
4cm_CsICrystalBlockN9.Position 0. -2.565 0.
4cm_CsICrystalBlockN9.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN10
4cm_CsICrystalBlockN10.Position 0. 3.135 0.
4cm_CsICrystalBlockN10.Mother 4cm_CsICore

4cm_CsICrystalBlock.Copy 4cm_CsICrystalBlockN11
4cm_CsICrystalBlockN11.Position 0. -3.135 0.
4cm_CsICrystalBlockN11.Mother 4cm_CsICore


// plus two layers of millipore on each end of the rows:

4cm_MilliPoreLayer.Copy 4cm_MilliPoreLayerA
4cm_MilliPoreLayerA.Position 0. -3.4375 0.
4cm_MilliPoreLayerA.Mother 4cm_CsICore

4cm_MilliPoreLayer.Copy 4cm_MilliPoreLayerB
4cm_MilliPoreLayerB.Position 0. 3.4375 0.
4cm_MilliPoreLayerB.Mother 4cm_CsICore


