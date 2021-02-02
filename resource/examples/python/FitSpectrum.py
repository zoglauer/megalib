

import argparse

import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...

GeometryName = ""
FileName = ""
ConfigurationName = ""

# Parse command line
parser = argparse.ArgumentParser(description='Fit a spectrum')
parser.add_argument('-f', '--filename', default='', help='File name used for training/testing')
parser.add_argument('-g', '--geometry', default='', help='Geometry with which the sim file was created')
parser.add_argument('-c', '--configuration', default='', help='A mimrec configuration file for event selections.')
parser.add_argument('-i', '--minenergy', default='0', help='The minimum energy for the sepctrum.')
parser.add_argument('-a', '--maxenergy', default='1000', help='The minimum energy for the sepctrum.')
parser.add_argument('-n', '--numberofbins', default='100', help='The number of bins in the sepctrum.')

args = parser.parse_args()

if args.filename != "":
  FileName = M.MString(args.filename)
else:
  print("You need to give a tra file name - Aborting!!")
  quit()
  
if args.geometry != "":
  GeometryName = M.MString(args.geometry)
else:
  print("You need to give a geometry file name - Aborting!!")
  quit()
  
if args.configuration != "":
  ConfigurationName = M.MString(args.configuration)

EnergyMin = float(args.minenergy)
EnergyMax = float(args.maxenergy)
EnergyBins = int(args.numberofbins)

if EnergyMax <= EnergyMin:
  print("The maximum energy really must be larger than the minimum one - Aborting!!")
  quit()

if EnergyBins < 1:
  print("At least one energy bin is required - Aborting!!")
  quit()


# Load geometry:
Geometry = M.MDGeometryQuest()
if Geometry.ScanSetupFile(GeometryName) == True:
  print("Geometry " + GeometryName.Data() + " loaded!")
else:
  print("Unable to load geometry " + GeometryName.Data() + " - Aborting!")
  quit()
    
Selector = M.MEventSelector()

if ConfigurationName != "":
  Settings = M.MSettingsMimrec()
  if Settings.Read(ConfigurationName) == False:
    print("Unable to load configuration " + ConfigurationName.Data() + " - Aborting!")
    quit()
  
  Selector.SetSettings(Settings)


Reader = M.MFileEventsTra()
if Reader.Open(FileName) == False:
  print("Unable to open file " + FileName.Data() + ". Aborting!")
  quit()
Reader.ShowProgress(True)

Spectrum = M.TH1D("Spectrum", "Spectrum", EnergyBins, EnergyMin, EnergyMax)

while True: 
  Event = Reader.GetNextEvent()
  if not Event:
    break
  M.SetOwnership(Event, True)
  
  if Selector.IsQualifiedEvent(Event) == True:
    Spectrum.Fill(Event.Ei());

Fit = M.TF1("DoubleLorentzAsymGausArm", "[0] + [1]*x + [2]*exp(-(x-[3])*(x-[3])/[4]/[4])");
Fit.SetBit(M.kCanDelete);
Fit.SetParNames("Offset", "Gradient", "Gaus Height", "Gaus Mean", "Gaus Sigma");
Fit.SetParameters(0, 1, Spectrum.GetMaximum(), 511, 5);

Spectrum.Fit(Fit, "IM E");

CanvasSpectrum = M.TCanvas()
CanvasSpectrum.cd()
Spectrum.Draw()
Fit.Draw("SAME")
CanvasSpectrum.Update()


# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
