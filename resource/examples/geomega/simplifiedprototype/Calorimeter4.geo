//
//
// This is a simplified 8cm calorimeter as it is implemented for the prototype:
//
// It needs an included Materials.geo-file



// 8cm CsI-bar
Volume CsICrystal4
CsICrystal4.Material CsI
CsICrystal4.Visibility 0
CsICrystal4.Shape BRIK 0.25 0.25 2.0

// Arrange 12 bars in a volume
Volume CsICrystal4Block       
CsICrystal4Block.Material Millipore
CsICrystal4Block.Visibility 0
CsICrystal4Block.Shape BRIK 0.285  3.42  2.0

// The pixeldetectors of the block
Volume PixelDetectorCal4         
PixelDetectorCal4.Material SiliconPIN
PixelDetectorCal4.Visibility 0
PixelDetectorCal4.Shape BRIK 2.85 3.42 0.015

// Some plastic for the blocks
Volume HybridCal4         
HybridCal4.Material PCB
HybridCal4.Visibility 0
HybridCal4.Color 2
HybridCal4.Shape BRIK 2.85 3.42 0.1

// A plastic connector
#Volume Connector4         
#Connector4.Material PCB
#Connector4.Visibility 1
#Connector4.Color 2
#Connector4.Shape BRIK 2.0 0.25 0.25

// Arrange 10 of the 12-bar-volumes to a block
Volume Calorimeter4        
Calorimeter4.Material Aluminium
Calorimeter4.Color 0
Calorimeter4.Visibility 1
#Calorimeter4.LineWidth 3
Calorimeter4.Shape BRIK 3.05 3.62 2.3


// Define 12 large crystals in a row

CsICrystal4.Copy CsICrystal4N00
CsICrystal4N00.Position  0.0 -3.135  0.0
CsICrystal4N00.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N01
CsICrystal4N01.Position  0.0 -2.565  0.0
CsICrystal4N01.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N02
CsICrystal4N02.Position  0.0 -1.995  0.0
CsICrystal4N02.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N03
CsICrystal4N03.Position  0.0 -1.425  0.0
CsICrystal4N03.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N04
CsICrystal4N04.Position  0.0 -0.855  0.0
CsICrystal4N04.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N05
CsICrystal4N05.Position  0.0 -0.285  0.0
CsICrystal4N05.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N06
CsICrystal4N06.Position  0.0 0.285  0.0
CsICrystal4N06.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N07
CsICrystal4N07.Position 0.0  0.855  0.0
CsICrystal4N07.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N08
CsICrystal4N08.Position 0.0 1.425  0.0
CsICrystal4N08.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N09
CsICrystal4N09.Position 0.0 1.995  0.0
CsICrystal4N09.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N10
CsICrystal4N10.Position 0.0 2.565  0.0
CsICrystal4N10.Mother CsICrystal4Block

CsICrystal4.Copy CsICrystal4N11
CsICrystal4N11.Position  0.0 3.135  0.0
CsICrystal4N11.Mother CsICrystal4Block



// Now put 10 rows into an array

CsICrystal4Block.Copy CsICrystal4BlockN01
CsICrystal4BlockN01.Position 2.565  0.0  0.115
CsICrystal4BlockN01.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN02
CsICrystal4BlockN02.Position 1.995  0.0  0.115
CsICrystal4BlockN02.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN03
CsICrystal4BlockN03.Position 1.425  0.0  0.115
CsICrystal4BlockN03.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN04
CsICrystal4BlockN04.Position 0.855  0.0  0.115
CsICrystal4BlockN04.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN05
CsICrystal4BlockN05.Position 0.285  0.0  0.115
CsICrystal4BlockN05.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN06
CsICrystal4BlockN06.Position -0.285  0.0  0.115
CsICrystal4BlockN06.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN07
CsICrystal4BlockN07.Position -0.855  0.0  0.115
CsICrystal4BlockN07.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN08
CsICrystal4BlockN08.Position -1.425  0.0  0.115
CsICrystal4BlockN08.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN09
CsICrystal4BlockN09.Position -1.995  0.0  0.115
CsICrystal4BlockN09.Mother Calorimeter4

CsICrystal4Block.Copy CsICrystal4BlockN10
CsICrystal4BlockN10.Position -2.565  0.0  0.115
CsICrystal4BlockN10.Mother Calorimeter4


// Photodiode arrays

PixelDetectorCal4.Copy PixelDetectorCal4N01
PixelDetectorCal4N01.Position 0.0 0.0 -1.9
PixelDetectorCal4N01.Mother Calorimeter4 


// Plastic hybrids:

HybridCal4.Copy HybridCal4N01
HybridCal4N01.Position 0.0 0.0 -2.015
HybridCal4N01.Mother Calorimeter4


// Plastic connectors:

#Connector4.Copy Connector4N01
#Connector4N01.Position 0.0 -4.0 -1.8
#Connector4N01.Mother Calorimeter4
