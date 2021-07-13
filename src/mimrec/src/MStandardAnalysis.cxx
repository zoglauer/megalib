/*
 * MStandardAnalysis.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MStandardAnalysis
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MStandardAnalysis.h"

// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TCanvas.h"

// MEGAlib libs:
#include "MInterface.h"
#include "MFitFunctions.h"
#include "MFileEventsTra.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MStandardAnalysis)
#endif


////////////////////////////////////////////////////////////////////////////////


MStandardAnalysis::MStandardAnalysis()
{
  // Construct an instance of MStandardAnalysis
}


////////////////////////////////////////////////////////////////////////////////


MStandardAnalysis::~MStandardAnalysis()
{
  // Delete this instance of MStandardAnalysis
}


////////////////////////////////////////////////////////////////////////////////


bool MStandardAnalysis::Analyze()
{
  //! Do all the analysis

  // Open the tra file
  if (MFile::FileExists(m_FileName) == false) {
    mgui<<"The tra file \""<<m_FileName<<"\" does not exist!!"<<error;
    return false;
  }

  MFileEventsTra EventFile;
  if (EventFile.Open(m_FileName) == false) {
    return false;
  }
  EventFile.ShowProgress();

  
  // Remove any energy and source cuts:
  m_Selector.SetFirstTotalEnergy(0.0, 1000000);
  m_Selector.SetSourceWindow(false);
  


  // Collect the data
  
  int Counts = 0;
  
  // Step 1: Determine energy resolution

  vector<double> Energies;
  
  double FirstWindow = 0.15*m_Energy;
  m_Selector.SetFirstTotalEnergy(m_Energy - FirstWindow, m_Energy + FirstWindow);

  MPhysicalEvent* Event;
  while ((Event = EventFile.GetNextEvent()) != 0) {
    if (m_Selector.IsQualifiedEvent(Event) == true) {
      Energies.push_back(Event->GetEnergy());
      ++Counts;
    }
    delete Event;
  }
  
  
  if (Energies.size() == 0) {
    cout<<"Error: No data found"<<endl;
    return false;
  }
  
  double EnergyRMS = 0.0;
  for (unsigned int a = 0; a < Energies.size(); ++a) {
    EnergyRMS += (Energies[a] - m_Energy)*(Energies[a] - m_Energy); 
  }
  EnergyRMS = sqrt(EnergyRMS/Energies.size());
  
  
  
  double SecondEnergyWindow = 3*EnergyRMS;
  
  cout<<"RMS with Average "<<m_Energy<<": "<<EnergyRMS<<endl;
  
  
  TH1D* EnergyHist = new TH1D("EnergySpectrum", "Energy spectrum", 400, m_Energy - SecondEnergyWindow, m_Energy + SecondEnergyWindow);
  EnergyHist->SetBit(kCanDelete);
  EnergyHist->SetXTitle("Energy [keV]");
  EnergyHist->SetYTitle("counts");
  EnergyHist->SetStats(false);
  EnergyHist->SetFillColor(8);
  EnergyHist->SetMinimum(0);

  for (unsigned int a = 0; a < Energies.size(); ++a) {
    EnergyHist->Fill(Energies[a]); 
  }
  
  TCanvas* EnergyCanvas = new TCanvas();
  EnergyCanvas->cd();
  EnergyHist->SetMinimum(0);
  EnergyHist->Draw();
  EnergyCanvas->Update();
  

  
  // Analyze Spectrum - Round 1
  
  cout<<"Performing first spectral fit -- getting a first guess on the width..."<<endl;
  
  TF1* SingleGaussR1 = new TF1("SingleGaussR1", Gauss, m_Energy - 0.1*EnergyRMS, m_Energy + 0.1*EnergyRMS, 3);
  SingleGaussR1->SetParameters(1, m_Energy, 1);
  SingleGaussR1->FixParameter(1, m_Energy);
  //SingleGaussR1->SetParLimits(1, 0.95*m_Energy, 1.05*m_Energy);
  SingleGaussR1->SetParLimits(2, 0.1, 10000);
  SingleGaussR1->SetParNames("Scaler", "Mean", "Sigma");
  SingleGaussR1->SetLineColor(2);

  EnergyHist->Fit(SingleGaussR1, "RI");

  if (SingleGaussR1 != 0) SingleGaussR1->Draw("SAME");
  EnergyCanvas->Modified();
  EnergyCanvas->Update();
  

  // Analyze spectrum - Round 2

  cout<<"Performing second spectral fit -- exact position and width..."<<endl;  
  
  TF1* SingleGaussR2 = new TF1("SingleGaussR2", Gauss, 
                                SingleGaussR1->GetParameter(1) - 1.0*SingleGaussR1->GetParameter(2), 
                                SingleGaussR1->GetParameter(1) + 1.5*SingleGaussR1->GetParameter(2), 3);
  SingleGaussR2->SetParameters(SingleGaussR1->GetParameter(0), SingleGaussR1->GetParameter(1), SingleGaussR1->GetParameter(2));
  SingleGaussR2->SetParLimits(1, SingleGaussR1->GetParameter(1) - 1.4*SingleGaussR1->GetParameter(2), 
                              SingleGaussR1->GetParameter(1) + 1.4*SingleGaussR1->GetParameter(2));
  SingleGaussR2->SetParLimits(2, 0.1, 10000);
  SingleGaussR2->SetParNames("Scaler", "Mean", "Sigma");
  SingleGaussR2->SetLineColor(3);
  
  EnergyHist->Fit(SingleGaussR2, "RILM");

  if (SingleGaussR2 != 0) SingleGaussR2->Draw("SAME");
  EnergyCanvas->Modified();
  EnergyCanvas->Update();

  double EnergyMean = SingleGaussR2->GetParameter(1);
  double EnergySigma = SingleGaussR2->GetParameter(2);
  double EnergySigmaError = SingleGaussR2->GetParError(2);
  
  cout<<"Mean: "<<EnergyMean<<endl;  
  
  
  // Step 2.1: Determine ARM - Round 1:
  double SigmaWindow = 1.4;
  double FirstARMWindow = 30;

  double DesiredCountsPerBin = 100;
  if (Counts/DesiredCountsPerBin < 30) {
    cout<<"Warning: ARM round 1: Very low count rate..."<<endl;
  }
  cout<<"Bins: "<<Counts/DesiredCountsPerBin<<endl;

  TH1D* ARMHistR1 = new TH1D("ARMSpectrumR1", "ARM spectrum", int(Counts/DesiredCountsPerBin), -FirstARMWindow, FirstARMWindow);
  ARMHistR1->SetBit(kCanDelete);
  ARMHistR1->SetXTitle("ARM [deg]");
  ARMHistR1->SetYTitle("counts");
  ARMHistR1->SetStats(false);
  ARMHistR1->SetFillColor(8);
  ARMHistR1->SetMinimum(0);
 
  MComptonEvent* ComptonEvent = 0; 
  // ... loop over all events and save a count in the belonging bin ...
  Counts = 0;
  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector.IsQualifiedEvent(Event) == true) {
      if (Event->GetEnergy() >= EnergyMean - SigmaWindow*EnergySigma && Event->GetEnergy() <= EnergyMean + SigmaWindow*EnergySigma) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

          double ARMValue = ComptonEvent->GetARMGamma(m_Position)*c_Deg;
          ARMHistR1->Fill(ARMValue);
          if (ARMValue >= -FirstARMWindow && ARMValue <= FirstARMWindow) ++Counts;
        }
      }
    }
    
    delete Event;
  }
  
  
  TCanvas* ARMCanvasR1 = new TCanvas();
  ARMCanvasR1->cd();
  ARMHistR1->SetMinimum(0);
  ARMHistR1->Draw();
  ARMCanvasR1->Update();

  cout<<"RMS: "<<ARMHistR1->GetRMS()<<endl;
  if (1.5*ARMHistR1->GetRMS() < FirstARMWindow) {
    FirstARMWindow = 1.5*ARMHistR1->GetRMS();
  }
  
  TF1* ARMFitR1 = new TF1("ARMFitR1", DoubleLorentzAsymGausArm, -FirstARMWindow, FirstARMWindow, 9);
  ARMFitR1->SetBit(kCanDelete);
  ARMFitR1->SetParNames("Offset", "Mean", 
                   "Lorentz Width1", "Lorentz Height1",
                   "Lorentz Width2", "Lorentz Height2",
                   "Gaus Height", "Gaus Sigma 1", "Gaus Sigma 2");
  ARMFitR1->SetParameters(0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1);
  ARMFitR1->SetParLimits(1, -FirstARMWindow, FirstARMWindow);
  ARMFitR1->FixParameter(0, 0);
  ARMFitR1->SetLineColor(3);
  
  ARMCanvasR1->cd();
  if (ARMFitR1 != 0) ARMHistR1->Fit(ARMFitR1, "RI");
  ARMHistR1->Draw("HIST");
  if (ARMFitR1 != 0) ARMFitR1->Draw("SAME");
  ARMCanvasR1->Modified();
  ARMCanvasR1->Update();
  
  double ARMFWHM = MInterface::GetFWHM(ARMFitR1, -180, 180);
  
  
  
  // Step 2.2: Determine ARM - Round 2:
  
  if (Counts/DesiredCountsPerBin < 30) {
    cout<<"Warning: ARM round 1: Very low count rate..."<<endl;
  }
  cout<<"Bins: "<<Counts/DesiredCountsPerBin<<endl;

  TH1D* ARMHistR2 = new TH1D("ARMSpectrumR2", "ARM spectrum", int(Counts/DesiredCountsPerBin), -ARMFWHM, ARMFWHM);
  ARMHistR2->SetBit(kCanDelete);
  ARMHistR2->SetXTitle("ARM [deg]");
  ARMHistR2->SetYTitle("counts");
  ARMHistR2->SetStats(false);
  ARMHistR2->SetFillColor(8);
  ARMHistR2->SetMinimum(0);
  
  // ... loop over all events and save a count in the belonging bin ...
  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent()) != 0) {

    // Only accept Comptons within the selected ranges...
    if (m_Selector.IsQualifiedEvent(Event) == true) {
      if (Event->GetEnergy() >= EnergyMean - SigmaWindow*EnergySigma && Event->GetEnergy() <= EnergyMean + SigmaWindow*EnergySigma) {
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          ComptonEvent = dynamic_cast<MComptonEvent*>(Event);

          ARMHistR2->Fill(ComptonEvent->GetARMGamma(m_Position)*c_Deg);
        }
      }
    }
    
    delete Event;
  }
  
  
  TCanvas* ARMCanvasR2 = new TCanvas();
  ARMCanvasR2->cd();
  ARMHistR2->SetMinimum(0);
  ARMHistR2->Draw();
  ARMCanvasR2->Update();

  TF1* ARMFitR2 = new TF1("ARMFitR2", DoubleLorentzAsymGausArm, -ARMFWHM, ARMFWHM, 9);
  ARMFitR2->SetBit(kCanDelete);
  ARMFitR2->SetParNames("Offset", "Mean", 
                   "Lorentz Width1", "Lorentz Height1",
                   "Lorentz Width2", "Lorentz Height2",
                   "Gaus Height", "Gaus Sigma 1", "Gaus Sigma 2");
  ARMFitR2->SetParameters(ARMFitR1->GetParameters());
  ARMFitR2->SetParLimits(1, -ARMFWHM, ARMFWHM);
  ARMFitR2->FixParameter(0, 0);
  ARMFitR2->SetLineColor(2);
  
  ARMCanvasR2->cd();
  if (ARMFitR2 != 0) ARMHistR2->Fit(ARMFitR2, "RI");
  ARMHistR2->Draw("HIST");
  if (ARMFitR2 != 0) ARMFitR2->Draw("SAME");
  ARMCanvasR2->Modified();
  ARMCanvasR2->Update();
  
  ARMFWHM = MInterface::GetFWHM(ARMFitR2, -180, 180);

  
  
  // Calculate the effective areas
  
  // (a) All
  int CountsAll = 0;
  // (b) Selected
  int CountsSelected = 0;
  // (c) Within 1.4 sigma of peak center
  int CountsSelectedPhotoPeak = 0;
  // (d) Comptons within 1.4 sigma of peak center
  int CountsSelectedComptonPhotoPeak = 0;
  // (e) Comptons within 1.4 sigma of peal center and with FWHM of ARM
  int CountsSelectedComptonARMPhotoPeak = 0;
  
  
  
  // Summary:
  
  // Loop a final time to get the counts ...
  EventFile.Rewind();
  while ((Event = EventFile.GetNextEvent()) != 0) {
    ++CountsAll;
    // Only accept Comptons within the selected ranges...
    if (m_Selector.IsQualifiedEvent(Event) == true) {
      ++CountsSelected;
      if (Event->GetEnergy() >= EnergyMean - SigmaWindow*EnergySigma && Event->GetEnergy() <= EnergyMean + SigmaWindow*EnergySigma) {
        ++CountsSelectedPhotoPeak;
        if (Event->GetType() == MPhysicalEvent::c_Compton) {
          ++CountsSelectedComptonPhotoPeak;
          ComptonEvent = dynamic_cast<MComptonEvent*>(Event);
          double ARMValue = ComptonEvent->GetARMGamma(m_Position)*c_Deg;
          if (ARMValue >= -ARMFWHM && ARMValue <= ARMFWHM) ++CountsSelectedComptonARMPhotoPeak;
        }
      }
    }
    
    delete Event;
  }

  cout<<"Summary: "<<endl;
  cout<<endl;
  cout<<"Test position: "<<m_Position<<" cm"<<endl;
  cout<<"Test energy:   "<<m_Energy<<" keV"<<endl;
  cout<<endl;
  cout<<"Energy resolution: ("<<EnergySigma<<" +- "<<EnergySigmaError<<") keV"<<endl;
  cout<<"Angular resolution: ("<<ARMFWHM<<" +- TBD) deg"<<endl;
  cout<<endl;
  cout<<"Counts - all:                                                 "<<CountsAll<<endl;
  cout<<"Counts - selected:                                            "<<CountsSelected<<endl;
  cout<<"Counts - selected within +-1.4 sigma:                         "<<CountsSelectedPhotoPeak<<endl;
  cout<<"Counts - selected Compton within +-1.4 sigma:                 "<<CountsSelectedComptonPhotoPeak<<endl;
  cout<<"Counts - selected Compton within +-1.4 sigma and +- FWHM ARM: "<<CountsSelectedComptonARMPhotoPeak<<endl;

  // Dump to file:
  
  MString OutFileName = m_FileName;
  m_FileName.ReplaceAll(".tra", ".sta");
  
  ofstream out;
  out.open(m_FileName);
  
  out<<"# Standard Analysis output file"<<endl;
  out<<endl;
  out<<"Type STA"<<endl;
  out<<"Version 1"<<endl;
  out<<endl;
  out<<"# Description of columns"<<endl;
  out<<"#  0: SA keyword"<<endl;
  out<<"#  1: Test energy [keV]"<<endl;
  out<<"#  2: Test position x [cm]"<<endl;
  out<<"#  3: Test position y [cm]"<<endl;
  out<<"#  4: Test position z [cm]"<<endl;
  out<<"#  5: Energy resolution [keV]"<<endl;
  out<<"#  6: Energy resolution uncertainty [keV]"<<endl;
  out<<"#  7: Angular resolution [deg]"<<endl;
  out<<"#  8: Angular resolution uncertainty [deg]"<<endl;
  out<<"#  9: Counts - all"<<endl;
  out<<"# 10: Counts - selected"<<endl;
  out<<"# 11: Counts - selected within +-1.4 sigma"<<endl;
  out<<"# 12: Counts - selected Compton within +-1.4 sigma"<<endl;
  out<<"# 13: Counts - selected Compton within +-1.4 sigma and +- FWHM ARM"<<endl;
  
  out<<"SA ";
  out<<m_Energy<<" ";
  out<<m_Position.X()<<" ";
  out<<m_Position.Y()<<" ";
  out<<m_Position.Z()<<" ";
  out<<EnergySigma<<" "<<EnergySigmaError<<" ";
  out<<ARMFWHM<<" 0.0 ";
  out<<CountsAll<<" ";
  out<<CountsSelected<<" ";
  out<<CountsSelectedPhotoPeak<<" ";
  out<<CountsSelectedComptonPhotoPeak<<" ";
  out<<CountsSelectedComptonARMPhotoPeak<<endl;

  out<<endl;
  out.close();
  
  return true;
}


// MStandardAnalysis.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
