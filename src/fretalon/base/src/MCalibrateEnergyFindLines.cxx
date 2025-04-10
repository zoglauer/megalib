/*
 * MCalibrateEnergyFindLines.cxx
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


// Include the header:
#include "MCalibrateEnergyFindLines.h"

// Standard libs:
#include <algorithm>
#include <type_traits>
using namespace std;

// ROOT libs:
#include "TF1.h"
#include "TMath.h"
#include "TBox.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MTimer.h"
#include "MMath.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MCalibrationSpectralPoint.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTemperature.h"
#include "MCalibrationFit.h"
#include "MCalibrationFitGaussian.h"
#include "MCalibrationFitGaussLandau.h"
#include "MCalibrationModel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MCalibrateEnergyFindLines)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateEnergyFindLines::MCalibrateEnergyFindLines() : MCalibrateEnergy()
{
  m_PeakParametrizationMethod = c_PeakParametrizationMethodBayesianBlockPeak;
  
  m_PeakParametrizationMethodFittedPeakBackgroundModel = MCalibrationFit::c_BackgroundModelLinear; 
  m_PeakParametrizationMethodFittedPeakEnergyLossModel = MCalibrationFit::c_EnergyLossModelNone; 
  m_PeakParametrizationMethodFittedPeakPeakShapeModel = MCalibrationFit::c_PeakShapeModelGaussian;
  
  m_TemperatureMin = -numeric_limits<double>::max();
  m_TemperatureMax = +numeric_limits<double>::max();

  m_Prior = 8;
  m_ExcludeFirstNumberOfBins = 25;
  m_MinimumPeakCounts = 100;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrateEnergyFindLines::~MCalibrateEnergyFindLines()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateEnergyFindLines::Calibrate()
{
  m_Results.Clear();
  m_Results.SetNumberOfReadOutDataGroups(m_ROGs.size());
  
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    FindPeaks(r);   
    FitPeaks(r);
  }
  
  CheckPeaks();
  
  // At this point we erase all bad spectral points
  m_Results.RemoveAllBadSpectralPoints();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Find the peaks in this read-out data group
bool MCalibrateEnergyFindLines::FindPeaks(unsigned int ROGID)
{
  if (g_Verbosity >= c_Info) cout<<endl<<"Finding peaks for ROG ID: "<<ROGID<<" ("<<m_ROGs[ROGID]->GetName()<<")"<<endl;
  
  // Other options are in the constructor
  double FirstPeakMinimumPeakCounts = 300; 

  double MinimumHeight = 1.5; 
  double MinimumBinWidthForBayesianBinner = 8;
  double ComptonEdgeThreshold = 0.5;
  double RangeBeyondPeak = 2.5;
  //double Epsilon = 0.01;

  MTimer T;
  // Step 1: Create a histogram with the correct binning:
  MBinnerBayesianBlocks Binner;
  Binner.SetMinimumBinWidth(MinimumBinWidthForBayesianBinner);
  Binner.SetMinMax(m_RangeMinimum, m_RangeMaximum);
  Binner.SetPrior(m_Prior);
  for (unsigned int d = 0; d < m_ROGs[ROGID]->GetNumberOfReadOutDatas(); ++d) {
    MReadOutDataTemperature* T = dynamic_cast<MReadOutDataTemperature*>(m_ROGs[ROGID]->GetReadOutData(d).Get(MReadOutDataTemperature::m_TypeID));
    if (T != nullptr) {
      if (T->GetTemperature() < m_TemperatureMin || T->GetTemperature() > m_TemperatureMax) {
        continue; 
      }
    }
    
    MReadOutDataADCValue* ADC = dynamic_cast<MReadOutDataADCValue*>(m_ROGs[ROGID]->GetReadOutData(d).Get(MReadOutDataADCValue::m_TypeID));
    if (ADC != nullptr) {
      Binner.Add(ADC->GetADCValue());
    }
  }
  TH1D* Data = Binner.GetNormalizedHistogram(MString("Data ") + ROGID, "ADC Values", "counts / ADC value");
  Binner.Clear();
  //cout<<"Binner: "<<T.GetElapsed()<<endl;
  
  if (Data->GetNbinsX() < 5) return true;
        
  // Step 2: Do some mimial smoothing to elimintae mini-peaks in the Bayesian block binned data
  //Data->Smooth(3); // --> Better to use a larger prior...
    
  // Step 3: Create a first derivation
  TH1D* FirstDerivation = 0;
  {
    TLockGuard G(g_Mutex);
    FirstDerivation = new TH1D(*Data);
  }
  FirstDerivation->SetTitle(MString("First derivation") + ROGID);
  FirstDerivation->Reset();
  for (int b = 2; b <= FirstDerivation->GetNbinsX(); ++b) {
    double Average = 0.5*(Data->GetBinContent(b)+Data->GetBinContent(b-1));
    FirstDerivation->SetBinContent(b, (Data->GetBinContent(b)-Data->GetBinContent(b-1))/
                                      (Data->GetBinCenter(b)-Data->GetBinCenter(b-1))/Average);
  }

  
  
  // Step 4: Find zero passages + --> -
  bool Start = true;
  bool FirstPeak = false;
  for (int b = 2; b <= FirstDerivation->GetNbinsX(); ++b) {
    if (FirstDerivation->GetBinContent(b) > 0 && FirstDerivation->GetBinContent(b+1) < 0) {
      // We have found a zero passage, i.e. a peak
      // Let's handle it
      
      if (g_Verbosity >= c_Chatty) cout<<endl<<"Zero passage found at "<<FirstDerivation->GetBinLowEdge(b+1)<<" - Start"<<endl;
      
      // ignore the first peak as background rollover or noise
      if (Start == true) {
        Start = false;
        FirstPeak = true;
        //cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: First peak"<<endl;
        //continue;
      } else {
        FirstPeak = false; 
      }

      /*
      // Ignore the last bin: 
      if (b+1 == FirstDerivation->GetNbinsX()) {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - peak is near last bin ignoring ..."<<endl;
        continue;
      }
      */
        
      // Create a peak
      MCalibrationSpectralPoint P;
      P.SetPeak(FirstDerivation->GetBinLowEdge(b+1));
      
      if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - investigating..."<<endl;
      
      // Find edges:
      int MaximumBin = b;
      for (int bb = MaximumBin-1; bb > 2; --bb) {
        if (FirstDerivation->GetBinContent(bb) > FirstDerivation->GetBinContent(MaximumBin)) {
          MaximumBin = bb;
        } else {
          break;
        }
      }
        
      int MinimumBin = b+1;
      for (int bb = MinimumBin+1; bb <= FirstDerivation->GetNbinsX(); ++bb) {
        if (FirstDerivation->GetBinContent(bb) < FirstDerivation->GetBinContent(MinimumBin)) {
          MinimumBin = bb;
        } else {
          break;
        }
      }
      
      /*
      // Make sure the left and right width are longer than the MinimumBinWidthForBayesianBinner
      if (FirstDerivation->GetBinLowEdge(b+1) - FirstDerivation->GetBinLowEdge(MaximumBin) < MinimumBinWidthForBayesianBinner + Epsilon ||
          FirstDerivation->GetBinLowEdge(MinimumBin) - FirstDerivation->GetBinLowEdge(b+1) < MinimumBinWidthForBayesianBinner + Epsilon) {
        cout<<FirstDerivation->GetBinLowEdge(b+1)<<": Just some single high/low count rate bin"<<endl;
        continue;
      }
      */
      
      
      // Minimum counts check
      if (g_Verbosity >= c_Chatty) cout<<"Maximum/Minimum bin: "<<MaximumBin<<":"<<MinimumBin<<endl;
      double CountsPerBinBefore = Data->GetBinContent(MaximumBin);
      int StartBin = MaximumBin;
      if (MaximumBin-1 >= 1) {
        if (Data->GetBinContent(MaximumBin-1) < Data->GetBinContent(MaximumBin)) { 
          CountsPerBinBefore = Data->GetBinContent(MaximumBin-1);
          StartBin = MaximumBin-1;
        }
      }
      double CountsPerBinAfter = Data->GetBinContent(MinimumBin);
      int StopBin = MinimumBin;
      if (MinimumBin+1 <= FirstDerivation->GetNbinsX()) {
        if (Data->GetBinContent(MinimumBin+1) < Data->GetBinContent(MinimumBin)) { 
          CountsPerBinAfter = Data->GetBinContent(MinimumBin+1);
          StopBin = MinimumBin+1;
        }
      }
        
      double CountsPerBin = 0;
      double Width = 0;
      double Height = 0;
      for (int bb = StartBin; bb <= StopBin; ++bb) {
        CountsPerBin += Data->GetBinContent(bb);
        if (Data->GetBinContent(bb) > Height) Height = Data->GetBinContent(bb);
        Width += Data->GetBinWidth(bb);
      }
      CountsPerBin -= 0.5*(CountsPerBinBefore + CountsPerBinAfter);
      if (g_Verbosity >= c_Chatty) cout<<FirstDerivation->GetBinLowEdge(b+1)<<": Height etc. : "<<CountsPerBinBefore<<":"<<Height<<":"<<CountsPerBinAfter<<endl;
      Height -= 0.5*(CountsPerBinBefore + CountsPerBinAfter);
      
      if (CountsPerBin*Width < m_MinimumPeakCounts) {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough counts per bin: "<<CountsPerBin*Width<<" (min: "<<m_MinimumPeakCounts<<")"<<endl;
        continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: Enough counts per bin: "<<CountsPerBin*Width<<" (min: "<<m_MinimumPeakCounts<<")"<<endl;
      }
      P.SetCounts(CountsPerBin*Width);
        
      if (Height < MinimumHeight) {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Peak height to small: "<<Height<<" (min: "<<MinimumHeight<<")"<<endl;
        continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: Peak heigh enough: "<<Height<<" (min: "<<MinimumHeight<<")"<<endl;        
      }
        
        
      double Minimum = FirstDerivation->GetBinContent(MinimumBin);
      double Maximum = FirstDerivation->GetBinContent(MaximumBin);
      double Volatility = Maximum - Minimum;
      if (!(Volatility > 0.015 && 
          ((Maximum > +0.0075 && Minimum < -0.0025) ||
           (Minimum < -0.0075 && Maximum > +0.0025)))) {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Derivative peak not strong enough: "<<FirstDerivation->GetBinContent(MaximumBin)<<" and "<<FirstDerivation->GetBinContent(MinimumBin)<<endl;
        continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: Derivative peak strong enough: "<<FirstDerivation->GetBinContent(MaximumBin)<<" and "<<FirstDerivation->GetBinContent(MinimumBin)<<endl;        
      }

      // First round of edges:
      P.SetLowEdge(FirstDerivation->GetBinLowEdge(MaximumBin-1));
      P.SetHighEdge(FirstDerivation->GetBinLowEdge(MinimumBin)+FirstDerivation->GetBinWidth(MinimumBin));

      
      // Optimize the fit window:
      
      // Find better peak:
      double Peak = 0;
      int PeakBin = b+1;
      for (int bb = MaximumBin; bb <= MinimumBin; ++bb) {
        if (Data->GetBinContent(bb) > Peak) {
          Peak = Data->GetBinContent(bb);
          P.SetPeak(Data->GetBinCenter(bb));
          PeakBin = bb;
        }
      }
      //! Determine the average value in the peak
      double Total = 0.0;
      unsigned int TotalCounts = 0;
      for (unsigned int d = 0; d < m_ROGs[ROGID]->GetNumberOfReadOutDatas(); ++d) {
        MReadOutDataADCValue* ADC = dynamic_cast<MReadOutDataADCValue*>(m_ROGs[ROGID]->GetReadOutData(d).Get(MReadOutDataADCValue::m_TypeID));
        if (ADC != nullptr) {
          if (ADC->GetADCValue() >= Data->GetBinLowEdge(PeakBin) && 
              ADC->GetADCValue() < Data->GetBinLowEdge(PeakBin) + Data->GetBinWidth(PeakBin)) {
            TotalCounts++;
            Total += ADC->GetADCValue();
          }
        }
      }
      P.SetPeak(Total/TotalCounts); // ???

      if (g_Verbosity >= c_Chatty) cout<<"Peak: "<<P.GetPeak()<<endl;
      if (g_Verbosity >= c_Chatty) cout<<"Peak bin: "<<PeakBin<<endl;
      
      // If this is the first peak it must be at least 3 bins away from the first bin and from any bin with less than 1 counts:
      //if (FirstPeak == true) {
        if (std::make_unsigned<int>::type(PeakBin) < m_ExcludeFirstNumberOfBins) {
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: First peak must be at least "<<m_ExcludeFirstNumberOfBins<<" bins away from start, and not only "<<PeakBin<<endl;
          continue;          
        } else {
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: First peak is at least "<<m_ExcludeFirstNumberOfBins<<" bins away from start: "<<PeakBin<<endl;
        }
      //}
      
      
      
      // Find the minimum and maximum of range around the peak
      
      // Let's find the right lower limit
      int RightLimitBin = PeakBin;
      double Value = Data->GetBinContent(PeakBin);
      for (int bb = PeakBin+1; bb <= FirstDerivation->GetNbinsX(); ++bb) {
        if (Data->GetBinContent(bb) <= Value) {
          Value = Data->GetBinContent(bb);
          RightLimitBin = bb;
        } else {
          break;
        }
      }
      P.SetHighEdge(Data->GetBinCenter(RightLimitBin));
      if (g_Verbosity >= c_Chatty) cout<<"Right limit bin: "<<RightLimitBin<<endl;
      //cout<<"Right edge until next increase: "<<P.GetHighEdge()<<endl;
      
      // Let's crawl down on the left side and symmetrize if possible
      int LeftLimitBin = PeakBin;
      Value = Data->GetBinContent(MaximumBin);
      for (int bb = MaximumBin - 1; bb >= 1; --bb) {
        if (Data->GetBinContent(bb) <= Value) {
          Value = Data->GetBinContent(bb);
          LeftLimitBin = bb;
        } else {
          break;
        }
      }
      P.SetLowEdge(Data->GetBinCenter(LeftLimitBin));
      if (g_Verbosity >= c_Chatty) cout<<"Left limit bin: "<<LeftLimitBin<<endl;
      
      // Compton edge check:
      // If the left peak-to-valley is less than 50% (=ComptonEdgeThreshold) of the right peak-to-valley 
      // then we probably have a Compton edge or another bad peak
      if (Data->GetBinContent(PeakBin) - Data->GetBinContent(LeftLimitBin) < ComptonEdgeThreshold * (Data->GetBinContent(PeakBin) - Data->GetBinContent(RightLimitBin))) {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: The left peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(LeftLimitBin)<<") is less than "<<100*ComptonEdgeThreshold<<"% of the right peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(RightLimitBin)<<"): We might have a Compton edge or other bad peak"<<endl;
        continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: The left peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(LeftLimitBin)<<") is at least than "<<100*ComptonEdgeThreshold<<"% of the right peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(RightLimitBin)<<"): Unlikely we have a Compton edge or other bad peak"<<endl;        
      }
      
      // Check if we have enough counts
      double Average = 0.5*(Data->GetBinWidth(RightLimitBin)*Data->GetBinContent(RightLimitBin) + Data->GetBinWidth(LeftLimitBin)*Data->GetBinContent(LeftLimitBin))/(Data->GetBinWidth(RightLimitBin) + Data->GetBinWidth(LeftLimitBin));
      double Integral = Data->Integral(LeftLimitBin, RightLimitBin, "width");
      double Excess = Integral - Average*(Data->GetXaxis()->GetBinUpEdge(RightLimitBin) - Data->GetBinLowEdge(LeftLimitBin));
      
      if (g_Verbosity >= c_Chatty) cout<<"Avg: "<<Average<<" Int: "<<Integral<<" Width: "<<Data->GetXaxis()->GetBinUpEdge(RightLimitBin) - Data->GetBinLowEdge(LeftLimitBin)<<endl;
      
      if (FirstPeak == true) {
        if (Excess < FirstPeakMinimumPeakCounts) {
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough peak counts for a first peak: "<<Excess<<endl;
          continue;
        } else {
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: Enough peak counts for a first peak: "<<Excess<<endl;          
        }
      } else {
        if (Excess < m_MinimumPeakCounts) {
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough peak counts: "<<Excess<<endl;
          continue;
        } else {          
          if (g_Verbosity >= c_Info) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Passed: Enough peak counts: "<<Excess<<endl;
       }
      }
      if (g_Verbosity >= c_Chatty) cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Estimated peak counts: "<<Excess<<endl;
      
      
      
      // Symmetrize - use the smallest window on both sides:
      if (P.GetPeak() - P.GetLowEdge() < P.GetHighEdge() - P.GetPeak()) {
        P.SetHighEdge(2.0*P.GetPeak() - P.GetLowEdge());
      } else {
        P.SetLowEdge(2.0*P.GetPeak() - P.GetHighEdge());   
      }
      
      /* Old symmetrize:
      if (2.0*P.GetPeak() - P.GetLowEdge() < Data->GetBinCenter(RightLimitBin)) {
        P.SetHighEdge(2.0*P.GetPeak() - P.GetLowEdge());
      } else {
        P.SetHighEdge(Data->GetBinCenter(RightLimitBin));
      }
      */

      // The right and left limit now define the floor
      // Now determine when we dropped to half left and right and 
      // take 4 (= RangeBeyondPeak) times that value as new limits (if they are smaller)
      int RightHalfBin = PeakBin+1;
      double RightHalf = 0.5 *(Data->GetBinContent(PeakBin) + Data->GetBinContent(RightLimitBin));
      for (int bb = PeakBin+1; bb <= Data->GetNbinsX(); ++bb) {
        if (g_Verbosity >= c_Chatty) cout<<"Check: "<<bb<<" - "<<Data->GetBinContent(bb)<<" vs. "<<RightHalf<<endl;
        if (Data->GetBinContent(bb) <= RightHalf) {
          double Range = Data->GetBinCenter(bb) - P.GetPeak();
          RightHalfBin = bb;
          if (g_Verbosity >= c_Chatty) cout<<"Range right: "<<Range<<" vs. "<<P.GetHighEdge()<<":"<<P.GetPeak() + 4*Range<<endl;
          if (P.GetHighEdge() > P.GetPeak() + RangeBeyondPeak*Range) {
            if (g_Verbosity >= c_Chatty) cout<<"New high edge from FWHM determination: "<<P.GetPeak() + 4*Range<<endl;
            P.SetHighEdge(P.GetPeak() + RangeBeyondPeak*Range);
          }
          break;
        }
      }
      int LeftHalfBin = PeakBin;
      double LeftHalf = 0.5 *(Data->GetBinContent(PeakBin) + Data->GetBinContent(LeftLimitBin));
      for (int bb = PeakBin; bb > 0; --bb) {
        if (g_Verbosity >= c_Chatty) cout<<"Check: "<<bb<<" - "<<Data->GetBinContent(bb)<<" vs. "<<LeftHalf<<endl;
        if (Data->GetBinContent(bb) <= LeftHalf) {
          double Range = P.GetPeak() - Data->GetBinCenter(bb);
          LeftHalfBin = bb;
          if (g_Verbosity >= c_Chatty) cout<<"Range left: "<<Range<<" vs. "<<P.GetLowEdge()<<":"<<P.GetPeak() - 4*Range<<endl;
          if (P.GetLowEdge() < P.GetPeak() - RangeBeyondPeak*Range) {
            if (g_Verbosity >= c_Chatty) cout<<"New high edge from FWHM determination: "<<P.GetPeak() - 4*Range<<endl;
            P.SetLowEdge(P.GetPeak() - RangeBeyondPeak*Range);
          }
          break;
        }
      }
      P.SetFWHM(Data->GetBinLowEdge(RightHalfBin) - Data->GetXaxis()->GetBinUpEdge(LeftHalfBin));
      if (g_Verbosity >= c_Chatty) cout<<"FWHM: "<<P.GetFWHM()<<endl;
      
      
      
      /*
      
      // Make sure the outer bins are not too large:
      double MaxPeakSize = Data->GetBinLowEdge(RightLimitBin) -  Data->GetXaxis()->GetBinUpEdge(LeftLimitBin); 
      cout<<"MaxPeakSize: "<<MaxPeakSize<<endl;
      
      if (Data->GetBinCenter(RightLimitBin) - Data->GetBinLowEdge(RightLimitBin) > 0.25*MaxPeakSize) {
        cout<<"Setting new high edge: "<<P.GetHighEdge()<<" --> "<<Data->GetBinLowEdge(RightLimitBin) + 0.5*MaxPeakSize<<endl;
        P.SetHighEdge(Data->GetBinLowEdge(RightLimitBin) + 0.25*MaxPeakSize);
      }
      if (Data->GetXaxis()->GetBinUpEdge(LeftLimitBin) - Data->GetBinCenter(LeftLimitBin) > 0.25*MaxPeakSize) {
        cout<<"Setting new low edge: "<<P.GetLowEdge()<<" --> "<<Data->GetXaxis()->GetBinUpEdge(LeftLimitBin) - MaxPeakSize<<endl;
        P.SetLowEdge(Data->GetXaxis()->GetBinUpEdge(LeftLimitBin) - 0.25*MaxPeakSize);
      }
      */
      
      //cout<<"Left edge until next increase: "<<P.GetLowEdge()<<endl;
      //cout<<"Symetrized right edge until next increase: "<<P.GetHighEdge()<<endl;
           
      m_Results.AddSpectralPoint(ROGID, P);
    }
  }
  //cout<<"Final: "<<T.GetElapsed()<<endl;
  
   
  if (m_DiagnosticsMode == true) {
    TLockGuard G(gROOTMutex);
    TCanvas* FirstDerivationCanvas = new TCanvas();
    FirstDerivationCanvas->cd();
    FirstDerivation->Draw();
    FirstDerivationCanvas->Update();
    
    TCanvas* DataCanvas = new TCanvas();
    DataCanvas->cd();
    Data->Draw();
    //cout<<"Found points: "<<endl;
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(ROGID); ++p) {
      MCalibrationSpectralPoint P = m_Results.GetSpectralPoint(ROGID, p);
      
      TBox* Box = new TBox(P.GetLowEdge(), 0.0, P.GetHighEdge(), (1-0.1*p)*Data->GetMaximum());
      //Box->SetLineColor(kRed+p);
      Box->SetFillColor(kRed+p);
      Box->Draw();
      //cout<<P<<endl;
    }
    Data->Draw("SAME");
    DataCanvas->Update();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Fit the peaks in this read-out data group
bool MCalibrateEnergyFindLines::FitPeaks(unsigned int ROGID)
{
  if (m_PeakParametrizationMethod == c_PeakParametrizationMethodBayesianBlockPeak) {
    // we are already done since this is used during peak finding 
  } else if (m_PeakParametrizationMethod == c_PeakParametrizationMethodFittedPeak) {
    MBinnerFixedNumberOfBins FitBinner;
    FitBinner.SetNumberOfBins(m_RangeMaximum - m_RangeMinimum);
    FitBinner.AlignBins(true);
    FitBinner.SetMinMax(m_RangeMinimum, m_RangeMaximum);
    
    for (unsigned int d = 0; d < m_ROGs[ROGID]->GetNumberOfReadOutDatas(); ++d) {
        MReadOutDataADCValue* ADC = dynamic_cast<MReadOutDataADCValue*>(m_ROGs[ROGID]->GetReadOutData(d).Get(MReadOutDataADCValue::m_TypeID));
      if (ADC != nullptr) {
        FitBinner.Add(ADC->GetADCValue());
      }
    }
    TH1D* FitData = FitBinner.GetNormalizedHistogram("Data - fitting resolution", "ADC Values", "counts / ADC value");
    FitBinner.Clear();
    
    // Fit each point
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(ROGID); ++p) {
      MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(ROGID, p);
      
      if (m_PeakParametrizationMethodFittedPeakPeakShapeModel == MCalibrationFit::c_PeakShapeModelGaussian) {
        MCalibrationFitGaussian G;
        G.SetBackgroundModel(m_PeakParametrizationMethodFittedPeakBackgroundModel);
        G.SetEnergyLossModel(m_PeakParametrizationMethodFittedPeakEnergyLossModel);
        G.SetGaussianMean(P.GetPeak());
        G.SetGaussianSigma(P.GetFWHM()/2.35);
        P.SetFit(G);
      } else if (m_PeakParametrizationMethodFittedPeakPeakShapeModel == MCalibrationFit::c_PeakShapeModelGaussLandau) {
        MCalibrationFitGaussLandau G;
        G.SetBackgroundModel(m_PeakParametrizationMethodFittedPeakBackgroundModel);
        G.SetEnergyLossModel(m_PeakParametrizationMethodFittedPeakEnergyLossModel);
        G.SetGaussianMean(P.GetPeak());
        G.SetGaussianSigma(P.GetFWHM()/2.35);
        P.SetFit(G);
      } else {
        new MExceptionUnknownMode("peak parametrization method peak shape", m_PeakParametrizationMethodFittedPeakPeakShapeModel);
        continue;
      }
    
      // Do the fit and do some average deviation checks
      double ChiSquareLimit = 2.0;
      double AverageDeviationLimit = 0.05;
      MCalibrationFit& Fit = P.GetFit();
      
      // Do we have a good fit at all
      bool IsGood = Fit.Fit(*FitData, P.GetLowEdge(), P.GetHighEdge());
      if (IsGood == false) {
        if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Rejected: The line could not be fit good enough."<<endl;
        P.IsGood(false);
        continue;
      }
      
      // Check for catastrophic errors
      if (!isfinite(Fit.GetReducedChisquare())) {
        if (g_Verbosity >= c_Error) cout<<P.GetPeak()<<" - Rejected: Chi-square is either nan or inf! Something is wrong with your fit!!"<<endl;
        P.IsGood(false);
        continue;
      }
      if (!isfinite(Fit.GetAverageDeviation())) {
        if (g_Verbosity >= c_Error) cout<<P.GetPeak()<<" - Rejected: Average deviation is either nan or inf! Something is wrong with your fit!!"<<endl;
        P.IsGood(false);
        continue;
      }
      
      // Check fit quality
      if (Fit.GetReducedChisquare() > ChiSquareLimit && Fit.GetAverageDeviation() > AverageDeviationLimit) {
        if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Rejected: Bad chisquare "<<Fit.GetReducedChisquare()<<" (with limit "<<ChiSquareLimit<<") and average deviation between fit and data ("<<100*Fit.GetAverageDeviation()<<"% vs. a limit of "<<100*AverageDeviationLimit<<"%)"<<endl;
        P.IsGood(false);
        continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Good chi square: "<<Fit.GetReducedChisquare()<<" (compare to an average deviation of "<<100*Fit.GetAverageDeviation()<<"%)"<<endl;
      }
      
      // No weak peaks on increasing edges
      if (Fit.Evaluate(P.GetLowEdge()) < 0.8*Fit.Evaluate(P.GetHighEdge()) &&
        Fit.Evaluate(P.GetHighEdge()) > 3.0 && Fit.Evaluate(P.GetPeak())/Fit.Evaluate(P.GetHighEdge()) < 10) {
        if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Rejected: The peak sits on a strong increasing incline (left "<<Fit.Evaluate(P.GetLowEdge())<<" is less than 80% of right "<<Fit.Evaluate(P.GetHighEdge())<<" and right has at least an 3 count average and the peak-to-right-edge ratio is smaller than 10)"<<endl;  
        //P.IsGood(false);
        //continue;
      } else {
        if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Passed: The peak sits NOT on a strong increasing incline (left "<<Fit.Evaluate(P.GetLowEdge())<<" is more than 80% of right "<<Fit.Evaluate(P.GetHighEdge())<<" or right has less than a 3 count average and the peak-to-right-edge ratio is not smaller than 10)"<<endl;          
      }
      
  
      // Good for now
      P.SetPeak(Fit.GetPeak());
      P.SetFWHM(Fit.GetFWHM());
      if (m_DiagnosticsMode == true) {
        TCanvas* C = new TCanvas();
        C->cd();
        Fit.Draw();
        FitData->DrawCopy("E SAME");
        C->Update();
      }
    }
    
  }
  

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Do a final check of all peaks
bool MCalibrateEnergyFindLines::CheckPeaks()
{
  // Find stuff which is off
  vector<double> FWHMes;
  
  // Eliminate too large peaks -- they are most likely noise of Compton edges
  double FWHMCutOffScaler = 2.5;
  
  FWHMes.clear();
  for (unsigned int rg = 0; rg < m_Results.GetNumberOfReadOutDataGroups(); ++rg) {
    for (unsigned int sp = 0; sp < m_Results.GetNumberOfSpectralPoints(rg); ++sp) {
      if (m_Results.GetSpectralPoint(rg, sp).IsGood() == false) continue;
      MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(rg, sp);
      FWHMes.push_back(P.GetFWHM());
    }
  }
  
  // Find the average value of the lower 50% + 1 entries
  if (FWHMes.size() >= 3) {
    sort(FWHMes.begin(), FWHMes.end()); 
  
    double LowerAverageFWHM = 0.0;
    for (unsigned int i = 0; i < FWHMes.size()/2 + 1; ++i) {
      LowerAverageFWHM += FWHMes[i]; 
    }
    LowerAverageFWHM /= (FWHMes.size()/2 + 1);
  
    for (unsigned int rg = 0; rg < m_Results.GetNumberOfReadOutDataGroups(); ++rg) {
      for (unsigned int sp = 0; sp < m_Results.GetNumberOfSpectralPoints(rg); ++sp) {
        if (m_Results.GetSpectralPoint(rg, sp).IsGood() == false) continue;
        MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(rg, sp);
        if (P.GetFWHM() > FWHMCutOffScaler*LowerAverageFWHM) {
          P.IsGood(false);
          if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Rejected: Excluding peak at "<<P.GetPeak()<<" with FWHM of "<<P.GetFWHM()<<" since its width is more than "<<FWHMCutOffScaler<<" times the with of the smaller 50% + 1 FWHMes ("<<LowerAverageFWHM<<")"<<endl;
        }
      }
    }
  }
    
  // Another sanity check:
  // Calculate the median FWHM
  // If one peak is at least 3x the average, then exclude it
  
  
  // Thomson Tau sanity check
  if (g_Verbosity >= c_Info) cout<<"Sanity check: median FWHM & modified Thomson Tau outlier test"<<endl;
  FWHMes.clear();
  for (unsigned int rg = 0; rg < m_Results.GetNumberOfReadOutDataGroups(); ++rg) {
    for (unsigned int sp = 0; sp < m_Results.GetNumberOfSpectralPoints(rg); ++sp) {
      if (m_Results.GetSpectralPoint(rg, sp).IsGood() == false) continue;
      MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(rg, sp);
      FWHMes.push_back(P.GetFWHM());
    }
  }
  
  /*
  if (FWHMes.size() >= 3) {
    // Calculate outliers via the modified thomson tau method
    MMath M;
    vector<bool> IsOutlier = M.ModifiedThomsonTauTest(FWHMes, 0.002);
    
    unsigned int Counter = 0;
    for (unsigned int rg = 0; rg < m_Results.GetNumberOfReadOutDataGroups(); ++rg) {
      for (unsigned int sp = 0; sp < m_Results.GetNumberOfSpectralPoints(rg); ++sp) {
        if (m_Results.GetSpectralPoint(rg, sp).IsGood() == false) continue;
        MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(rg, sp);
        if (IsOutlier[Counter] == true) {
          P.IsGood(false);
          if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Rejected: Excluding peak at "<<P.GetPeak()<<" with FWHM of "<<P.GetFWHM()<<" (TS: "<<FWHMes[Counter]<<") as outlier using modified Thomson Tau test"<<endl;
        } else {
          if (g_Verbosity >= c_Info) cout<<P.GetPeak()<<" - Passed: FWHM test: Keeping peak at "<<P.GetPeak()<<" with FWHM of "<<P.GetFWHM()<<" TS: "<<FWHMes[Counter]<<")"<<endl;          
        }
        ++Counter;
      }
    }
    
  } else {
    if (g_Verbosity >= c_Info) cout<<"Not enough FWHMes found for FWHM sanity check!"<<endl; 
  }
  */
  
  return true;
}


// MCalibrateEnergyFindLines.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
