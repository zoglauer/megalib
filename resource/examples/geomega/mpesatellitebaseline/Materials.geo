// 

// This file has to be included via:
// Include Materials.geo

// Defined materials:
// 
// Vacuum
// Air
// Pressurized air
// Silicon
// CsI
// PCB
// Aluminium
// NE110
// Millipore

Material CsI
CsI.Density                     4.5
CsI.Component                   132.9 55. 1
CsI.Component                   126.7 53. 1 
CsI.Sensitivity                 1

Material Vacuum                          
Vacuum.Density                  1E-16         
Vacuum.RadiationLength          1E+16          
Vacuum.Component                1 1 1 
Vacuum.Sensitivity              1
	                                       
Material Air                          
Air.Density                     1.205E-03
Air.RadiationLength             30423         
Air.Component                   14.0 7.0 4 
Air.Component                   16.0 8.0 1 
Air.Sensitivity                 1
	                                       
Material PressurizedAir                          
PressurizedAir.Density          2.5        
PressurizedAir.Component        14.0  7.0  4 // N
PressurizedAir.Component        16.0  8.0  1 // O
PressurizedAir.Sensitivity      1

Material Silicon
Silicon.Density                 2.33
Silicon.RadiationLength         9.35
Silicon.Component               28.08 14.0 1
Silicon.Sensitivity             1   

Material SiliconPIN
SiliconPIN.Density                 2.33
SiliconPIN.RadiationLength         9.35
SiliconPIN.Component               28.08 14.0 1
SiliconPIN.Sensitivity             1   

Material Germanium
Germanium.Density               5.323
Germanium.RadiationLength       2.30
Germanium.Component             72.59 32.0 1
Germanium.Sensitivity           1   

Material Xenon
Xenon.Density               3.52
Xenon.RadiationLength       2.40
Xenon.Component             131.29 54.0 1
Xenon.Sensitivity           1   

Material Pb
Pb.Density               11.35
Pb.RadiationLength       0.56
Pb.Component             207.19 82.0 1
Pb.Sensitivity           1   

Material NaI
NaI.Density                     43.67
NaI.Component                   22.98 11. 1
NaI.Component                   126.7 53. 1 
NaI.Sensitivity                 1

Material PCB
PCB.Density                     1.2
PCB.Component                   1.0   1.0  8   // H
PCB.Component                   12.1  6.0  5   // C
PCB.Component                   16.0  8.0  2   // O
PCB.Sensitivity                 1

Material MgS
MgS.Density                     1.0
MgS.Component                   24.30 12.0  1   // Mg
MgS.Component                   32.06  16.0  1   // S
MgS.Sensitivity                 1

Material CdTe
CdTe.Density                     1.0
CdTe.Component                   112.4 48.0  1   // Cd
CdTe.Component                   127.6  52.0  1   // Te
CdTe.Sensitivity                 1

Material NE213
NE213.Density                     1.0
NE213.Component                   1.0 1.0  10   // H
NE213.Component                   12.1 6.0  8   // C
NE213.Sensitivity                 1

Material Aluminium
Aluminium.Density               2.7
Aluminium.RadiationLength       8.9
Aluminium.Component             27.0 13.0 1
Aluminium.Sensitivity           1   

Material Diamant
Diamant.Density               3.51 
Diamant.RadiationLength       12.2
Diamant.Component             12.0 6.0 1
Diamant.Sensitivity           1   

Material NE110
NE110.Density                   1.03
NE110.Component                 1.0   1.0  84  // H
NE110.Component                 12.1  6.0  74  // C
NE110.Component                 16.0  8.0   1  // O
NE110.Sensitivity               1

Material Millipore                                        
Millipore.Density               1.00
Millipore.Component             1.0   1.0  10  // H
Millipore.Component             12.1  6.0  6   // C
Millipore.Component             16.0  8.0  5   // O
Millipore.Sensitivity           1
