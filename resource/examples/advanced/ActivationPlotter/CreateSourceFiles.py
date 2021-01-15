# Create geometry files


MaterialListFile = open('MaterialList.txt', 'r') 
Lines = MaterialListFile.readlines() 


for Line in Lines:
  # Retrieve the material data from the file 
  Tokens = Line.split()
  
  if len(Tokens) < 5:
    continue

  Name = Tokens.pop(0)
  Density = float(Tokens.pop(0))
  Type = Tokens.pop(0)
  Materials = []
  Amount = []
  while len(Tokens) >= 2:
    Materials.append(Tokens.pop(0))
    Amount.append(Tokens.pop(0))
    
  # Create the geometry file
  BaseName = Name
  GeoFileName = BaseName + ".geo.setup"
  Out = open(GeoFileName, 'w')
  print(f"# Automatically generated geometry for activation simulations", file=Out)
  print(f" ", file=Out)
  print(f"SurroundingSphere 101  0.0  0.0  0.0  101", file=Out)
  print(f" ", file=Out)
  print(f"Material Vacuum", file=Out)                      
  print(f"Vacuum.Density    1E-12", file=Out)    
  print(f"Vacuum.Component  H 1", file=Out)
  print(f" ", file=Out)
  print(f"Volume World", file=Out)
  print(f"World.Material Vacuum", file=Out)
  print(f"World.Visibility 0", file=Out)
  print(f"World.Shape BRIK 200. 200. 200.", file=Out)
  print(f"World.Mother 0", file=Out)
  print(f" ", file=Out)
  print(f"Material M", file=Out)
  print(f"M.Density 1.0", file=Out)
  if Type == "a":
    for idx in range(0, len(Materials)):
      print(f"M.Component  {Materials[idx]} {Amount[idx]}", file=Out)
  else:
    for idx in range(0, len(Materials)):
      print(f"M.ComponentByMass  {Materials[idx]} {Amount[idx]}", file=Out)

  print(f" ", file=Out)
  print(f"Volume V", file=Out)
  print(f"V.Material M", file=Out)
  print(f"V.Visibility 1", file=Out)
  print(f"V.Shape SPHERE 99 100", file=Out)
  print(f"V.Mother World", file=Out)
  print(f" ", file=Out)
  print(f"#Simple                  ActiveDetector", file=Out)
  print(f" ", file=Out)
  print(f"ActiveDetector.SensitiveVolume     V", file=Out)
  print(f" ", file=Out)
  print(f"ActiveDetector.NoiseThresholdEqualTriggerThreshold true", file=Out)
  print(f"ActiveDetector.TriggerThreshold    12", file=Out)
  print(f" ", file=Out)
  print(f"ActiveDetector.EnergyResolution    Ideal", file=Out)
  print(f" ", file=Out)
  print(f"#Trigger ActiveDetectorTrigger", file=Out)
  print(f"ActiveDetectorTrigger.Veto false", file=Out)
  print(f"ActiveDetectorTrigger.TriggerByChannel true", file=Out)
  print(f"ActiveDetectorTrigger.Detector ActiveDetector 1", file=Out)
  print(f" ", file=Out)

  Out.close()
  
  # Create the source files
  SourceFileName = BaseName + ".Step1.source"
  Out = open(SourceFileName, 'w')
  
  print(f"# Automatically generated source file for activation simulations - step 1", file=Out)
  print(f" ", file=Out)
  print(f"Version                            1", file=Out)
  print(f"Geometry                           {GeoFileName}", file=Out)
  print(f"DetectorTimeConstant               0.000005", file=Out)
  print(f" ", file=Out)
  print(f"PhysicsListHD                      qgsp-bic-hp", file=Out)
  print(f"PhysicsListEM                      LivermorePol", file=Out)
  print(f"DecayMode                          ActivationBuildup", file=Out)
  print(f" ", file=Out)
  print(f"StoreSimulationInfo                all", file=Out)
  print(f" ", file=Out)
  print(f"Run SpaceSim", file=Out)
  print(f"SpaceSim.FileName                  {BaseName}_ActivationStep1", file=Out)
  print(f"SpaceSim.Time                      2", file=Out)
  print(f"SpaceSim.IsotopeProductionFile     {BaseName}_Isotopes", file=Out)
  print(f" ", file=Out)
  print(f"Include CosmicProtonsSpenvis.partial.source", file=Out)
  print(f"Include CosmicAlphasSpenvis.partial.source", file=Out)
  print(f"Include AlbedoProtonMizuno.partial.source", file=Out)
  print(f"Include AlbedoNeutronsKole.partial.source", file=Out)
  print(f"Include TrappedProtonsSpenvis.partial.source", file=Out)
  print(f" ", file=Out)

  Out.close()  
  
  
  SourceFileName = BaseName + ".Step2.source"
  Out = open(SourceFileName, 'w')
  
  print(f"# Automatically generated source file for activation simulations - step 2", file=Out)
  print(f" ", file=Out)
  print(f"Version                            1", file=Out)
  print(f"Geometry                           {GeoFileName}", file=Out)
  print(f"DetectorTimeConstant               0.000005", file=Out)
  print(f" ", file=Out)
  print(f"PhysicsListHD                      qgsp-bic-hp", file=Out)
  print(f"PhysicsListEM                      LivermorePol", file=Out)
  print(f" ", file=Out)
  print(f"StoreSimulationInfo                all", file=Out)
  print(f" ", file=Out)
  print(f"Activator A", file=Out)
  print(f"A.ActivationMode          ConstantIrradiation  31556736", file=Out)
  print(f"A.ActivationFile          {BaseName}_Activation.dat", file=Out)
  print(f"A.IsotopeProductionFile   {BaseName}_Isotopes.inc1.dat", file=Out)
  print(f" ", file=Out)

  Out.close()
  
  
  SourceFileName = BaseName + ".Step3.source"
  Out = open(SourceFileName, 'w')
  
  print(f"# Automatically generated source file for activation simulations - step 3", file=Out)
  print(f" ", file=Out)
  print(f"Version                            1", file=Out)
  print(f"Geometry                           {GeoFileName}", file=Out)
  print(f"DetectorTimeConstant               0.000005", file=Out)
  print(f" ", file=Out)
  print(f"PhysicsListHD                      qgsp-bic-hp", file=Out)
  print(f"PhysicsListEM                      LivermorePol", file=Out)
  print(f"DecayMode                          ActivationDelayedDecay", file=Out)
  print(f" ", file=Out)
  print(f"Run ActivationStep3", file=Out)
  print(f"ActivationStep3.FileName                         {BaseName}_Decay", file=Out)
  print(f"ActivationStep3.Time                             1000", file=Out)
  print(f" ", file=Out)
  print(f"ActivationStep3.ActivationSources                {BaseName}_Activation.dat", file=Out)
  print(f" ", file=Out)
  print(f" ", file=Out)

  Out.close()

# Finally create the launch script:
Out = open('launch.sh', 'w')
print(f"#! /bin/bash", file=Out)
print(f" ", file=Out)
print("for S1 in `ls *Step1.source`; do", file=Out)
print("  mwait -p=cosima -i=cores", file=Out)
print("  echo \"Launching ${S1}\"", file=Out)
print("  cosima -z ${S1} > /dev/null &", file=Out)
print("done", file=Out)
print("wait", file=Out)
print("for S2 in `ls *Step2.source`; do", file=Out)
print("  mwait -p=cosima -i=cores", file=Out)
print("  echo \"Launching ${S2}\"", file=Out)
print("  cosima -z ${S2} > /dev/null &", file=Out)
print("done", file=Out)
print("wait", file=Out)
print("for S3 in `ls *Step3.source`; do", file=Out)
print("  mwait -p=cosima -i=cores", file=Out)
print("  echo \"Launching ${S3}\"", file=Out)
print("  cosima -z ${S3} > /dev/null &", file=Out)
print("done", file=Out)
print("wait", file=Out)
  
Out.close()
  

