// Materials used for all geometries
// In alphabetical order!


AbsorptionFileDirectory $(MEGALIB)/resource/examples/geomega/materials

// Values of ACSPaper are not correct !!!
// I suppose the density could be about 1.0, but this is not for sure!!!
Material ACSPaper
ACSPaper.Density               1.00
ACSPaper.Component             1.0   1.0  10  // H
ACSPaper.Component             12.1  6.0  6   // C
ACSPaper.Component             16.0  8.0  5   // O


// Air at sealevel and 20 degrees 
Material Air                          
Air.Density                     1.2041E-03     
Air.ComponentByMass             14.0    7.0  0.7556
Air.ComponentByMass             16.0    8.0  0.2315
Air.ComponentByMass             39.94  18.0  0.0129 


Material Aluminium
Aluminium.Density               2.7
Aluminium.Component             27.0 13.0 1


Material Alu6061
Alu6061.Density               2.6989
# radiation length not needed since this is a mixture ... ;-)
Alu6061.ComponentByMass       28.086  14.  0.0060
Alu6061.ComponentByMass       55.847  26.  0.0070
Alu6061.ComponentByMass       63.546  29.  0.0030
Alu6061.ComponentByMass       54.938  25.  0.0015
Alu6061.ComponentByMass       24.305  12.  0.0100
Alu6061.ComponentByMass       51.996  24.  0.0020
Alu6061.ComponentByMass       65.390  30.  0.0025
Alu6061.ComponentByMass       47.880  22.  0.0015
Alu6061.ComponentByMass       26.982  13.  0.9665

Material BGO            
BGO.Density               7.1
BGO.Component             209.0 83  4  // Bi
BGO.Component             72.61 32  3   // Ge 
BGO.Component             16.00 8  12   // O

Material CarbonCyanateEster
CarbonCyanateEster.Density          1.2
CarbonCyanateEster.ComponentByMass  12.1   6.0  0.9191
CarbonCyanateEster.ComponentByMass  14.0   7.0  0.0310
CarbonCyanateEster.ComponentByMass  16.0   8.0  0.0354
CarbonCyanateEster.ComponentByMass  1.0    1.0  0.0145

Material CdTe
CdTe.Density                     5.86
CdTe.Component                   112.4 48.0  1   // Cd
CdTe.Component                   127.6  52.0  1   // Te

Material CircuitBoard
CircuitBoard.Density            1.8
CircuitBoard.ComponentByMass    1.0    1.0  0.030  // H  
CircuitBoard.ComponentByMass    12.1   6.0  0.174  // C  
CircuitBoard.ComponentByMass    16.0   8.0  0.392  // O
CircuitBoard.ComponentByMass    27.0  13.0  0.100  // Al
CircuitBoard.ComponentByMass    28.1  14.0  0.244  // Si
CircuitBoard.ComponentByMass    55.8  26.0  0.010  // Fe
CircuitBoard.ComponentByMass    63.5  29.0  0.030  // Cu
CircuitBoard.ComponentByMass    118.7 50.0  0.010  // Sn
CircuitBoard.ComponentByMass    207.2 82.0  0.010  // Pb

// Values of Copper from the internet:
// Actually there are dozens of different versions of copper
Material Copper
Copper.Density                     8.954
Copper.Component                   63.546  29.0  1   // Cu
Copper.RadiationLength             1.43655

Material CsI
CsI.Density                     4.5
CsI.Component                   132.9 55. 1
CsI.Component                   126.7 53. 1 

Material CZT
CZT.Density                     6.06
CZT.ComponentByAtoms            112.411 48   9
CZT.ComponentByAtoms             65.39  30   1
CZT.ComponentByAtoms            127.6   52  10

Material Densimet180            
Densimet180.Density               18    
Densimet180.ComponentByMass       183.8  74  0.95
Densimet180.ComponentByMass       55.84  26  0.16
Densimet180.ComponentByMass       58.69  28  0.44

Material Diamant
Diamant.Density               3.51
Diamant.RadiationLength       12.15
Diamant.Component             12.0 6.0 1  

Material GePassive
GePassive.Density               5.323
GePassive.RadiationLength       2.30
GePassive.Component             72.59 32.0 1  

Material Germanium
Germanium.Density               5.323
Germanium.RadiationLength       2.30
Germanium.Component             72.59 32.0 1   

// Values of Glass from the internet:
// As there are several values for glass density depending on the special type of glass I suppose an average value of 2.5 here.
Material Glass
Glass.Density                     2.5
Glass.Component                   28.0855   14.0  1   // Si
Glass.Component                   15.9994    8.0  2   // O

Material LaBr3
LaBr3.Density                   5.29
LaBr3.Component                 138.9   57  1
LaBr3.Component                 79.9    35  3

Material Indium
Indium.Density                  7.310
Indium.Component                114.818     49.0  1
Indium.RadiationLength          1.21054

Material Iron
Iron.Density 			7.87
Iron.Component  		55.847 26.0 1   // Fe
Iron.RadiationLength            1.75845

Material MgS
MgS.Density                     1.0
MgS.Component                   24.30 12.0  1   // Mg
MgS.Component                   32.06  16.0  1   // S

Material Millipore                                        
Millipore.Density               1.00
Millipore.Component             1.0   1.0  10  // H
Millipore.Component             12.1  6.0  6   // C
Millipore.Component             16.0  8.0  5   // O

Material MLI
MLI.Density                  1.404
MLI.ComponentByMass          1.00797     1.0  0.0364
MLI.ComponentByMass          12.01115    6.0  0.6553
MLI.ComponentByMass          14.0067     7.0  0.0327
MLI.ComponentByMass          15.9994     8.0  0.2756

Material NE110
NE110.Density                   1.03
NE110.Component                 1.0   1.0  84  // H
NE110.Component                 12.1  6.0  74  // C
NE110.Component                 16.0  8.0   1  // O

Material NE213
NE213.Density                     1.0
NE213.Component                   1.0 1.0  10   // H
NE213.Component                   12.1 6.0  8   // C

Material NaI
NaI.Density                     3.67
NaI.Component                   22.98 11. 1
NaI.Component                   126.7 53. 1 

Material Nylon
Nylon.Density                     	1.13
Nylon.Component                     1.0   1.0  26  // H
Nylon.Component                     12.1  6.0  12  // C
Nylon.Component                     16.0  8.0   4  // O
Nylon.Component                   	14.0  7.0   2  // N

Material Pb
Pb.Density               11.35
Pb.RadiationLength       0.56
Pb.Component             207.19 82.0 1 
          
Material PCB
PCB.Density                     1.2
PCB.Component                   1.0   1.0  8   // H
PCB.Component                   12.1  6.0  5   // C
PCB.Component                   16.0  8.0  2   // O

// Values of Peek are not correct, except for the density (=1.31) !!!
Material Peek
Peek.Density                     1.31
Peek.Component                   1.0   1.0  8   // H
Peek.Component                   12.1  6.0  5   // C
Peek.Component                   16.0  8.0  2   // O

// Values of Plastic Isolation  are not correct !!!
Material Plastic_Isolation
Plastic_Isolation.Density                     1.31
Plastic_Isolation.Component                   1.0   1.0  8   // H
Plastic_Isolation.Component                   12.1  6.0  5   // C
Plastic_Isolation.Component                   16.0  8.0  2   // O

// Values execpt for density are not correct!!!
// For the density of wood I found defferent values. For pure wood they range from 0.45 to 0.72,
// and for plywood it is no different! So I take an average value of 0.6
Material Plywood
Plywood.Density 			             0.6
Plywood.Component               1.0   1.0  8   // H
Plywood.Component               12.1  6.0  5   // C
Plywood.Component               16.0  8.0  2   // O

// Values are a pure guess
Material PMTPlastic
PMTPlastic.Density                     1.0
PMTPlastic.Component               1.0   1.0  8   // H
PMTPlastic.Component               12.1  6.0  5   // C
PMTPlastic.Component               16.0  8.0  2   // O

Material PressurizedAir                          
PressurizedAir.Density          2.5        
PressurizedAir.Component        14.0  7.0  4 // N
PressurizedAir.Component        16.0  8.0  1 // O

Material ro3003   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro3003.Density            2.1
ro3003.ComponentByMass    12.011   6.0    0.1606        // C
ro3003.ComponentByMass    18.998   9.0    0.2544        // F
ro3003.ComponentByMass    16.0     8.0    0.3061        // O
ro3003.ComponentByMass    28.086  14.0    0.2522        // Si
ro3003.ComponentByMass   204.383  81.0    0.0269        // Tl

Material ro3006   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro3006.Density            2.6
ro3006.ComponentByMass    12.011   6.0    0.1219        // C
ro3006.ComponentByMass    18.998   9.0    0.1931        // F
ro3006.ComponentByMass    16.0     8.0    0.3074        // O
ro3006.ComponentByMass    28.086  14.0    0.1130        // Si
ro3006.ComponentByMass   204.383  81.0    0.2647        // Tl

Material ro3010   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro3010.Density            3.0
ro3010.ComponentByMass    12.011   6.0    0.1045        // C
ro3010.ComponentByMass    18.998   9.0    0.1655        // F
ro3010.ComponentByMass    16.0     8.0    0.3050        // O
ro3010.ComponentByMass    28.086  14.0    0.0397        // Si
ro3010.ComponentByMass   204.383  81.0    0.3854        // Tl

Material ro4003   // one of circuit board materials by Rogers Corporation
                  // used in NCT
ro4003.Density            1.79
ro4003.ComponentByMass    12.011   6.0    0.1520        // C
ro4003.ComponentByMass    16.0     8.0    0.4290        // O
ro4003.ComponentByMass     1.0     1.0    0.0180        // H
ro4003.ComponentByMass    28.086  14.0    0.3300        // Si
ro4003.ComponentByMass    40.078  20.0    0.0370        // Ca
ro4003.ComponentByMass    26.982  13.0    0.0190        // Al
ro4003.ComponentByMass    10.811   5.0    0.0110        // B
ro4003.ComponentByMass    39.098  19.0    0.0010        // K
ro4003.ComponentByMass    24.305  12.0    0.0050        // Mg

Material roTMM3   // one of circuit board materials by Rogers Corporation
                  // used in NCT
roTMM3.Density            1.78
roTMM3.ComponentByMass    12.011   6.0    0.1201        // C
roTMM3.ComponentByMass    16.0     8.0    0.4270        // O
roTMM3.ComponentByMass     1.0     1.0    0.0198        // H
roTMM3.ComponentByMass    28.086  14.0    0.3616        // Si
roTMM3.ComponentByMass    47.88   22.0    0.0153        // Ti
roTMM3.ComponentByMass   137.327  56.0    0.0098        // Ba
roTMM3.ComponentByMass    40.078  20.0    0.0022        // Ca
roTMM3.ComponentByMass    26.982  13.0    0.0011        // Al
roTMM3.ComponentByMass    10.811   5.0    0.0003        // B
roTMM3.ComponentByMass    24.305  12.0    0.0002        // Mg
roTMM3.ComponentByMass    32.066  16.0    0.0010        // S


Material Silicon
Silicon.Density                 2.33
Silicon.RadiationLength         9.35
Silicon.Component               28.08 14.0 1 

Material SiliconPIN
SiliconPIN.Density                 2.33
SiliconPIN.RadiationLength         9.35
SiliconPIN.Component               28.08 14.0 1

// Values of Steel are not exact, but the same as Iron:
Material Steel
Steel.Density 			7.87
Steel.Component  		55.85 26.0 365   // Fe
Steel.Component  		51.99 24.0  90   // Cr
Steel.Component  		58.69 28.0  42   // Ni
Steel.Component  		28.08 14.0   5   // Si
Steel.Component  		54.93 25.0   5   // Mn
Steel.Component  		12.01  6.0   5   // C


// stainless steel according to P.Turin for NCT screws
Material Steel_18_8
Steel_18_8.Density              7.87
Steel_18_8.ComponentByMass      51.99 24.0  0.18   // Cr
Steel_18_8.ComponentByMass      58.69 28.0  0.09   // Ni
Steel_18_8.ComponentByMass      12.01  6.0  0.013  // C
Steel_18_8.ComponentByMass      55.85 26.0  0.717   // Fe

// Values of Tin from the internet:
Material Tin
Tin.Density                     7.29
Tin.Component                   118.710   50.0  1   // Sn
Tin.RadiationLength             1.20968

Material Vacuum                          
Vacuum.Density                  1E-12         
Vacuum.RadiationLength          1E+12          
Vacuum.Component                1 1 1 

Material Xenon
Xenon.Density               3.52
Xenon.RadiationLength       2.40
Xenon.Component             131.29 54.0 1

Material XenonLXeGRIT
XenonLXeGRIT.Density               3.0
XenonLXeGRIT.RadiationLength       2.82711
XenonLXeGRIT.Component             131.29 54.0 1
   

















