import ROOT as M
import argparse
import os

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# Parse the arguments
parser = argparse.ArgumentParser(description='Remove part of the overflow contamination')
parser.add_argument('-f', '--file', default='Isotropic.ContTest.p1.tra.gz', help='The tra file')
parser.add_argument('-g', '--geometry', default='Ancillary/massmodel/COSISMEX.analysis.geo.setup', help='The geometry')

args = parser.parse_args()

GeometryFileName = args.geometry
GeometryFileName = os.path.expandvars(GeometryFileName)
if not os.path.exists(GeometryFileName):
  print("Unable to read geometry " + GeometryFileName + " - Aborting!")
  quit()

FileName = args.file
FileName = os.path.expandvars(FileName)
if not os.path.exists(FileName):
  print("Unable to read data file " + FileName + " - Aborting!")
  quit()
  

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryFileName)) == True:
  print("Geometry " + GeometryFileName + " loaded!")
else:
  print("Unable to load geometry " + GeometryFileName + " - Aborting!")
  quit()


# Load input tra file
Reader = M.MFileEventsTra()
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open input file " + FileName + ". Aborting!")
  quit()
Reader.GetObservationTime()

# Load output tra file
OutputFileName = FileName
OutputFileName = OutputFileName.replace('.tra.', '.filtered.tra.')
Writer = M.MFileEventsTra()
if Writer.Open(OutputFileName, M.MFile.c_Write) == False:
  print("Unable to open output file " + OutputFileName + ". Aborting!")
  quit()
   
Writer.TransferInformation(Reader)
Writer.SetFileType("TRA")
Writer.WriteHeader()


HistBeforeFilter = M.TH1D("Energy Before Filter", "Energy Before Filter", 500, 0, 10000)
HistAfterFilter = M.TH1D("Energy After Filter", "Energy After Filter", 500, 0, 10000)


while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  M.SetOwnership(Event, True)
  
  if Event.GetType() == M.MPhysicalEvent.c_Compton:
    IsOverflow = False
    for h in range(0, Event.GetNHits()):
      HitEnergy = Event.GetHit(h).GetEnergy() 
      if 1799.999 < HitEnergy < 1800.001:
        IsOverflow = True
      elif 3599.999 < HitEnergy < 3600.001:
         IsOverflow = True
      elif 5399.999 < HitEnergy < 5400.001:       
         IsOverflow = True
      elif 7199.999 < HitEnergy < 7200.001:
         IsOverflow = True
      elif 8999.999 < HitEnergy < 9000.001:
        IsOverflow = True

    HistBeforeFilter.Fill(Event.Ei())
    if IsOverflow == False:
      HistAfterFilter.Fill(Event.Ei())
      Writer.AddEvent(Event)

    if Event.Ei() < 50:
      print("{}".format(Event.Ei()))

Reader.Close()

Writer.WriteFooter()
Writer.Close()

CanvasBeforeFilter = M.TCanvas()
CanvasBeforeFilter.cd()
HistBeforeFilter.Draw()
CanvasBeforeFilter.Update()

CanvasAfterFilter = M.TCanvas()
CanvasAfterFilter.cd()
HistAfterFilter.Draw()
CanvasAfterFilter.Update()


# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
