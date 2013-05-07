//
//
// This is a simplified 8cm calorimeter as it is implemented for the prototype:
//
// It needs an included Materials.geo-file



// 8cm CsI-bar
Volume CsICrystal8
CsICrystal8.Material CsI
CsICrystal8.Visibility 0
CsICrystal8.Shape BRIK 0.25 0.25 4.0

// Arrange 11 bars in a volume
Volume CsICrystal8Block       
CsICrystal8Block.Material Millipore
CsICrystal8Block.Visibility 0
CsICrystal8Block.Shape BRIK 0.27  2.97  4.0

// The pixeldetectors of the block
Volume PixelDetectorCal8         
PixelDetectorCal8.Material SiliconPIN
PixelDetectorCal8.Visibility 0
PixelDetectorCal8.Color 5
PixelDetectorCal8.Shape BRIK 2.97 2.97 0.015

// Some plastic for the blocks
Volume HybridCal8         
HybridCal8.Material PCB
HybridCal8.Visibility 0
HybridCal8.Color 2
HybridCal8.Shape BRIK 2.97 2.97 0.1

// Arrange 10 of the 12-bar-volumes to a block
Volume Calorimeter8        
Calorimeter8.Material Aluminium
Calorimeter8.Color 38
Calorimeter8.Visibility 1
Calorimeter8.Shape BRIK 3.1 3.1 4.33



// Define 12 large crystals in a row

CsICrystal8.Copy CsICrystal8N00
CsICrystal8N00.Position  0.0 -2.7  0.0
CsICrystal8N00.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N01
CsICrystal8N01.Position  0.0 -2.16  0.0
CsICrystal8N01.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N02
CsICrystal8N02.Position  0.0 -1.62  0.0
CsICrystal8N02.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N03
CsICrystal8N03.Position  0.0 -1.08  0.0
CsICrystal8N03.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N04
CsICrystal8N04.Position  0.0 -0.54  0.0
CsICrystal8N04.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N05
CsICrystal8N05.Position  0.0  0.0  0.0
CsICrystal8N05.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N06
CsICrystal8N06.Position  0.0 0.54  0.0
CsICrystal8N06.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N07
CsICrystal8N07.Position 0.0  1.08  0.0
CsICrystal8N07.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N08
CsICrystal8N08.Position 0.0 1.62  0.0
CsICrystal8N08.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N09
CsICrystal8N09.Position 0.0 2.16  0.0
CsICrystal8N09.Mother CsICrystal8Block

CsICrystal8.Copy CsICrystal8N10
CsICrystal8N10.Position 0.0 2.7  0.0
CsICrystal8N10.Mother CsICrystal8Block



// Now put 10 rows into an array

CsICrystal8Block.Copy CsICrystal8BlockN01
CsICrystal8BlockN01.Position 2.7 0.0 0.0
CsICrystal8BlockN01.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN02
CsICrystal8BlockN02.Position 2.16 0.0 0.0
CsICrystal8BlockN02.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN03
CsICrystal8BlockN03.Position 1.62 0.0 0.0
CsICrystal8BlockN03.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN04
CsICrystal8BlockN04.Position 1.08 0.0 0.0
CsICrystal8BlockN04.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN05
CsICrystal8BlockN05.Position 0.54 0.0 0.0
CsICrystal8BlockN05.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN06
CsICrystal8BlockN06.Position 0.0 0.0 0.0
CsICrystal8BlockN06.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN07
CsICrystal8BlockN07.Position -0.54 0.0 0.0
CsICrystal8BlockN07.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN08
CsICrystal8BlockN08.Position -1.08 0.0 0.0
CsICrystal8BlockN08.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN09
CsICrystal8BlockN09.Position -1.62 0.0 0.0
CsICrystal8BlockN09.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN10
CsICrystal8BlockN10.Position -2.16 0.0 0.0
CsICrystal8BlockN10.Mother Calorimeter8

CsICrystal8Block.Copy CsICrystal8BlockN11
CsICrystal8BlockN11.Position -2.7 0.0 0.0
CsICrystal8BlockN11.Mother Calorimeter8


// Photodiode arrays

PixelDetectorCal8.Copy PixelDetectorCal8N01
PixelDetectorCal8N01.Position 0.0 0.0 4.015
PixelDetectorCal8N01.Mother Calorimeter8 

PixelDetectorCal8.Copy PixelDetectorCal8N02
PixelDetectorCal8N02.Position 0.0 0.0 -4.015
PixelDetectorCal8N02.Mother Calorimeter8 


// Plastic hybrids:

HybridCal8.Copy HybridCal8N01
HybridCal8N01.Position 0.0 0.0 4.13
HybridCal8N01.Mother Calorimeter8

HybridCal8.Copy HybridCal8N02
HybridCal8N02.Position 0.0 0.0 -4.13
HybridCal8N02.Mother Calorimeter8