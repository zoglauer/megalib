// A spheric geometry for testing activation simulations

Name ActivationUnitTests-Sphere
Version 1.0

SurroundingSphere 1.01  0.0  0.0  0.0  1.01

Include $(MEGALIB)/resource/examples/geomega/materials/Materials.geo

Volume WorldVolume             
WorldVolume.Material Vacuum
WorldVolume.Visibility 0   
WorldVolume.Shape BRIK 10. 10. 10.
WorldVolume.Mother 0

Volume ActivationSphere
ActivationSphere.Material ActivationMaterial
ActivationSphere.Visibility 1
ActivationSphere.Color 2
ActivationSphere.Shape SPHE 0.0 1.0
ActivationSphere.Position 0.0 0.0 0.0
ActivationSphere.Mother WorldVolume
  
Scintillator ActivatedDetector
ActivatedDetector.SensitiveVolume ActivationSphere
ActivatedDetector.EnergyResolution Ideal
ActivatedDetector.TriggerThreshold 0.0001
ActivatedDetector.NoiseThresholdEqualTriggerThreshold true

Trigger All
All.Veto false
All.TriggerByChannel true
All.Detector ActivatedDetector 1


