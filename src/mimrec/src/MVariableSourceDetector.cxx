/*
 * MVariableSourceDetector.cxx
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
#include "MVariableSourceDetector.h"

// Standard libs:
#include <algorithm>

// ROOT libs:

// MEGAlib libs:
#include "MInterface.h"
#include "MImage.h"
#include "MImagerExternallyManaged.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MVariableSourceDetector)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MVariableSourceDetector::MVariableSourceDetector(MSettingsImaging* ImagingSettings, MSettingsEventSelections* EventSelectionSettings, MDGeometryQuest* Geometry)
{
  SetImagingSettings(ImagingSettings);
  SetEventSelectionSettings(EventSelectionSettings);
  m_Geometry = Geometry;
  
  m_Imager = nullptr;
  
  m_IntegrationTimes.push_back(15);
  m_TriggerThresholds.push_back(60);
  
  int NBins = 100;
  double* Axis = MInterface::CreateAxisBins(1, 10000, NBins, true);
  for (unsigned int t = 0; t < m_IntegrationTimes.size(); ++t) {
    m_RelativePeakHeights.push_back(new TH1D("", "", NBins, Axis));
    m_RelativePeakHeights.back()->SetXTitle("Maximum peak-height in image [a.u.]");
    m_RelativePeakHeights.back()->SetYTitle("Number of occurances");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MVariableSourceDetector::~MVariableSourceDetector()
{
  //
}


////////////////////////////////////////////////////////////////////////////////


//! Create the imager
void MVariableSourceDetector::CreateImager()
{
  if (m_Imager != nullptr) delete m_Imager;
  
  m_Imager = new MImagerExternallyManaged(m_ImagingSettings->GetCoordinateSystem());

  m_Imager->SetGeometry(m_Geometry);
  m_Imager->UseGUI(false);  

  m_Imager->SetImagingSettings(m_ImagingSettings);
  m_Imager->SetEventSelectionSettings(m_EventSelectionSettings);
  
  m_Imager->Initialize();
}


////////////////////////////////////////////////////////////////////////////////


void MVariableSourceDetector::BackprojectEvent(MPhysicalEvent* Event)
{
  //! Do the backprojection

  if (m_Imager == nullptr) CreateImager();
  
  MBPData* BP = m_Imager->CalculateResponseSlice(Event);
  
  if (BP != nullptr) {
    m_Backprojections.push_back(BP);
    m_Times.push_back(Event->GetTime());
    m_Own.push_back(true);
    m_Events.push_back(Event);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MVariableSourceDetector::AddBackprojection(MBPData* BP, MTime Time, bool Own)
{
  //! Add backprojection

  m_Backprojections.push_back(BP);
  m_Times.push_back(Time);
  m_Own.push_back(Own);
}
  

////////////////////////////////////////////////////////////////////////////////


void MVariableSourceDetector::Trim(MTime Time)
{
  //! Trim data, i.e. remove data older than the given time

  while (m_Times.back() < Time) {
    bool Own = m_Own.front();
    m_Own.pop_front();
    if (Own == true) delete m_Backprojections.front();
    m_Backprojections.pop_front();
    m_Times.pop_front();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Save the peak heights
bool MVariableSourceDetector::ReadPeakHeightHistory(MString FileName)
{
  vector<double> BinEdges;
  
  m_IntegrationTimes.clear();
  m_RelativePeakHeights.clear();

  ifstream in;
  in.open(FileName);
  
  MString Line;
  while (in.good()) {
    Line.ReadLine(in); 
    if (Line.BeginsWith("Times")) {
      MTokenizer T;
      T.Analyze(Line);
      m_IntegrationTimes = T.GetTokenAtAsDoubleVector(1);
    } else if (Line.BeginsWith("BinEdges")) {
      MTokenizer T;
      T.Analyze(Line);
      BinEdges = T.GetTokenAtAsDoubleVector(1);      
    } else if (Line.BeginsWith("Histogram")) {
      TH1D* Hist = new TH1D("", "", BinEdges.size()-1, &BinEdges[0]);
      Hist->SetXTitle("Peak-height in image [a.u.]");
      Hist->SetYTitle("Occurances per bin");
      
      MTokenizer T;
      T.Analyze(Line);
      vector<double> Content = T.GetTokenAtAsDoubleVector(1); 
      
      for (unsigned int i = 0; i < Content.size(); ++i) {
        Hist->SetBinContent(i+1, Content[i]);
      }
      
      m_RelativePeakHeights.push_back(Hist);
    }
  }
 
  in.close();
  
  for (unsigned int i = 0; i < m_RelativePeakHeights.size(); ++i) {
    TCanvas* C = new TCanvas();
    C->cd();
    C->SetLogx();
    C->SetLogy();
    m_RelativePeakHeights[i]->Draw();
    C->Update();
  }
  
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Save the peak heights
bool MVariableSourceDetector::SavePeakHeightHistory(MString FileName)
{
  ofstream out;
  out.open(FileName);
  
  out<<"# Relative peak height history"<<endl;
  out<<endl;
  out<<"Times ";
  for (unsigned int t = 0; t < m_IntegrationTimes.size(); ++t) {
    out<<m_IntegrationTimes[t]<<" ";
  }  
  out<<endl;
  out<<"BinEdges ";
  for (int b = 1; b <= m_RelativePeakHeights[0]->GetXaxis()->GetNbins(); ++b) {
    out<<m_RelativePeakHeights[0]->GetXaxis()->GetBinLowEdge(b)<<" ";
  }
  out<<m_RelativePeakHeights[0]->GetXaxis()->GetBinUpEdge(m_RelativePeakHeights[0]->GetXaxis()->GetNbins())<<endl;
  
  for (unsigned int h = 0; h < m_RelativePeakHeights.size(); ++h) {
    out<<"Histogram ";
    for (int b = 1; b <= m_RelativePeakHeights[h]->GetXaxis()->GetNbins(); ++b) {
      out<<m_RelativePeakHeights[h]->GetBinContent(b)<<" ";
    }
    out<<endl;
  }
  
  out.close();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MVariableSourceDetector::IntensiveSearch(MTime& Start, MTime& Stop) 
{
  MImagerExternallyManaged Imager(m_ImagingSettings->GetCoordinateSystem());
  Imager.SetGeometry(m_Geometry);
  Imager.SetImagingSettings(m_ImagingSettings);
  Imager.SetEventSelectionSettings(m_EventSelectionSettings);
  Imager.UseGUI(false);
  Imager.Initialize();
  
  // Determine the peak to average in the given time interval
  MTime S = Start;
  deque<MTime>::iterator TimeIter = upper_bound(m_Times.begin(), m_Times.end(), S);
  deque<MBPData*>::iterator BPIter = m_Backprojections.begin() + (TimeIter - m_Times.begin());
  
  vector<MBPData*> BPs;
  while ((*TimeIter) < Stop) {
    BPs.push_back(*BPIter);
    ++BPIter;
    ++TimeIter;
       
    if (TimeIter == m_Times.end()) break;
  }
  vector<MImage*> Images = Imager.Deconvolve(BPs);  
  
  double Max;
  vector<double> Coordinates;
  double Average = Images.back()->GetAverage();
  Images.back()->DetermineMaximum(Max, Coordinates);
  double Peak = Max/Average;
  
  ostringstream out;
  out<<Start<<" - "<<Stop<<":  max="<<Max<<"  avg="<<Average<<"   peak="<<Peak;
  
  Images.back()->SetTitle(out.str());
  cout<<out.str()<<endl;

  cout<<"New title: "<<Images.back()->GetTitle()<<endl;

  
  // Now move forward in time and see if we can increase it:
  double Advance = 5.0;
  while (true) {
    MTime NewStop = Stop + Advance;
    vector<MBPData*> NewBPs = BPs;
    while ((*TimeIter) < NewStop) {
      NewBPs.push_back(*BPIter);
      ++BPIter;
      ++TimeIter;
     
      if (TimeIter == m_Times.end()) break;
    }
    vector<MImage*> NewImages = Imager.Deconvolve(NewBPs);
    
    double NewMax;
    vector<double> NewCoordinates;
    double NewAverage = NewImages.back()->GetAverage();
    NewImages.back()->DetermineMaximum(NewMax, NewCoordinates);
    double NewPeak = NewMax/NewAverage;
    
    ostringstream out2;
    out2<<Start<<" - "<<NewStop<<":  max="<<NewMax<<"  avg="<<NewAverage<<"   peak="<<NewPeak<<endl;
    NewImages.back()->SetTitle(out2.str());
    cout<<out2.str()<<endl;
  
    cout<<"New title ("<<(long) NewImages.back()<<"): "<<NewImages.back()->GetTitle()<<endl;
    
    if (NewPeak > Peak) {
      Peak = NewPeak;
      Max = NewMax;
      Average = NewAverage;
      Stop = NewStop;
      BPs = NewBPs;
    } else {
      break; 
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MVariableSourceLocation> MVariableSourceDetector::Search(bool Learn)
{
  //! Search for sources and return their locations

  cout<<"Starting search: "<<m_Backprojections.size()<<":"<<m_Times.size()<<endl;
  
  // Split the data in 2-sec intervals form the front
  unsigned int Index = 0;
  unsigned int Start = 0;
  unsigned int MaxIndex = m_Times.size();
  
  // Create the time slices
  MTime Front = m_Times.front();
  MTime Back = m_Times.back();
  

  
  for (unsigned int t = 0; t < m_IntegrationTimes.size(); ++t) { 
    cout<<"Time intervall: "<<Front<<" -> "<<Back<<endl;
    
    vector<MTime> Stops;
    Stops.push_back(Front);
    while (Stops.back() < Back) Stops.push_back(Stops.back() + m_IntegrationTimes[t]);
    for (unsigned int s = 1; s < Stops.size(); ++s) { // Skip partially filled last one

      // Get a new imager
      MImagerExternallyManaged Imager(m_ImagingSettings->GetCoordinateSystem());
      Imager.SetGeometry(m_Geometry);
      Imager.SetImagingSettings(m_ImagingSettings);
      Imager.SetEventSelectionSettings(m_EventSelectionSettings);
      Imager.UseGUI(false);
      Imager.Initialize();
      
      vector<MBPData*> BPs;
      Start = Index;
      while (Index < MaxIndex && m_Times[Index] < Stops[s]) {
        if (m_Times[Index] < Stops[s-1]) {
          ++Index;
          continue; // Protect from time hick-ups
        }
        BPs.push_back(m_Backprojections[Index]);
        ++Index;
      }
      cout<<"Range: "<<m_Times[Start]<<":"<<m_Times[Index-1]<<endl;
      
      vector<MImage*> ReconImages = Imager.Deconvolve(BPs);
      
      double Max;
      vector<double> Coordinates;
      //double Average = ReconImages.back()->GetAverage();
      ReconImages.back()->DetermineMaximum(Max, Coordinates);
      
      double Peak = Max;
      m_RelativePeakHeights[t]->Fill(Peak);
      
      if (Peak > m_TriggerThresholds[t]) {
        cout<<"Peak found at "<<Stops[s-1]<<"   to "<<Stops[s]<<" with: "<<Peak<<" - # BPs: "<<BPs.size()<<" (avg: "<<double(MaxIndex)/Stops.size()<<")"<<endl;
        TCanvas* I = new TCanvas();
        ReconImages.back()->Display(I);
        I->SaveAs(MString("Found") + MString(Stops[s-1].GetAsSeconds()) + ".png");

        /*
        unsigned int i = Start;
        while (i < Index) {
          cout<<m_Events[i]->ToString()<<endl;
          ++i;
        }
        */
        /*
        
        MTime GRBStart = Stops[s-1];
        MTime GRBStop = Stops[s];
        
        
        
        IntensiveSearch(GRBStart, GRBStop);
        
        // New Image:
        MImagerExternallyManaged Imager(m_ImagingSettings->GetCoordinateSystem());
        Imager.SetGeometry(m_Geometry);
        Imager.SetImagingSettings(m_ImagingSettings);
        Imager.SetEventSelectionSettings(m_EventSelectionSettings);
        Imager.UseGUI(false);
        Imager.Initialize();        
        
        // Find first time larger than GRBStart
        deque<MTime>::iterator GRBTimeIter = upper_bound(m_Times.begin(), m_Times.end(), GRBStart);
        deque<MBPData*>::iterator GRBBPIter = m_Backprojections.begin() + (GRBTimeIter - m_Times.begin());
        
        vector<MBPData*> GRBBPs;
        while ((*GRBTimeIter) < GRBStop) {
          GRBBPs.push_back(*GRBBPIter);
          ++GRBBPIter;
          ++GRBTimeIter;
        
          if (GRBTimeIter == m_Times.end()) break;
        }
        vector<MImage*> GRBImages = Imager.Deconvolve(GRBBPs);
        
        TCanvas* C = new TCanvas();
        GRBImages.back()->Display(C);
        */
      }
    }
  }
  
  for (unsigned int h = 0; h < m_RelativePeakHeights.size(); ++h) {
    TCanvas* C = new TCanvas();
    C->cd();
    C->SetLogx();
    C->SetLogy();
    m_RelativePeakHeights[h]->Draw();
    C->Update();
  }
  
  SavePeakHeightHistory("RelativePeakHeightHistory.txt");
  
  
  vector<MVariableSourceLocation> Locations;
  
  
  
  
  cout<<"Done! Found "<<Locations.size()<<" source locations "<<endl;
  
  return Locations;
}


// MVariableSourceDetector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
