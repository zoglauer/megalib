// Materials used for all geometries
// In alphabetical order!


AbsorptionFileDirectory $(MEGALIB)/resource/examples/geomega/materials

// Values of ACSPaper are not correct !!!
// I suppose the density could be about 1.0, but this is not for sure!!!
Material ACSPaper
ACSPaper.Density               1.00
ACSPaper.Component             H  10  
ACSPaper.Component             C  6   
ACSPaper.Component             O  5   


// Air at sealevel, 101.325 kPa, and 20 degrees - multiple times verified
Material Air                          
Air.Density                     1.2041E-03     
Air.ComponentByMass             N  0.7556
Air.ComponentByMass             O   0.2315
Air.ComponentByMass             Ar   0.0129 


Material Aluminium
Aluminium.Density               2.7
Aluminium.Component             Al  1


Material Alu6061
Alu6061.Density               2.6989
Alu6061.ComponentByMass       Si   0.0060
Alu6061.ComponentByMass       Fe   0.0070
Alu6061.ComponentByMass       Cu   0.0030
Alu6061.ComponentByMass       Mn   0.0015
Alu6061.ComponentByMass       Mg   0.0100
Alu6061.ComponentByMass       Cr   0.0020
Alu6061.ComponentByMass       Zn   0.0025
Alu6061.ComponentByMass       Ti   0.0015
Alu6061.ComponentByMass       Al   0.9665

Material BGO            
BGO.Density               7.1
BGO.Component             Bi  4  
BGO.Component             Ge  3   
BGO.Component             O   12   

Material CarbonCyanateEster
CarbonCyanateEster.Density          1.2
CarbonCyanateEster.ComponentByMass  C   0.9191
CarbonCyanateEster.ComponentByMass  N   0.0310
CarbonCyanateEster.ComponentByMass  O   0.0354
CarbonCyanateEster.ComponentByMass  H   0.0145

Material CdTe
CdTe.Density                     5.86
CdTe.Component                   Cd   1   
CdTe.Component                   Te   1   

Material CircuitBoard
CircuitBoard.Density            1.8
CircuitBoard.ComponentByMass    H   0.030  
CircuitBoard.ComponentByMass    C   0.174  
CircuitBoard.ComponentByMass    O   0.392  
CircuitBoard.ComponentByMass    Al   0.100  
CircuitBoard.ComponentByMass    Si   0.244  
CircuitBoard.ComponentByMass    Fe   0.010  
CircuitBoard.ComponentByMass    Cu   0.030  
CircuitBoard.ComponentByMass    Sn   0.010  
CircuitBoard.ComponentByMass    Pb   0.010  

// Values of Copper from the internet:
// Actually there are dozens of different versions of copper
Material Copper
Copper.Density                     8.954
Copper.Component                   Cu  1   

Material CsI
CsI.Density                     4.5
CsI.Component                   Cs  1
CsI.Component                   I   1 

Material CZT
CZT.Density                     6.06
CZT.ComponentByAtoms            Cd    9
CZT.ComponentByAtoms            Zn   1
CZT.ComponentByAtoms            Te   10

Material Densimet180            
Densimet180.Density               18    
Densimet180.ComponentByMass       W    0.95
Densimet180.ComponentByMass       Fe   0.16
Densimet180.ComponentByMass       Ni   0.44

Material Diamant
Diamant.Density               3.51
Diamant.Component             C  1  

Material GePassive
GePassive.Density               5.323
GePassive.Component             Ge  1  

Material Germanium
Germanium.Density               5.323
Germanium.Component             Ge  1   

// Values of Glass from the internet:
// As there are several values for glass density depending on the special type of glass I suppose an average value of 2.5 here.
Material Glass
Glass.Density                     2.5
Glass.Component                   Si   1   
Glass.Component                   O   2   

Material LaBr3
LaBr3.Density                   5.29
LaBr3.Component                 La   1
LaBr3.Component                 Br   3

Material Indium
Indium.Density                  7.310
Indium.Component                In   1

Material Iron
Iron.Density                    7.87
Iron.Component                  Fe  1   

Material MgS
MgS.Density                     1.0
MgS.Component                   Mg  1   
MgS.Component                   S   1   

Material Millipore                                        
Millipore.Density               1.00
Millipore.Component             H   10  
Millipore.Component             C   6   
Millipore.Component             O   5   

Material MLI
MLI.Density                  1.404
MLI.ComponentByMass          H   0.0364
MLI.ComponentByMass          C   0.6553
MLI.ComponentByMass          N   0.0327
MLI.ComponentByMass          O   0.2756

Material NE110
NE110.Density                   1.03
NE110.Component                 H   84  
NE110.Component                 C   74  
NE110.Component                 O    1 

Material NE213
NE213.Density                     1.0
NE213.Component                   H   10   
NE213.Component                   C   8   

Material NaI
NaI.Density                     3.67
NaI.Component                   Na  1
NaI.Component                   I   1 

Material Nylon
Nylon.Density                  1.13
Nylon.Component                H   26  
Nylon.Component                C   12  
Nylon.Component                O    4  
Nylon.Component                N    2  

Material Pb
Pb.Density               11.35
Pb.Component             Pb  1 
          
Material PCB
PCB.Density                     1.2
PCB.Component                   H   8   
PCB.Component                   C   5   
PCB.Component                   O   2   

// Values of Peek are not correct, except for the density (=1.31) !!!
Material Peek
Peek.Density                     1.31
Peek.Component                   H   8   
Peek.Component                   C   5  
Peek.Component                   O   2  

// Values of Plastic Isolation  are not correct !!!
Material Plastic_Isolation
Plastic_Isolation.Density         1.31
Plastic_Isolation.Component       H   8   
Plastic_Isolation.Component       C   5  
Plastic_Isolation.Component       O   2  

// Values except for density are not correct!!!
// For the density of wood I found defferent values. For pure wood they range from 0.45 to 0.72,
// and for plywood it is no different! So I take an average value of 0.6
Material Plywood
Plywood.Density 			             0.6
Plywood.Component               H   8   
Plywood.Component               C   5   
Plywood.Component               O   2   

// Values are a pure guess
Material PMTPlastic
PMTPlastic.Density                     1.0
PMTPlastic.Component               H   8   
PMTPlastic.Component               C   5   
PMTPlastic.Component               O   2   

Material PressurizedAir                          
PressurizedAir.Density          2.5        
PressurizedAir.Component        N   4 
PressurizedAir.Component        O   1 

// One of the circuit board materials by Rogers Corporation
Material ro3003   
ro3003.Density            2.1
ro3003.ComponentByMass    C     0.1606        
ro3003.ComponentByMass    F     0.2544        
ro3003.ComponentByMass    O     0.3061        
ro3003.ComponentByMass    Si     0.2522        
ro3003.ComponentByMass    Tl     0.0269        

// one of circuit board materials by Rogers Corporation
Material ro3006   
ro3006.Density            2.6
ro3006.ComponentByMass    C     0.1219        
ro3006.ComponentByMass    F     0.1931        
ro3006.ComponentByMass    O    0.3074        
ro3006.ComponentByMass    Si     0.1130        
ro3006.ComponentByMass    Tl     0.2647        

// one of circuit board materials by Rogers Corporation
Material ro3010   
ro3010.Density            3.0
ro3010.ComponentByMass    C     0.1045       
ro3010.ComponentByMass    F     0.1655       
ro3010.ComponentByMass    O     0.3050       
ro3010.ComponentByMass    Si     0.0397        
ro3010.ComponentByMass    Tl     0.3854        

// one of circuit board materials by Rogers Corporation
Material ro4003   
ro4003.Density            1.79
ro4003.ComponentByMass    C    0.1520        
ro4003.ComponentByMass    O     0.4290       
ro4003.ComponentByMass    H      0.0180        
ro4003.ComponentByMass    Si     0.3300        
ro4003.ComponentByMass    Ca     0.0370        
ro4003.ComponentByMass    Al     0.0190        
ro4003.ComponentByMass    B     0.0110        
ro4003.ComponentByMass    K     0.0010       
ro4003.ComponentByMass    Mg    0.0050       

// one of circuit board materials by Rogers Corporation
Material roTMM3   
roTMM3.Density            1.78
roTMM3.ComponentByMass    C      0.1201        
roTMM3.ComponentByMass    O      0.4270        
roTMM3.ComponentByMass    H      0.0198        
roTMM3.ComponentByMass    Si     0.3616        
roTMM3.ComponentByMass    Ti     0.0153        
roTMM3.ComponentByMass    Ba     0.0098        
roTMM3.ComponentByMass    Ca     0.0022        
roTMM3.ComponentByMass    Al     0.0011        
roTMM3.ComponentByMass    B      0.0003       
roTMM3.ComponentByMass    Mg     0.0002        
roTMM3.ComponentByMass    S      0.0010       


Material Silicon
Silicon.Density                 2.33
Silicon.Component               Si  1 

Material SiliconPIN
SiliconPIN.Density              2.33
SiliconPIN.Component            Si  1

// Values of Steel are not exact, but the same as Iron:
Material Steel
Steel.Density       7.87
Steel.Component     Fe  365   
Steel.Component     Cr   90   
Steel.Component     Ni   42   
Steel.Component     Si    5   
Steel.Component     Mn    5   
Steel.Component     C     5   


// stainless steel according to P.Turin for NCT screws
Material Steel_18_8
Steel_18_8.Density              7.87
Steel_18_8.ComponentByMass      Cr   0.18   
Steel_18_8.ComponentByMass      Ni   0.09   
Steel_18_8.ComponentByMass      C    0.013 
Steel_18_8.ComponentByMass      Fe   0.717   

// Values of Tin from the internet:
Material Tin
Tin.Density                     7.29
Tin.Component                   Sn  1

Material Vacuum                          
Vacuum.Density                  1E-12         
Vacuum.Component                H   1 

Material Xenon
Xenon.Density                   3.52
Xenon.Component                 Xe  1

Material XenonLXeGRIT
XenonLXeGRIT.Density            3.0
XenonLXeGRIT.Component          Xe  1
   

















