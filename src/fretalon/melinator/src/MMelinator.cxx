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
#include <pwd.h>
#include <unistd.h>
#include <algorithm>
#include <new>
#include <map>
#include <thread>
using namespace std;

// ROOT libs:
#include "TObject.h"
#include "TList.h"
#include "TBox.h"
#include "TLine.h"
#include "TMarker.h"
#include "TEllipse.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TGaxis.h"
#include "TGraphErrors.h"
#include "TSystem.h"
#include "TH2.h"
#include "TPaveText.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLatex.h"

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
#include "MReadOutDataTemperature.h"
#include "MCalibrateEnergyAssignEnergies.h"
#include "MCalibrateEnergyDetermineModel.h"
#include "MCalibrateEnergyFindLines.h"
#include "MCalibrationSpectralPoint.h"
#include "MCalibrationSpectrum.h"
#include "MBinner.h"
#include "MBinnerFixedNumberOfBins.h"
#include "MBinnerFixedCountsPerBin.h"
#include "MBinnerBayesianBlocks.h"
#include "MReadOutDataADCValue.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MMelinator)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Thread entry point for the parallel calibration
void MMelinatorCallParallelSpectrumCreationThread(void* Address)
{
  MMelinatorThreadCaller* Caller = (MMelinatorThreadCaller*) Address;

  MMelinator* M = Caller->GetThreadCaller();
  M->CreateSpectrumParallel(Caller->GetThreadID(), Caller->GetID1(), Caller->GetID2());
}


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MMelinator::MMelinator()
{
  m_HistogramMin = 0;
  m_HistogramMax = 1000;
  m_HistogramBinningMode = c_HistogramBinningModeFixedNumberOfBins;
  m_HistogramBinningModeValue = 100;
  m_HistogramChanged = true;
  m_HistogramCollection = -1;

  m_PeakFindingPrior = 8;
  m_PeakFindingExcludeFirstNumberOfBins = 25;
  m_PeakFindingMinimumPeakCounts = 100;

  m_PeakParametrizationMethod = MCalibrateEnergyFindLines::c_PeakParametrizationMethodBayesianBlockPeak;
  m_PeakParametrizationMethodFittedPeakBackgroundModel = MCalibrationFit::c_BackgroundModelLinear;
  m_PeakParametrizationMethodFittedPeakEnergyLossModel = MCalibrationFit::c_EnergyLossModelNone;
  m_PeakParametrizationMethodFittedPeakPeakShapeModel = MCalibrationFit::c_PeakShapeModelGaussian;
  
  m_CalibrationModelEnergyAssignmentMethod = MCalibrateEnergyAssignEnergyModes::e_LinearZeroCrossing;
  m_CalibrationModelDeterminationMethod = MCalibrateEnergyDetermineModel::c_CalibrationModelStepWise;
  
  m_NThreads = thread::hardware_concurrency();
  if (m_NThreads < 1) m_NThreads = 1;
  
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
  m_SelectedDetectorSide = -1;

  m_SelectedTemperatureMin = -numeric_limits<double>::max();
  m_SelectedTemperatureMax = +numeric_limits<double>::max();

  m_HistogramChanged = true;
  m_HistogramCollection = -1;
  for (auto H: m_Histograms) delete H;
  m_Histograms.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Load the calibration data containing the given isotopes - return false if an error occurred
//! This function performs parallel loading of all given files
bool MMelinator::Load(const vector<MString>& FileNames, const vector<vector<MIsotope> >& Isotopes, const vector<unsigned int>& GroupIDs)
{
  MTimer LoadingTimer;
  
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
      return false; // we cannot simply ignore it here otherwise the code below will break down
    }
    m_CalibrationFileNames.push_back(FileNames[f]);
    ProgressBar.SetTitle(m_CalibrationFileNames.size()-1, MFile::GetBaseName(FileNames[f]));
    ProgressBar.SetMinMax(m_CalibrationFileNames.size()-1, 0.0, Reader.GetFileLength());
    Reader.Close();
  }
  ProgressBar.SetMinimumChange(0.2*FileNames.size());
  ProgressBar.Create();
  gSystem->ProcessEvents();
  
  //m_CalibrationFileNames = FileNames;
  //m_Isotopes = Isotopes;
  
  //! Map the external group IDs to internal IDs starting at 0 and combined the isotopes of the individial groups
  m_GroupIDs.clear();
  unsigned int InternalID = 0;
  map<unsigned int, unsigned int> IDMap; // maps external ID to internal ID
  map<unsigned int, MString> NameMap; // maps internal ID to file names
  map<unsigned int, vector<MIsotope>> IsotopeMap; // maps internal ID to isotopes
  for (unsigned int g = 0; g < GroupIDs.size(); ++g) {
    if (FileNames[g] == "") continue;
    
    unsigned int CurrentInternalID = 0;
    map<unsigned int, unsigned int>::iterator I = IDMap.find(GroupIDs[g]);
    if (I == IDMap.end()) {
      IDMap[GroupIDs[g]] = InternalID;
      CurrentInternalID = InternalID;
      InternalID++;
    } else {
      CurrentInternalID = (*I).second;
    }
    // Add, sort and remove duplicate isotopes
    for (unsigned int i = 0; i < Isotopes[g].size(); ++i) {
      bool Found = false;
      for (unsigned int e = 0; e < IsotopeMap[CurrentInternalID].size(); ++e) {
        if (IsotopeMap[CurrentInternalID][e] == Isotopes[g][i]) {
          Found = true; 
        }
      }
      if (Found == false) {
        IsotopeMap[CurrentInternalID].push_back(Isotopes[g][i]);
      }
    }
    
    //IsotopeMap[CurrentInternalID].insert(IsotopeMap[CurrentInternalID].end(), Isotopes[g].begin(), Isotopes[g].end());
    //sort(IsotopeMap[CurrentInternalID].begin(), IsotopeMap[CurrentInternalID].end());
    //IsotopeMap[CurrentInternalID].erase(unique(IsotopeMap[CurrentInternalID].begin(), IsotopeMap[CurrentInternalID].end()), IsotopeMap[CurrentInternalID].end());
    
    m_GroupIDs.push_back(IDMap[GroupIDs[g]]);
    MString Name = MFile::GetBaseName(FileNames[g]);
    if (NameMap[CurrentInternalID] == "") {
      NameMap[CurrentInternalID] += Name;
    } else {
      NameMap[CurrentInternalID] += " & " + Name;
    }
  }
  
  if (g_Verbosity >= c_Chatty) {
    cout<<"Group ID mapping (external to internal): "<<endl;
    for (unsigned int g = 0; g < m_GroupIDs.size(); ++g) {
      cout<<GroupIDs[g]<<" --> "<<m_GroupIDs[g]<<endl; 
    }
    cout<<"External group ID to internal group ID to name mapping: "<<endl;
    for (auto N: IDMap) {
      cout<<N.first<<" --> "<<N.second<<" --> "<<NameMap[N.second]<<endl; 
    }
    cout<<"Internal group ID to isotope mapping: "<<endl;
    for (auto N: IsotopeMap) {
      cout<<N.first<<" --> ";
      for (auto I: N.second) {
        cout<<I.GetName()<<" "; 
      }
      cout<<endl;
    }
  }
  
  //! Set up the read-out data groups in the store
  for (auto N: NameMap) {
    unsigned int GroupID = m_Store.AddReadOutDataGroup(N.second);
    if (GroupID != N.first) {
      cout<<"ERROR: Something went wrong with the group ID creation: internal group ID="<<N.first<<" vs. read out data group ID="<<GroupID<<endl;
      return false;
    }
  }
  
  //! Now make sure the groups have all the same isotopes
  m_Isotopes.clear();
  for (unsigned int g = 0; g < m_GroupIDs.size(); ++g) {
    m_Isotopes.push_back(IsotopeMap[m_GroupIDs[g]]);
  }

  m_NLinesToConsider = 0;
  for (auto N: IsotopeMap) {
    for (auto I: N.second) {
      m_NLinesToConsider += I.GetNLines();
    }
  }
  //cout<<"Lines: "<<m_NLinesToConsider<<endl;
  
  if (g_Verbosity >= c_Chatty) {
    cout<<"Isotopes: "<<endl;
    for (auto V: m_Isotopes) {
      cout<<" --> ";
      for (auto I: V) {
        cout<<I.GetName()<<" "; 
      }
      cout<<endl;
    }
  }
  
  
  m_CalibrationFileLoadingProgress.resize(NFiles);
  for (unsigned int c = 0; c < m_CalibrationFileLoadingProgress.size(); ++c) {
    m_CalibrationFileLoadingProgress[c] = 0.0;
  }
  
  unsigned int NThreads = m_CalibrationFileNames.size();

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
    
    long Counter = 0;
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a 10 ms
      TThread::Sleep(0, 10000000);
      // Whenever we slept ~0.1 sec update the UI
      if (++Counter % 10 == 0) {
        gSystem->ProcessEvents();
      }
      
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
  
  if (g_Verbosity >= c_Info) cout<<"Loading finished after "<<LoadingTimer.GetElapsed()<<" seconds"<<endl;

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
  if (g_Verbosity >= c_Info) cout<<"Parallel loading thread #"<<ThreadID<<" has started"<<endl;
  
  m_ThreadIsInitialized[ThreadID] = true;
  
  // We are likely to run out of memory so catch some bad_alloc exceptions
  try {
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
      while (Reader.ReadNext(Sequence, m_SelectedDetectorID, m_SelectedDetectorSide) == true) {
        // Since we do energy calibration, exclude everything with more than the number of good hits
        //if (Sequence.HasIdenticalReadOutElementTypes() == true) {
        //  if (Sequence.GetNumberOfReadOuts() > 0 && 
        //    Sequence.GetNumberOfReadOuts() != Sequence.GetReadOut(0).GetReadOutElement().GetMinimumNumberOfReadOutsForGoodInteraction()) {
        //    continue;
        //  }
        //}
        vector<unsigned int> ToRemove;
        for (unsigned int d = 0; d < Sequence.GetNumberOfReadOuts(); ++d) { 
          MReadOutDataTemperature* T = dynamic_cast<MReadOutDataTemperature*>(Sequence.GetReadOut(d).GetReadOutData().Get(MReadOutDataTemperature::m_TypeID));
          if (T != nullptr) {
            if (T->GetTemperature() < m_SelectedTemperatureMin || T->GetTemperature() > m_SelectedTemperatureMax) {
              ToRemove.push_back(d);
            }
          }
        }
        for (unsigned int t = ToRemove.size() - 1; t < ToRemove.size(); --t) {
          Sequence.RemoveReadOut(ToRemove[t]); 
        }
        
        
        Store.Add(Sequence, 0);
        
        if (m_ThreadShouldTerminate[ThreadID] == true) break;
        
        ++NewCounter;
        if (++Counter%10000 == 0) {
          double Pos = Reader.GetFilePosition();
          if (Pos > 0) {
            m_CalibrationFileLoadingProgress[ID] = Pos; 
          }
          
          
          // Check that we still have enough memory left:
          //cout<<"Check: "<<sizeof(Sequence)*NewCounter<<":"<<160000*m_NLinesToConsider*m_Store.GetNumberOfReadOutCollections()<<":"<<m_NLinesToConsider<<":"<<m_Store.GetNumberOfReadOutCollections()<<endl;
          unsigned long Reserve = 5000000 + sizeof(Sequence)*NewCounter + 160000*m_NLinesToConsider*m_Store.GetNumberOfReadOutCollections();
          char* Memory = new(nothrow) char[Reserve];
          if (Memory == 0) {
            if (g_Verbosity >= c_Warning) cout<<"Cannot reserve "<<sizeof(char)*Reserve<<" bytes --> Close to out of memory... Stopping to read more events..."<<endl;
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
  } catch (bad_alloc&) {
    ProcInfo_t Info;
    gSystem->GetProcInfo(&Info);
    cout<<endl;
    cout<<"We are out of memory... "<<endl;
    cout<<"Melinator used "<<(double) (Info.fMemResident)/1024/1024<<"GB before the crash"<<endl;
    cout<<endl;
    abort();
  }
  
  m_ThreadIsFinished[ThreadID] = true;

  if (g_Verbosity >= c_Info) cout<<"Parallel loading thread #"<<ThreadID<<" has finished"<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Draw the spectrum of all read-out data groups in the collection into the given canvas 
void MMelinator::DrawSpectrum(TCanvas& Canvas, unsigned int Collection, unsigned int Line)
{
  MReadOutCollection& C = GetCollection(Collection);
  
  if (m_HistogramCollection != Collection) m_HistogramChanged = true;
  m_HistogramCollection = Collection;
  
  vector<int> Colors;
  Colors.push_back(kRed);
  Colors.push_back(kBlue);
  Colors.push_back(kGreen+1);
  Colors.push_back(kMagenta+2);
  Colors.push_back(kAzure-2);
  Colors.push_back(kCyan+2);
  Colors.push_back(kYellow);
  Colors.push_back(kTeal);
  Colors.push_back(kSpring);
  Colors.push_back(kOrange);
  Colors.push_back(kPink-2);
  Colors.push_back(kViolet+2);
  Colors.push_back(kGreen+4);
  Colors.push_back(kYellow+3);
  Colors.push_back(kRed+3);
  Colors.push_back(kBlue+4);
  Colors.push_back(kCyan+4);

  
  
  //Canvas.SetBit(kNoContextMenu);
  Canvas.SetBit(kCannotPick);
  Canvas.Clear();
  Canvas.cd();
  Canvas.SetGridx();
  Canvas.SetGridy();
  //Canvas.SetLogy();
  
  Canvas.SetLeftMargin(0.1);
  Canvas.SetRightMargin(0.05);
  Canvas.SetTopMargin(0.05);
  Canvas.SetBottomMargin(0.14);
  

  if (m_HistogramChanged == true) {

    for (auto H: m_Histograms) delete H;
    m_Histograms.clear();
    

    // <-- Begin time critical

    unsigned int NThreads = C.GetNumberOfReadOutDataGroups();

    if (NThreads > 1) {
      m_Histograms.resize(NThreads);

      m_ThreadNextItem = 0;
      m_Threads.resize(NThreads);
      m_ThreadIsInitialized.resize(NThreads);
      m_ThreadShouldTerminate.resize(NThreads);
      m_ThreadIsFinished.resize(NThreads);

      // Start threads
      for (unsigned int t = 0; t < NThreads; ++t) {
        TString Name = "Loading thread #";
        Name += t;
        //cout<<"Creating thread: "<<Name<<endl;
        TThread* Thread = new TThread(Name, (void(*) (void *)) &MMelinatorCallParallelSpectrumCreationThread, (void*) new MMelinatorThreadCaller(this, t, Collection, t));
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

      }

      long Counter = 0;
      bool ThreadsAreRunning = true;
      while (ThreadsAreRunning == true) {

        // Sleep for a 10 ms
        TThread::Sleep(0, 10000000);
        // Whenever we slept ~0.1 sec update the UI
        if (++Counter % 10 == 0) {
          gSystem->ProcessEvents();
        }

        int Running = 0;
        ThreadsAreRunning = false;
        for (unsigned int t = 0; t < NThreads; ++t) {
          if (m_ThreadIsFinished[t] == false) {
            ThreadsAreRunning = true;
            Running++;
          }
        }
      }

      // None of the threads are running any more --- kill them
      for (unsigned int t = 0; t < NThreads; ++t) {
        m_Threads[t]->Kill();
        m_Threads[t] = 0;
      }

    } else {
      //! Create the histograms
      for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g)  {
        MReadOutDataGroup& G = C.GetReadOutDataGroup(g);
        TH1D* H = CreateSpectrum(G.GetName(), G, m_HistogramMin, m_HistogramMax, m_HistogramBinningMode, m_HistogramBinningModeValue);
        if (H->GetMaximum() > 0) {
          m_Histograms.push_back(H);
        } else {
          m_Histograms.push_back(nullptr);
          delete H;
        }
      }
    }
    // --> end time critical


    double Max = 0;
    double Min = numeric_limits<double>::max();
    for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
      if (m_Histograms[h] == 0) continue;
      if (m_Histograms[h]->GetMaximum() > Max) {
        Max = m_Histograms[h]->GetMaximum(); 
      }
      for (int b = 1; b <= m_Histograms[h]->GetNbinsX(); ++b) { 
        if (m_Histograms[h]->GetBinContent(b) > 0 && m_Histograms[h]->GetBinContent(b) < Min) {
          Min = m_Histograms[h]->GetBinContent(b); 
        }
      }
    }


    for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
      if (m_Histograms[h] == 0) continue;
      m_Histograms[h]->SetMaximum(1.1*Max);
      m_Histograms[h]->SetMinimum(0.9*Min);
      if (h < Colors.size()) {
        m_Histograms[h]->SetLineColor(Colors[h]);
      } else {
        m_Histograms[h]->SetLineColor(kBlack);
      }
      m_Histograms[h]->SetLineWidth(2);
      
      //m_Histograms[h]->GetXaxis()->SetLabelOffset(0.0);
      m_Histograms[h]->GetXaxis()->SetLabelSize(0.05);
      m_Histograms[h]->GetXaxis()->SetTitleSize(0.06);
      m_Histograms[h]->GetXaxis()->SetTitleOffset(1.1);
      m_Histograms[h]->GetXaxis()->CenterTitle(true);
      m_Histograms[h]->GetXaxis()->SetMoreLogLabels(true);
      
      //m_Histograms[h]->GetYaxis()->SetLabelOffset(0.001);
      m_Histograms[h]->GetYaxis()->SetLabelSize(0.05);
      m_Histograms[h]->GetYaxis()->SetTitleSize(0.06);
      m_Histograms[h]->GetYaxis()->SetTitleOffset(0.8);
      m_Histograms[h]->GetYaxis()->CenterTitle(true);
    }
  }

  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    if (m_Histograms[h] == 0) continue;
    
    if (h == 0) {
      m_Histograms[h]->DrawCopy("HIST");
    } else {
      m_Histograms[h]->DrawCopy("HIST SAME");
    }
  }

  
  vector<int> ColorOffsets;
  ColorOffsets.push_back(-4);
  ColorOffsets.push_back(-7);
  ColorOffsets.push_back(-9);
  ColorOffsets.push_back(-10);
  
  unsigned int LineID = 0;
  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    //! Draw the line-fit regions:
    unsigned int CalibrationIndex = m_CalibrationStore.FindCalibration(C.GetReadOutElement());
    if (CalibrationIndex != g_UnsignedIntNotDefined) {
      MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(CalibrationIndex)));
      if (C != nullptr) {
        for (unsigned int p = 0; p < C->GetNumberOfSpectralPoints(h); ++p) {
          MCalibrationSpectralPoint P = C->GetSpectralPoint(h, p);
          TBox* Box = new TBox(P.GetLowEdge(), 0.0, P.GetHighEdge(), m_Histograms[h]->GetMaximum());
          if (p < ColorOffsets.size()) {
            Box->SetFillColor(Colors[h]+ColorOffsets[p]);
          } else {
            Box->SetFillColor(Colors[h]+ColorOffsets[3]);
          }
          Box->SetFillStyle(3001);
          Box->Draw("SAME");
          //cout<<LineID<<":"<<Line<<endl;
          if (LineID == Line) {
            TMarker* Marker = new TMarker(P.GetPeak(), 0.9*m_Histograms[h]->GetMaximum(), 20);
            Marker->Draw("SAME");
          }
          ++LineID;
        }
      }
    } else {
      if (g_Verbosity >= c_Info) cout<<"No calibration found!"<<endl;
    }
  }
  
  double ySizeMin = 0.84 - 0.075*m_Histograms.size();
  if (ySizeMin < 0.5) ySizeMin = 0.5;
  TLegend* Legend = new TLegend(0.8, ySizeMin, 0.94, 0.94, NULL, "brNDC");
  Legend->SetHeader("Isotope list:");
  
  // Final draw so that everything is on top of each other:
  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    if (m_Histograms[h] == 0) continue;
    m_Histograms[h]->DrawCopy("SAME");
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
    Legend->AddEntry(m_Histograms[h], Names, "l");
  }
  Legend->Draw("SAME");
  
  
  // Draw the axis of the first histogram again:
  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    if (m_Histograms[h] == 0) continue;
    m_Histograms[h]->DrawCopy("AXIS SAME");
    break;
  }

  Canvas.Update();
  m_HistogramChanged = false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if we have an energy calibration model
bool MMelinator::HasEnergyCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    return C->HasEnergyModel();
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the energy calibration model of the spectra
MCalibrationModel& MMelinator::GetEnergyCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    if (C->HasEnergyModel()) {
      return C->GetEnergyModel();
    }
  }
  
  throw MExceptionObjectDoesNotExist("No energy calibration model available");  
  
  return C->GetEnergyModel();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if we have an FWHM calibration model
bool MMelinator::HasFWHMCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    return C->HasFWHMModel();
  }
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Get the FWHM calibration model of the spectra
MCalibrationModel& MMelinator::GetFWHMCalibrationModel(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  if (C != nullptr) {
    if (C->HasFWHMModel()) {
      return C->GetFWHMModel();
    }
  }
  
  throw MExceptionObjectDoesNotExist("No FWHM calibration model available");  
  
  return C->GetFWHMModel();
}


////////////////////////////////////////////////////////////////////////////////

    
//! Get the number of calibration point in the spectra
unsigned int MMelinator::GetNumberOfCalibrationSpectralPoints(unsigned int Collection)
{
  if (Collection < m_CalibrationStore.GetNumberOfElements()) {
    MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
    if (C != nullptr) {
      unsigned int Points = 0;
      for (unsigned int g = 0; g <C->GetNumberOfReadOutDataGroups(); ++g) {
        Points += C->GetNumberOfSpectralPoints(g);
      }
      return Points;
    }
  }
  
  return 0;
}


////////////////////////////////////////////////////////////////////////////////

  
//! Return the given spectral point
//! If it doesn't exist, the exception MExceptionIndexOutOfBounds is thrown
MCalibrationSpectralPoint& MMelinator::GetCalibrationSpectralPoint(unsigned int Collection, unsigned int Line) 
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
  
  return C->GetSpectralPoint(0, 0); // Wrong, but we never reach that point...
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
            
            Spectrum->SetFillStyle(0);
            //Spectrum->SetAxisRange(P.GetLowEdge(), P.GetHighEdge());
            Spectrum->SetMaximum(1.1*Spectrum->GetMaximum());
            
            //Spectrum->GetXaxis()->SetLabelOffset(0.0);
            Spectrum->GetXaxis()->SetLabelSize(0.05);
            Spectrum->GetXaxis()->SetTitleSize(0.06);
            Spectrum->GetXaxis()->SetTitleOffset(0.9);
            Spectrum->GetXaxis()->CenterTitle(true);
            Spectrum->GetXaxis()->SetMoreLogLabels(true);
            Spectrum->GetXaxis()->SetNdivisions(506, true);
    
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

//! Return the calibration fit quality
double MMelinator::GetCalibrationQuality(unsigned int Collection)
{
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));

  double FitValue = -1;
  if (C != nullptr && C->HasEnergyModel() == true) {
    FitValue = C->GetEnergyModel().GetFitQuality();
    if (/*FitValue < 0 ||*/ std::isnan(FitValue) == true || std::isinf(FitValue) == true) {
      FitValue = 100;
    }
  }
  
  return FitValue;
}


////////////////////////////////////////////////////////////////////////////////



//! Draw the calibration into the Canvas for the given Collection
void MMelinator::DrawCalibration(TCanvas& Canvas, unsigned int Collection, bool UseEnergy)
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
  
  
  double MaximumX = 0.0; 
  double MaximumY = 0.0; 
  MCalibrationSpectrum* C = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection)));
  vector<MCalibrationSpectralPoint> Points; 
  TGraphErrors* Graph = new TGraphErrors();
  if (C != nullptr) {
    Points = C->GetUniquePoints();
    Graph->Set(Points.size());
    for (unsigned int p = 0; p < Points.size(); ++p) {
      if (UseEnergy == true) {
        Graph->SetPoint(p, Points[p].GetPeak(), Points[p].GetEnergy());
        if (Points[p].GetPeak() > MaximumX) MaximumX = Points[p].GetPeak();
        if (Points[p].GetEnergy() > MaximumY) MaximumY = Points[p].GetEnergy();
      } else {
        Graph->SetPoint(p, Points[p].GetEnergy(), Points[p].GetEnergyFWHM());
        if (Points[p].GetEnergy() > MaximumX) MaximumX = Points[p].GetEnergy();
        if (Points[p].GetEnergyFWHM() > MaximumY) MaximumY = Points[p].GetEnergyFWHM();
      }
    }
  }
  
  
  //cout<<"Drawing graph..."<<endl;
  Graph->Sort();
  Graph->SetMinimum(0);
  Graph->SetMaximum(1.1*MaximumY);
  Graph->Draw(); // Draw in order to have all axis!
  
  Graph->SetTitle("");

  if (UseEnergy == true) {
    Graph->GetXaxis()->SetTitle("read-out units");
  } else {
    Graph->GetXaxis()->SetTitle("energy [keV]");
  }
  // Graph->GetXaxis()->SetLabelOffset(0.0);
  Graph->GetXaxis()->SetLabelSize(0.05);
  Graph->GetXaxis()->SetTitleSize(0.06);
  Graph->GetXaxis()->SetTitleOffset(0.9);
  Graph->GetXaxis()->CenterTitle(true);
  Graph->GetXaxis()->SetMoreLogLabels(true);
  Graph->GetXaxis()->SetLimits(0.0, 1.1*MaximumX);
  Graph->GetXaxis()->SetNdivisions(505, true);
  
  if (UseEnergy == true) {
    Graph->GetYaxis()->SetTitle("energy [keV]");
  } else {
    Graph->GetYaxis()->SetTitle("FWHM [keV]");
  }
  // Graph->GetYaxis()->SetLabelOffset(0.001);
  Graph->GetYaxis()->SetLabelSize(0.05);
  Graph->GetYaxis()->SetTitleSize(0.06);
  Graph->GetYaxis()->SetTitleOffset(1.4);
  Graph->GetYaxis()->CenterTitle(true);
  Graph->GetYaxis()->SetNdivisions(509, true);

  if (C != nullptr && C->HasEnergyModel() == true) {
    Graph->Draw("A*");
  } else {
    Graph->Draw("A*");
  }
  
  if (C != nullptr && C->HasEnergyModel() == true) {
    MCalibrationModel& Model = (UseEnergy == true) ? C->GetEnergyModel() : C->GetFWHMModel();
    Model.Draw("SAME");

    TGraph* Residuals = new TGraph(Points.size());

    // Residuals:
    double Min = +numeric_limits<double>::max();
    double Max = -numeric_limits<double>::max();
    for (unsigned int p = 0; p < Points.size(); ++p) {
      if (Points[p].IsGood() == false) continue;
      double Value = 0.0;
      if (UseEnergy == true) {
        Value = Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak()); 
      } else {
        Value = Points[p].GetEnergyFWHM() - Model.GetFitValue(Points[p].GetEnergy()); 
      }
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
      //cout<<Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak())<<endl;
      if (UseEnergy == true) {
        Residuals->SetPoint(p, Points[p].GetPeak(), 1.1*MaximumY/ResidualRange * (Points[p].GetEnergy() - Model.GetFitValue(Points[p].GetPeak()) - Min));
      } else {
        Residuals->SetPoint(p, Points[p].GetEnergy(), 1.1*MaximumY/ResidualRange * (Points[p].GetEnergyFWHM() - Model.GetFitValue(Points[p].GetEnergy()) - Min));
      }
    }
    
   
    Residuals->SetMarkerColor(kBlue);
    Residuals->SetMarkerStyle(33);
    Residuals->Draw("SAME *");
    
    //TGaxis *axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(), gPad->GetUxmax(), gPad->GetUymax(), Min, Max, 510, "+L");
    TGaxis *axis = new TGaxis(1.1*MaximumX, 0, 1.1*MaximumX, 1.1*MaximumY/ResidualRange * (Max-Min), Min, Max, 510, "+L");
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
    axis->Draw("SAME");
    
    if (Min < 0 && Max > 0) {
      TLine* Zero = new TLine(0, -Min/(Max-Min) * 1.1*MaximumY, 1.1*MaximumX, -Min/(Max-Min) * 1.1*MaximumY);
      Zero->SetLineColor(kBlue);
      Zero->Draw("SAME");
    }
    
  }
  
  Canvas.Update();
}


////////////////////////////////////////////////////////////////////////////////


//! Perform parallel spectrum creation
bool MMelinator::CreateSpectrumParallel(unsigned int ThreadID, unsigned int CollectionID, unsigned int DataGroupID)
{
  if (g_Verbosity >= c_Info) cout<<"Parallel spectrum creation thread #"<<ThreadID<<" has started"<<endl;

  m_ThreadIsInitialized[ThreadID] = true;

  MReadOutCollection& C = GetCollection(CollectionID);
  MReadOutDataGroup& G = C.GetReadOutDataGroup(DataGroupID);
  TH1D* H = CreateSpectrum(G.GetName(), G, m_HistogramMin, m_HistogramMax, m_HistogramBinningMode, m_HistogramBinningModeValue);
  if (H->GetMaximum() > 0) {
    m_Histograms[DataGroupID] = H;
  } else {
    m_Histograms[DataGroupID] = nullptr;
    {
      TLockGuard G(g_Mutex);
      delete H;
    }
  }

  m_ThreadIsFinished[ThreadID] = true;

  if (g_Verbosity >= c_Info) cout<<"Parallel spectrum creation thread #"<<ThreadID<<" has finished"<<endl;

  return true;
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
    MReadOutDataADCValue* ADC = dynamic_cast<MReadOutDataADCValue*>(G.GetReadOutData(d).Get(MReadOutDataADCValue::m_TypeID));
    if (ADC != nullptr) {
      Binner->Add(ADC->GetADCValue());
    }
  }

  TH1D* Histogram = 0;
  Histogram = Binner->GetNormalizedHistogram(Title, "read-out units", "counts / read-out unit");
  Histogram->SetBit(kCanDelete);
  Histogram->SetBit(TH1::kNoTitle); 
  Histogram->SetFillStyle(0);

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
  MTimer Timer;
  
  MGUIProgressBar ProgressBar;
  ProgressBar.SetTitles("Calibration", "Progress of calibration");
  ProgressBar.SetMinMax(0, m_Store.GetNumberOfReadOutCollections()); 
  
  m_TimeToFindLines = 0.0;
  m_TimeToAssignEnergies = 0.0;
  m_TimeToDetermineModel = 0.0;  
  
  
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
      //cout<<"Creating thread: "<<Name<<endl;
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
        gSystem->ProcessEvents();
      }    

      //cout<<Name<<" is running"<<endl;
    }
    
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a while...
      TThread::Sleep(0, 10000000);
      gSystem->ProcessEvents();
     
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
      if (Calibrate(c, ShowDiagnostics) == false) continue;
      ProgressBar.SetValue(c);
      if (ProgressBar.TestCancel() == true) break;
    }
  }
  
  //if (g_Verbosity >= c_Info) 
  {
    cout<<"Time spent in calibration: "<<Timer.GetElapsed()<<" sec"<<endl;
    cout<<"  + Time spent to find line: "<<m_TimeToFindLines<<" sec"<<endl;
    cout<<"  + Time spent to assign energies: "<<m_TimeToAssignEnergies<<" sec"<<endl;
    cout<<"  + Time spent to determine model: "<<m_TimeToDetermineModel<<" sec"<<endl;
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

    Calibrate(ID);
    
    if (m_ThreadShouldTerminate[ThreadID] == true) break;
  }
    
  m_ThreadIsFinished[ThreadID] = true;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Perform the calibration of the given collection
bool MMelinator::Calibrate(unsigned int Collection, bool ShowDiagnostics)
{
  // We are likely to run out of memory so catch some bad_alloc exceptions
  try {
    MReadOutCollection& C = GetCollection(Collection);
    m_CalibrationStore.Remove(C.GetReadOutElement());
    
    unsigned int Verbosity = g_Verbosity;
    if (ShowDiagnostics == true) g_Verbosity = c_Info;

    MTimer Timer;
  
    
    // Step 1: find the lines
    Timer.Reset();
    MCalibrateEnergyFindLines FindLines;
    FindLines.SetDiagnosticsMode(ShowDiagnostics);
    FindLines.SetRange(m_HistogramMin, m_HistogramMax);
    FindLines.SetTemperatureWindow(m_SelectedTemperatureMin, m_SelectedTemperatureMax);
    FindLines.SetBayesianBlockPrior(m_PeakFindingPrior);
    FindLines.SetNumberOfExcludedBinsAtLowEnergies(m_PeakFindingExcludeFirstNumberOfBins);
    FindLines.SetMinimumNumberOfPeakCounts(m_PeakFindingMinimumPeakCounts);
    //cout<<"T1: "<<Timer.GetElapsed()<<endl;
    for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
      FindLines.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    }
    //cout<<"T2: "<<Timer.GetElapsed()<<endl;
    FindLines.SetPeakParametrizationMethod(m_PeakParametrizationMethod);
    FindLines.SetPeakParametrizationMethodFittedPeakOptions(m_PeakParametrizationMethodFittedPeakBackgroundModel, m_PeakParametrizationMethodFittedPeakEnergyLossModel, m_PeakParametrizationMethodFittedPeakPeakShapeModel);
    //cout<<"T3: "<<Timer.GetElapsed()<<endl;
    
    if (FindLines.Calibrate() == false) {
      cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
      g_Verbosity = Verbosity;
      return false;
    }
    //cout<<"T4: "<<Timer.GetElapsed()<<endl;
    m_TimeToFindLines += Timer.GetElapsed();
    
    
    // Step 2: Assign the energies
    Timer.Reset();
    MCalibrateEnergyAssignEnergies AssignEnergies;
    AssignEnergies.SetMode(m_CalibrationModelEnergyAssignmentMethod);
    AssignEnergies.SetDiagnosticsMode(ShowDiagnostics);
    AssignEnergies.SetRange(m_HistogramMin, m_HistogramMax);
    for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
      //AssignEnergies.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
      AssignEnergies.AddIsotopes(m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    }
    AssignEnergies.SetCalibrationResult(FindLines.GetCalibrationResult());
    
    if (AssignEnergies.Calibrate() == false) {
      cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
      g_Verbosity = Verbosity;
      return false;
    }
    m_TimeToAssignEnergies += Timer.GetElapsed();
    
    
    // Step 3: Determine model
    Timer.Reset();
    MCalibrateEnergyDetermineModel DetermineModel;
    DetermineModel.SetDiagnosticsMode(ShowDiagnostics);
    DetermineModel.SetRange(m_HistogramMin, m_HistogramMax);
    for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
      //AssignEnergies.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
      AssignEnergies.AddIsotopes(m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    }
    DetermineModel.SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod);
    DetermineModel.SetCalibrationModelDeterminationMethodFittingEnergyOptions(m_CalibrationModelDeterminationMethodFittingEnergyModel);
    DetermineModel.SetCalibrationModelDeterminationMethodFittingFWHMOptions(m_CalibrationModelDeterminationMethodFittingFWHMModel);
    DetermineModel.SetCalibrationResult(AssignEnergies.GetCalibrationResult());
    
    if (DetermineModel.Calibrate() == false) {
      cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
      g_Verbosity = Verbosity;
      return false;
    }
    m_TimeToDetermineModel += Timer.GetElapsed();
    
    
    
    // Step 4: Set the result in the store
    TThread::Lock(); // <-- this function can be called from the thread so we have to protect it!
    m_CalibrationStore.Add(C.GetReadOutElement(), DetermineModel.GetCalibrationResult());
    TThread::UnLock();
    
    g_Verbosity = Verbosity;
  } catch (bad_alloc&) {
    ProcInfo_t Info;
    gSystem->GetProcInfo(&Info);
    cout<<endl;
    cout<<"We are out of memory... "<<endl;
    cout<<"Melinator used "<<(double) (Info.fMemResident)/1024/1024<<"GB before the crash"<<endl;
    cout<<endl;
    abort();
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Given an existing calibration for the collection, do a reassignment of energies and determination of the model
bool MMelinator::ReCalibrateModel(unsigned int Collection)
{
  MReadOutCollection& C = GetCollection(Collection);

  MCalibrationSpectrum* CS = dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(Collection))); 
  
  // Step A: Assign the energies
  MCalibrateEnergyAssignEnergies AssignEnergies;
  AssignEnergies.SetMode(m_CalibrationModelEnergyAssignmentMethod);
  AssignEnergies.SetRange(m_HistogramMin, m_HistogramMax);
  for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
    //AssignEnergies.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    AssignEnergies.AddIsotopes(m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
  }
  AssignEnergies.SetCalibrationResult(*CS);
  
  if (AssignEnergies.Calibrate() == false) {
    cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
    m_CalibrationStore.Remove(C.GetReadOutElement());
    return false;
  }
  
  
  // Step B: Determine model
  MCalibrateEnergyDetermineModel DetermineModel;
  DetermineModel.SetRange(m_HistogramMin, m_HistogramMax);
  for (unsigned int g = 0; g < C.GetNumberOfReadOutDataGroups(); ++g) {
    //AssignEnergies.AddReadOutDataGroup(C.GetReadOutDataGroup(g), m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
    AssignEnergies.AddIsotopes(m_Isotopes[distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g))]);
  }
  DetermineModel.SetCalibrationModelDeterminationMethod(m_CalibrationModelDeterminationMethod);
  DetermineModel.SetCalibrationModelDeterminationMethodFittingEnergyOptions(m_CalibrationModelDeterminationMethodFittingEnergyModel);
  DetermineModel.SetCalibrationModelDeterminationMethodFittingFWHMOptions(m_CalibrationModelDeterminationMethodFittingFWHMModel);
  DetermineModel.SetCalibrationResult(AssignEnergies.GetCalibrationResult());
  
  if (DetermineModel.Calibrate() == false) {
    cout<<"Calibration failed for read-out element "<<C.GetReadOutElement().ToString()<<endl;
    m_CalibrationStore.Remove(C.GetReadOutElement());
    return false;
  }
  
  
  m_CalibrationStore.Add(C.GetReadOutElement(), DetermineModel.GetCalibrationResult());
  
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
  out<<"# Used files and isotopes:"<<endl;
  for (unsigned int f = 0; f < m_CalibrationFileNames.size(); ++f) {
    out<<"# File: \""<<m_CalibrationFileNames[f]<<"\" with isotopes: ";
    for (unsigned int i = 0; i < m_Isotopes[f].size(); ++i) {
      if (i != 0) out<<", ";
      out<<m_Isotopes[f][i].ToString();
    }
    out<<endl;
  }
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
      out<<endl;
      out<<"# ROU: "<<ROE.ToParsableString(true)<<endl;
      out<<"# "<<C->ToParsableString("peakparametrization", true)<<endl;
      out<<"CP "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString("pakw", true)<<endl;
      if (C->HasEnergyModel() == true) {
        out<<"CM "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString("energymodel", true)<<endl;
      }
      if (C->HasFWHMModel() == true) {
        out<<"CR "<<ROE.ToParsableString(true)<<" "<<C->ToParsableString("fwhmmodel", true)<<endl;
      }
    }
  }
  
  out.close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a calibration report
bool MMelinator::CreateReport()
{
  // Create a calibration report

  double MaxADCs = 5000;
  double MaxEnergy = 500;
  MString OutputFileName = "MelinatorReport.pdf";


  // Section 1: Create histograms

  TH2D* AllFittedEnergies = new TH2D("Fitted", "Fitted", MaxEnergy/2, 0, MaxEnergy, m_CalibrationStore.GetNumberOfElements()+1, -0.5, m_CalibrationStore.GetNumberOfElements() + 0.5);
  AllFittedEnergies->SetXTitle("Energy [keV]");
  AllFittedEnergies->SetYTitle("ReadOutUnit ID");
  AllFittedEnergies->SetContour(99);


  TH2D* AllRawADCs = new TH2D("RawADCs", "RawADCs", MaxADCs/4, 0, MaxADCs, m_CalibrationStore.GetNumberOfElements()+1, -0.5, m_CalibrationStore.GetNumberOfElements() + 0.5);
  AllRawADCs->SetXTitle("ADCs [rou's]");
  AllRawADCs->SetYTitle("ReadOutUnit ID");
  AllRawADCs->SetContour(99);


  // If we have double-sided strips make one histogram per detector and side
  map<pair<unsigned int, bool>, MString> TitlesMap;
  map<pair<unsigned int, bool>, TH2D*> RawADCsMap;
  map<pair<unsigned int, bool>, TH2D*> FittedEnergiesMap;
  // Outer map: detector ID, side; inner map: rog, isotope
  map<pair<unsigned int, bool>, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>> FWHMesMap;
  map<pair<unsigned int, bool>, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>> ResidualsMap;

  if (m_CalibrationStore.GetReadOutElement(0).IsOfType("doublesidedstrip") == true) {
    // Step 1: Maximum strip ID
    unsigned int MaxStripID = 0;
    for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
      MReadOutElement& ROE = m_CalibrationStore.GetReadOutElement(c);
      if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
        MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
        if (ROE_DS.GetStripID() > MaxStripID) {
          MaxStripID = ROE_DS.GetStripID();
        }
      }
    }
    // Step 2: Create histograms
    for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
      MReadOutElement& ROE = m_CalibrationStore.GetReadOutElement(c);
      if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
        MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
        pair<unsigned int, bool> DetectorAndSide = { ROE_DS.GetDetectorID(), ROE_DS.IsLowVoltageStrip() };
        auto MapIter = RawADCsMap.find(DetectorAndSide);
        if (MapIter == RawADCsMap.end()) {
          MString Name = "Detector ";
          Name += ROE_DS.GetDetectorID();
          if (ROE_DS.IsLowVoltageStrip() == true) {
            Name += ", low voltage side";
          } else {
            Name += ", high voltage side";
          }

          TitlesMap[DetectorAndSide] = Name;

          RawADCsMap[DetectorAndSide] = new TH2D("", "ADC spectrum", MaxADCs/2, 0, MaxADCs, MaxStripID+1, -0.5, MaxStripID + 0.5);
          RawADCsMap[DetectorAndSide]->SetXTitle("ADC units");
          RawADCsMap[DetectorAndSide]->SetYTitle("Strip ID");
          RawADCsMap[DetectorAndSide]->SetContour(99);

          FittedEnergiesMap[DetectorAndSide] = new TH2D("", "Energy spectrum", MaxEnergy/2, 0, MaxEnergy, MaxStripID+1, -0.5, MaxStripID + 0.5);
          FittedEnergiesMap[DetectorAndSide]->SetXTitle("Energy [keV]");
          FittedEnergiesMap[DetectorAndSide]->SetYTitle("Strip ID");
          FittedEnergiesMap[DetectorAndSide]->SetContour(99);
        }

        MCalibration& C = m_CalibrationStore.GetCalibration(c);
        MCalibrationSpectrum& CS = static_cast<MCalibrationSpectrum&>(C);
        for (unsigned int g = 0; g < CS.GetNumberOfReadOutDataGroups(); ++g) {
          for (unsigned int i = 0; i < m_Isotopes[g].size(); ++i) {
            for (unsigned int l = 0; l < m_Isotopes[g][i].GetNLines(); ++l) {
              if (m_Isotopes[g][i].GetPrimaryLine() == l || m_Isotopes[g][i].GetSecondaryLine() == l)  {
                tuple<unsigned int, unsigned int, unsigned int> ROGIDAndIsotope = { g, i, l };
                // Inefficiency: we loop over everything... but avoiding it is a complicated optimization
                // Use try_emplace to avoid that
                FWHMesMap.try_emplace(DetectorAndSide, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>());
                FWHMesMap[DetectorAndSide].try_emplace(ROGIDAndIsotope, new TH1D("", "FWHMes", MaxStripID+1, -0.5, MaxStripID + 0.5));
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetXTitle("Strip ID");
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetYTitle("FWHM [keV]");
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetLineWidth(2);

                ResidualsMap.try_emplace(DetectorAndSide, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>());
                ResidualsMap[DetectorAndSide].try_emplace(ROGIDAndIsotope, new TH1D("", "Residuals", MaxStripID+1, -0.5, MaxStripID + 0.5));
                ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->SetXTitle("Strip ID");
                ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->SetYTitle("Residuals [keV]");
                ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->SetLineWidth(2);
              }
            }
          }
        }
      } // is // double-sided strip in calibration store
    } // ROE in calibration store
  } // double-sided strip



  // Fill the plots

  for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
    MReadOutElement& ROE = m_CalibrationStore.GetReadOutElement(c);
    TH2D* CurrentHistEnergy = nullptr;
    TH2D* CurrentHistADCs = nullptr;
    unsigned int CurrentStripID = 0;
    if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
      MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
      pair<unsigned int, bool> DetectorAndSide = { ROE_DS.GetDetectorID(), ROE_DS.IsLowVoltageStrip() };
      CurrentHistEnergy = FittedEnergiesMap[DetectorAndSide];
      CurrentHistADCs = RawADCsMap[DetectorAndSide];
      CurrentStripID = ROE_DS.GetStripID();
    }
    MCalibrationSpectrum& CS = dynamic_cast<MCalibrationSpectrum&>(m_CalibrationStore.GetCalibration(c));
    MCalibrationModel& M = CS.GetEnergyModel();

    unsigned int IDofROC = m_Store.FindReadOutCollection(ROE);
    if (IDofROC == g_UnsignedIntNotDefined) continue;
    MReadOutCollection& ROC = m_Store.GetReadOutCollection(IDofROC);
    for (unsigned int g = 0; g < ROC.GetNumberOfReadOutDataGroups(); ++g) {
      MReadOutDataGroup& ROG = ROC.GetReadOutDataGroup(g);
      for (unsigned int d = 0; d < ROG.GetNumberOfReadOutDatas(); ++d) {
        MReadOutData& ROD = ROG.GetReadOutData(d);
        MReadOutDataADCValue* ROD_ADC = dynamic_cast<MReadOutDataADCValue*>(ROD.Get(MReadOutDataADCValue::m_TypeID));
        if (ROD_ADC != nullptr) {
          double ADCs = ROD_ADC->GetADCValue();
          double Energy = M.GetFitValue(ADCs);
          AllFittedEnergies->Fill(Energy, c);
          AllRawADCs->Fill(ADCs, c);
          if (CurrentHistEnergy != nullptr) {
            CurrentHistEnergy->Fill(Energy, CurrentStripID);
            CurrentHistADCs->Fill(ADCs, CurrentStripID);
          }
        }
      }
    }

    if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
      MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
      pair<unsigned int, bool> DetectorAndSide = { ROE_DS.GetDetectorID(), ROE_DS.IsLowVoltageStrip() };

      for (unsigned int g = 0; g < CS.GetNumberOfReadOutDataGroups(); ++g) {
        for (unsigned int i = 0; i < m_Isotopes[g].size(); ++i) {
          for (unsigned int l = 0; l < m_Isotopes[g][i].GetNLines(); ++l) {
            if (m_Isotopes[g][i].GetPrimaryLine() == l || m_Isotopes[g][i].GetSecondaryLine() == l)  {
              tuple<unsigned int, unsigned int, unsigned int> ROGIDAndIsotope = { g, i, l };

              // Loop over all spectral point, and find the isotope and the line
              for (unsigned int ii = 0; ii < CS.GetNumberOfSpectralPoints(g); ++ii) {
                MCalibrationSpectralPoint& SP = CS.GetSpectralPoint(g, ii);
                if (SP.GetIsotope() == m_Isotopes[g][i]) {
                  if (fabs(SP.GetEnergy() - m_Isotopes[g][i].GetLineEnergy(l)) < 0.01 && SP.HasFit() == true) {
                    FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetTitle(MString("FWHM per strip for ") + m_Isotopes[g][i].GetName() + ", " + SP.GetEnergy() + " keV line"); // I know it is set multiple times...
                    FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->Fill(CurrentStripID, SP.GetEnergyFWHM());
                    ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->SetTitle(MString("Residuals line/fit per strip for ") + m_Isotopes[g][i].GetName() + ", " + SP.GetEnergy() + " keV line"); // I know it is set multiple times...
                    double Residual = SP.GetEnergy() - M.GetFitValue(SP.GetPeak());
                    ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->Fill(CurrentStripID, Residual);
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  // Create the canvases

  gROOT->SetBatch(true);

  bool FoundHist = false;
  auto IterTitles = TitlesMap.begin();
  auto IterEnergy = FittedEnergiesMap.begin();
  auto IterADCs = RawADCsMap.begin();
  auto IterFWHMes = FWHMesMap.begin();
  auto IterResiduals = ResidualsMap.begin();

  TCanvas* TitleCanvas = new TCanvas("Melinator Calibration Report", "Melinator Calibration Report", 850, 1100);
  TitleCanvas->cd();

  TPaveText* Title = new TPaveText(0.1, 0.75, 0.9, 0.9, "NDC");
  Title->AddText("Melinator Calibration Report");
  Title->SetTextFont(62);
  Title->SetTextSize(0.05);
  Title->SetFillColor(0);
  Title->SetBorderSize(0);
  Title->SetLineColor(0);
  Title->Draw();



  // Align text using TLatex
  float yStart = 0.70;
  float yStep = 0.03;
  float xLabel = 0.195;
  float xValue = 0.39;
  unsigned int CurrentStep = 0;

  TLatex* LatexLabel;
  TLatex* LatexValue;

  unsigned int TextFont = 42;
  double TextSize = 0.02;


  TString UserName = "";

  struct passwd *pw = getpwuid(getuid());
  if (pw && pw->pw_gecos && strlen(pw->pw_gecos) > 0) {
    // Some systems use comma-separated GECOS fields: "Full Name,Room,Work Phone,..."
    UserName = pw->pw_gecos;
    Ssiz_t commaPos = UserName.First(',');
    if (commaPos != kNPOS) {
      UserName.Remove(commaPos);  // Only keep the full name
    }
  }

  // Fall backs:
  if (UserName.IsNull()) UserName = gSystem->Getenv("USER");
  if (UserName.IsNull()) UserName = gSystem->Getenv("LOGNAME");
  if (UserName.IsNull()) UserName = "unknown";

  LatexLabel = new TLatex(xLabel, yStart - CurrentStep*yStep, "Created by:");
  LatexLabel->SetTextFont(TextFont); LatexLabel->SetTextSize(TextSize); LatexLabel->Draw();
  LatexValue = new TLatex(xValue, yStart - CurrentStep*yStep, UserName.Data());
  LatexValue->SetTextFont(TextFont); LatexValue->SetTextSize(TextSize); LatexValue->Draw();
  CurrentStep++;

  TDatime Now;
  TString DateTime = Form("%04d-%02d-%02d %02d:%02d:%02d", Now.GetYear(), Now.GetMonth(), Now.GetDay(),  Now.GetHour(), Now.GetMinute(), Now.GetSecond());

  LatexLabel = new TLatex(xLabel, yStart - CurrentStep*yStep, "Created on:");
  LatexLabel->SetTextFont(TextFont); LatexLabel->SetTextSize(TextSize); LatexLabel->Draw();
  LatexValue = new TLatex(xValue, yStart - CurrentStep*yStep, DateTime);
  LatexValue->SetTextFont(TextFont); LatexValue->SetTextSize(TextSize); LatexValue->Draw();
  CurrentStep++;


  TString GitHash = gSystem->GetFromPipe("cd $MEGALIB; git rev-parse --short HEAD");
  if (GitHash.IsNull()) GitHash = "unknown";

  LatexLabel = new TLatex(xLabel, yStart - CurrentStep*yStep, "MEGAlib git commit:");
  LatexLabel->SetTextFont(TextFont); LatexLabel->SetTextSize(TextSize); LatexLabel->Draw();
  LatexValue = new TLatex(xValue, yStart - CurrentStep*yStep, GitHash.Data());
  LatexValue->SetTextFont(TextFont); LatexValue->SetTextSize(TextSize); LatexValue->Draw();
  CurrentStep++;


  LatexLabel = new TLatex(xLabel, yStart - CurrentStep*yStep, "Data sets:");
  LatexLabel->SetTextFont(TextFont); LatexLabel->SetTextSize(TextSize); LatexLabel->Draw();

  for (unsigned int f = 0; f < m_CalibrationFileNames.size(); ++f) {
    MString FileName = m_CalibrationFileNames[f];
    FileName = MFile::GetBaseName(FileName);
    FileName += " (";
    for (unsigned int i = 0; i < m_Isotopes[f].size(); ++i) {
      FileName += m_Isotopes[f][i].GetName();
      if (i < m_Isotopes[f].size() - 1) {
        FileName += ", ";
      }
    }
    FileName += ")";

    LatexValue = new TLatex(xValue, yStart - CurrentStep*yStep, FileName.Data());
    LatexValue->SetTextFont(TextFont); LatexValue->SetTextSize(TextSize); LatexValue->Draw();
    CurrentStep++;
  }


  TitleCanvas->Print(OutputFileName + "(");


  while (IterTitles != TitlesMap.end() && IterEnergy != FittedEnergiesMap.end() && IterADCs != RawADCsMap.end() && IterFWHMes != FWHMesMap.end() && IterResiduals != ResidualsMap.end()) {

    // Page 1: Spectra

    TCanvas* SpectrumPages = new TCanvas("", "", 850, 1100);
    SpectrumPages->SetLogz();
    SpectrumPages->cd();

    TPad* TitlePad = new TPad("padTitle", "Title", 0.0, 0.9, 1.0, 1.0);
    TitlePad->Draw();
    TitlePad->cd();

    TPaveText* Title = new TPaveText(0.0, 0.0, 1.0, 1.0, "brNDC");
    Title->AddText(IterTitles->second);
    Title->SetFillColor(0);     // No background color
    Title->SetBorderSize(0);    // No border
    Title->SetLineColor(0);     // Remove outline
    Title->SetTextFont(TextFont);
    Title->SetTextSize(0.4);
    Title->Draw();

    SpectrumPages->cd();

    TPad* ADCPad = new TPad("ADCPad", "ADCPad", 0.0, 0.45, 1.0, 0.9);
    ADCPad->Draw();
    ADCPad->cd();
    ADCPad->SetLogz();
    IterADCs->second->Draw("colz");

    SpectrumPages->cd();

    TPad* EnergyPad = new TPad("EnergyPad", "EnergyPad", 0.0, 0.0, 1.0, 0.45);
    EnergyPad->Draw();
    EnergyPad->cd();
    EnergyPad->SetLogz();
    IterEnergy->second->Draw("colz");

    SpectrumPages->cd();
    SpectrumPages->Update();

    SpectrumPages->Print(OutputFileName);   // middle pages


    // Page 2+ is lines:

    auto IterFWHMesInner = IterFWHMes->second.begin();
    auto IterResidualsInner = IterResiduals->second.begin();
    while (IterResidualsInner != IterResiduals->second.end() && IterFWHMesInner != IterFWHMes->second.end()) {

      TCanvas* LineFitPages = new TCanvas("", "", 850, 1100);
      LineFitPages->cd();

      TPad* TitlePadLineFitPages = new TPad("padTitle", "Title", 0.0, 0.9, 1.0, 1.0);
      TitlePadLineFitPages->Draw();
      TitlePadLineFitPages->cd();

      TPaveText* TitleLineFitPages = new TPaveText(0.0, 0.0, 1.0, 1.0, "brNDC");
      TitleLineFitPages->AddText(IterTitles->second);
      TitleLineFitPages->SetFillColor(0);     // No background color
      TitleLineFitPages->SetBorderSize(0);    // No border
      TitleLineFitPages->SetLineColor(0);     // Remove outline
      TitleLineFitPages->SetTextFont(TextFont);
      TitleLineFitPages->SetTextSize(0.4);
      TitleLineFitPages->Draw();

      LineFitPages->cd();


      TPad* FWHMPad = new TPad("", "", 0.0, 0.45, 1.0, 0.9);
      FWHMPad->Draw();
      FWHMPad->cd();
      IterFWHMesInner->second->Draw("HIST ");

      LineFitPages->cd();

      TPad* ResidualsPad = new TPad("", "", 0.0, 0.0, 1.0, 0.45);
      ResidualsPad->Draw();
      ResidualsPad->cd();
      IterResidualsInner->second->Draw("HIST ");

      LineFitPages->cd();
      LineFitPages->Update();

      if (next(IterFWHMesInner) == IterFWHMes->second.end() && next(IterTitles) == TitlesMap.end()) {
        LineFitPages->Print(OutputFileName + ")");  // close PDF
      } else {
        LineFitPages->Print(OutputFileName);   // middle pages
      }

      ++IterFWHMesInner;
      ++IterResidualsInner;
    }

    FoundHist = true;

    ++IterTitles;
    ++IterEnergy;
    ++IterADCs;
    ++IterFWHMes;
    ++IterResiduals;
  }

  gROOT->SetBatch(false);


  int Return = gSystem->Exec(MString("") + "[ -x \"$(command -v xdg-open)\" ] && xdg-open " + OutputFileName + " || { [ -x \"$(command -v open)\" ] && open " + OutputFileName + "; }");

  if (Return != 0) {
    mgui<<"Failed to open PDF. Return code: "<<Return<<error;
  }

  if (FoundHist == false) {
    TCanvas* C = new TCanvas();
    C->SetLogz();
    C->cd();
    AllFittedEnergies->Draw("colz");
    C->Update();
  }


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
