import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

GeometryFileName = "Test.geo.setup"
FileName = "Test.inc1.id1.tra.gz"

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryFileName)) == True:
  print("Geometry " + GeometryFileName + " loaded!")
else:
  print("Unable to load geometry " + GeometryFileName + " - Aborting!")
  quit()
    
# Open the tra reader
Reader = M.MFileEventsTra()
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()

# Create a tra writer
Writer = M.MFileEventsTra()
if Writer.Open(M.MString("Test.selected.tra"), M.MFile.c_Write) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()

# Open a setting file
Settings = M.MSettingsMimrec()
Settings.Read("Test.mimrec.cfg")

# and populate the event selector with it
Selector = M.MEventSelector()
Selector.SetSettings(Settings)


# Parse the tra file and all events passing the event selector are stored in the new file
while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break

  if Selector.IsQualifiedEventFast(Event) == True:
    Writer.AddEvent(Event);
  
  M.SetOwnership(Event, True)

    
# Close the tra file writer
Writer.Close();
  
print("Done")

