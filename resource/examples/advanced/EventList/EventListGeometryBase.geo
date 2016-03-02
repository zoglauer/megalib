#---------------------------------------------------------
# Very simple Germanium ACT - similar to NCT 


Name EventList
Version 2.0

SurroundingSphere 1  0.0  0.0  0.0  1

# Use this volume, to test the surrounding sphere:
#Volume Sphere
#Sphere.Material Vacuum
#Sphere.Shape SPHE 0.0 10.0 0.0 180.0 0.0 360.
#Sphere.Position 0.0 0.0 0.0
#Sphere.Mother WorldVolume


Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo



#------------------------------------------------------------
# Volume section:


Volume WorldVolume             
WorldVolume.Material Vacuum
WorldVolume.Visibility 0   
WorldVolume.Shape BRIK 50. 50. 50.
WorldVolume.Mother 0


// The Compton scatterer

Volume Scatterer
Scatterer.Visibility 1
Scatterer.Color 3
Scatterer.Shape TUBS 0 1.0 3.0
Scatterer.Material NE213
Scatterer.Position 0 0 10
Scatterer.Mother WorldVolume


// The absorbers

Volume Absorber 
Absorber.Visibility 1
Absorber.Color 4
Absorber.Shape TUBS 0 2.0 5.0
Absorber.Material NaI


For I 12 0 { 2.0*TMath::Pi()/12.0 }
  Absorber.Copy Absorber_%I
  Absorber_%I.Position { 13*cos($I) }  { 13*sin($I) } 10
  Absorber_%I.Rotation 0 90 { $I * 180 / TMath::Pi() }
  Absorber_%I.Mother WorldVolume
Done


//------------------------------------------------------------
// Define Germanium strip detector:

// One layer of the tracker
// GeStripDetector is the aluminum cryostat surrounding the 
// Ge detectors - they are called 'GeWafer' ....

Constant DetectorHalfWidth  8.0
Constant DetectorHalfHeight 0.75

Constant CryoThickness  0.1
Constant CryoOffset     0.5

Constant CryoTotalHeight { 2*(DetectorHalfHeight + CryoOffset + CryoThickness ) } 

Volume GeStripDetector           
GeStripDetector.Material Aluminium
GeStripDetector.Visibility 1
GeStripDetector.Color 2
GeStripDetector.Shape BRIK  { DetectorHalfWidth + CryoOffset + CryoThickness }  { DetectorHalfWidth + CryoOffset  + CryoThickness }  { DetectorHalfHeight + CryoOffset  + CryoThickness } 

Volume CryoInnerHull 
CryoInnerHull.Material Vacuum 
CryoInnerHull.Visibility 1
CryoInnerHull.Shape BRIK  { DetectorHalfWidth + CryoOffset }  { DetectorHalfWidth + CryoOffset }  { DetectorHalfHeight + CryoOffset } 

Volume GeWafer
GeWafer.Material Germanium
GeWafer.Visibility 1
GeWafer.Color 6
GeWafer.Shape BRIK  DetectorHalfWidth  DetectorHalfWidth  DetectorHalfHeight

GeWafer.Copy GeWaferN01
GeWaferN01.Position  0.0 0.0 0.0
GeWaferN01.Mother CryoInnerHull

CryoInnerHull.Copy Cryo
Cryo.Position 0.0 0.0 0.0
Cryo.Mother GeStripDetector



// Line-Up some Ge-Detector
For J 4 -10 { -CryoTotalHeight }
  GeStripDetector.Copy Ge_%J
  Ge_%J.Position  0.0  0.0  $J 
  Ge_%J.Mother WorldVolume
Done







#------------------------------------------------------------
# Detector and trigger section:


MDStrip3D                  GStrip

GStrip.DetectorVolume      GeWafer
GStrip.SensitiveVolume     GeWafer

GStrip.StructuralPitch      0.0   0.0    0.0
GStrip.StructuralOffset     0.0   0.0    CryoTotalHeight

GStrip.Offset              0.0   0.0
GStrip.StripNumber         160    160

GStrip.NoiseThreshold      15
GStrip.TriggerThreshold    30

GStrip.DepthResolution     10  0.02
GStrip.DepthResolution     100  0.02
GStrip.DepthResolution     1000  0.02

GStrip.EnergyResolution Gaus 10      10        0.417   
GStrip.EnergyResolution Gaus 20      20        0.425           
GStrip.EnergyResolution Gaus 50      50        0.4375    
GStrip.EnergyResolution Gaus 100     100       0.454     
GStrip.EnergyResolution Gaus 200     200       0.492     
GStrip.EnergyResolution Gaus 500     500       0.5875      
GStrip.EnergyResolution Gaus 1000    1000      0.7217       
GStrip.EnergyResolution Gaus 2000    2000      0.9317      
GStrip.EnergyResolution Gaus 5000    5000      1.3819       
GStrip.EnergyResolution Gaus 10000   10000     1.9094                     
GStrip.EnergyResolution Gaus 100000  100000    5.907        


