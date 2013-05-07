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
// Pb
// Xenon
// PCB
// Aluminium
// NE110
// Millipore

// Iron
// Peek
// Steel
// Plastic_Isolation
// Copper

// Glas
// PMT_Plastic
// ACSPaper
// Nylon
// Wood


Material Vacuum                          
Vacuum.Density                  1.0E-12   
Vacuum.RadiationLength          1E+12       
Vacuum.Component                1.0 1.0 1 
Vacuum.Sensitivity              1
	                                       
Material Air                          
Air.Density                     1.205E-03
Air.Component                   14.0  7.0  4 // N
Air.Component                   16.0  8.0  1 // O
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

Material CsI
CsI.Density                     4.5
CsI.Component                   132.9 55. 1
CsI.Component                   126.7 53. 1 
CsI.Sensitivity                 1

Material PCB
PCB.Density                     1.2
PCB.Component                   1.0   1.0  8   // H
PCB.Component                   12.1  6.0  5   // C
PCB.Component                   16.0  8.0  2   // O
PCB.Sensitivity                 1

Material Aluminium
Aluminium.Density               2.7
Aluminium.RadiationLength       8.9
Aluminium.Component             27.0 13.0 1
Aluminium.Sensitivity           1   

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


// additional defined Materials:

Material Iron
Iron.Density 			7.87
Iron.Component  		55.847 26.0 1   // Fe
Iron.RadiationLength            1.75845
Iron.Sensitivity		1

// Values of Peek are not correct, except for the density (=1.31) !!!
Material Peek
Peek.Density                     1.31
Peek.Component                   1.0   1.0  8   // H
Peek.Component                   12.1  6.0  5   // C
Peek.Component                   16.0  8.0  2   // O
Peek.Sensitivity                 1

// Values of Steel:
Material Steel
Steel.Density 			7.87
Steel.Component  		55.85 26.0 365   // Fe
Steel.Component  		51.99 24.0  90   // Cr
Steel.Component  		58.69 28.0  42   // Ni
Steel.Component  		28.08 14.0   5   // Si
Steel.Component  		54.93 25.0   5   // Mn
Steel.Component  		12.01  6.0   5   // C
Steel.Sensitivity		1


// Values of Plastic Isolation  are not correct !!!
Material Plastic_Isolation
Plastic_Isolation.Density                     1.31
Plastic_Isolation.Component                   1.0   1.0  8   // H
Plastic_Isolation.Component                   12.1  6.0  5   // C
Plastic_Isolation.Component                   16.0  8.0  2   // O
Plastic_Isolation.Sensitivity                 1

// Values of Copper from the internet:
Material Copper
Copper.Density                     8.954
Copper.Component                   63.546  29.0  1   // Cu
Copper.RadiationLength             1.43655
Copper.Sensitivity                 1

// Values of Glass from the internet:
// As there are several values for glass density depending on the special type of glass I suppose an average value of 2.5 here.
Material Glass
Glass.Density                     2.5
Glass.Component                   28.0855   14.0  1   // Si
Glass.Component                   15.9994    8.0  2   // O
Glass.Sensitivity                 1

// Values of Tin from the internet:
Material Tin
Tin.Density                     7.29
Tin.Component                   118.710   50.0  1   // Sn
Tin.RadiationLength             1.20968
Tin.Sensitivity                 1

// Values of PMTPlastic are not correct !!! densitiy is supposed to be 1 g/cm^3
Material PMTPlastic
PMTPlastic.Density                     1.0
PMTPlastic.Component               1.0   1.0  8   // H
PMTPlastic.Component               12.1  6.0  5   // C
PMTPlastic.Component               16.0  8.0  2   // O
PMTPlastic.Sensitivity                 1


// Values of ACSPaper are not correct !!!
// I suppose the density could be about 1.0, but this is not for sure!!!
Material ACSPaper
ACSPaper.Density               1.00
ACSPaper.Component             1.0   1.0  10  // H
ACSPaper.Component             12.1  6.0  6   // C
ACSPaper.Component             16.0  8.0  5   // O
ACSPaper.Sensitivity           1




// I found several values for the density of nylon on different internet pages. Most of them ranged from 1.0 to 1.2,
// so I take the middle with 1.1.
// Note that the Component specification is still nonsense!
Material Nylon
Nylon.Density                     	1.13
Nylon.Component                     1.0   1.0  26  // H
Nylon.Component                     12.1  6.0  12  // C
Nylon.Component                     16.0  8.0   4  // O
Nylon.Component                   	14.0  7.0   2  // N
Nylon.Sensitivity                 	1



// Values execpt for density are not correct!!!
// For the desnsity of wood i found defferent values. For pure wood they range from 0.45 to 0.72,
// and for plywood it is no different! So I take an average value of 0.6
Material Plywood
Plywood.Density 			             0.6
Plywood.Component               1.0   1.0  8   // H
Plywood.Component               12.1  6.0  5   // C
Plywood.Component               16.0  8.0  2   // O
Plywood.Sensitivity                1
