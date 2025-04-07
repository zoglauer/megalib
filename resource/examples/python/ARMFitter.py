import math
import ctypes
import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

FileName = "Test.inc1.id1.tra.gz"
GeometryFileName = "Test.geo.setup"
SettingsFileName = "Test.mimrec.cfg"
TestPosition = M.MVector(0, 0, 1E20)
CoordinateSystem = M.MCoordinateSystem.c_Spheric

# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(M.MString(GeometryFileName)) == True:
  print("Geometry " + GeometryFileName + " loaded!")
else:
  print("Unable to load geometry " + GeometryFileName + " - Aborting!")
  quit()
    
# Load the data
Reader = M.MFileEventsTra()
if Reader.Open(M.MString(FileName)) == False:
  print("Unable to open file " + FileName + ". Aborting!")
  quit()

# Set up the event selector
Settings = M.MSettingsMimrec()
Settings.Read(SettingsFileName)

Selector = M.MEventSelector()
Selector.SetGeometry(Geometry)
Selector.SetSettings(Settings);

# Set up the fitter
Fitter = M.MARMFitter()
Fitter.SetNumberOfBins(101);
Fitter.SetMaximumARMValue(15);
Fitter.SetFitFunction(M.MARMFitFunctionID.c_AsymmetricGaussLorentzLorentz);
Fitter.UseOptimizedBinning();


C = M.MComptonEvent()

while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  M.SetOwnership(Event, True)
  
  if Selector.IsQualifiedEvent(Event):
    if Event.GetType() == M.MPhysicalEvent.c_Compton:
      C = Event
      Fitter.AddARMValue(math.degrees(Event.GetARMGamma(TestPosition, CoordinateSystem)))


Fitter.Fit(20);

if Fitter.WasFittingSuccessful():
  FitterCanvas = M.TCanvas();
  FitterCanvas.SetTitle("ARM of Compton cone");
  FitterCanvas.cd();
  Fitter.Draw();
  FitterCanvas.Modified();
  FitterCanvas.Update();

  print(Fitter.ToString())


# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
