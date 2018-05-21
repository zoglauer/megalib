import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...


GeometryName = "$(MEGALIB)/resource/examples/geomega/special/Max.geo.setup"
FileName = "Test.inc1.id1.sim"

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryName)) == True:
  print("Geometry " + GeometryName + " loaded!")
else:
  print("Unable to load geometry "+ GeometryName + " - Aborting!")
  quit()
    

Reader = M.MFileEventsSim(Geometry)
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()

Hist = M.TH2D("Energy", "Energy", 100, 0, 600, 100, 0, 600)
Hist.SetXTitle("Input energy [keV]")
Hist.SetYTitle("Measured energy [keV]")

while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  
  if Event.GetNIAs() > 0:
    Hist.Fill(Event.GetIAAt(0).GetSecondaryEnergy(), Event.GetREnergy());

Canvas = M.TCanvas()
Canvas.cd()
Hist.Draw("colz")
Canvas.Update()



# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
