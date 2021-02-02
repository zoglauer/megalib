import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...


GeometryName = "$(MEGALIB)/resource/examples/geomega/special/Mini.geo.setup"
FileName = "EnergyContainment.inc1.id1.sim"

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryName)) == True:
  print("Geometry " + GeometryName + " loaded!")
else:
  print("Unable to load geometry " + GeometryName + " - Aborting!")
  quit()
    

Reader = M.MFileEventsSim(Geometry)
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()
Reader.ShowProgress(True)

Hist = M.TH1D("Containment", "Containment", 500, 0, 2100)
Hist.SetXTitle("Measured energy [keV]")

NumberOfTriggers = 0
while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  M.SetOwnership(Event, True)
  
  ED = 0.0;
  for i in range (0, Event.GetNHTs()):
    ED += Event.GetHTAt(i).GetEnergy();
  for i in range (0, Event.GetNGRs()):
    ED += Event.GetGRAt(i).GetEnergy();

  if ED > 0:
    NumberOfTriggers += 1

  Hist.Fill(ED, 1.0);

SimulatedEvents = Reader.GetSimulatedEvents()
for i in range(0, SimulatedEvents - NumberOfTriggers):
  Hist.Fill(0, 1)
  
AverageContainment = Hist.Integral() / SimulatedEvents   
print("Average energy containment: {} keV".format(AverageContainment))


Canvas = M.TCanvas()
Canvas.cd()
Canvas.SetLogy()
Hist.Draw()
Canvas.Update()



# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
