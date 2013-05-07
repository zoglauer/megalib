/*
 * MSpectralAnalyzer.cxx
 *
 *
 * Copyright (C) by Michelle Galloway & Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Michelle Galloway & Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MSpectralAnalyzer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSpectralAnalyzer.h"
#include "MMath.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
//#include "MFileEventsTra.h"
//#include "MFileEventsEvta.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSpectralAnalyzer)
#endif


////////////////////////////////////////////////////////////////////////////////


// Sort two isotopes by their rating
bool SortIsotopesByRating(MIsotope* A, MIsotope* B) { return (A->GetRating() < B->GetRating()); }


////////////////////////////////////////////////////////////////////////////////


MSpectralAnalyzer::MSpectralAnalyzer()
{
  // Construct an instance of MSpectralAnalyzer
  
  m_InitialSpectrum = 0;
  m_IsBatch = false;
}


////////////////////////////////////////////////////////////////////////////////


MSpectralAnalyzer::~MSpectralAnalyzer()
{
  // Delete this instance of MSpectralAnalyzer
  
  Reset();
  for (unsigned int i = 0; i < m_ComparisonIsotopes.size(); ++i) {
    delete m_ComparisonIsotopes[i];
  }
  m_ComparisonIsotopes.clear();
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::Reset()
{
  //! Resets all automatically filled data sets, e.g. the initial and intermediate histograms, the results, etc.
  
  delete m_InitialSpectrum;
  m_InitialSpectrum = 0;
  
  m_Isotopes.clear();
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    delete m_Peaks[p];  
  }
  m_Peaks.clear();
  
  for (unsigned int i = 0; i < m_ComparisonIsotopes.size(); ++i) {
    for (unsigned int l = 0; l < m_ComparisonIsotopes[i]->GetNLines(); ++l) {
      m_ComparisonIsotopes[i]->SetLineFound(l, false);
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::SetSpectrum(TH1D* Spectrum)
{
  //! Set the initial histogram using a TH1D Spectrum
  
  Reset();
  
  m_InitialSpectrum = new TH1D(*Spectrum); // Copy constructor...
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::SetSpectrum(int NBins, double EnergyMin, double EnergyMax, int BinningMode)
{
  //! Set the initial spectrum which will late be gradually filled with individual energies
  //! Binning modes:
  //! 0: Normal mode
  //! 1: Log mode
  //! 2: Variable bin width
  
  if (BinningMode == 1) {
    if (EnergyMin <= 0) EnergyMin = 1;
  }
  if (NBins < 1) {
    merr<<"Number of bins ("<<NBins<<") must be >= 1! Using 2!"<<show;
    NBins = 2;
  }
  if (EnergyMin >= EnergyMax) {
    merr<<"Minimum ("<<EnergyMin<<") must be smaller than maximum ("<<EnergyMax<<")! Using Max = Min + 10.0!"<<show;
    EnergyMax = EnergyMin + 10.0;
  }
  
  vector<MDDetector*> Detectors;
  if (BinningMode == 2) {
    for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
      for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
        if (m_Geometry->GetTriggerAt(t)->IncludesDetectorAsPositiveTrigger(m_Geometry->GetDetectorAt(d)) == true) {
          Detectors.push_back(m_Geometry->GetDetectorAt(d));
          break;
        }
      }
    }
    if (Detectors.size() == 0) {
      mout<<"Error: Variable bin width only supported if you have a triggering (not vetoing) detector in your geometry"<<endl;
      mout<<"       Using standard binning."<<endl;
      BinningMode = 0;
    } else {
      //cout<<"Detectors used to calculate average energy resolution: ";
      //for (unsigned int d = 0; d < Detectors.size(); ++d) {
      //  cout<<Detectors[d]->GetName()<<" ";
      //}
      //cout<<endl;
    }
  }

  // regular histogram routine
  double Dist = 0.0;
  double* Bins = new double[NBins+1];

  vector<double> vBins;
  double E = EnergyMin;
  vBins.push_back(E);
  double var = 1.7;


  if (BinningMode == 1) { // logarithmic
    EnergyMin = log(EnergyMin);
    EnergyMax = log(EnergyMax);
    Dist = (EnergyMax-EnergyMin)/(NBins);
    for (int i = 0; i < NBins + 1; ++i) {
      Bins[i] = exp(EnergyMin + i*Dist);
    }
  } else if (BinningMode == 2) { // variable bin size

    while (E <= EnergyMax) {
      double AverageResolution = 0.0;
      for (unsigned int d = 0; d < Detectors.size(); ++d) {
        AverageResolution += pow(Detectors[d]->GetEnergyResolution(E/Detectors.size()), 2);
      }
      Dist = sqrt(AverageResolution); // Div by zero taken care of earlier
      E += Dist/var;
      vBins.push_back(E);
    }
    delete [] Bins;
    Bins = new double[vBins.size()];
    for (unsigned int b = 0; b < vBins.size(); ++b) {
      Bins[b] = vBins[b];
    }
    NBins = vBins.size() - 1;
  } else { // standard
    Dist = (EnergyMax-EnergyMin)/(NBins);
    for (int i = 0; i < NBins + 1; ++i) {
      Bins[i] = EnergyMin + i*Dist;
    }   
  }
    
  // Create histogram
  m_InitialSpectrum = new TH1D("InitialSpectrum", "Initial Spectrum", NBins, Bins);
  m_InitialSpectrum->SetStats(false);
  m_InitialSpectrum->SetXTitle("Energy [keV]");
  m_InitialSpectrum->SetYTitle("Counts");
  m_InitialSpectrum->SetFillColor(8);
  m_InitialSpectrum->SetLabelSize(0.06f, "X");
  m_InitialSpectrum->SetLabelSize(0.06f, "Y");
  m_InitialSpectrum->SetTitleSize(0.06f, "X");
  m_InitialSpectrum->SetTitleSize(0.06f, "Y");
  m_InitialSpectrum->SetTitleOffset(1.0, "X");
  m_InitialSpectrum->SetTitleOffset(0.5, "Y");
  
  delete [] Bins;
  
    
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::FillSpectrum(double Energy)
{
  //! Adds the given energy to an existing (!) spectrum
  
  if (m_InitialSpectrum == 0) return false;
  
  m_InitialSpectrum->Fill(Energy);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::FillSpectrum(vector<double> Energies)
{
  //! Adds the given energies to an existing (!) spectrum
  
  if (m_InitialSpectrum == 0) return false;
  
  for (unsigned int e = 0; e < Energies.size(); ++e) {
    m_InitialSpectrum->Fill(Energies[e]);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSpectralAnalyzer::ResetSpectrum()
{
  //! Resets the spectrum
  
  m_InitialSpectrum->Reset();
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::SetIsotopeFileName(MString IsotopeFileName) 
{ 
  // Set the name of the file with the stored isotopes:
  
  m_IsotopeFileName = IsotopeFileName;
  
  MFile::ExpandFileName(m_IsotopeFileName);
  if (MFile::Exists(m_IsotopeFileName) == false) return false;
  
  // Load the isotopes:
  MParser iParser;
  iParser.Open(m_IsotopeFileName);

  m_ComparisonIsotopes.clear();
  for (unsigned int i = 0; i < iParser.GetNLines(); ++i) {
    if (iParser.GetTokenizerAt(i)->GetNTokens() == 0) continue;
    if (iParser.GetTokenizerAt(i)->GetNTokens() != 4) {
      mout<<"Wrong number of arguments: "<<iParser.GetTokenizerAt(i)->GetText()<<endl;
      continue;
    }
    bool Found = false;
    MString Name = iParser.GetTokenizerAt(i)->GetTokenAtAsString(0);
    unsigned int Nucleons = iParser.GetTokenizerAt(i)->GetTokenAtAsInt(1);
    for (unsigned int s = 0; s < m_ComparisonIsotopes.size(); ++s) {
      if (m_ComparisonIsotopes[s]->GetElement() == Name && m_ComparisonIsotopes[s]->GetNucleons() == Nucleons) {
        m_ComparisonIsotopes[s]->AddLine(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(2), iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(3));
        Found = true;
        break;
      }
    }
    if (Found == false) {
      MIsotope* Isotope = new MIsotope();
      Isotope->SetElement(Name);
      Isotope->SetNucleons(Nucleons);
      Isotope->AddLine(iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(2), iParser.GetTokenizerAt(i)->GetTokenAtAsDouble(3));
      m_ComparisonIsotopes.push_back(Isotope);
    }
  }
  
  //cout<<"Isotope list: "<<endl;
  //for (unsigned int i = 0; i < m_ComparisonIsotopes.size(); ++i) {
  //  cout<<*m_ComparisonIsotopes[i];
  //}
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////


vector<MIsotope> MSpectralAnalyzer::GetIsotopes() 
{ 
  //! Return a COPY of the list of isotopes

  vector<MIsotope> Isotopes;
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    Isotopes.push_back(*m_Isotopes[i]);
  } 

  return Isotopes;
}


////////////////////////////////////////////////////////////////////////////////


bool MSpectralAnalyzer::FindIsotopes()
{
  //! Do all the analysis and find the peaks & isotopes
  //! Do background fit and find peaks
  
  //cout<<"Do peak finding, peak selection & isotope ID"<<endl;

  // Determine the detectors for calculating the average evtlist.datenergy resolution:
  vector<MDDetector*> Detectors;
  for (unsigned int d = 0; d < m_Geometry->GetNDetectors(); ++d) {
    for (unsigned int t = 0; t < m_Geometry->GetNTriggers(); ++t) {
      if (m_Geometry->GetTriggerAt(t)->IncludesDetectorAsPositiveTrigger(m_Geometry->GetDetectorAt(d)) == true) {
        Detectors.push_back(m_Geometry->GetDetectorAt(d));
        break;
      }
    }
  }
  if (Detectors.size() == 0) {
    mout<<"Error: calculating the average energy resolution only works if we have a detector in the geometry file"<<endl;
    return false;
  }
 
  
  if (m_InitialSpectrum == 0) {
    merr<<"Spectrum not intialized!"<<endl;
    return false;
  }
  

  TH1D* PeakHist = new TH1D(*m_InitialSpectrum);
  PeakHist->SetTitle("PeakHist");
  TH1D* BkgrdHist = new TH1D(*m_InitialSpectrum);
  BkgrdHist->SetTitle("BkgrdHist");
  TH1D* FittingHist = new TH1D(*m_InitialSpectrum);
  FittingHist->SetTitle("FittingHist");

  TCanvas* PeakCanvas = 0;
  if (m_IsBatch == false) {
    PeakCanvas = new TCanvas("Peaks", "Peaks", 900, 600);
    PeakCanvas->cd();
    PeakCanvas->Divide(1,2);
    PeakCanvas->cd(1);
    gPad->SetPad(0.0, 0.5, 1.0, 1.0);
    m_InitialSpectrum->Draw();
    PeakCanvas->Update();
    
    PeakCanvas->cd(2);
    gPad->SetPad(0.0, 0.0, 1.0, 0.5);
    BkgrdHist->SetTitle("Background Estimate");
    PeakHist->SetTitleFont(50);
    BkgrdHist->Draw();
    PeakCanvas->Modified();
    PeakCanvas->Update();
  }
  
  
  // Step 1: Search for peaks
  
  int Peaksigma = 3;
  double threshold = 0.003;
  
  TSpectrum* PeakFinder = new TSpectrum();
  MString Option = "";
  if (m_IsBatch == true) Option = "goff";
  int NPeaksFound = PeakFinder->Search(PeakHist, Peaksigma, Option, threshold);
  float* PeakEnergies = PeakFinder->GetPositionX();
  
  if (m_IsBatch == false) {
    PeakCanvas->cd(1);
    PeakHist->SetTitle("Found Peaks");
    m_InitialSpectrum->Draw();
    PeakCanvas->Modified();
    PeakCanvas->Update();
  }
  
  
  // Step 2: Get peak energies, detector energy resolutions, etc. at the peaks
 
  for (int p = 0; p < NPeaksFound; ++p) {
    double PeakEnergy = PeakEnergies[p];
    double PeakBinContent = PeakHist->GetBinContent(PeakHist->GetXaxis()->FindBin(PeakEnergy));  // <- That's definitely wrong -- need to integrate!!
    
    double PeakOneSigmaWidth = 0.0;
    for (unsigned int d = 0; d < Detectors.size(); ++d) {
      PeakOneSigmaWidth += Detectors[d]->GetEnergyResolution(PeakEnergy/Detectors.size(), MVector(0.0, 0.0, 0.0))*Detectors[d]->GetEnergyResolution(PeakEnergy/Detectors.size(), MVector(0.0, 0.0, 0.0));
    }
    PeakOneSigmaWidth = sqrt(PeakOneSigmaWidth); // Div by zero taken care of earlier
      
    // Background estimate over found peak region    
    double BackgroundMin = (PeakEnergy - 12*PeakOneSigmaWidth);
    // Upper limit whould include Compton peak belong to this peak just in case this is a Compton edge...
    double BackgroundMax = PeakEnergy + max(12*PeakOneSigmaWidth, 6*PeakOneSigmaWidth + 0.5*(PeakEnergy + sqrt(PeakEnergy*PeakEnergy + 2*c_E0*PeakEnergy)));

    
    /* eliminate background estimate errors where energy falls to 0 below threshold
     * EnergyMin is first energy above threshold
     * could find with detector thresholds in geometry file? */
    
    double LowEnd = 0;
    double HighEnd = 0;
    double EnergyMin = 0;

    if (PeakEnergy > 50) {
      LowEnd = BackgroundMin;
    } else if (PeakEnergy < 50) {
      EnergyMin = m_InitialSpectrum->GetBinContent(m_InitialSpectrum->FindBin(BackgroundMin));
      if (EnergyMin > 1) {
        LowEnd = BackgroundMin;
      }
      else if (EnergyMin < 1) {
        for (double i = BackgroundMin; i < PeakEnergy; i++){
          double set = m_InitialSpectrum->GetBinContent(m_InitialSpectrum->FindBin(i));
          if (set < 1) {
            continue;
          }
          else if (set > 1) {
            LowEnd = i;
            break;
          }
        }
      }
    }
    
          
    // check this           
    double EnergyMax = m_InitialSpectrum->GetMaximumBin();
    if (BackgroundMax < EnergyMax) {
      HighEnd = BackgroundMax;
    } else if (BackgroundMax > EnergyMax) {
      HighEnd = EnergyMax;
    }
    
    // set clipping window for bkgrd, roughly 2*FWHM
    //int Bkgrdsigma = (2.355*PeakOneSigmaWidth);
    
    BkgrdHist->GetXaxis()->SetRange(BkgrdHist->GetXaxis()->FindBin(LowEnd), BkgrdHist->GetXaxis()->FindBin(HighEnd));
    
    TH1* BackgroundHist = PeakFinder->Background(BkgrdHist, 20, "BackSmoothing3 BackOrder8 Compton"); //Bkgrdsigma, "Compton" "R" "+");
    BackgroundHist->GetXaxis()->SetRange(BackgroundHist->GetXaxis()->FindBin(BackgroundMin), BackgroundHist->GetXaxis()->FindBin(BackgroundMax));
    
    // Fit to compare to estimates sigma:
    for (int b = 1; b <= FittingHist->GetXaxis()->GetNbins(); ++b) {
      FittingHist->SetBinContent(b, m_InitialSpectrum->GetBinContent(b) - BackgroundHist->GetBinContent(b));
    }
    TF1* GaussFit = new TF1("GaussPeakFitter", Gauss2, PeakEnergy - 4*PeakOneSigmaWidth, PeakEnergy + 4*PeakOneSigmaWidth, 4);
    GaussFit->SetParameters(10, 1000, PeakEnergy, PeakOneSigmaWidth);
    GaussFit->SetParLimits(3, 0.1*PeakOneSigmaWidth, 10*PeakOneSigmaWidth);
    FittingHist->Fit(GaussFit, "RNQ");
    //TCanvas* FC = new TCanvas();
    //FC->cd();
    //FittingHist->DrawCopy();
    //FC->Update();
    double FittedPeakOneSigmaWidth = GaussFit->GetParameter(3);
    delete GaussFit;

    // Elimiate all peak which have at least a factor of 2 divation between measuremenets and expectations
    double Acceptance = 1.75;
    if (FittedPeakOneSigmaWidth < PeakOneSigmaWidth/Acceptance || FittedPeakOneSigmaWidth > Acceptance*PeakOneSigmaWidth) {
      //cout<<"Eliminating peak "<<PeakEnergy<<" due to sigma difference: "<<FittedPeakOneSigmaWidth<<" vs. "<<PeakOneSigmaWidth<<endl;
      continue;
    } else {
      //cout<<"Passing peak "<<PeakEnergy<<" due to sigma difference: "<<FittedPeakOneSigmaWidth<<" vs. "<<PeakOneSigmaWidth<<endl;
    }

    if (m_IsBatch == false) {
      PeakCanvas->cd(2);
      BkgrdHist->GetXaxis()->SetRange(LowEnd, HighEnd);
      BkgrdHist->Draw("SAME");  //zooms in for each bkgrd estimate, but only shows last one at end (does not preserve info)
      //PeakHist->Draw("SAME") // does not zoom in for each estimate, but shows all estimates at end (messy)
      //PeakHist->DrawCopy(); //zooms in for each bkgrd estimate, but only shows last one at end (does not preserve info)
      //PeakHist->DrawCopy("SAME"); //zooms in for each bkgrd estimate, but only shows last one at end (does not preserve info)
      //BackgroundHist->SetFillColor(10);
      BackgroundHist->Draw("SAME");
      PeakCanvas->Modified();
      PeakCanvas->Update();
    
      PeakCanvas->cd(1);
      BackgroundHist->SetFillColor(0);
      m_InitialSpectrum->Draw();
      BackgroundHist->DrawCopy("SAME");
      PeakCanvas->Modified();
      PeakCanvas->Update();   
    }

    // Determine all counts
    int PeakShapeMin = PeakHist->GetXaxis()->FindBin(PeakEnergy - 1.4*PeakOneSigmaWidth);
    int PeakShapeMax = PeakHist->GetXaxis()->FindBin(PeakEnergy + 1.4*PeakOneSigmaWidth);

    // Calculate all counts -- make sure we have at least one to prevent div by zero later on
    double IntegralTotalCounts = m_InitialSpectrum->Integral(PeakShapeMin, PeakShapeMax);
    double IntegralBackgroundCounts = BackgroundHist->Integral(PeakShapeMin, PeakShapeMax);
    if (IntegralBackgroundCounts < 1) IntegralBackgroundCounts = 1;
    double IntegralSignalCounts = IntegralTotalCounts - IntegralBackgroundCounts;
    if (IntegralSignalCounts < 1) IntegralSignalCounts = 1;


    // Eliminations by count and signal-to-noise ratio:

    // Make sure we have enough counts in the peak before accept it:
    if (IntegralSignalCounts < m_PoissonLimit) {
      //cout<<"Eliminating peak "<<PeakEnergy<<" due to statistics limits: "<<IntegralSignalCounts<<" vs. "<<m_PoissonLimit<<endl;
      continue;
    }
    // Make sure we are at least N (= m_SignaltoNoiseRatio) sigma above background
    if (IntegralSignalCounts < m_SignaltoNoiseRatio * sqrt(IntegralBackgroundCounts)) {
      //cout<<"Eliminating peak "<<PeakEnergy<<" due to signal to noise ratio: "<<IntegralSignalCounts/sqrt(IntegralBackgroundCounts)<<" vs. "<<m_SignaltoNoiseRatio<<endl;
      continue;
    }

    // We have an acceptable peak!

    // Draw polymarker in original histogram:
    if (m_IsBatch == false) {
      TPolyMarker* PM = new TPolyMarker(1, &PeakEnergy, &PeakBinContent);
      m_InitialSpectrum->GetListOfFunctions()->Add(PM);
      PM->SetMarkerStyle(23);
      PM->SetMarkerColor(kRed);
      PM->SetMarkerSize(1.3);
    }
      
    // Create the peak information
    MPeak* P = new MPeak();
    P->SetEnergy(PeakEnergy);
    P->SetEnergySigma(PeakOneSigmaWidth);
    P->SetPeakCounts(IntegralSignalCounts);
    P->SetBackgroundCounts(IntegralBackgroundCounts);
    m_Peaks.push_back(P);
  }
  


  // Step 2: isotope matching:

  double Interval = 0.0;
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    // Compare with each line from each isotope in the list:
    for (unsigned int i = 0; i < m_ComparisonIsotopes.size(); ++i) {
      for (unsigned int l = 0; l < m_ComparisonIsotopes[i]->GetNLines(); ++l) {
        // The interval size is defined by ... see below
        Interval = m_EnergyRange*m_Peaks[p]->GetEnergySigma()/sqrt(m_Peaks[p]->GetSignificance());
        // Add one histogram bin size to account for histogramming uncertainty:
        Interval += m_InitialSpectrum->GetBinWidth(m_InitialSpectrum->FindBin(m_Peaks[p]->GetEnergy()));
        if ((m_ComparisonIsotopes[i]->GetLineEnergy(l) > m_Peaks[p]->GetEnergy() - Interval) &&
            (m_ComparisonIsotopes[i]->GetLineEnergy(l) < m_Peaks[p]->GetEnergy() + Interval)) {
          m_Peaks[p]->AddIsotope(m_ComparisonIsotopes[i], l);
          bool Found = false;
          for (unsigned int s = 0; s < m_Isotopes.size(); ++s) {
            if (m_ComparisonIsotopes[i]->GetElement() == m_Isotopes[s]->GetElement() && m_ComparisonIsotopes[i]->GetNucleons() == m_Isotopes[s]->GetNucleons()) {
              Found = true;
              break;
            }
          }
          if (Found == false) {
            m_Isotopes.push_back(m_ComparisonIsotopes[i]);
          }
        }
      }
    }
  }

  if (m_Peaks.size() == 0) {
    cout<<"No peaks found"<<endl;
    return true;
  }
  
  /*
  cout<<"Initial results:"<<endl;
  cout<<endl<<"Peaks: "<<endl;
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    cout<<*m_Peaks[p];  
  }
  cout<<endl<<"Isotopes: "<<endl;
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    cout<<*m_Isotopes[i];  
  }
  */
  
  //
  // Step 3: Cleaning up the isotope list:
  //
  
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    m_Isotopes[i]->SetRating(0);
  }
  
  // Check how many lines are present use the branching ratio as rating:
  double Rating = 0;
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    for (unsigned int i = 0; i < m_Peaks[p]->GetNIsotopes(); ++i) {
      Rating = m_Peaks[p]->GetIsotope(i)->GetLineBranchingRatio(m_Peaks[p]->GetIsotopeLineID(i));
      m_Peaks[p]->GetIsotope(i)->AddRating(Rating);
    }
  }
  // Normalize rating by the sum of the branching ratios
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    Rating = 0;
    for (unsigned int l = 0; l < m_Isotopes[i]->GetNLines(); ++l) {
      Rating += m_Isotopes[i]->GetLineBranchingRatio(l);
    }
    m_Isotopes[i]->SetRating(m_Isotopes[i]->GetRating()/Rating);
    //cout<<"Rating (A): "<<m_Isotopes[i]->GetName()<<": "<<m_Isotopes[i]->GetRating()<<endl;
  }
  // Multiple this by how many lines can be explained:
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    Rating = 0;
    for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
      if (m_Peaks[p]->ContainsIsotope(m_Isotopes[i]) == true) Rating += 1.0;
    }
    Rating /= m_Peaks.size();
    m_Isotopes[i]->SetRating(m_Isotopes[i]->GetRating()*Rating);
    //cout<<"Rating (B): "<<m_Isotopes[i]->GetName()<<": "<<m_Isotopes[i]->GetRating()<<endl;
  }

  // Sort all isotopes by their rating:
  sort(m_Isotopes.begin(), m_Isotopes.end(), SortIsotopesByRating);
  // Remove all isotopes below a rating of 1 as long as still all peaks are found...
  
  for (vector<MIsotope*>::iterator I = m_Isotopes.begin(); I != m_Isotopes.end(); ) {
    MIsotope* ToBeRemoved = (*I);
    bool Remove = true;
    for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
      if (m_Peaks[p]->ContainsIsotope(ToBeRemoved) && m_Peaks[p]->GetNIsotopes() == 1) {
        //cout<<"  Not removing since the follwoing line would have no more elements: "<<m_Peaks[p]->GetEnergy()<<endl;
        Remove = false;
        break;
      }
    }
    if (Remove == false) {
      ++I;
      continue;
    }
    //cout<<"  Removing due to bad ratings: "<<ToBeRemoved->GetName()<<endl;
    for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
      m_Peaks[p]->RemoveIsotope(ToBeRemoved);
    } 
    I = m_Isotopes.erase(I);
  }
  /*
  cout<<"Intermediate results:"<<endl;
  cout<<endl<<"Peaks: "<<endl;
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    cout<<*m_Peaks[p];  
  }
  cout<<endl<<"Isotopes: "<<endl;
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    cout<<*m_Isotopes[i];  
  }
  */

  // Go over all peaks, find the contribution with the lowest branching ratio, and remove it if still all peaks can be found
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    list<MIsotope*> ToBeRemovedList;
    list<MIsotope*>::iterator I;
    list<double> BranchingRatios;
    list<double>::iterator B;
    // Create an ordered list of isotopes per peak according to branching ratios
    for (unsigned int i = 0; i < m_Peaks[p]->GetNIsotopes(); ++i) {
      if (ToBeRemovedList.size() == 0) {
        ToBeRemovedList.push_back(m_Peaks[p]->GetIsotope(i));
        BranchingRatios.push_back(m_Peaks[p]->GetIsotope(i)->GetLineBranchingRatio(m_Peaks[p]->GetIsotopeLineID(i)));
      } else {
        bool Added = false;
        double Ratio = m_Peaks[p]->GetIsotope(i)->GetLineBranchingRatio(m_Peaks[p]->GetIsotopeLineID(i));
        for (I = ToBeRemovedList.begin(), B = BranchingRatios.begin(); I != ToBeRemovedList.end(); ++I, ++B) {
          if (Ratio < (*B)) {
            ToBeRemovedList.insert(I, m_Peaks[p]->GetIsotope(i));
            BranchingRatios.insert(B, Ratio);
            Added = true;
            break;
          }
        }
        if (Added == false) {
          ToBeRemovedList.push_back(m_Peaks[p]->GetIsotope(i));
          BranchingRatios.push_back(Ratio);
        }
      }
    }
    //cout<<"Ordered list for "<<m_Peaks[p]->GetEnergy()<<endl;
    //for (I = ToBeRemovedList.begin(), B = BranchingRatios.begin(); I != ToBeRemovedList.end(); ++I, ++B) {
    //  cout<<"  "<<(*I)->GetElement()<<" with "<<(*B)<<endl;
    //}
    
    // Eliminate the isotopes with worst branching ratio:
    while (true) {
      if (ToBeRemovedList.size() < 2) break;
      MIsotope* ToBeRemoved = ToBeRemovedList.front();
      for (unsigned int p2 = 0; p2 < m_Peaks.size(); ++p2) {
        if (m_Peaks[p2]->ContainsIsotope(ToBeRemoved) && m_Peaks[p2]->GetNIsotopes() == 1) break;
      }
      //cout<<"Removing: "<<*(ToBeRemovedList.front())<<endl;
      m_Peaks[p]->RemoveIsotope(ToBeRemovedList.front());
      // If the isotope does not longer appear in any peak, then erase it too:
      bool Found = false;
      for (unsigned int p2 = 0; p2 < m_Peaks.size(); ++p2) {
        if (m_Peaks[p2]->ContainsIsotope(ToBeRemoved) == true) {
          Found = true;
          break;
        }
      }
      if (Found == false) {
        for (vector<MIsotope*>::iterator I = m_Isotopes.begin(); I != m_Isotopes.end(); ++I) {
          if ((*I) == ToBeRemoved) {
            m_Isotopes.erase(I);
            break;
          }
        }
      }
      ToBeRemovedList.erase(ToBeRemovedList.begin());
    }
  }
  
  // Eliminate Compton edges

  
  //
  // Step 4: Dump the results:
  //
  
  for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
    for (unsigned int i = 0; i < m_Peaks[p]->GetNIsotopes(); ++i) {
      m_Peaks[p]->GetIsotope(i)->SetLineFound(m_Peaks[p]->GetIsotopeLineID(i), true);
    }
  }

  //cout<<endl<<"Peaks: "<<endl;
  //for (unsigned int p = 0; p < m_Peaks.size(); ++p) {
  // cout<<*m_Peaks[p];  
  //}
  cout<<endl<<"Isotopes: "<<endl;
  for (unsigned int i = 0; i < m_Isotopes.size(); ++i) {
    cout<<*m_Isotopes[i];  
  }
  
  return true;
}

// MSpectralAnalyzer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
