import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

GeometryName = "$(MEGALIB)/resource/examples/geomega/special/Max.geo.setup"
FileName = "Test.inc1.id1.tra"

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryName)) == True:
  print("Geometry " + GeometryName + " loaded!")
else:
  print("Unable to load geometry " + GeometryName + " - Aborting!")
  quit()
    

Reader = M.MFileEventsTra()
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()

HistCompton = M.TH1D("Energy of Compton events", "Energy of Compton Events", 100, 0, 1000)
HistPhoto = M.TH1D("Energy of single-site events", "Energy of single-site Events", 100, 0, 1000)


while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  
  if Event.GetType() == M.MPhysicalEvent.c_Compton:
    HistCompton.Fill(Event.Ei());
  elif Event.GetType() == M.MPhysicalEvent.c_Photo:
    HistPhoto.Fill(Event.Ei());

CanvasCompton = M.TCanvas()
CanvasCompton.cd()
HistCompton.Draw()
CanvasCompton.Update()

CanvasPhoto = M.TCanvas()
CanvasPhoto.cd()
HistPhoto.Draw()
CanvasPhoto.Update()


# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
