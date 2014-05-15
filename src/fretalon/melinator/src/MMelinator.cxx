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
using namespace std;

// ROOT libs:
#include "TBox.h"
#include "TLine.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLegend.h"
#include "TGraphErrors.h"

// MEGAlib libs:
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
  
  m_NThreads = 1;
  
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
}


////////////////////////////////////////////////////////////////////////////////


//! Load the calibration data containing the given isotopes
bool MMelinator::Load(const MString& FileName, const vector<MIsotope>& Isotopes)
{
  // Open the reader
  MFileReadOuts Reader;
  if (Reader.Open(FileName) == false) {
    return false;
  }
  Reader.ShowProgress();
  Reader.SetProgressTitle("Melinator: Loading", MString("Loading ") + MFile::GetBaseName(FileName) + MString(" ..."));

  // Add a new group to the store
  unsigned int GroupID = m_Store.AddReadOutDataGroup(FileName);
  m_GroupIDs.push_back(GroupID);
  m_Isotopes.push_back(Isotopes);
  
  MReadOutSequence Sequence;
  while (Reader.ReadNext(Sequence) == true) {
    m_Store.Add(Sequence, GroupID);
  }
  
  Reader.Close();
  
  // Let's ensure that both stores have the same read-out elements at the same positions
  m_CalibrationStore.Clear();
  for (unsigned int c = 0; c < m_Store.GetNumberOfReadOutCollections(); ++c) {
    m_CalibrationStore.Add(m_Store.GetReadOutCollection(c).GetReadOutElement());
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Load the calibration data containing the given isotopes - return false if an error occurred
//! This function performs parallel loading of all given files
bool MMelinator::Load(const vector<MString>& FileNames, const vector<vector<MIsotope> >& Isotopes)
{
  Clear();
  
  MGUIMultiProgressBar ProgressBar(FileNames.size());
  ProgressBar.SetTitles("Melinator Progress", "Progress of reading the calibration files");
  
  // Check for consistency and file size
  if (FileNames.size() == 0) return false;
  if (FileNames.size() != Isotopes.size()) return false;
  for (unsigned int f = 0; f < FileNames.size(); ++f) {
    MFileReadOuts Reader;
    if (Reader.Open(FileNames[f]) == false) {
      return false;
    }
    ProgressBar.SetTitle(f, MFile::GetBaseName(FileNames[f]));
    ProgressBar.SetMinMax(f, 0.0, Reader.GetFileLength());
    Reader.Close();
  }
  ProgressBar.SetMinimumChange(0.2*FileNames.size());
  ProgressBar.Create();
  gSystem->ProcessEvents();
  
  m_CalibrationFileNames = FileNames;
  m_Isotopes = Isotopes;
  
  m_CalibrationFileLoadingProgress.resize(FileNames.size());
  for (unsigned int c = 0; c < m_CalibrationFileLoadingProgress.size(); ++c) {
    m_CalibrationFileLoadingProgress[c] = 0.0;
  }
  
  unsigned int NThreads = m_NThreads;
  if (m_CalibrationFileNames.size() < NThreads) NThreads = m_CalibrationFileNames.size();  
  if (NThreads < 1) NThreads = 1;
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
    
    TThread::Lock();
    unsigned int GroupID = m_Store.AddReadOutDataGroup(m_CalibrationFileNames[ID]);
    m_GroupIDs.push_back(GroupID);
    TThread::UnLock();

    // Cannot show progress this way 
    // Reader.ShowProgress();
    // Reader.SetProgressTitle("Melinator: Loading", MString("Loading ") + MFile::GetBaseName(m_CalibrationFileNames[ID]) + MString(" ..."));

    MReadOutStore Store;
    Store.AddReadOutDataGroup("Dummy");
    
    MReadOutSequence Sequence;
    long Counter = 0;
    long NewCounter = 0;
    while (Reader.ReadNext(Sequence) == true) {
      Store.Add(Sequence, 0);
      ++NewCounter;
      if (++Counter%10000 == 0) {
        double Pos = Reader.GetFilePosition();
        if (Pos > 0) {
          m_CalibrationFileLoadingProgress[ID] = Pos; 
        }
        if (m_ThreadShouldTerminate[ThreadID] == true) break;
        
        // Check that we still have enough memory left:
        unsigned long Reserve = 32*(NewCounter+2*100000) + 200000*m_Store.GetNumberOfReadOutCollections();
        int* Memory = new(nothrow) int[Reserve];
        if (Memory == 0) {
          cout<<"Cannot reserve "<<sizeof(int)*Reserve<<" bytes --> Close to out of memory... Stopping to read more events..."<<endl;
          break;
        } else {
          delete Memory;
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
  Colors.push_back(kGreen);
  Colors.push_back(kRed);
  Colors.push_back(kBlue);
  Colors.push_back(kViolet);
  Colors.push_back(kYellow);
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
  
  // Final draw so that evergything is on top of each other:
  for (unsigned int h = 0; h < Histograms.size(); ++h) {
    if (Histograms[h] == 0) continue;
    Histograms[h]->Draw("SAME");
    MString Names;
    for (unsigned int i = 0; i < m_Isotopes[h].size(); ++i) {
      Names += "^{";
      Names += m_Isotopes[h][i].GetNucleons();
      Names += "}";
      Names += m_Isotopes[h][i].GetElement();
      if (i < m_Isotopes[h].size()-1) {
        Names += ", ";
      }
    }
    Legend->AddEntry(Histograms[h], Names);
  }
  Legend->Draw("SAME");

  Canvas.Update();
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

  Canvas.SetLeftMargin(0.12);
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
    
            //Spectrum->GetYaxis()->SetLabelOffset(0.001);
            Spectrum->GetYaxis()->SetLabelSize(0.05);
            Spectrum->GetYaxis()->SetTitleSize(0.06);
            Spectrum->GetYaxis()->SetTitleOffset(1.0);
            Spectrum->GetYaxis()->CenterTitle(true);
        
            Spectrum->Draw();
            
            if (P.HasFit() == true) {
              MCalibrationFit F = P.GetFit();
              F.Draw("SAME");
            }
            
            TLine* Line = new TLine(P.GetPeak(), Spectrum->GetMaximum(), P.GetPeak(), 0);
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
  
  TGraphErrors* Graph = new TGraphErrors(GetNumberOfCalibrationSpectralPoints(Collection));
  
  double MaximumPeak = 0.0; 
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    unsigned int Points = 0;
    for (unsigned int g = 0; g < C->GetNumberOfReadOutDataGroups(); ++g) {
      for (unsigned int p = 0; p < C->GetNumberOfSpectralPoints(g); ++p) {
        MCalibrationSpectralPoint P = C->GetSpectralPoint(g, p);
        if (P.IsGood() == false) continue;
        Graph->SetPoint(Points, P.GetPeak(), P.GetEnergy());
        if (P.GetPeak() > MaximumPeak) MaximumPeak = P.GetPeak();
        Points++;
      }
    }
    Graph->Set(Points);
  }
  
  
  //cout<<"Drawing graph..."<<endl;
  Graph->Sort();
  Graph->SetMinimum(0);
  Graph->Draw(); // Draw in order to have all axis!
  
  Graph->SetTitle("");

  Graph->GetXaxis()->SetTitle("Read-out units");
  // Graph->GetXaxis()->SetLabelOffset(0.0);
  Graph->GetXaxis()->SetLabelSize(0.05);
  Graph->GetXaxis()->SetTitleSize(0.06);
  Graph->GetXaxis()->SetTitleOffset(0.9);
  Graph->GetXaxis()->CenterTitle(true);
  Graph->GetXaxis()->SetMoreLogLabels(true);
  Graph->GetXaxis()->SetLimits(0.0, 1.1*MaximumPeak);
  Graph->GetXaxis()->SetNdivisions(509, true);
  
  Graph->GetYaxis()->SetTitle("Energy [keV]");
  // Graph->GetYaxis()->SetLabelOffset(0.001);
  Graph->GetYaxis()->SetLabelSize(0.05);
  Graph->GetYaxis()->SetTitleSize(0.06);
  Graph->GetYaxis()->SetTitleOffset(1.4);
  Graph->GetYaxis()->CenterTitle(true);
  Graph->GetYaxis()->SetNdivisions(509, true);

  Graph->Draw("AL*");
  
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
  
  if (m_NThreads < 1) m_NThreads = 1;
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
      Cali.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[g]);
    }
    Cali.SetPeakParametrizationMethod(m_PeakParametrizationMethod);
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
    Cali.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[g]);
  }
  Cali.SetPeakParametrizationMethod(m_PeakParametrizationMethod);
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
  out<<"CF doublesidedstrip pointsadctokev"<<endl;
  out<<endl;
  
  for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
    MReadOutElement& ROE = m_CalibrationStore.GetReadOutElement(c);
    MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(c))); // TF1 make problems when copying the thing --> pointer
    if (C != nullptr) {
      // Make a list of the points and store them for sorting
      out<<"CP "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString(true)<<endl;
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
