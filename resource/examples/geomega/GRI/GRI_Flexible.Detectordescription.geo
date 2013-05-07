
//------------------------------------------------------------
// Detector information:

MDStrip3D               CZTDetectorBottom

CZTDetectorBottom.SensitiveVolume      CZTWafer_B
CZTDetectorBottom.DetectorVolume       CZTWafer_B

CZTDetectorBottom.StructuralPitch      0 0 0
CZTDetectorBottom.StructuralOffset     0 0 0

CZTDetectorBottom.Offset               0.0 0.0
CZTDetectorBottom.StripNumber          10 10

CZTDetectorBottom.EnergyResolution Gauss      14     14     1.15
CZTDetectorBottom.EnergyResolution Gauss      60     60     1.26
CZTDetectorBottom.EnergyResolution Gauss     122    122     1.47
CZTDetectorBottom.EnergyResolution Gauss     662    662     4.125
CZTDetectorBottom.EnergyResolution Gauss   10000  10000    16.04

CZTDetectorBottom.EnergyResolutionDepthCorrection -0.35 1.959
CZTDetectorBottom.EnergyResolutionDepthCorrection -0.25 1.486
CZTDetectorBottom.EnergyResolutionDepthCorrection -0.15 1.275
CZTDetectorBottom.EnergyResolutionDepthCorrection -0.05 1.156
CZTDetectorBottom.EnergyResolutionDepthCorrection  0.05 1.083
CZTDetectorBottom.EnergyResolutionDepthCorrection  0.15 1.037
CZTDetectorBottom.EnergyResolutionDepthCorrection  0.25 1.011
CZTDetectorBottom.EnergyResolutionDepthCorrection  0.35 1.000

CZTDetectorBottom.DepthResolution      0.0  0.06

CZTDetectorBottom.TriggerThreshold     10. 1.0
CZTDetectorBottom.TriggerThresholdDepthCorrection -0.35 3.989
CZTDetectorBottom.TriggerThresholdDepthCorrection -0.25 2.258
CZTDetectorBottom.TriggerThresholdDepthCorrection -0.15 1.649
CZTDetectorBottom.TriggerThresholdDepthCorrection -0.05 1.348
CZTDetectorBottom.TriggerThresholdDepthCorrection  0.05 1.178
CZTDetectorBottom.TriggerThresholdDepthCorrection  0.15 1.078
CZTDetectorBottom.TriggerThresholdDepthCorrection  0.25 1.023
CZTDetectorBottom.TriggerThresholdDepthCorrection  0.35 1.000
CZTDetectorBottom.NoiseThresholdEqualsTriggerThreshold true

CZTDetectorBottom.FailureRate          0.00223



MDStrip3D               CZTDetectorSide

CZTDetectorSide.SensitiveVolume      CZTWafer_S
CZTDetectorSide.DetectorVolume       CZTWafer_S

CZTDetectorSide.StructuralPitch      0 0 0
CZTDetectorSide.StructuralOffset     0 0 0

CZTDetectorSide.Offset               0.0 0.0
CZTDetectorSide.StripNumber          10 10

CZTDetectorSide.EnergyResolution  Gauss      14        14     1.15
CZTDetectorSide.EnergyResolution  Gauss      60        60     1.26
CZTDetectorSide.EnergyResolution  Gauss     122       122     1.47
CZTDetectorSide.EnergyResolution  Gauss     662       662     4.125
CZTDetectorSide.EnergyResolution  Gauss   10000     10000    16.04

CZTDetectorSide.EnergyResolutionDepthCorrection -0.35 1.959
CZTDetectorSide.EnergyResolutionDepthCorrection -0.25 1.486
CZTDetectorSide.EnergyResolutionDepthCorrection -0.15 1.275
CZTDetectorSide.EnergyResolutionDepthCorrection -0.05 1.156
CZTDetectorSide.EnergyResolutionDepthCorrection  0.05 1.083
CZTDetectorSide.EnergyResolutionDepthCorrection  0.15 1.037
CZTDetectorSide.EnergyResolutionDepthCorrection  0.25 1.011
CZTDetectorSide.EnergyResolutionDepthCorrection  0.35 1.000

CZTDetectorSide.DepthResolution      0.0  0.06

CZTDetectorSide.TriggerThreshold     10. 1.0
CZTDetectorSide.TriggerThresholdDepthCorrection -0.35 3.989
CZTDetectorSide.TriggerThresholdDepthCorrection -0.25 2.258
CZTDetectorSide.TriggerThresholdDepthCorrection -0.15 1.649
CZTDetectorSide.TriggerThresholdDepthCorrection -0.05 1.348
CZTDetectorSide.TriggerThresholdDepthCorrection  0.05 1.178
CZTDetectorSide.TriggerThresholdDepthCorrection  0.15 1.078
CZTDetectorSide.TriggerThresholdDepthCorrection  0.25 1.023
CZTDetectorSide.TriggerThresholdDepthCorrection  0.35 1.000
CZTDetectorSide.NoiseThresholdEqualsTriggerThreshold true

CZTDetectorSide.FailureRate          0.00223




MDStrip2D                  SiDetector
SiDetector.SensitiveVolume     SiWafer
SiDetector.DetectorVolume      SiWafer

SiDetector.StructuralOffset    0.0   0.0   0.0
SiDetector.StructuralPitch     0.0   0.0   Si_LayerDistance

SiDetector.Offset              0.15  0.15
SiDetector.StripNumber          128   128

SiDetector.NoiseThreshold       12
SiDetector.TriggerThreshold     16
SiDetector.Overflow             1000

SiDetector.EnergyResolution  Gauss    10    10  2
SiDetector.EnergyResolution  Gauss   100   100  2
SiDetector.EnergyResolution  Gauss  1000  1000  2

SiDetector.FailureRate 0.005




Scintillator                   ShieldTopDetector

ShieldTopDetector.SensitiveVolume      ShieldTopActive
ShieldTopDetector.DetectorVolume       ShieldTopActive

ShieldTopDetector.EnergyResolution Gauss   10   10  20.0
ShieldTopDetector.EnergyResolution Gauss  100  100  20.0

ShieldTopDetector.TriggerThreshold     200.0
ShieldTopDetector.NoiseThresholdEqualsTriggerThreshold true
ShieldTopDetector.FailureRate          0.0



Scintillator     ShieldBottomDetector

ShieldBottomDetector.DetectorVolume     ShieldBottomActive
ShieldBottomDetector.SensitiveVolume    ShieldBottomActive

ShieldBottomDetector.EnergyResolution Gauss   10   10  10.0
ShieldBottomDetector.EnergyResolution Gauss  100  100  10.0

ShieldBottomDetector.TriggerThreshold    70.0
ShieldBottomDetector.NoiseThresholdEqualsTriggerThreshold true
ShieldBottomDetector.FailureRate         0.0


Scintillator     ShieldSideDetector

ShieldSideDetector.DetectorVolume     ShieldSideActive
ShieldSideDetector.SensitiveVolume    ShieldSideActive

ShieldSideDetector.EnergyResolution Gauss   10   10  10.0
ShieldSideDetector.EnergyResolution Gauss  100  100  10.0

ShieldSideDetector.TriggerThreshold    70.0
ShieldSideDetector.NoiseThresholdEqualsTriggerThreshold true
ShieldSideDetector.FailureRate         0.0




# *************************************************************************
# Trigger conditions
# *************************************************************************



Trigger TriggerSi
TriggerSi.Veto false
TriggerSi.TriggerByChannel true
TriggerSi.Detector SiDetector 1

Trigger TriggerCZTBottom
TriggerCZTBottom.Veto false
TriggerCZTBottom.TriggerByChannel true
TriggerCZTBottom.Detector CZTDetectorBottom 1

Trigger TriggerCZTSide
TriggerCZTSide.Veto false
TriggerCZTSide.TriggerByChannel true
TriggerCZTSide.Detector CZTDetectorSide 1

Trigger TriggerShieldSide
TriggerShieldSide.Veto true
TriggerShieldSide.TriggerByChannel true
TriggerShieldSide.Detector ShieldSideDetector 1

Trigger TriggerShieldBottom
TriggerShieldBottom.Veto true
TriggerShieldBottom.TriggerByChannel true
TriggerShieldBottom.Detector ShieldBottomDetector 1

Trigger TriggerShieldTop
TriggerShieldTop.Veto true
TriggerShieldTop.TriggerByChannel true
TriggerShieldTop.Detector ShieldTopDetector 1

#Trigger Guardring
#Guardring.Veto true
#Guardring.TriggerByDetector true
#Guardring.GuardringDetectorType Strip3D 1
