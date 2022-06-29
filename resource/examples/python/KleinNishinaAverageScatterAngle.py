import ROOT as M

# Load MEGAlib into ROOT
M.gSystem.Load("$(MEGALIB)/lib/libMEGAlib.so")

# Initialize MEGAlib
G = M.MGlobal()
G.Initialize()

# We are good to go ...


EnergyBins = 30

Hist = M.TH1D("ACSA", "Average Compton Scatter Angle", EnergyBins, 0, 2000)
Hist.SetXTitle("Energy [keV]")
Hist.SetYTitle("Avg. scatter angle [degree]")

Hist2D = M.TH2D("KN", "KN", 180, 0, 180, EnergyBins, 0, 2000)


for e in range(1, EnergyBins+1):
  Energy = Hist.GetXaxis().GetBinCenter(e)
  TotalIntegral = 0
  for Angle in range (1, 180):
    KN = M.MComptonEvent.GetDifferentalKleinNishina(Energy, Angle*M.TMath.DegToRad())* 2*M.TMath.Pi() * (M.TMath.Cos((Angle-1)*M.TMath.DegToRad()) - M.TMath.Cos(Angle*M.TMath.DegToRad()))
    Hist2D.Fill(Angle, Energy, KN)
    TotalIntegral += M.MComptonEvent.GetDifferentalKleinNishina(Energy, Angle*M.TMath.DegToRad()) * 2*M.TMath.Pi() * (M.TMath.Cos((Angle-1)*M.TMath.DegToRad()) - M.TMath.Cos(Angle*M.TMath.DegToRad()))
  Integral = 0
  for Angle in range (1, 180):
    Integral += M.MComptonEvent.GetDifferentalKleinNishina(Energy, Angle*M.TMath.DegToRad()) * 2*M.TMath.Pi() * (M.TMath.Cos((Angle-1)*M.TMath.DegToRad()) - M.TMath.Cos(Angle*M.TMath.DegToRad()))
    if Integral > 0.5*TotalIntegral:
      Hist.SetBinContent(e, Angle)
      break

Canvas = M.TCanvas()
Canvas.cd()
Hist.Draw()
Canvas.Update()

Canvas2 = M.TCanvas()
Canvas2.cd()
Hist2D.Draw("colz")
Canvas2.Update()



# Prevent the canvases from being closed
import os
print("ATTENTION: Please exit by clicking: File -> Close ROOT! Do not just close the window by clicking \"x\"")
print("           ... and if you didn't honor this warning, and are stuck, execute the following in a new terminal: kill " + str(os.getpid()))
M.gApplication.Run()
