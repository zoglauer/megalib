//
//
// This is a simplified 8cm calorimeter as it is implemented for the prototype:
//
// It needs an included Materials.geo-file



// 8cm CsI-bar
Volume CsICrystal2
CsICrystal2.Material CsI
CsICrystal2.Visibility 0
CsICrystal2.Shape BRIK 0.25 0.25 1.0

// Arrange 12 bars in a volume
Volume CsICrystal2Block       
CsICrystal2Block.Material Millipore
CsICrystal2Block.Visibility 0
CsICrystal2Block.Shape BRIK 0.285  3.42  1.0

// The pixeldetectors of the block
Volume PixelDetectorCal2         
PixelDetectorCal2.Material SiliconPIN
PixelDetectorCal2.Visibility 0
PixelDetectorCal2.Shape BRIK 2.85 3.42 0.015

// Some plastic for the blocks
Volume HybridCal2         
HybridCal2.Material PCB
HybridCal2.Visibility 0
HybridCal2.Color 2
HybridCal2.Shape BRIK 2.85 3.42 0.1

// A plastic connector
#Volume Connector2         
#Connector2.Material PCB
#Connector2.Visibility 1
#Connector2.Color 2
#Connector2.Shape BRIK 2.0 0.25 0.25

// Arrange 10 of the 12-bar-volumes to a block
Volume Calorimeter2        
Calorimeter2.Material Aluminium
Calorimeter2.Color 0
Calorimeter2.Visibility 1
#Calorimeter2.LineWidth 3
Calorimeter2.Shape BRIK 3.05 3.62 1.3


// Define 12 large crystals in a row

CsICrystal2.Copy CsICrystal2N00
CsICrystal2N00.Position  0.0 -3.135  0.0
CsICrystal2N00.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N01
CsICrystal2N01.Position  0.0 -2.565  0.0
CsICrystal2N01.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N02
CsICrystal2N02.Position  0.0 -1.995  0.0
CsICrystal2N02.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N03
CsICrystal2N03.Position  0.0 -1.425  0.0
CsICrystal2N03.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N04
CsICrystal2N04.Position  0.0 -0.855  0.0
CsICrystal2N04.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N05
CsICrystal2N05.Position  0.0 -0.285  0.0
CsICrystal2N05.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N06
CsICrystal2N06.Position  0.0 0.285  0.0
CsICrystal2N06.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N07
CsICrystal2N07.Position 0.0  0.855  0.0
CsICrystal2N07.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N08
CsICrystal2N08.Position 0.0 1.425  0.0
CsICrystal2N08.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N09
CsICrystal2N09.Position 0.0 1.995  0.0
CsICrystal2N09.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N10
CsICrystal2N10.Position 0.0 2.565  0.0
CsICrystal2N10.Mother CsICrystal2Block

CsICrystal2.Copy CsICrystal2N11
CsICrystal2N11.Position  0.0 3.135  0.0
CsICrystal2N11.Mother CsICrystal2Block



// Now put 10 rows into an array

CsICrystal2Block.Copy CsICrystal2BlockN01
CsICrystal2BlockN01.Position 2.565  0.0  0.115
CsICrystal2BlockN01.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN02
CsICrystal2BlockN02.Position 1.995  0.0  0.115
CsICrystal2BlockN02.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN03
CsICrystal2BlockN03.Position 1.425  0.0  0.115
CsICrystal2BlockN03.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN04
CsICrystal2BlockN04.Position 0.855  0.0  0.115
CsICrystal2BlockN04.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN05
CsICrystal2BlockN05.Position 0.285  0.0  0.115
CsICrystal2BlockN05.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN06
CsICrystal2BlockN06.Position -0.285  0.0  0.115
CsICrystal2BlockN06.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN07
CsICrystal2BlockN07.Position -0.855  0.0  0.115
CsICrystal2BlockN07.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN08
CsICrystal2BlockN08.Position -1.425  0.0  0.115
CsICrystal2BlockN08.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN09
CsICrystal2BlockN09.Position -1.995  0.0  0.115
CsICrystal2BlockN09.Mother Calorimeter2

CsICrystal2Block.Copy CsICrystal2BlockN10
CsICrystal2BlockN10.Position -2.565  0.0  0.115
CsICrystal2BlockN10.Mother Calorimeter2


// Photodiode arrays

PixelDetectorCal2.Copy PixelDetectorCal2N01
PixelDetectorCal2N01.Position 0.0 0.0 -0.9
PixelDetectorCal2N01.Mother Calorimeter2 


// Plastic hybrids:

HybridCal2.Copy HybridCal2N01
HybridCal2N01.Position 0.0 0.0 -1.015
HybridCal2N01.Mother Calorimeter2


// Plastic connectors:

#Connector2.Copy Connector2N01
#Connector2N01.Position 0.0 -4.0 -0.8
#Connector2N01.Mother Calorimeter2
