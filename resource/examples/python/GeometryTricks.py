import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

GeometryFileName = "Test.geo.setup"

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryFileName)) == True:
  print("Geometry " + GeometryFileName + " loaded!")
else:
  print("Unable to load geometry " + GeometryFileName + " - Aborting!")
  quit()
    
# Find a volume location by volume name
VolumeName = M.MString("GeWafer")

World = Geometry.GetWorldVolume()
Locations = World.GetCenterLocations(VolumeName)

if Locations.size() > 0:
  print(f"Center locations for volume {VolumeName}")
  for L in Locations:
    print(f"({L[0]}, {L[1]}, {L[2]}) cm")
else:
  print(f"Volume {VolumeName} not found in volume tree")

