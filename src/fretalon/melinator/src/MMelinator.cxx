/*
 * MMelinator.cxx
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
// MMelinator
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MMelinator.h"

// Standard libs:
#include <algorithm>
#include <new>
#include <map>
using namespace std;

// ROOT libs:
#include "TBox.h"
#include "TLine.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TGaxis.h"
#include "TGraphErrors.h"

// MEGAlib libs:
#include "MFile.h"
#include "MExceptions.h"
#include "MStreams.h"
#include "MGUIProgressBar.h"
#include "MGUIMultiProgressBar.h"
#include "MFileReadOuts.h"
#include "MReadOutSequence.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutDataADCValue.h"
#include "MCalibrateLines.h"
#include "MCalibrationSpectralPoint.h"
#include "MCalibrationSpectrum.h"
#include "MBinner.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MReadOutDataInterfaceADCValue.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MMelinator::MMelinator()
{
  m_HistogramMin = 0;
  m_HistogramMax = 1000;
  m_HistogramBinningMode = c_HistogramBinningModeFixedNumberOfBins;
  m_HistogramBinningModeValue = 100;

  m_PeakParametrizationMethod = MCalibrateLines::c_PeakParametrizationMethodBayesianBlockPeak;
  m_PeakParametrizationMethodFittedPeakBackgroundModel = MCalibrationFit::c_BackgroundModelLinear;
  m_PeakParametrizationMethodFittedPeakEnergyLossModel = MCalibrationFit::c_EnergyLossModelNone;
  m_PeakParametrizationMethodFittedPeakPeakShapeModel = MCalibrationFit::c_PeakShapeModelGaussian;
  
  m_CalibrationModelDeterminationMethod = MCalibrateLines::c_CalibrationModelStepWise;
  
  m_NThreads = 4;
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MMelinator::~MMelinator()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clear the data
void MMelinator::Clear()
{
  m_Store.Clear();
  m_GroupIDs.clear();
  m_CalibrationFileNames.clear();
  m_Isotopes.clear();
  
  m_SelectedDetectorID = -1;
}


////////////////////////////////////////////////////////////////////////////////


//! Load the calibration data containing the given isotopes - return false if an error occurred
//! This function performs parallel loading of all given files
bool MMelinator::Load(const vector<MString>& FileNames, const vector<vector<MIsotope> >& Isotopes, const vector<unsigned int>& GroupIDs)
{
  int NFiles = 0;
  for (unsigned int f = 0; f < FileNames.size(); ++f) {
    if (FileNames[f] == "") continue;
    ++NFiles;
  }

    
  MGUIMultiProgressBar ProgressBar(NFiles);
  ProgressBar.SetTitles("Melinator Progress", "Progress of reading the calibration files");
  
  // Check for consistency and file size
  if (FileNames.size() == 0) return false;
  if (FileNames.size() != Isotopes.size()) return false;
  m_CalibrationFileNames.clear();
  for (unsigned int f = 0; f < FileNames.size(); ++f) {
    if (FileNames[f] == "") continue;
    
    MFileReadOuts Reader;
    if (Reader.Open(FileNames[f]) == false) {
      return false;
    }
    ProgressBar.SetTitle(f, MFile::GetBaseName(FileNames[f]));
    ProgressBar.SetMinMax(f, 0.0, Reader.GetFileLength());
    Reader.Close();
    m_CalibrationFileNames.push_back(FileNames[f]);
  }
  ProgressBar.SetMinimumChange(0.2*FileNames.size());
  ProgressBar.Create();
  gSystem->ProcessEvents();
  
  //m_CalibrationFileNames = FileNames;
  //m_Isotopes = Isotopes;
  
  //! Map the external group IDs to internal IDs starting at 0 and combined the isotopes of the individial groups
  m_GroupIDs.clear();
  unsigned int InternalID = 0;
  map<unsigned int, unsigned int> IDMap;
  map<unsigned int, MString> NameMap;
  map<unsigned int, vector<MIsotope>> IsotopeMap;
  for (unsigned int g = 0; g < GroupIDs.size(); ++g) {
    if (FileNames[g] == "") continue;
    
    map<unsigned int, unsigned int>::iterator I = IDMap.find(GroupIDs[g]);
    if (I == IDMap.end()) {
      IDMap[GroupIDs[g]] = InternalID;
      InternalID++;
    } 
    // Add, sort and remove duplicate isotopes
    IsotopeMap[GroupIDs[g]].insert(IsotopeMap[GroupIDs[g]].end(), Isotopes[g].begin(), Isotopes[g].end());
    sort(IsotopeMap[GroupIDs[g]].begin(), IsotopeMap[GroupIDs[g]].end());
    IsotopeMap[GroupIDs[g]].erase(unique(IsotopeMap[GroupIDs[g]].begin(), IsotopeMap[GroupIDs[g]].end()), IsotopeMap[GroupIDs[g]].end());
    
    m_GroupIDs.push_back(IDMap[GroupIDs[g]]);
    MString Name = MFile::GetBaseName(FileNames[g]);
    if (NameMap[GroupIDs[g]] == "") {
      NameMap[GroupIDs[g]] += Name;
    } else {
      NameMap[GroupIDs[g]] += " & " + Name;
    }
  }
  
  /*
  cout<<"Group ID mapping: "<<endl;
  for (unsigned int g = 0; g < m_GroupIDs.size(); ++g) {
    cout<<GroupIDs[g]<<" --> "<<m_GroupIDs[g]<<endl; 
  }
  cout<<"Group ID to name mapping: "<<endl;
  for (auto N: IDMap) {
    cout<<N.first<<" --> "<<N.second<<" --> "<<NameMap[N.first]<<endl; 
  }
  cout<<"Group ID to isotope mapping: "<<endl;
  for (auto N: IsotopeMap) {
    cout<<N.first<<" --> ";
    for (auto I: N.second) {
      cout<<I.GetName()<<" "; 
    }
    cout<<endl;
  }
  */
  
  //! Set up the read-out data groups in the store
  for (auto N: NameMap) {
    unsigned int GroupID = m_Store.AddReadOutDataGroup(N.second);
    if (GroupID != IDMap[N.first]) {
      cout<<"ERROR: Something went wrong with the group ID creation!"<<endl;
      return false;
    }
  }
  
  //! Now make sure the groups have all the same isotopes
  m_Isotopes.clear();
  for (unsigned int g = 0; g < GroupIDs.size(); ++g) {
    m_Isotopes.push_back(IsotopeMap[GroupIDs[g]]); 
  }
  /*
  cout<<"Isotopes: "<<endl;
  for (auto V: m_Isotopes) {
    cout<<" --> ";
    for (auto I: V) {
      cout<<I.GetName()<<" "; 
    }
    cout<<endl;
  }
  */
  
  
  m_CalibrationFileLoadingProgress.resize(NFiles);
  for (unsigned int c = 0; c < m_CalibrationFileLoadingProgress.size(); ++c) {
    m_CalibrationFileLoadingProgress[c] = 0.0;
  }
  
  unsigned int NThreads = m_NThreads;
  NThreads = m_CalibrationFileNames.size();  
  //if (NThreads < 1) NThreads = 1;
  m_ThreadNextItem = 0;
  m_Threads.resize(NThreads);
  m_ThreadIsInitialized.resize(NThreads);
  m_ThreadShouldTerminate.resize(NThreads);
  m_ThreadIsFinished.resize(NThreads);
  
  if (NThreads > 0) {
    
    // Start threads
    for (unsigned int t = 0; t < NThreads; ++t) {
      TString Name = "Loading thread #";
      Name += t;
      //cout<<"Creating thread: "<<Name<<endl;
      TThread* Thread = new TThread(Name, (void(*) (void *)) &MMelinatorCallParallelLoadingThread, (void*) new MMelinatorThreadCaller(this, t));
      m_Threads[t] = Thread;
      m_ThreadIsInitialized[t] = false;
      m_ThreadShouldTerminate[t] = false;
      m_ThreadIsFinished[t] = false;

      Thread->Run();
      
      // Wait until thread is initialized:
      while (m_ThreadIsInitialized[t] == false && m_ThreadIsFinished[t] == false) {
        // Sleep for a while...
        TThread::Sleep(0, 10000000);
      }    

      //cout<<Name<<" is running"<<endl;
    }
    
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a while...
      TThread::Sleep(0, 10000000);
      //gSystem->ProcessEvents();
      
      int Running = 0;
      ThreadsAreRunning = false;
      for (unsigned int t = 0; t < NThreads; ++t) {
        if (m_ThreadIsFinished[t] == false) {
          ThreadsAreRunning = true;
          Running++;
        }
      }
      
      // Update the progress bar:
      for (unsigned int c = 0; c < m_CalibrationFileLoadingProgress.size(); ++c) {
        ProgressBar.SetValue(c, m_CalibrationFileLoadingProgress[c]);
      }
      ProgressBar.SetMinimumChange(0.1 + Running*0.2);
      if (ProgressBar.HasBeenCancelled() == true) {
        for (unsigned int t = 0; t < NThreads; ++t) {
          m_ThreadShouldTerminate[t] = true;
        }
      }
    }

    // None of the threads are running any more --- kill them
    for (unsigned int t = 0; t < NThreads; ++t) {
      m_Threads[t]->Kill();
      m_Threads[t] = 0;
    }
    
    //cout<<"All threads have finished"<<endl;
  }
  // Non-threaded mode
  else {
    LoadParallel(0);
  }
  
    
  // Let's ensure that both store have the same read-out elements at the same positions
  m_CalibrationStore.Clear();
  for (unsigned int c = 0; c < m_Store.GetNumberOfReadOutCollections(); ++c) {
    m_CalibrationStore.Add(m_Store.GetReadOutCollection(c).GetReadOutElement());
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Thread entry point for the parallel calibration
void MMelinatorCallParallelLoadingThread(void* Address)
{
  MMelinator* M = ((MMelinatorThreadCaller*) Address)->GetThreadCaller();
  M->LoadParallel(((MMelinatorThreadCaller*) Address)->GetThreadID());
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration of the given collection
bool MMelinator::LoadParallel(unsigned int ThreadID)
{
  cout<<"Parallel loading thread #"<<ThreadID<<" has started"<<endl;
  
  m_ThreadIsInitialized[ThreadID] = true;
  
  while (true) {
    TThread::Lock();
    unsigned int ID = m_ThreadNextItem;
    ++m_ThreadNextItem;
    TThread::UnLock();
    
    if (ID >= m_CalibrationFileNames.size()) break;
    
    MFileReadOuts Reader;
    if (Reader.Open(m_CalibrationFileNames[ID]) == false) {
      return false;
    }
    
    unsigned int GroupID = m_GroupIDs[ID];
    
    // Cannot show progress this way 
    // Reader.ShowProgress();
    // Reader.SetProgressTitle("Melinator: Loading", MString("Loading ") + MFile::GetBaseName(m_CalibrationFileNames[ID]) + MString(" ..."));

    MReadOutStore Store;
    Store.AddReadOutDataGroup("Dummy");
    
    MReadOutSequence Sequence;
    long Counter = 0;
    long NewCounter = 0;
    while (Reader.ReadNext(Sequence, m_SelectedDetectorID) == true) {
      Store.Add(Sequence, 0);
        
      ++NewCounter;
      if (++Counter%10000 == 0) {
        double Pos = Reader.GetFilePosition();
        if (Pos > 0) {
          m_CalibrationFileLoadingProgress[ID] = Pos; 
        }
        if (m_ThreadShouldTerminate[ThreadID] == true) break;
        
        // Check that we still have enough memory left:
        unsigned long Reserve = 32*(NewCounter+2*100000) + 300000*m_Store.GetNumberOfReadOutCollections();
        int* Memory = new(nothrow) int[Reserve];
        if (Memory == 0) {
          cout<<"Cannot reserve "<<sizeof(int)*Reserve<<" bytes --> Close to out of memory... Stopping to read more events..."<<endl;
          break;
        } else {
          delete [] Memory;
        }
      
        //cout<<GroupID<<": "<<Counter<<endl;
        if (Counter == 10000 || Counter % 100000 == 0) { 
          if (TThread::TryLock() == 0) {
            m_Store.Move(Store, GroupID);
            TThread::UnLock();
            NewCounter = 0;
          }
       }
      }
    }
    Reader.Close();
    
    TThread::Lock();
    m_Store.Move(Store, GroupID);
    TThread::UnLock();
    
    m_CalibrationFileLoadingProgress[ID] = numeric_limits<double>::max(); 
  }

  m_ThreadIsFinished[ThreadID] = true;

  cout<<"Parallel loading thread #"<<ThreadID<<" has finished"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Draw the spectrum of all read-out data groups in the collection into the given canvas 
void MMelinator::DrawSpectrum(TCanvas& Canvas, unsigned int Collection, unsigned int Line)
{
  MReadOutCollection& C = GetCollection(Collection);
  
  //Canvas.SetBit(kNoContextMenu);
  Canvas.SetBit(kCannotPick);
  Canvas.Clear();
  Canvas.cd();
  Canvas.SetGridx();
  Canvas.SetGridy();
  //Canvas.SetLogy();

  Canvas.SetLeftMargin(0.08);
  Canvas.SetRightMargin(0.05);
  Canvas.SetTopMargin(0.05);
  Canvas.SetBottomMargin(0.12);

  
  //! Create the histograms
  vector<TH1D*> Histograms;
  for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
    MReadOutDataGroup& G = C.GetReadOutDataGroup(g);
    TH1D* H = CreateSpectrum(G.GetName(), G, m_HistogramMin, m_HistogramMax, m_HistogramBinningMode, m_HistogramBinningModeValue);
    if (H->GetMaximum() > 0) {
      Histograms.push_back(H); //C.GetReadOutElement().ToString(), G));
    } else {
      Histograms.push_back(0);
      delete H;
    }
  }
  
  double Max = 0;
  double Min = numeric_limits<double>::max();
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;
    if (Histograms[h]->GetMaximum() > Max) {
      Max = Histograms[h]->GetMaximum(); 
    }
    if (Histograms[h]->GetMinimum() < Min) {
      Min = Histograms[h]->GetMinimum(); 
    }
  }
  
        
  vector<int> Colors;
  Colors.push_back(kGreen+1);
  Colors.push_back(kRed);
  Colors.push_back(kBlue);
  Colors.push_back(kYellow);
  Colors.push_back(kViolet);
  Colors.push_back(kCyan);
  Colors.push_back(kMagenta);
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;
    Histograms[h]->SetMaximum(1.1*Max);
    Histograms[h]->SetMinimum(0.9*Min);
    if (h < Colors.size()) {
      Histograms[h]->SetLineColor(Colors[h]+1);
    } else {
      Histograms[h]->SetLineColor(kBlack);
    }
    Histograms[h]->SetLineWidth(2);

    //Histograms[h]->GetXaxis()->SetLabelOffset(0.0);
    Histograms[h]->GetXaxis()->SetLabelSize(0.05);
    Histograms[h]->GetXaxis()->SetTitleSize(0.06);
    Histograms[h]->GetXaxis()->SetTitleOffset(0.9);
    Histograms[h]->GetXaxis()->CenterTitle(true);
    Histograms[h]->GetXaxis()->SetMoreLogLabels(true);
    
    //Histograms[h]->GetYaxis()->SetLabelOffset(0.001);
    Histograms[h]->GetYaxis()->SetLabelSize(0.05);
    Histograms[h]->GetYaxis()->SetTitleSize(0.06);
    Histograms[h]->GetYaxis()->SetTitleOffset(0.6);
    Histograms[h]->GetYaxis()->CenterTitle(true);
  }
  
  vector<int> ColorOffsets;
  ColorOffsets.push_back(-4);
  ColorOffsets.push_back(-7);
  ColorOffsets.push_back(-9);
  ColorOffsets.push_back(-10);
  
  unsigned int LineID = 0;
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;

    if (h == 0) {
      Histograms[h]->Draw();
    } else {
      Histograms[h]->Draw("SAME");
    }

    //! Draw the line-fit regions:
    unsigned int CalibrationIndex = m_CalibrationStore.FindCalibration(C.GetReadOutElement());
    if (CalibrationIndex != g_UnsignedIntNotDefined) {
      MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(CalibrationIndex)));
      if (C != nullptr) {
        for (unsigned int p = 0; p < C->GetNumberOfSpectralPoints(h); ++p) {
          MCalibrationSpectralPoint P = C->GetSpectralPoint(h, p);
          TBox* Box = new TBox(P.GetLowEdge(), 0.0, P.GetHighEdge(), Histograms[h]->GetMaximum());
          if (p < ColorOffsets.size()) {
            Box->SetFillColor(Colors[h]+ColorOffsets[p]);
          } else {
            Box->SetFillColor(Colors[h]+ColorOffsets[3]);
          }
          Box->SetFillStyle(3001);
          Box->Draw("SAME");
          cout<<LineID<<":"<<Line<<endl;
          if (LineID == Line) {
            TMarker* Marker = new TMarker(P.GetPeak(), 0.9*Histograms[h]->GetMaximum(), 20);
            Marker->Draw("SAME");
          }
          ++LineID;
        }
      }
    } else {
      cout<<"No calibration found!"<<endl;
    }
  }
      
  double ySizeMin = 0.84 - 0.075*Histograms.size();
  TLegend* Legend = new TLegend(0.8, ySizeMin, 0.94, 0.94, NULL, "brNDC");
  Legend->SetHeader("Isotope list:");
  
  // Final draw so that everything is on top of each other:
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;
    Histograms[h]->Draw("SAME");
    MString Names;
    unsigned int IsotopeIndex = distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), h));
    for (unsigned int i = 0; i < m_Isotopes[IsotopeIndex].size(); ++i) {
      Names += "^{";
      Names += m_Isotopes[IsotopeIndex][i].GetNucleons();
      Names += "}";
      Names += m_Isotopes[IsotopeIndex][i].GetElement();
      if (i < m_Isotopes[IsotopeIndex].size()-1) {
        Names += ", ";
      }
    }
    Legend->AddEntry(Histograms[h], Names);
  }
  Legend->Draw("SAME");
  
  // Draw the axis of the first histogram again:
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;
    Histograms[h]->Draw("AXIS SAME");
    break;
  }

  Canvas.Update();
}


////////////////////////////////////////////////////////////////////////////////

    
//! Return true if we have calibration model
bool MMelinator::HasCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    return C->HasModel();
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////

    
//! Get the calibration model of the spectra
MCalibrationModel& MMelinator::GetCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    if (C->HasModel()) {
      return C->GetModel();
    }
  }
  
  throw MExceptionObjectDoesNotExist("No calibration model available");  
  
  return C->GetModel();
}


////////////////////////////////////////////////////////////////////////////////

    
//! Get the number of calibration point in the spectra
unsigned int MMelinator::GetNumberOfCalibrationSpectralPoints(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    unsigned int Points = 0;
    for (unsigned int g = 0; g <C->GetNumberOfReadOutDataGroups(); ++g) {
      Points += C->GetNumberOfSpectralPoints(g);
    }
    return Points;
  }
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////

  
//! Return the given spectral point
//! If it doesn't exist, the exception MExceptionIndexOutOfBounds is thrown
MCalibrationSpectralPoint MMelinator::GetCalibrationSpectralPoint(unsigned int Collection, unsigned int Line) 
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    unsigned int Points = 0;
    for (unsigned int g = 0; g < C->GetNumberOfReadOutDataGroups(); ++g) {
      for (unsigned int p = 0; p < C->GetNumberOfSpectralPoints(g); ++p) {
        if (Points == Line) {
          return C->GetSpectralPoint(g, p);
        } else {
          Points++;
        }
      }
    }
  }
 
  throw MExceptionIndexOutOfBounds(0, GetNumberOfCalibrationSpectralPoints(Collection), Line);  
  
  return MCalibrationSpectralPoint();
}


////////////////////////////////////////////////////////////////////////////////

  
//! Draw the line fit into the Canvas for the given Collection
void MMelinator::DrawLineFit(TCanvas& Canvas, unsigned int Collection, unsigned int Line, 
                             unsigned int HistogramBinningMode, double HistogramBinningModeValue)
{
  MReadOutCollection& Coll = GetCollection(Collection);
  
  //Canvas.SetBit(kNoContextMenu);
  Canvas.SetBit(kCannotPick);
  Canvas.Clear();
  Canvas.cd();
  Canvas.SetGridx();
  Canvas.SetGridy();
  //Canvas.SetLogy();

  Canvas.SetLeftMargin(0.17);
  Canvas.SetRightMargin(0.05);
  Canvas.SetTopMargin(0.05);
  Canvas.SetBottomMargin(0.12);

  unsigned int LineID = 0;
  for (unsigned int g = 0; g < Coll.GetNumberOfReadOutDataGroups(); ++g) {

    //! Draw the line-fit regions:
    unsigned int CalibrationIndex = m_CalibrationStore.FindCalibration(Coll.GetReadOutElement());
    if (CalibrationIndex != g_UnsignedIntNotDefined) {
      MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(CalibrationIndex)));
      if (C != nullptr) {
        for (unsigned int p = 0; p < C->GetNumberOfSpectralPoints(g); ++p) {
          if (LineID == Line) {
            MCalibrationSpectralPoint P = C->GetSpectralPoint(g, p);

            TH1D* Spectrum = CreateSpectrum("", Coll.GetReadOutDataGroup(g), P.GetLowEdge(), P.GetHighEdge(), HistogramBinningMode, HistogramBinningModeValue);
            //Spectrum->SetAxisRange(P.GetLowEdge(), P.GetHighEdge());
            Spectrum->SetMaximum(1.1*Spectrum->GetMaximum());
            
            //Spectrum->GetXaxis()->SetLabelOffset(0.0);
            Spectrum->GetXaxis()->SetLabelSize(0.05);
            Spectrum->GetXaxis()->SetTitleSize(0.06);
            Spectrum->GetXaxis()->SetTitleOffset(0.9);
            Spectrum->GetXaxis()->CenterTitle(true);
            Spectrum->GetXaxis()->SetMoreLogLabels(true);
            Spectrum->GetXaxis()->SetNdivisions(509, true);
    
            //Spectrum->GetYaxis()->SetLabelOffset(0.001);
            Spectrum->GetYaxis()->SetLabelSize(0.05);
            Spectrum->GetYaxis()->SetTitleSize(0.06);
            Spectrum->GetYaxis()->SetTitleOffset(1.4);
            Spectrum->GetYaxis()->CenterTitle(true);
            Spectrum->GetYaxis()->SetNdivisions(509, true);
        
            Spectrum->Draw();
            
            if (P.HasFit() == true) {
              MCalibrationFit F = P.GetFit();
              F.Draw("SAME");
            }
            
            TLine* Line = new TLine(P.GetPeak(), Spectrum->GetMaximum(), P.GetPeak(), Spectrum->GetMinimum());
            Line->SetLineWidth(3);
            Line->Draw("SAME");
          }
          ++LineID;
        }
      }
    }
  }
    
  Canvas.Update();
}


////////////////////////////////////////////////////////////////////////////////


//! Draw the calibration into the Canvas for the given Collection
void MMelinator::DrawCalibration(TCanvas& Canvas, unsigned int Collection)
{
  //Canvas.SetBit(kNoContextMenu);
  //Canvas.Range(0, 1, 0, 1);
  Canvas.SetBit(kCannotPick);
  Canvas.Clear();
  Canvas.cd();
  Canvas.SetGridx();
  Canvas.SetGridy();
  //Canvas.SetLogy();

  Canvas.SetLeftMargin(0.17);
  Canvas.SetRightMargin(0.17);
  Canvas.SetTopMargin(0.05);
  Canvas.SetBottomMargin(0.12);
  
  
  double MaximumEnergy = 0.0; 
  double MaximumPeak = 0.0; 
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  vector<MCalibrationSpectralPoint> Points; 
  TGraphErrors* Graph = new TGraphErrors();
  if (C != nullptr) {
    Points = C->GetUniquePoints();
    Graph->Set(Points.size());
    for (unsigned int p = 0; p < Points.size(); ++p) {
      Graph->SetPoint(p, Points[p].GetPeak(), Points[p].GetEnergy());
      if (Points[p].GetPeak() > MaximumPeak) MaximumPeak = Points[p].GetPeak();
      if (Points[p].GetEnergy() > MaximumEnergy) MaximumEnergy = Points[p].GetEnergy();
    }
  }
  
  
  //cout<<"Drawing graph..."<<endl;
  Graph->Sort();
  Graph->SetMinimum(0);
  Graph->SetMaximum(1.1*MaximumEnergy);
  Graph->Draw(); // Draw in order to have all axis!
  
  Graph->SetTitle("");

  Graph->GetXaxis()->SetTitle("read-out units");
  // Graph->GetXaxis()->SetLabelOffset(0.0);
  Graph->GetXaxis()->SetLabelSize(0.05);
  Graph->GetXaxis()->SetTitleSize(0.06);
  Graph->GetXaxis()->SetTitleOffset(0.9);
  Graph->GetXaxis()->CenterTitle(true);
  Graph->GetXaxis()->SetMoreLogLabels(true);
  Graph->GetXaxis()->SetLimits(0.0, 1.1*MaximumPeak);
  Graph->GetXaxis()->SetNdivisions(509, true);
  
  Graph->GetYaxis()->SetTitle("energy [keV]");
  // Graph->GetYaxis()->SetLabelOffset(0.001);
  Graph->GetYaxis()->SetLabelSize(0.05);
  Graph->GetYaxis()->SetTitleSize(0.06);
  Graph->GetYaxis()->SetTitleOffset(1.4);
  Graph->GetYaxis()->CenterTitle(true);
  Graph->GetYaxis()->SetNdivisions(509, true);

  if (C != nullptr && C->HasModel() == true) {
    Graph->Draw("A*");
  } else {
    Graph->Draw("A*");
  }
  
  if (C != nullptr && C->HasModel() == true) {
    MCalibrationModel& Model = C->GetModel();
    Model.Draw("SAME");

    TGraph* Residuals = new TGraph(Points.size());

    // Residuals:
    double Min = +numeric_limits<double>::max();
    double Max = -numeric_limits<double>::max();
    for (unsigned int p = 0; p < Points.size(); ++p) {
      if (Points[p].IsGood() == false) continue;
      double Value = Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak()); 
      if (Value < Min) Min = Value;
      if (Value > Max) Max = Value;
      //Residuals->SetPoint(p, Points[p].GetPeak(), Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak()));
    }
    
    if (Min < 0) {
      Min *= 1.1;
    }
    if (Max > 0) {
      Max *= 1.1;
    }
    double ResidualRange = Max-Min;
    for (unsigned int p = 0; p < Points.size(); ++p) {
      if (Points[p].IsGood() == false) continue;
      cout<<Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak())<<endl;
      Residuals->SetPoint(p, Points[p].GetPeak(), 1.1*MaximumEnergy/ResidualRange * (Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak()) - Min));
    }
    
   
    Residuals->SetMarkerColor(kBlue);
    Residuals->SetMarkerStyle(33);
    Residuals->Draw("SAME *");
    
    //TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(), gPad->GetUxmax(), gPad->GetUymax(), Min, Max, 510, "+L");
    TGaxis *axis = new TGaxis(1.1*MaximumPeak, 0, 1.1*MaximumPeak, 1.1*MaximumEnergy/ResidualRange * (Max-Min), Min, Max, 510, "+L");
    axis->SetLineColor(kBlue);
    axis->SetLabelColor(kBlue);
    axis->SetTitleColor(kBlue);
    axis->SetTitle("residuals from model [keV]");
    axis->SetLabelSize(0.05);
    axis->SetLabelFont(Graph->GetXaxis()->GetLabelFont());
    axis->SetLabelSize(Graph->GetXaxis()->GetLabelSize());
    axis->SetTitleSize(0.06);
    axis->SetTitleOffset(1.4);
    axis->SetTitleFont(Graph->GetXaxis()->GetTitleFont());
    axis->SetTitleSize(Graph->GetXaxis()->GetTitleSize());
    axis->CenterTitle(true);
    axis->SetNdivisions(509);
    axis->Draw();
    
  }
  
  Canvas.Update();
}


////////////////////////////////////////////////////////////////////////////////


//! Create the histogram for the given read-out data group
TH1D* MMelinator::CreateSpectrum(const MString& Title, MReadOutDataGroup& G, double Min, double Max, unsigned int HistogramBinningMode, double HistogramBinningModeValue)
{
  MBinner* Binner = 0;
  if (HistogramBinningMode == c_HistogramBinningModeFixedNumberOfBins) {
    MBinnerFixedNumberOfBins* B = new MBinnerFixedNumberOfBins();
    B->SetNumberOfBins(HistogramBinningModeValue);
    B->AlignBins(true);
    Binner = B;
  } else if (HistogramBinningMode == c_HistogramBinningModeFixedCountsPerBin) {
    MBinnerFixedCountsPerBin* B = new MBinnerFixedCountsPerBin();
    B->SetCountsPerBin(HistogramBinningModeValue);
    Binner = B;
  } else if (HistogramBinningMode == c_HistogramBinningModeBayesianBlocks) {
    MBinnerBayesianBlocks* B = new MBinnerBayesianBlocks();
    B->SetMinimumBinWidth(2);
    B->SetPrior(HistogramBinningModeValue);
    Binner = B;
  }
  
  Binner->SetMinMax(Min, Max);
  for (unsigned int d = 0; d < G.GetNumberOfReadOutDatas(); ++d) {
    MReadOutDataInterfaceADCValue* ADC = dynamic_cast<MReadOutDataInterfaceADCValue*>(&(G.GetReadOutData(d)));
    if (ADC != nullptr) {
      Binner->Add(ADC->GetADCValue());
    }
  }
  
  TH1D* Histogram = 0;
  Histogram = Binner->GetNormalizedHistogram(Title, "read-out units", "counts / read-out unit");
  Histogram->SetBit(kCanDelete);
  Histogram->SetBit(TH1::kNoTitle); 

  delete Binner;
  
  return Histogram;
}


////////////////////////////////////////////////////////////////////////////////


//! Thread entry point for the parallel calibration
void MMelinatorCallParallelCalibrationThread(void* Address)
{
  MMelinator* M = ((MMelinatorThreadCaller*) Address)->GetThreadCaller();
  M->CalibrateParallel(((MMelinatorThreadCaller*) Address)->GetThreadID());
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration of the given collection
bool MMelinator::Calibrate(bool ShowDiagnostics)
{
  MGUIProgressBar ProgressBar;
  ProgressBar.SetTitles("Calibration", "Progress of calibration");
  ProgressBar.SetMinMax(0, m_Store.GetNumberOfReadOutCollections()); 
  
  cout<<"Fixing threads to 1 due to some root bugs..."<<endl;
  m_NThreads = 1;
  //if (m_NThreads < 1) m_NThreads = 1;
  m_ThreadNextItem = 0;
  m_Threads.resize(m_NThreads);
  m_ThreadIsInitialized.resize(m_NThreads);
  m_ThreadIsFinished.resize(m_NThreads);
  m_ThreadShouldTerminate.resize(m_NThreads);
 
  if (m_NThreads > 1) {
        
    // Start threads
    for (unsigned int t = 0; t < m_NThreads; ++t) {
      MString Name = "Calibration thread #";
      Name += t;
      //cout<<"Creatung thread: "<<Name<<endl;
      TThread* Thread = new TThread(Name, (void(*) (void *)) &MMelinatorCallParallelCalibrationThread, (void*) new MMelinatorThreadCaller(this, t));
      m_Threads[t] = Thread;
      m_ThreadIsInitialized[t] = false;
      m_ThreadIsFinished[t] = false;
      m_ThreadShouldTerminate[t] = false;

      Thread->Run();
      
      // Wait until thread is initialized:
      while (m_ThreadIsInitialized[t] == false && m_ThreadIsFinished[t] == false) {
        // Sleep for a while...
        TThread::Sleep(0, 10000000);
      }    

      //cout<<Name<<" is running"<<endl;
    }
    
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a while...
      TThread::Sleep(0, 10000000);
      
      ThreadsAreRunning = false;
      for (unsigned int t = 0; t < m_NThreads; ++t) {
        if (m_ThreadIsFinished[t] == false) {
          ThreadsAreRunning = true;
          break;
        }
      }
      
      // Update the progress bar:
      ProgressBar.SetValue(m_ThreadNextItem);
      if (ProgressBar.TestCancel() == true) {
        for (unsigned int t = 0; t < m_NThreads; ++t) {
          m_ThreadShouldTerminate[t] = true;
        }
      }

    }

    // None of the threads are running any more --- kill them
    for (unsigned int t = 0; t < m_NThreads; ++t) {
      m_Threads[t]->Kill();
      m_Threads[t] = 0;
    }
    
    //cout<<"All threads have finished..."<<endl;
  }
  // Non-threaded mode
  else {
    for (unsigned int c = 0; c < m_Store.GetNumberOfReadOutCollections(); ++c) {
      if (Calibrate(c, ShowDiagnostics) == false) return false;
      ProgressBar.SetValue(c);
      if (ProgressBar.TestCancel() == true) break;
    }
  }  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration of the given collection
bool MMelinator::CalibrateParallel(unsigned int ThreadID)
{
  m_ThreadIsInitialized[ThreadID] = true;

  while (true) {
    TThread::Lock();
    unsigned int ID = m_ThreadNextItem;
    ++m_ThreadNextItem;
    TThread::UnLock();
    
    if (ID >= m_Store.GetNumberOfReadOutCollections()) break;
    
    MReadOutCollection& C = GetCollection(ID);

    MCalibrateLines Cali;
    for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
      Cali.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    }
    Cali.SetPeakParametrizationMethod(m_PeakParametrizationMethod);
    Cali.SetPeakParametrizationMethodFittedPeakOptions(m_PeakParametrizationMethodFittedPeakBackgroundModel, m_PeakParametrizationMethodFittedPeakEnergyLossModel, m_PeakParametrizationMethodFittedPeakPeakShapeModel);
    Cali.SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod);
    Cali.SetCalibrationModelDeterminationMethodFittingOptions(m_CalibrationModelDeterminationMethodFittingModel);
    
    Cali.SetDiagnosticsMode(false);
    Cali.SetRange(m_HistogramMin, m_HistogramMax);
    cout<<"Max: "<<m_HistogramMax<<endl;
  
    if (Cali.Calibrate() == false) {
      cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
      return false;
    }
    
  
    TThread::Lock();
    m_CalibrationStore.Add(C.GetReadOutElement(), Cali.GetCalibration());
    TThread::UnLock();
    
    if (m_ThreadShouldTerminate[ThreadID] == true) break;
  }
    
  m_ThreadIsFinished[ThreadID] = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration of the given collection
bool MMelinator::Calibrate(unsigned int Collection, bool ShowDiagnostics)
{
  MReadOutCollection& C = GetCollection(Collection);

  MCalibrateLines Cali;
  for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
    Cali.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
  }
  Cali.SetPeakParametrizationMethod(m_PeakParametrizationMethod);
  Cali.SetPeakParametrizationMethodFittedPeakOptions(m_PeakParametrizationMethodFittedPeakBackgroundModel, m_PeakParametrizationMethodFittedPeakEnergyLossModel, m_PeakParametrizationMethodFittedPeakPeakShapeModel);
  Cali.SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod);
  Cali.SetCalibrationModelDeterminationMethodFittingOptions(m_CalibrationModelDeterminationMethodFittingModel);
  Cali.SetDiagnosticsMode(ShowDiagnostics);
  Cali.SetRange(m_HistogramMin, m_HistogramMax);
  cout<<"Max: "<<m_HistogramMax<<endl;
  
  if (Cali.Calibrate() == false) {
    cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
    return false;
  }
  
  m_CalibrationStore.Add(C.GetReadOutElement(), Cali.GetCalibration());
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Save the calibration in e-cal format
bool MMelinator::Save(MString FileName)
{
  ofstream out;
  out.open(FileName);
  if (out.is_open() == false) {
    merr<<"Unable to open file: "<<FileName<<endl;
    return false;
  }
  
  out<<"# Energy calibration file created with Melinator"<<endl;
  out<<" "<<endl;
  out<<"TYPE ECAL"<<endl;
  out<<" "<<endl;
  //out<<"CF doublesidedstrip pointsadctokev"<<endl;
  //out<<endl;
  
  for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
    MReadOutElement& ROE = m_CalibrationStore.GetReadOutElement(c);
    MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(c))); // TF1 make problems when copying the thing --> pointer
    if (C != nullptr) {
      // Make a list of the points and store them for sorting
      out<<"CP "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString("pakw", true)<<endl;
      if (C->HasModel() == true) {
        out<<"CM "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString("model", true)<<endl;
      }
    }
  }
  
  out.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Dump the content into a string
MString MMelinator::ToString() const
{
  ostringstream os;
  os<<"Melinator with this data store:"<<endl;
  os<<m_Store;
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
std::ostream& operator<<(std::ostream& os, const MMelinator& R)
{
  os<<R.ToString();
  return os;
}


// MMelinator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
