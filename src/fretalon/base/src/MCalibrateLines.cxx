/*
 * MCalibrateLines.cxx
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
#include "MCalibrateLines.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:
#include "TF1.h"
#include "TMath.h"
#include "TBox.h"

// MEGAlib libs:
#include "MExceptions.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MCalibrationSpectralPoint.h"
#include "MReadOutDataInterfaceADCValue.h"
#include "MCalibrationFit.h"
#include "MCalibrationFitGaussian.h"
#include "MCalibrationFitGaussLandau.h"
#include "MCalibrationModel.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MCalibrateLines)
#endif


////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////


double GaussLandauFlatBackground(double *x, double *par)
{
  // par[0] is Offset
  // par[1] is Landau-height
  // par[2] is Landau-mean
  // par[3] is Landau-sigma
  // par[4] is Gaus-height
  // par[5] is Gaus-mean
  // par[6] is Gaus-sigma

  double fitval = par[0];
  double arg = 0;

  if (par[6] != 0) arg = ((x[0] - par[5])/par[6]);
  fitval += par[4]*TMath::Exp(-0.5*arg*arg);

  fitval += par[1]*TMath::Landau(-x[0] + par[2], 0, par[3]);

  return fitval;
}


double GaussLandauLinearBackground(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is Gaus-mean
  // par[5] is Gaus-sigma
  // par[6] is Gradient
  // par[7] is Offset

  double fitval = par[6]*x[0] + par[7];
  double arg = 0;

  if (par[5] != 0) arg = ((x[0] - par[4])/par[5]);
  fitval += par[3]*TMath::Exp(-0.5*arg*arg);

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}



double GaussLandauFixedMeanPol0Background(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is Gaus-sigma
  // par[5] is Offset

  double fitval = par[5];
  double arg = 0;

  if (par[4] != 0) {
    arg = ((x[0] - par[1])/par[4]);
    fitval += par[3]*TMath::Exp(-0.5*arg*arg);
  }

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}



double GaussLandauFixedMeanPol1Background(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is Gaus-sigma
  // par[5] is Gradient
  // par[6] is Offset

  double fitval = par[5]*x[0] + par[6];
  double arg = 0;

  if (par[4] != 0) {
    arg = ((x[0] - par[1])/par[4]);
    fitval += par[3]*TMath::Exp(-0.5*arg*arg);
  }

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}


double GaussLandauFixedMeanPol2Background(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is Gaus-sigma
  // par[5] is pol0
  // par[6] is pol1
  // par[7] is pol2

  double fitval = par[5] + par[6]*x[0] + par[7]*x[0]*x[0];
  double arg = 0;

  if (par[4] != 0) {
    arg = ((x[0] - par[1])/par[4]);
    fitval += par[3]*TMath::Exp(-0.5*arg*arg);
  }

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}


double GaussLandauFixedSigmaPol2Background(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is Gaus-mean
  // par[5] is pol0
  // par[6] is pol1
  // par[7] is pol2

  double fitval = par[5] + par[6]*x[0] + par[7]*x[0]*x[0];
  double arg = 0;

  if (par[4] != 0) {
    arg = ((x[0] - par[4])/par[2]);
    fitval += par[3]*TMath::Exp(-0.5*arg*arg);
  }

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}


double GaussLandauFixedMeanFixedSigmaPol2Background(double *x, double *par)
{
  // par[0] is Landau-height
  // par[1] is Landau-mean
  // par[2] is Landau-sigma
  // par[3] is Gaus-height
  // par[4] is pol0
  // par[5] is pol1
  // par[6] is pol2

  double fitval = par[4] + par[5]*x[0] + par[6]*x[0]*x[0];
  double arg = 0;

  if (par[4] != 0) {
    arg = ((x[0] - par[1])/par[2]);
    fitval += par[3]*TMath::Exp(-0.5*arg*arg);
  }

  fitval += par[0]*TMath::Landau(-x[0] + par[1], 0, par[2]);

  return fitval;
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MCalibrateLines::MCalibrateLines() : MCalibrate()
{
  m_PeakParametrizationMethod = c_PeakParametrizationMethodBayesianBlockPeak;
  
  m_PeakParametrizationMethodFittedPeakBackgroundModel = MCalibrationFit::c_BackgroundModelLinear; 
  m_PeakParametrizationMethodFittedPeakEnergyLossModel = MCalibrationFit::c_EnergyLossModelNone; 
  m_PeakParametrizationMethodFittedPeakPeakShapeModel = MCalibrationFit::c_PeakShapeModelGaussian;
  
  m_CalibrationModelDeterminationMethod = c_CalibrationModelStepWise;
  m_CalibrationModelDeterminationMethodFittingModel = MCalibrationModel::c_CalibrationModelPoly3;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MCalibrateLines::~MCalibrateLines()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Add a read-out data group and the associated isotopes
void MCalibrateLines::AddReadOutDataGroup(const MReadOutDataGroup& ROG, const vector<MIsotope>& Isotopes)
{
  m_ROGs.push_back(ROG);
  m_Isotopes.push_back(Isotopes);
  vector<MCalibrationSpectralPoint> P;
  m_SpectralPoints.push_back(P);
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration
bool MCalibrateLines::Calibrate()
{
  m_Results.Clear();
  m_Results.SetNumberOfReadOutDataGroups(m_ROGs.size());
  
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    FindPeaks(r);   
    FitPeaks(r);
  }

  AssignEnergies();
  
  DetermineModels();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Find the peaks in this read-out data group
bool MCalibrateLines::FindPeaks(unsigned int ROGID)
{
  cout<<"Finding peaks for ROG ID: "<<ROGID<<endl;
  
  int Prior = 8;
  
  int FirstPeakMinimumBinID = 7;
  double FirstPeakMinimumPeakCounts = 300; 
  
  double MinimumPeakCounts = 100; 
  double MinimumHeight = 1.5; 
  double MinimumBinWidthForBayesianBinner = 8;
  double ComptonEdgeThreshold = 0.5;
  double RangeBeyondPeak = 2.5;
  //double Epsilon = 0.01;

  // Step 1: Create a histogram with the correct binning:
  MBinnerBayesianBlocks Binner;
  Binner.SetMinimumBinWidth(MinimumBinWidthForBayesianBinner);
  Binner.SetMinMax(m_RangeMinimum, m_RangeMaximum);
  Binner.SetPrior(Prior);
  for (unsigned int d = 0; d < m_ROGs[ROGID].GetNumberOfReadOutDatas(); ++d) {
    MReadOutDataInterfaceADCValue* ADC = dynamic_cast<MReadOutDataInterfaceADCValue*>(&(m_ROGs[ROGID].GetReadOutData(d)));
    if (ADC != nullptr) {
      Binner.Add(ADC->GetADCValue());
    }
  }
  TH1D* Data = Binner.GetNormalizedHistogram(MString("Data ") + ROGID, "ADC Values", "counts / ADC value");
  Binner.Clear();
  
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
      
      cout<<endl<<"Start"<<endl;
      
      // ignore the first peak as background rollover or noise
      if (Start == true) {
        Start = false;
        FirstPeak = true;
        //cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: First peak"<<endl;
        //continue;
      } else {
        FirstPeak = false; 
      }

      // Ignore the last bin: 
      if (b+1 == FirstDerivation->GetNbinsX()) continue;
        
      // Create a peak
      MCalibrationSpectralPoint P;
      P.SetPeak(FirstDerivation->GetBinLowEdge(b+1));
      
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
      cout<<"Maximum/Minimum bin: "<<MaximumBin<<":"<<MinimumBin<<endl;
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
      cout<<FirstDerivation->GetBinLowEdge(b+1)<<": Height etc. : "<<CountsPerBinBefore<<":"<<Height<<":"<<CountsPerBinAfter<<endl;
      Height -= 0.5*(CountsPerBinBefore + CountsPerBinAfter);
      
      if (CountsPerBin*Width < MinimumPeakCounts) {
        cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough counts per bin: "<<CountsPerBin*Width<<" (min: "<<MinimumPeakCounts<<")"<<endl;
        continue;
      }
      P.SetCounts(CountsPerBin*Width);
        
      if (Height < MinimumHeight) {
        cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Peak height to small: "<<Height<<" (min: "<<MinimumHeight<<")"<<endl;
        continue;
      }
        
        
      double Minimum = FirstDerivation->GetBinContent(MinimumBin);
      double Maximum = FirstDerivation->GetBinContent(MaximumBin);
      double Volatility = Maximum - Minimum;
      if (!(Volatility > 0.015 && 
          ((Maximum > +0.0075 && Minimum < -0.0025) ||
           (Minimum < -0.0075 && Maximum > +0.0025)))) {
        cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Derivative peak not strong enough: "<<FirstDerivation->GetBinContent(MaximumBin)<<" and "<<FirstDerivation->GetBinContent(MinimumBin)<<endl;
        continue;
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
      for (unsigned int d = 0; d < m_ROGs[ROGID].GetNumberOfReadOutDatas(); ++d) {
        MReadOutDataInterfaceADCValue* ADC = dynamic_cast<MReadOutDataInterfaceADCValue*>(&(m_ROGs[ROGID].GetReadOutData(d)));
        if (ADC != nullptr) {
          if (ADC->GetADCValue() >= Data->GetBinLowEdge(PeakBin) && 
              ADC->GetADCValue() < Data->GetBinLowEdge(PeakBin) + Data->GetBinWidth(PeakBin)) {
            TotalCounts++;
            Total += ADC->GetADCValue();
          }
        }
      }
      P.SetPeak(Total/TotalCounts); // ???

      cout<<"Peak: "<<P.GetPeak()<<endl;
      cout<<"Peak bin: "<<PeakBin<<endl;
      
      // If this is the first peak it must be at least 3 bins away from the first bin and from any bin with less than 1 counts:
      if (FirstPeak == true) {
        if (PeakBin < FirstPeakMinimumBinID) {
          cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: First peak must be at least "<<FirstPeakMinimumBinID<<" bins away from start"<<endl;
          continue;          
        }
      }
      
      
      
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
      cout<<"Right limit bin: "<<RightLimitBin<<endl;
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
      cout<<"Left limit bin: "<<LeftLimitBin<<endl;
      
      // Compton edge check:
      // If the left peak-to-valley is less than 50% (=ComptonEdgeThreshold) of the right peak-to-valley 
      // then we probably have a Compton edge or another bad peak
      if (Data->GetBinContent(PeakBin) - Data->GetBinContent(LeftLimitBin) < ComptonEdgeThreshold * (Data->GetBinContent(PeakBin) - Data->GetBinContent(RightLimitBin))) {
        cout<<FirstDerivation->GetBinLowEdge(b+1)<<": The left peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(LeftLimitBin)<<") is less than "<<100*ComptonEdgeThreshold<<"% of the right peak-to-valley ("<<Data->GetBinContent(PeakBin) - Data->GetBinContent(RightLimitBin)<<"): We might have a Compton edge or other bad peak"<<endl;
        continue;
      }
      
      // Check if we have enough counts
      double Average = 0.5*(Data->GetBinWidth(RightLimitBin)*Data->GetBinContent(RightLimitBin) + Data->GetBinWidth(LeftLimitBin)*Data->GetBinContent(LeftLimitBin))/(Data->GetBinWidth(RightLimitBin) + Data->GetBinWidth(LeftLimitBin));
      double Integral = Data->Integral(LeftLimitBin, RightLimitBin, "width");
      double Excess = Integral - Average*(Data->GetXaxis()->GetBinUpEdge(RightLimitBin) - Data->GetBinLowEdge(LeftLimitBin));
      
      cout<<"Avg: "<<Average<<" Int: "<<Integral<<" Width: "<<Data->GetXaxis()->GetBinUpEdge(RightLimitBin) - Data->GetBinLowEdge(LeftLimitBin)<<endl;
      
      if (FirstPeak == true) {
        if (Excess < FirstPeakMinimumPeakCounts) {
          cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough peak counts for a first peak: "<<Excess<<endl;
          continue;
        }         
      } else {
        if (Excess < MinimumPeakCounts) {
          cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Rejected: Not enough peak counts: "<<Excess<<endl;
          continue;
        } 
      }
      cout<<FirstDerivation->GetBinLowEdge(b+1)<<" - Estimated peak counts: "<<Excess<<endl;
      
      
      
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
        cout<<"Check: "<<bb<<" - "<<Data->GetBinContent(bb)<<" vs. "<<RightHalf<<endl;
        if (Data->GetBinContent(bb) <= RightHalf) {
          double Range = Data->GetBinCenter(bb) - P.GetPeak();
          RightHalfBin = bb;
          cout<<"Range right: "<<Range<<" vs. "<<P.GetHighEdge()<<":"<<P.GetPeak() + 4*Range<<endl;
          if (P.GetHighEdge() > P.GetPeak() + RangeBeyondPeak*Range) {
            cout<<"New high edge from FWHM determination: "<<P.GetPeak() + 4*Range<<endl;
            P.SetHighEdge(P.GetPeak() + RangeBeyondPeak*Range);
          }
          break;
        }
      }
      int LeftHalfBin = PeakBin;
      double LeftHalf = 0.5 *(Data->GetBinContent(PeakBin) + Data->GetBinContent(LeftLimitBin));
      for (int bb = PeakBin; bb > 0; --bb) {
        cout<<"Check: "<<bb<<" - "<<Data->GetBinContent(bb)<<" vs. "<<LeftHalf<<endl;
        if (Data->GetBinContent(bb) <= LeftHalf) {
          double Range = P.GetPeak() - Data->GetBinCenter(bb);
          LeftHalfBin = bb;
          cout<<"Range left: "<<Range<<" vs. "<<P.GetLowEdge()<<":"<<P.GetPeak() - 4*Range<<endl;
          if (P.GetLowEdge() < P.GetPeak() - RangeBeyondPeak*Range) {
            cout<<"New high edge from FWHM determination: "<<P.GetPeak() - 4*Range<<endl;
            P.SetLowEdge(P.GetPeak() - RangeBeyondPeak*Range);
          }
          break;
        }
      }
      P.SetFWHM(Data->GetBinLowEdge(RightHalfBin) - Data->GetXaxis()->GetBinUpEdge(LeftHalfBin));
      cout<<"FWHM: "<<P.GetFWHM()<<endl;
      
      
      
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
bool MCalibrateLines::FitPeaks(unsigned int ROGID)
{
  if (m_PeakParametrizationMethod == c_PeakParametrizationMethodBayesianBlockPeak) {
    // we are already done since this is used during peak finding 
  } else if (m_PeakParametrizationMethod == c_PeakParametrizationMethodFittedPeak) {
    MBinnerFixedNumberOfBins FitBinner;
    FitBinner.SetNumberOfBins(m_RangeMaximum - m_RangeMinimum);
    FitBinner.AlignBins(true);
    FitBinner.SetMinMax(m_RangeMinimum, m_RangeMaximum);
    
    for (unsigned int d = 0; d < m_ROGs[ROGID].GetNumberOfReadOutDatas(); ++d) {
      MReadOutDataInterfaceADCValue* ADC = dynamic_cast<MReadOutDataInterfaceADCValue*>(&(m_ROGs[ROGID].GetReadOutData(d)));
      if (ADC != nullptr) {
        FitBinner.Add(ADC->GetADCValue());
      }
    }
    TH1D* FitData = FitBinner.GetNormalizedHistogram("Data - fitting resolution", "ADC Values", "counts / ADC value");
    FitBinner.Clear();

  
    // Fit each point
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(ROGID); ++p) {
      MCalibrationSpectralPoint& P = m_Results.GetSpectralPoint(ROGID, p);
      
      cout<<m_PeakParametrizationMethodFittedPeakPeakShapeModel<<endl;
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
    
      // Somehow the copy constructor of ROOT's TF1 gives trouble
      MCalibrationFit& Fit = P.GetFit();
      bool IsGood = Fit.Fit(*FitData, P.GetLowEdge(), P.GetHighEdge());
      cout<<"Result: "<<(IsGood ? "true" : "false")<<endl;
      P.IsGood(IsGood);
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

class Match 
{
public:
  Match(unsigned int ROG, unsigned int Point,  unsigned int Isotope, unsigned int Line) {
    m_ROG = ROG;
    m_Point = Point;
    m_Isotope = Isotope;
    m_Line = Line;
  }
  unsigned int m_ROG;
  unsigned int m_Point;
  unsigned int m_Isotope;
  unsigned int m_Line;
  double m_Conversion;
  double m_QualityFactor;
};


//! Assign energies to the different spectral points
bool MCalibrateLines::AssignEnergies()
{
  vector<Match> Matches;
    
  // Create all combinations
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
        for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
          Match M(r, i, p, l);
          M.m_Conversion = m_Isotopes[r][i].GetLineEnergy(l)/m_Results.GetSpectralPoint(r, p).GetPeak();
          Matches.push_back(M);
        }
      }
    }
  }
  
  // Calculate the quality factor:
  for (unsigned int m = 0; m < Matches.size(); ++m) {
    double QualityFactor = 0.0;
    int QualityFactorCounter = 0.;
    for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
      for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
        if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
        double Energy = m_Results.GetSpectralPoint(r, p).GetPeak() * Matches[m].m_Conversion;
        
        // Find closest line:
        unsigned int closest_i = 0;
        unsigned int closest_l = 0;
        double Difference = numeric_limits<double>::max();
        for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
          for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
            if (fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l)) < Difference) {
              Difference = fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l));
              closest_i = i;
              closest_l = l;
            }
          }
        }
        
        QualityFactor += fabs(Energy - m_Isotopes[r][closest_i].GetLineEnergy(closest_l))/m_Isotopes[r][closest_i].GetLineEnergy(closest_l);
        QualityFactorCounter++;
      }
    }
    Matches[m].m_QualityFactor = QualityFactor/QualityFactorCounter;
  }
    
  // Find the best (=smallest) quality factor
  unsigned int BestMatch = 0;
  double BestMatchQualityFactor = Matches[BestMatch].m_QualityFactor;
  for (unsigned int m = 1; m < Matches.size(); ++m) {
    if (Matches[m].m_QualityFactor < BestMatchQualityFactor) {
      BestMatchQualityFactor = Matches[m].m_QualityFactor;
      BestMatch = m;
    }
  }
    
  // Finally do the assignment:  
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      double Energy = m_Results.GetSpectralPoint(r, p).GetPeak() * Matches[BestMatch].m_Conversion;
        
      // Find closest line:
      unsigned int closest_i = 0;
      unsigned int closest_l = 0;
      double Difference = numeric_limits<double>::max();
      for (unsigned int i = 0; i < m_Isotopes[r].size(); ++i) {
        for (unsigned int l = 0; l < m_Isotopes[r][i].GetNLines(); ++l) {
          if (fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l)) < Difference) {
            Difference = fabs(Energy - m_Isotopes[r][i].GetLineEnergy(l));
            closest_i = i;
            closest_l = l;
          }
        }
      }
      m_Results.GetSpectralPoint(r, p).SetIsotope(m_Isotopes[r][closest_i]);
      m_Results.GetSpectralPoint(r, p).SetEnergy(m_Isotopes[r][closest_i].GetLineEnergy(closest_l)); 
      m_Results.GetSpectralPoint(r, p).IsGood(!m_Isotopes[r][closest_i].GetLineExcludeFlag(closest_l));
    }
  }

  // If one peak is used twice - throw out the one with lower count rate
  //bool FoundNotGood = false;
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      if (m_Results.GetSpectralPoint(r, p).IsGood() == false) continue;
      double Energy = m_Results.GetSpectralPoint(r, p).GetEnergy();
      for (unsigned int q = p+1; q < m_Results.GetNumberOfSpectralPoints(r); ++q) {
        if (m_Results.GetSpectralPoint(r, q).IsGood() == false) continue;
        if (m_Results.GetSpectralPoint(r, q).GetEnergy() == Energy) {
          if (m_Results.GetSpectralPoint(r, p).GetCounts() < m_Results.GetSpectralPoint(r, q).GetEnergy()) {
            m_Results.GetSpectralPoint(r, p).IsGood(false);
            cout<<"Bad: "<<m_Results.GetSpectralPoint(r, p)<<endl;
            //FoundNotGood = true;
            break;
          } else {
            m_Results.GetSpectralPoint(r, q).IsGood(false);  
            cout<<"Bad: "<<m_Results.GetSpectralPoint(r, q)<<endl;
            //FoundNotGood = true;
          }
        }
      }
    }
  }
   
  //! Rinse and repeat?
  //if (FoundNotGood == true) {
  //  AssignEnergies();
  //}
      
  /*
  cout<<"Calibration result: "<<endl;
  for (unsigned int r = 0; r < m_ROGs.size(); ++r) {
    for (unsigned int p = 0; p < m_Results.GetNumberOfSpectralPoints(r); ++p) {
      cout<<m_Results.GetSpectralPoint(r, p)<<endl;
    }
  }
  */ 
      
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Determine the calibration model
bool MCalibrateLines::DetermineModels()
{
  // Assemble the unique lines:
  vector<MCalibrationSpectralPoint> Points = m_Results.GetUniquePoints();
  if (Points.size() < 2) return true;
  
  if (m_CalibrationModelDeterminationMethod == c_CalibrationModelStepWise) {
    // we are already done since this is used during peak finding 
    cout<<"Doing calibration model steps"<<endl;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelFit) {
    cout<<"Doing calibration model fit"<<endl;
    
    // Set up the model:
    MCalibrationModel* Model = 0;
    if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1) {
      Model = new MCalibrationModelPoly1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly2) {
      Model = new MCalibrationModelPoly2();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly3) {
      Model = new MCalibrationModelPoly3();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly4) {
      Model = new MCalibrationModelPoly4();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Inv1) {
      Model = new MCalibrationModelPoly1Inv1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp1) {
      Model = new MCalibrationModelPoly1Exp1();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp2) {
      Model = new MCalibrationModelPoly1Exp2();
    } else if (m_CalibrationModelDeterminationMethodFittingModel == MCalibrationModel::c_CalibrationModelPoly1Exp3) {
      Model = new MCalibrationModelPoly1Exp3();
    } else {
      new MExceptionUnknownMode("fitting model to determine calibration model", m_CalibrationModelDeterminationMethodFittingModel);
      return false;
    }
    
    double Quality = Model->Fit(Points);
    cout<<"Fit quality: "<<Quality<<endl;
    
    m_Results.SetModel(*Model);
    
    //delete Model;
  } else if (m_CalibrationModelDeterminationMethod == c_CalibrationModelBestFit) {
    cout<<"Find best (fitted) calibration model"<<endl;
    
    // Assemble the models
    vector<MCalibrationModel*> Models;
    Models.push_back(new MCalibrationModelPoly1());
    Models.push_back(new MCalibrationModelPoly2());
    Models.push_back(new MCalibrationModelPoly3());
    Models.push_back(new MCalibrationModelPoly4());
    Models.push_back(new MCalibrationModelPoly1Inv1());
    Models.push_back(new MCalibrationModelPoly1Exp1());
    Models.push_back(new MCalibrationModelPoly1Exp2());
    Models.push_back(new MCalibrationModelPoly1Exp3());
    
    vector<double> Results;
    for (unsigned int m = 0; m < Models.size(); ++m) {
      Results.push_back(Models[m]->Fit(Points));
      cout<<"Model "<<Models[m]->GetName()<<": "<<Results.back()<<endl;
    }
    
    vector<double>::iterator MinI;
    MinI = min_element(Results.begin(), Results.end());
    
    int Min = int(MinI -  Results.begin());
    cout<<"Best model: "<<Models[Min]->GetName()<<endl;
    
    m_Results.SetModel(*Models[Min]);
    
    for (unsigned int m = 0; m < Models.size(); ++m) {
      //delete Models[m];
    }
  } else {
    new MExceptionUnknownMode("calibration model determination method", m_CalibrationModelDeterminationMethod);
    return false;
  }
  
  return true;
}



////////////////////////////////////////////////////////////////////////////////


//! Get/compile the calibration result
MCalibrationSpectrum MCalibrateLines::GetCalibration()
{
  return m_Results;
}


// MCalibrateLines.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
