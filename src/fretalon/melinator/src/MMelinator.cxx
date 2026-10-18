/*
 * MMelinator.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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
#include <cstring>
#include <sys/stat.h>
#include <iomanip>
using namespace std;

// Standard libs:
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <thread>
#include <cstring>
#include <zlib.h>
#include <cstdio>
#include <fstream>
#include <string>
#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#endif

//! Set as soon as any loading thread detects that the system is close to out of memory
static std::atomic<bool> g_MelinatorOutOfMemory(false);

//! One block of raw text from a calibration file, cut at the beginning of an event, which is passed from the reader to the parser threads
struct MelinatorBlock
{
  //! The text (owned by the pipeline's buffer pool)
  char* Data = nullptr;
  //! The number of valid bytes
  size_t Size = 0;
  //! The index of the file in the list of calibration files
  unsigned int FileID = 0;
  //! The progress in the file (in bytes) which is completed when this block is parsed
  double Progress = 0.0;
};

//! The pipeline between the one reader thread and the many parser threads
struct MelinatorPipeline
{
  std::mutex Mutex;
  //! Parsers wait on this for new blocks
  std::condition_variable BlockAvailable;
  //! The reader waits on this for free buffers
  std::condition_variable BufferAvailable;
  //! Blocks filled by the reader, waiting for a parser
  std::deque<MelinatorBlock> Queue;
  //! Unused buffers
  std::vector<char*> FreeBuffers;
  //! All buffers allocated so far
  unsigned int NAllocated = 0;
  //! Maximum number of buffers (= maximum number of blocks in flight)
  unsigned int MaxBuffers = 0;
  //! Size of the block (without the slack)
  size_t BlockSize = 0;
  //! The number of reader threads which have not yet finished
  unsigned int ReadersRunning = 0;
  //! All readers have finished (or have stopped)
  bool ReaderDone = false;
  //! Everyone should stop
  bool Stop = false;
  
  //! Free everything and prepare for a new load
  void Reset(size_t NewBlockSize, unsigned int NewMaxBuffers, unsigned int NewReaders) {
    std::lock_guard<std::mutex> Lock(Mutex);
    for (char* B: FreeBuffers) delete [] B;
    FreeBuffers.clear();
    for (MelinatorBlock& B: Queue) delete [] B.Data;
    Queue.clear();
    NAllocated = 0;
    MaxBuffers = NewMaxBuffers;
    BlockSize = NewBlockSize;
    ReadersRunning = NewReaders;
    ReaderDone = (NewReaders == 0);
    Stop = false;
  }
};

static MelinatorPipeline g_MelinatorPipeline;

//! One task of a reader thread: Read the events of a part of one file
//! The part contains all events whose "SE" line starts in [Start, End) --- End = 0 means until the end of the file
struct MelinatorReadTask
{
  unsigned int FileID = 0;
  long long Start = 0;
  long long End = 0;
};

//! All reader tasks of the current load (one reader thread per task)
static std::vector<MelinatorReadTask> g_MelinatorReadTasks;

//! Bytes of the buffer behind the block size, to hold the beginning of the next event
static const size_t g_MelinatorBlockSlack = 1024*1024;

//! Only one thread at a time is allowed to open a file (the opening of read-out files is not thread-safe)
static std::mutex g_MelinatorOpenMutex;

// ROOT libs:
#include "TROOT.h"
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
#include "TVirtualX.h"

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


//! Draw canvases off-screen while this object exists, and restore the previous state afterwards
//! gROOT->SetBatch(true) alone is no longer enough in an interactive session: Since ROOT 6.40, canvases created in batch mode
//! still paint via the X11 backend (gVirtualX) without a window, and crash --- thus also switch gVirtualX to the batch backend
class MMelinatorBatchDrawing
{
public:
  MMelinatorBatchDrawing() : m_WasBatch(gROOT->IsBatch()), m_VirtualX(gVirtualX) {
    gROOT->SetBatch(true);
    if (gGXBatch != nullptr) gVirtualX = gGXBatch;
  }
  ~MMelinatorBatchDrawing() {
    gVirtualX = m_VirtualX;
    gROOT->SetBatch(m_WasBatch);
  }
private:
  //! The batch mode before
  bool m_WasBatch;
  //! The graphics backend before
  TVirtualX* m_VirtualX;
};


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
  
  m_ReportFileName = "MelinatorReport.pdf";

  m_ReportADCHistogramMinimum = 0;
  m_ReportADCHistogramMaximum = 10000;
  m_ReportADCHistogramBins = 1000;

  m_ReportEnergyHistogramMinimum = 0;
  m_ReportEnergyHistogramMaximum = 2000;
  m_ReportEnergyHistogramBins = 500;

  m_ReportThresholdHistogramMinimum = 0;
  m_ReportThresholdHistogramMaximum = 40;
  m_ReportThresholdHistogramBins = 40;

  m_ReportOverflowHistogramMinimum = 1500;
  m_ReportOverflowHistogramMaximum = 2500;
  m_ReportOverflowHistogramBins = 100;

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
  
  //m_SelectedDetectorID = -1;
  // ADDED
  m_SelectedDetectorIDs.clear();
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
  
  // All files are read in parallel: One reader thread per file, and additional readers for the larger files, 
  // which read different parts of the same file, so that all readers have about the same amount of work.
  // As many parser threads as we have cores (the readers are mostly waiting for the disk)
  unsigned int NCores = std::thread::hardware_concurrency();
  if (NCores < 2) NCores = 2;
  
  g_MelinatorReadTasks.clear();
  {
    // Only uncompressed files can be split
    vector<long long> Sizes(m_CalibrationFileNames.size(), 0);
    long long TotalSize = 0;
    for (unsigned int f = 0; f < m_CalibrationFileNames.size(); ++f) {
      struct stat Info;
      if (stat(m_CalibrationFileNames[f].Data(), &Info) == 0) Sizes[f] = (long long) Info.st_size;
      TotalSize += Sizes[f];
    }
    // Aim for roughly NCores/2 readers' worth of work, but never parts smaller than 1 GB
    const long long PartSize = std::max(1LL << 30, TotalSize / (long long) std::max(1U, NCores/2));
    for (unsigned int f = 0; f < m_CalibrationFileNames.size(); ++f) {
      const bool Zipped = m_CalibrationFileNames[f].EndsWith(".gz");
      long long NParts = Zipped ? 1 : std::max(1LL, (Sizes[f] + PartSize/2) / PartSize);
      for (long long p = 0; p < NParts; ++p) {
        MelinatorReadTask Task;
        Task.FileID = f;
        Task.Start = (NParts == 1) ? 0 : Sizes[f] * p / NParts;
        Task.End = (NParts == 1 || p == NParts-1) ? 0 : Sizes[f] * (p+1) / NParts;
        g_MelinatorReadTasks.push_back(Task);
      }
    }
  }
  const unsigned int NReaders = g_MelinatorReadTasks.size();
  const unsigned int NParsers = NCores;
  unsigned int NThreads = (NReaders > 0) ? NReaders + NParsers : 0;
  if (g_Verbosity >= c_Info) cout<<"Loading with "<<NReaders<<" reader threads and "<<NParsers<<" parser threads"<<endl;
  // 32 MB blocks, and a few more buffers than threads, so that the readers are always ahead
  g_MelinatorPipeline.Reset(32*1024*1024, NReaders + NParsers + 3, NReaders);

  g_MelinatorOutOfMemory = false;
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


//! Get the total and the available (free + easily reclaimable) system memory in GB
//! Returns false if this cannot be determined on this platform
static bool MelinatorGetMemory(double& TotalGB, double& AvailableGB)
{
#if defined(__APPLE__)
  int64_t Total = 0;
  size_t Size = sizeof(Total);
  if (sysctlbyname("hw.memsize", &Total, &Size, nullptr, 0) != 0 || Total <= 0) return false;

  vm_size_t PageSize = 0;
  if (host_page_size(mach_host_self(), &PageSize) != KERN_SUCCESS) return false;
  vm_statistics64_data_t VM;
  mach_msg_type_number_t Count = HOST_VM_INFO64_COUNT;
  if (host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t) &VM, &Count) != KERN_SUCCESS) return false;

  // Free + inactive (file cache) + purgeable pages can be reclaimed without swapping; speculative pages are included in free
  double AvailableBytes = (double) (VM.free_count + VM.inactive_count + VM.purgeable_count) * (double) PageSize;
  TotalGB = (double) Total/1024.0/1024.0/1024.0;
  AvailableGB = AvailableBytes/1024.0/1024.0/1024.0;
  return true;
#elif defined(__linux__)
  std::ifstream In("/proc/meminfo");
  if (In.is_open() == false) return false;

  double TotalKB = 0.0;
  double AvailableKB = -1.0;
  std::string Key;
  double Value;
  std::string Rest;
  while (In >> Key >> Value) {
    std::getline(In, Rest);
    if (Key == "MemTotal:") TotalKB = Value;
    if (Key == "MemAvailable:") AvailableKB = Value;
    if (TotalKB > 0 && AvailableKB >= 0) break;
  }
  if (TotalKB <= 0 || AvailableKB < 0) return false;
  TotalGB = TotalKB/1024.0/1024.0;
  AvailableGB = AvailableKB/1024.0/1024.0;

  // On ZFS the ARC cache is NOT part of MemAvailable, even though it is given back under memory pressure:
  // Count everything above the minimum ARC size as available
  std::ifstream Arc("/proc/spl/kstat/zfs/arcstats");
  if (Arc.is_open() == true) {
    double ArcSize = -1.0;
    double ArcMin = -1.0;
    std::string ArcLine;
    while (std::getline(Arc, ArcLine)) {
      // Lines look like "<name> <type> <data>" - the two header lines do not match this and are skipped
      char Name[128];
      int Type;
      double Data;
      if (sscanf(ArcLine.c_str(), "%127s %d %lf", Name, &Type, &Data) == 3) {
        if (std::string(Name) == "size") ArcSize = Data;
        if (std::string(Name) == "c_min") ArcMin = Data;
        if (ArcSize >= 0 && ArcMin >= 0) break;
      }
    }
    if (ArcSize > ArcMin && ArcMin >= 0) {
      AvailableGB += (ArcSize - ArcMin)/1024.0/1024.0/1024.0;
    }
  }
  return true;
#else
  return false;
#endif
}

//! Return true if the memory available to the system is below the safety limit:
//! max(8 GB, 5% of the total memory). If the memory cannot be determined, return false
static bool MelinatorMemoryIsLow(double& AvailableGB, double& ThresholdGB)
{
  double TotalGB = 0.0;
  if (MelinatorGetMemory(TotalGB, AvailableGB) == false) return false;
  ThresholdGB = std::max(8.0, 0.05*TotalGB);
  return AvailableGB < ThresholdGB;
}

//! Get a free buffer from the pool --- blocks until one is available, returns nullptr if we should stop
static char* MelinatorAcquireBuffer()
{
  MelinatorPipeline& P = g_MelinatorPipeline;
  std::unique_lock<std::mutex> Lock(P.Mutex);
  while (true) {
    if (P.Stop == true) return nullptr;
    if (P.FreeBuffers.empty() == false) {
      char* B = P.FreeBuffers.back();
      P.FreeBuffers.pop_back();
      return B;
    }
    if (P.NAllocated < P.MaxBuffers) {
      ++P.NAllocated;
      Lock.unlock();
      return new char[P.BlockSize + g_MelinatorBlockSlack];
    }
    P.BufferAvailable.wait_for(Lock, std::chrono::milliseconds(100));
  }
}

//! Give a buffer back to the pool
static void MelinatorReleaseBuffer(char* Buffer)
{
  MelinatorPipeline& P = g_MelinatorPipeline;
  {
    std::lock_guard<std::mutex> Lock(P.Mutex);
    P.FreeBuffers.push_back(Buffer);
  }
  P.BufferAvailable.notify_one();
}

//! Hand a filled block to the parsers
static void MelinatorPushBlock(const MelinatorBlock& Block)
{
  MelinatorPipeline& P = g_MelinatorPipeline;
  {
    std::lock_guard<std::mutex> Lock(P.Mutex);
    P.Queue.push_back(Block);
  }
  P.BlockAvailable.notify_one();
}

//! Return true if there is a "\nSE" + (new line, return, or space) at index i (needs i+3 < Size)
static inline bool MelinatorIsEventStart(const char* Buffer, size_t i)
{
  return (Buffer[i] == '\n' && Buffer[i+1] == 'S' && Buffer[i+2] == 'E' && (Buffer[i+3] == '\n' || Buffer[i+3] == '\r' || Buffer[i+3] == ' '));
}

//! A reader thread: Read one part of a file in big blocks of raw text, cut each block at the start of an event, and hand it to the parser threads
//! Plain and gzip-compressed files are both supported (gzread handles both), but only plain files can be read in parts
static void MelinatorReadRange(const MString& FileName, const MelinatorReadTask& Task, const std::vector<bool>& ShouldTerminate)
{
  MelinatorPipeline& P = g_MelinatorPipeline;
  
  const bool Ranged = (Task.Start > 0 || Task.End > 0);
  
  gzFile File = gzopen(FileName.Data(), "rb");
  if (File == nullptr) {
    cout<<"ERROR: Unable to open file for reading: "<<FileName<<endl;
  } else {
    gzbuffer(File, 1024*1024);
    
    std::vector<char> Carry; // The beginning of the next event, which did not fit into the last block
    
    // Absolute file position of the first byte in the buffer (only used for ranges)
    long long AbsStart = 0; 
    bool Aligned = true;
    if (Task.Start > 0) {
      // Right after gzopen zlib does not yet know that the file is uncompressed, and gzseek would read and discard
      // everything up to the seek position. gzdirect makes zlib look at the file, after which gzseek is a real lseek
      gzdirect(File);
      // Start one byte early, so that we can see a new line right in front of Start
      AbsStart = Task.Start - 1;
      gzseek(File, (z_off_t) AbsStart, SEEK_SET);
      Aligned = false;
    }
    
    double LastOffset = 0.0;
    bool Finished = false;
    
    while (Finished == false) {
      // Stop on request, or if we are close to out of memory
      for (bool T: ShouldTerminate) {
        if (T == true) {
          std::lock_guard<std::mutex> Lock(P.Mutex);
          P.Stop = true;
        }
      }
      if (g_MelinatorOutOfMemory.load() == false) {
        double AvailableGB = 0.0;
        double ThresholdGB = 0.0;
        if (MelinatorMemoryIsLow(AvailableGB, ThresholdGB) == true) {
          if (g_MelinatorOutOfMemory.exchange(true) == false) {
            cout<<endl;
            cout<<"WARNING: Close to out of memory (available: "<<AvailableGB<<" GB, limit: "<<ThresholdGB<<" GB)"<<endl;
            cout<<"         Stopping to read more events in ALL files --> the loaded data is incomplete!"<<endl;
            cout<<endl;
          }
        }
      }
      if (g_MelinatorOutOfMemory.load() == true) break;
      
      char* Buffer = MelinatorAcquireBuffer();
      if (Buffer == nullptr) break;
      
      // Start with the left-overs of the last block, then fill up
      if (Carry.empty() == false) memcpy(Buffer, Carry.data(), Carry.size());
      const size_t CarrySize = Carry.size();
      Carry.clear();
      int NRead = gzread(File, Buffer + CarrySize, (unsigned int) P.BlockSize);
      if (NRead < 0) {
        cout<<"ERROR: Reading failed in file: "<<FileName<<endl;
        NRead = 0;
      }
      size_t Total = CarrySize + (size_t) NRead;
      const bool EndOfFile = (NRead < (int) P.BlockSize);
      
      // (1) The first block of a part in the middle of a file: skip everything up to the first event start 
      if (Aligned == false) {
        size_t Begin = 0;
        bool Found = false;
        for (size_t i = 0; i + 3 < Total; ++i) {
          if (MelinatorIsEventStart(Buffer, i)) {
            Begin = i + 1;
            Found = true;
            break;
          }
        }
        if (Found == false) {
          // Keep the last bytes, since an event start could be split by the block border
          const size_t Keep = (Total > 4) ? 4 : Total;
          Carry.assign(Buffer + Total - Keep, Buffer + Total);
          AbsStart += (long long) (Total - Keep);
          MelinatorReleaseBuffer(Buffer);
          if (EndOfFile == true) Finished = true;
          continue;
        }
        memmove(Buffer, Buffer + Begin, Total - Begin);
        Total -= Begin;
        AbsStart += (long long) Begin;
        Aligned = true;
      }
      
      // (2) Where do we cut?
      size_t Cut = Total;
      bool EndReached = EndOfFile; 
      if (Task.End > 0 && AbsStart >= Task.End) {
        // Already everything here belongs to the next part
        Cut = 0;
        EndReached = true;
      } else if (Task.End > 0 && AbsStart + (long long) Total > Task.End) {
        // The end of our part is in this block: Cut at the first event start at or behind the end
        EndReached = false;
        for (size_t i = (size_t) (Task.End - AbsStart - 1); i + 3 < Total; ++i) {
          if (MelinatorIsEventStart(Buffer, i)) {
            Cut = i + 1;
            EndReached = true;
            break;
          }
        }
      }
      
      if (EndReached == false && Total > 4) {
        // Cut in front of the last "SE" line, so that the block ends with a complete event
        // If we cannot find one (extremely long event), we just cut here --- individual events at the border don't matter
        for (size_t i = Total - 4; i > 0; --i) {
          if (MelinatorIsEventStart(Buffer, i)) {
            if (Total - (i+1) <= g_MelinatorBlockSlack) Cut = i + 1;
            break;
          }
        }
        Carry.assign(Buffer + Cut, Buffer + Total);
      }
      
      if (Cut > 0) {
        MelinatorBlock Block;
        Block.Data = Buffer;
        Block.Size = Cut;
        Block.FileID = Task.FileID;
        if (Ranged == true) {
          Block.Progress = (double) Cut;
        } else {
          const double Offset = (double) gzoffset(File);
          Block.Progress = Offset - LastOffset;
          LastOffset = Offset;
        }
        MelinatorPushBlock(Block);
        AbsStart += (long long) Cut;
      } else {
        MelinatorReleaseBuffer(Buffer);
      }
      
      if (EndReached == true) Finished = true;
    }
    
    gzclose(File);
  }
  
  // We are done
  {
    std::lock_guard<std::mutex> Lock(P.Mutex);
    if (P.ReadersRunning > 0) --P.ReadersRunning;
    if (P.ReadersRunning == 0) P.ReaderDone = true;
  }
  P.BlockAvailable.notify_all();
}


//! The parallel loading: The first threads are the readers (one per part of a file), all others are parsers
bool MMelinator::LoadParallel(unsigned int ThreadID)
{
  if (g_Verbosity >= c_Info) cout<<"Parallel loading thread #"<<ThreadID<<" has started"<<endl;
  
  TThread::Lock();
  m_ThreadIsInitialized[ThreadID] = true;
  TThread::UnLock();
  
  MelinatorPipeline& P = g_MelinatorPipeline;
  
  // We are likely to run out of memory so catch some bad_alloc exceptions
  try {
    if (ThreadID < g_MelinatorReadTasks.size()) {
      // ---- A reader: Each one reads one part of one file
      const MelinatorReadTask& Task = g_MelinatorReadTasks[ThreadID];
      MelinatorReadRange(m_CalibrationFileNames[Task.FileID], Task, m_ThreadShouldTerminate);
    } else {
      // ---- A parser
      
      // With exactly one detector, let the reader do the filtering (as before);
      // with several, read all detectors and remove the unwanted read-outs below
      const int ReaderDetectorID = (m_SelectedDetectorIDs.size() == 1) ? m_SelectedDetectorIDs[0] : -1;
      
      // One reader per file, since each file can declare different read-out units
      // Files are opened to read the read-out units, but closed again right away since we parse from memory
      map<unsigned int, MFileReadOuts*> Readers;
      
      MReadOutStore Store;
      Store.AddReadOutDataGroup("Dummy");
      MReadOutSequence Sequence;
      
      while (true) {
        MelinatorBlock Block;
        {
          std::unique_lock<std::mutex> Lock(P.Mutex);
          while (P.Queue.empty() == true && P.ReaderDone == false && P.Stop == false) {
            P.BlockAvailable.wait_for(Lock, std::chrono::milliseconds(100));
          }
          if (P.Queue.empty() == true) break; // Reader is done or we should stop
          Block = P.Queue.front();
          P.Queue.pop_front();
        }
        
        if (m_ThreadShouldTerminate[ThreadID] == true) {
          MelinatorReleaseBuffer(Block.Data);
          continue;
        }
        
        MFileReadOuts*& Reader = Readers[Block.FileID];
        if (Reader == nullptr) {
          Reader = new MFileReadOuts();
          bool Opened = false;
          {
            std::lock_guard<std::mutex> Lock(g_MelinatorOpenMutex);
            Opened = Reader->Open(m_CalibrationFileNames[Block.FileID]);
          }
          if (Opened == false) {
            cout<<"ERROR: Unable to read the read-out units of file "<<m_CalibrationFileNames[Block.FileID]<<endl;
          }
          Reader->Close();
        }
        
        Reader->ResetEventState();
        const char* Cursor = Block.Data;
        const char* End = Block.Data + Block.Size;
        while (Reader->ReadNextFromMemory(Sequence, Cursor, End, ReaderDetectorID, m_SelectedDetectorSide) == true) {
          vector<unsigned int> ToRemove;
          for (unsigned int d = 0; d < Sequence.GetNumberOfReadOuts(); ++d) { 
            // ADDED
            if (m_SelectedDetectorIDs.size() > 1) {
              int DetID = (int) Sequence.GetReadOut(d).GetReadOutElement().GetDetectorID();
              if (find(m_SelectedDetectorIDs.begin(), m_SelectedDetectorIDs.end(), DetID) == m_SelectedDetectorIDs.end()) {
                ToRemove.push_back(d);
                continue;
              }
            }
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
        }
        
        MelinatorReleaseBuffer(Block.Data);
        
        // Hand the parsed data over to the main store
        TThread::Lock();
        m_Store.Move(Store, m_GroupIDs[Block.FileID]);
        m_CalibrationFileLoadingProgress[Block.FileID] += Block.Progress;
        TThread::UnLock();
      }
      
      for (auto R: Readers) delete R.second;
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
  
  TThread::Lock();
  m_ThreadIsFinished[ThreadID] = true;
  TThread::UnLock();
  
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
  Canvas.SetTopMargin(0.10);
  Canvas.SetBottomMargin(0.14);
  

  if (m_HistogramChanged == true) {

    for (auto H: m_Histograms) {
      delete H;
    }
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

  // The first histogram which exists creates the frame with the axes --- this is not necessarily histogram 0,
  // since a read-out data group without any counts in the range has no histogram
  bool FirstHistogram = true;
  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    if (m_Histograms[h] == nullptr) continue;
    
    if (FirstHistogram == true) {
      m_Histograms[h]->DrawCopy("HIST");
      FirstHistogram = false;
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
    if (m_Histograms[h] == nullptr) continue; // No histogram, no spectrum, nothing to mark
    
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
  TLegend* Legend = new TLegend(0.8, ySizeMin, 0.94, 0.88, NULL, "brNDC");
  Legend->SetHeader("Isotope list:");
  
  // Final draw so that everything is on top of each other:
  for (unsigned int h = 0; h < m_Histograms.size(); ++h) {
    if (m_Histograms[h] == nullptr) continue;
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
    if (m_Histograms[h] == nullptr) continue;
    m_Histograms[h]->DrawCopy("AXIS SAME");
    break;
  }

  if (m_Histograms.size() > 0) {
    MString SummaryText = MString("Spectrum for ") + C.GetReadOutElement().ToString();
    TLatex* Summary = new TLatex();
    Summary->SetNDC();  // Use NDC coordinates
    Summary->SetTextAlign(22);
    Summary->DrawLatex(0.5, 0.95, SummaryText);
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
  //Canvas.SetBit(kCannotPick);
  Canvas.Clear();
  Canvas.cd();
  Canvas.SetGridx();
  Canvas.SetGridy();
  //Canvas.SetLogy();

  Canvas.SetLeftMargin(0.17);
  Canvas.SetRightMargin(0.05);
  Canvas.SetTopMargin(0.10);
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
            Spectrum->SetMaximum(1.2*Spectrum->GetMaximum());
            
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
            
            TLine* Line = new TLine(P.GetPeak(), 0.85*Spectrum->GetMaximum(), P.GetPeak(), Spectrum->GetMinimum());
            Line->SetLineWidth(3);
            Line->Draw("SAME");

            if (P.HasFit() && P.IsGood()) {
              ostringstream FitQualityString;
              FitQualityString<<"#chi^{2}/ndf = "<<fixed<<setprecision(3)<<P.GetFit().GetReducedChisquare();
              TLatex* FitQualityLabel1 = new TLatex(P.GetLowEdge() + 0.05*(P.GetHighEdge() - P.GetLowEdge()), 0.935*Spectrum->GetMaximum(), FitQualityString.str().c_str());
              FitQualityLabel1->SetTextAlign(11);
              FitQualityLabel1->Draw("SAME");
              TLatex* FitQualityLabel2 = new TLatex(P.GetLowEdge() + 0.05*(P.GetHighEdge() - P.GetLowEdge()), 0.885*Spectrum->GetMaximum(), "(for unbinned fit)");
              FitQualityLabel2->SetTextSize(0.04);
              FitQualityLabel2->SetTextAlign(11);
              FitQualityLabel2->Draw("SAME");
            }

            ostringstream Text;
            if (P.IsGood()) {
              MIsotope I = P.GetIsotope();
              Text<<I.GetName()<<" ("<<fixed<<setprecision(1)<<P.GetEnergy()<<" keV at "<<fixed<<setprecision(1)<<P.GetPeak()<<" rou's)";
            } else {
              Text<<"Excluded line at "<<fixed<<setprecision(1)<<P.GetPeak()<<" rou's";
            }
            TLatex* Summary = new TLatex(0.5*(P.GetLowEdge() + P.GetHighEdge()), 1.05*Spectrum->GetMaximum(), Text.str().c_str());
            Summary->SetTextAlign(22);
            Summary->Draw("SAME");

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
  Canvas.SetTopMargin(0.10);
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
  
  // Check for the model we actually draw: A spectrum can have an energy model but no line-width model
  if (C != nullptr && ((UseEnergy == true) ? C->HasEnergyModel() : C->HasFWHMModel()) == true) {
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
    
    ostringstream Text;
    if (UseEnergy == true) {
      Text<<"Energy Model: "<<Model.GetName();
    } else {
      Text<<"FWHM Model: "<<Model.GetName();
    }
    TLatex* ModelName = new TLatex(0.5*(0 + MaximumX), 1.05*1.1*MaximumY, Text.str().c_str());
    ModelName->SetTextAlign(22);
    ModelName->Draw("SAME");

    ostringstream FitQualityString;
    FitQualityString<<"#chi^{2}/ndf = "<<fixed<<setprecision(3)<<Model.GetFitQuality();
    TLatex* FitQualityLabel = new TLatex(0.05*(0 + MaximumX), 1.025*MaximumY, FitQualityString.str().c_str());
    FitQualityLabel->SetTextAlign(11);
    FitQualityLabel->Draw("SAME");

  } else {
    ostringstream Text;
    if (UseEnergy == true) {
      Text<<"Energy Model: interpolation";
    } else {
      Text<<"FWHM Model: interpolation";
    }
    TLatex* ModelName = new TLatex(0.5*(0 + MaximumX), 1.05*Graph->GetMaximum(), Text.str().c_str());
    ModelName->SetTextAlign(22);
    ModelName->Draw("SAME");
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
bool MMelinator::SaveEcal(MString FileName)
{
  if (m_CalibrationStore.GetNumberOfElements() == 0) {
    mgui<<"You need to run the calibration first, before saving the results."<<error;
    return true; // True: No subsequent error messages required
  } else {
    bool FoundOne = false;
    for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
      if (dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(c))) != nullptr) {
        FoundOne = true;
        break;
      }
    }
    if (FoundOne == false) {
      mgui<<"You need to run the calibration first, before saving the results."<<error;
      return true; // True: No subsequent error messages required
    }
  }

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
  out<<endl;
  out<<"# Keyword description"<<endl;
  out<<"# CL - calibration points: the fit parameters plus uncertainty for each calibration point"<<endl;
  out<<"# CP - calibration points: the data points going into the calibration"<<endl;
  out<<"#    pawk = ADCs, Energy in keV, FWHM of energy in keV for each point"<<endl;
  out<<"# CM - calibration model energy: the energy calibration model"<<endl;
  out<<"# CR - calibration model fwhm: the FWHM calibration model"<<endl;
  out<<endl;
  out<<endl;
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
      vector<MCalibrationSpectralPoint> Points = C->GetUniquePoints();
      for (unsigned int s = 0; s < Points.size(); ++s) {
        out<<"CL "<<ROE.ToParsableString(true)<<"  "<<C->ToParsableString("peakparametrizationshort", false)<<"  "<<s<<" "<<Points[s].ToParsableString("param+error", true)<<endl;
      }
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


//! Create a ROOT file with all the histograms
bool MMelinator::SaveHistograms(MString FileName)
{
  TFile* File = new TFile(FileName, "RECREATE");

  MGUIProgressBar Progress("Status", "Status of histogram creation");
  Progress.SetMinMax(0, GetNumberOfCollections());

  for (unsigned int c = 0; c < GetNumberOfCollections(); ++c) {
    gSystem->ProcessEvents(); // For an unknown reason, I really need 2 ProcessEvents here...
    Progress.SetValue(c);
    gSystem->ProcessEvents(); // Added for smoother UI updates...
    if (Progress.TestCancel() == true) break;

    const MReadOutElement& ROE = GetCollection(c).GetReadOutElement();
    MString DetName = MString("Detector_") + ROE.GetDetectorID();
    MString SideName = "";
    MString StripName = "";
    if (ROE.IsOfType("singlessidedstrip") == true) {
      StripName = MString("Strip_") + static_cast<const MReadOutElementStrip&>(ROE).GetStripID();
    } else if (ROE.IsOfType("doublesidedstrip") == true) {
      SideName = MString("Side_") + (static_cast<const MReadOutElementDoubleStrip&>(ROE).IsLowVoltageStrip() ? "LV" : "HV");
      StripName = MString("Strip_") + static_cast<const MReadOutElementDoubleStrip&>(ROE).GetStripID();
    }

    TDirectory* CurrentDir = File;
    TDirectory* NewDir = nullptr;
    NewDir = CurrentDir->GetDirectory(DetName);
    if (NewDir == nullptr) NewDir = File->mkdir(DetName);
    CurrentDir->cd(NewDir->GetPath());
    CurrentDir = NewDir;

    if (SideName != "") {
      NewDir = CurrentDir->GetDirectory(SideName);
      if (NewDir == nullptr) NewDir = CurrentDir->mkdir(SideName);
      CurrentDir->cd(NewDir->GetPath());
      CurrentDir = NewDir;
    }

    if (StripName != "") {
      NewDir = CurrentDir->GetDirectory(StripName);
      if (NewDir == nullptr) NewDir = CurrentDir->mkdir(StripName);
      CurrentDir->cd(NewDir->GetPath());
      CurrentDir = NewDir;
    }

    MMelinatorBatchDrawing BatchDrawing; // Until the end of this loop iteration
    TCanvas SpectralCanvas;
    DrawSpectrum(SpectralCanvas, c, -1);
    SpectralCanvas.SetName("Spectrum");
    SpectralCanvas.Write();

    TCanvas CalibrationEnergyCanvas;
    DrawCalibration(CalibrationEnergyCanvas, c, true);
    CalibrationEnergyCanvas.SetName("CalibrationEnergy");
    CalibrationEnergyCanvas.Write();

    TCanvas CalibrationFWHMCanvas;
    DrawCalibration(CalibrationFWHMCanvas, c, false);
    CalibrationFWHMCanvas.SetName("CalibrationFWHM");
    CalibrationFWHMCanvas.Write();

    for (unsigned int s = 0; s < GetNumberOfCalibrationSpectralPoints(c); ++s) {
      MCalibrationSpectralPoint P = GetCalibrationSpectralPoint(c, s);
      MIsotope I = P.GetIsotope();
      ostringstream Text;
      Text<<"LineFit_"<<I.GetName()<<"_"<<fixed<<setprecision(1)<<P.GetEnergy()<<"keV";
      TCanvas LineFitCanvas;
      DrawLineFit(LineFitCanvas, c, s, c_HistogramBinningModeFixedNumberOfBins, 50);
      LineFitCanvas.SetName(Text.str().c_str());
      LineFitCanvas.Write();
    }
  }

  // ROOT bug I guess:
  // After File->Close() the histograms stored in m_Histograms are no longer available
  // My guess is that ROOT deletes them?
  // Anyway, this has to be here for ROOT 6.34 or we crash.
  // If ROOT does not delete them, this here is a memory leak.
  m_Histograms.clear();
  m_HistogramChanged = true;

  // Save and close
  File->Close();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Create a calibration report
bool MMelinator::CreateReport()
{
  if (m_CalibrationStore.GetNumberOfElements() == 0) {
    mgui<<"You need to run the calibration first, before creating a report."<<error;
    return true; // True: No subsequent error messages required
  } else {
    bool FoundOne = false;
    for (unsigned int c = 0; c < m_CalibrationStore.GetNumberOfElements(); ++c) {
      if (dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(c))) != nullptr) {
        FoundOne = true;
        break;
      }
    }
    if (FoundOne == false) {
      mgui<<"You need to run the calibration first, before creating a report."<<error;
      return true; // True: No subsequent error messages required
    }
  }

  // Section 1: Create histograms

  TH2D* AllFittedEnergies = new TH2D("Fitted", "Fitted", m_ReportEnergyHistogramBins, m_ReportEnergyHistogramMinimum, m_ReportEnergyHistogramMaximum, m_CalibrationStore.GetNumberOfElements()+1, -0.5, m_CalibrationStore.GetNumberOfElements() + 0.5);
  AllFittedEnergies->SetXTitle("Energy [keV]");
  AllFittedEnergies->SetYTitle("ReadOutUnit ID");
  AllFittedEnergies->SetContour(99);


  TH2D* AllRawADCs = new TH2D("RawADCs", "RawADCs", m_ReportADCHistogramBins, m_ReportADCHistogramMinimum, m_ReportADCHistogramMaximum, m_CalibrationStore.GetNumberOfElements()+1, -0.5, m_CalibrationStore.GetNumberOfElements() + 0.5);
  AllRawADCs->SetXTitle("ADCs [rou's]");
  AllRawADCs->SetYTitle("ReadOutUnit ID");
  AllRawADCs->SetContour(99);


  // If we have double-sided strips make one histogram per detector and side
  map<pair<unsigned int, bool>, MString> TitlesMap;
  map<pair<unsigned int, bool>, TH2D*> ADCsMap;
  map<pair<unsigned int, bool>, TH2D*> EnergiesMap;
  map<pair<unsigned int, bool>, TH2D*> ThresholdsMap;
  map<pair<unsigned int, bool>, TH2D*> OverflowsMap;
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
        auto MapIter = ADCsMap.find(DetectorAndSide);
        if (MapIter == ADCsMap.end()) {
          MString Name = "Detector ";
          Name += ROE_DS.GetDetectorID();
          if (ROE_DS.IsLowVoltageStrip() == true) {
            Name += ", low voltage side";
          } else {
            Name += ", high voltage side";
          }

          TitlesMap[DetectorAndSide] = Name;

          ADCsMap[DetectorAndSide] = new TH2D("", "ADC spectrum", m_ReportADCHistogramBins, m_ReportADCHistogramMinimum, m_ReportADCHistogramMaximum, MaxStripID+1, -0.5, MaxStripID + 0.5);
          ADCsMap[DetectorAndSide]->SetXTitle("ADC units");
          ADCsMap[DetectorAndSide]->SetYTitle("Strip ID");
          ADCsMap[DetectorAndSide]->SetContour(99);

          EnergiesMap[DetectorAndSide] = new TH2D("", "Energy spectrum", m_ReportEnergyHistogramBins, m_ReportEnergyHistogramMinimum, m_ReportEnergyHistogramMaximum, MaxStripID+1, -0.5, MaxStripID + 0.5);
          EnergiesMap[DetectorAndSide]->SetXTitle("Energy [keV]");
          EnergiesMap[DetectorAndSide]->SetYTitle("Strip ID");
          EnergiesMap[DetectorAndSide]->SetContour(99);

          ThresholdsMap[DetectorAndSide] = new TH2D("", "Thresholds", m_ReportThresholdHistogramBins, m_ReportThresholdHistogramMinimum, m_ReportThresholdHistogramMaximum, MaxStripID+1, -0.5, MaxStripID + 0.5);
          ThresholdsMap[DetectorAndSide]->SetXTitle("Energy [keV]");
          ThresholdsMap[DetectorAndSide]->SetYTitle("Strip ID");
          ThresholdsMap[DetectorAndSide]->SetContour(99);

          OverflowsMap[DetectorAndSide] = new TH2D("", "Overflows", m_ReportOverflowHistogramBins, m_ReportOverflowHistogramMinimum, m_ReportOverflowHistogramMaximum, MaxStripID+1, -0.5, MaxStripID + 0.5);
          OverflowsMap[DetectorAndSide]->SetXTitle("Energy [keV]");
          OverflowsMap[DetectorAndSide]->SetYTitle("Strip ID");
          OverflowsMap[DetectorAndSide]->SetContour(99);
        }

        MCalibration& C = m_CalibrationStore.GetCalibration(c);
        if (dynamic_cast<MCalibrationSpectrum*>(&C) == nullptr) continue;
        MCalibrationSpectrum& CS = static_cast<MCalibrationSpectrum&>(C);
        for (unsigned int g = 0; g < CS.GetNumberOfReadOutDataGroups(); ++g) {
          unsigned int IsotopeIndex = distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g));
          for (unsigned int i = 0; i < m_Isotopes[IsotopeIndex].size(); ++i) {
            for (unsigned int l = 0; l < m_Isotopes[IsotopeIndex][i].GetNLines(); ++l) {
              if (m_Isotopes[IsotopeIndex][i].GetPrimaryLine() == static_cast<int>(l) || m_Isotopes[IsotopeIndex][i].GetSecondaryLine() == static_cast<int>(l))  {
                tuple<unsigned int, unsigned int, unsigned int> ROGIDAndIsotope = { IsotopeIndex, i, l };
                // Inefficiency: we loop over everything... but avoiding it is a complicated optimization
                // Use try_emplace to avoid that

                MString Name;
                Name += m_Isotopes[IsotopeIndex][i].GetName();
                Name += " ";
                Name += m_Isotopes[IsotopeIndex][i].GetLineEnergy(l);
                Name += " keV";

                FWHMesMap.try_emplace(DetectorAndSide, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>());
                FWHMesMap[DetectorAndSide].try_emplace(ROGIDAndIsotope, new TH1D("", MString("FWHMes") + Name, MaxStripID+1, -0.5, MaxStripID + 0.5));
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetXTitle("Strip ID");
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetYTitle("FWHM [keV]");
                FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetLineWidth(2);

                ResidualsMap.try_emplace(DetectorAndSide, map<tuple<unsigned int, unsigned int, unsigned int>, TH1D*>());
                ResidualsMap[DetectorAndSide].try_emplace(ROGIDAndIsotope, new TH1D("", MString("Residuals") + Name, MaxStripID+1, -0.5, MaxStripID + 0.5));
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
    TH2D* CurrentHistThresholds = nullptr;
    TH2D* CurrentHistOverflows = nullptr;
    unsigned int CurrentStripID = 0;
    if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
      MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
      pair<unsigned int, bool> DetectorAndSide = { ROE_DS.GetDetectorID(), ROE_DS.IsLowVoltageStrip() };
      CurrentHistEnergy = EnergiesMap[DetectorAndSide];
      CurrentHistADCs = ADCsMap[DetectorAndSide];
      CurrentHistThresholds = ThresholdsMap[DetectorAndSide];
      CurrentHistOverflows = OverflowsMap[DetectorAndSide];
      CurrentStripID = ROE_DS.GetStripID();
    }
    if (dynamic_cast<MCalibrationSpectrum*>(&(m_CalibrationStore.GetCalibration(c))) == nullptr) continue;
    MCalibrationSpectrum& CS = dynamic_cast<MCalibrationSpectrum&>(m_CalibrationStore.GetCalibration(c));
    if (CS.HasEnergyModel() == false) continue;
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
            CurrentHistOverflows->Fill(Energy, CurrentStripID);
            CurrentHistThresholds->Fill(Energy, CurrentStripID);
            CurrentHistADCs->Fill(ADCs, CurrentStripID);
          }
        }
      }
    }

    if (m_CalibrationStore.GetReadOutElement(c).IsOfType("doublesidedstrip") == true) {
      MReadOutElementDoubleStrip& ROE_DS = static_cast<MReadOutElementDoubleStrip&>(ROE);
      pair<unsigned int, bool> DetectorAndSide = { ROE_DS.GetDetectorID(), ROE_DS.IsLowVoltageStrip() };

      for (unsigned int g = 0; g < CS.GetNumberOfReadOutDataGroups(); ++g) {
        unsigned int IsotopeIndex = distance(m_GroupIDs.begin(), find(m_GroupIDs.begin(), m_GroupIDs.end(), g));
        for (unsigned int i = 0; i < m_Isotopes[IsotopeIndex].size(); ++i) {
          for (unsigned int l = 0; l < m_Isotopes[IsotopeIndex][i].GetNLines(); ++l) {
            if (m_Isotopes[IsotopeIndex][i].GetPrimaryLine() == static_cast<int>(l) || m_Isotopes[IsotopeIndex][i].GetSecondaryLine() == static_cast<int>(l))  {
              tuple<unsigned int, unsigned int, unsigned int> ROGIDAndIsotope = { IsotopeIndex, i, l };

              // Loop over all spectral point, and find the isotope and the line
              for (unsigned int ii = 0; ii < CS.GetNumberOfSpectralPoints(g); ++ii) {
                MCalibrationSpectralPoint& SP = CS.GetSpectralPoint(g, ii);
                if (SP.GetIsotope() == m_Isotopes[IsotopeIndex][i]) {
                  if (fabs(SP.GetEnergy() - m_Isotopes[IsotopeIndex][i].GetLineEnergy(l)) < 0.01 && SP.HasFit() == true) {
                    FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->SetTitle(MString("FWHM per strip for ") + m_Isotopes[IsotopeIndex][i].GetName() + ", " + SP.GetEnergy() + " keV line"); // I know it is set multiple times...
                    FWHMesMap[DetectorAndSide][ROGIDAndIsotope]->Fill(CurrentStripID, SP.GetEnergyFWHM());
                    ResidualsMap[DetectorAndSide][ROGIDAndIsotope]->SetTitle(MString("Residuals line/fit per strip for ") + m_Isotopes[IsotopeIndex][i].GetName() + ", " + SP.GetEnergy() + " keV line"); // I know it is set multiple times...
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

  MMelinatorBatchDrawing BatchDrawing; // Until the end of this function

  auto IterTitles = TitlesMap.begin();
  auto IterEnergies = EnergiesMap.begin();
  auto IterADCs = ADCsMap.begin();
  auto IterThresholds = ThresholdsMap.begin();
  auto IterOverflows = OverflowsMap.begin();
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


  TitleCanvas->Print(m_ReportFileName + "(");


  while (IterTitles != TitlesMap.end() && IterEnergies != EnergiesMap.end() && IterADCs != ADCsMap.end() && IterThresholds != ThresholdsMap.end() && IterOverflows != OverflowsMap.end() && IterFWHMes != FWHMesMap.end() && IterResiduals != ResidualsMap.end()) {

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
    IterEnergies->second->Draw("colz");

    SpectrumPages->cd();
    SpectrumPages->Update();

    SpectrumPages->Print(m_ReportFileName);   // middle pages


    // Page 2: Thresholds & Overflows

    TCanvas* ThresholdsPages = new TCanvas("", "", 850, 1100);
    ThresholdsPages->SetLogz();
    ThresholdsPages->cd();

    TPad* ThresholdsTitlePad = new TPad("padTitle", "Title", 0.0, 0.9, 1.0, 1.0);
    ThresholdsTitlePad->Draw();
    ThresholdsTitlePad->cd();

    TPaveText* ThresholdsTitle = new TPaveText(0.0, 0.0, 1.0, 1.0, "brNDC");
    ThresholdsTitle->AddText(IterTitles->second);
    ThresholdsTitle->SetFillColor(0);     // No background color
    ThresholdsTitle->SetBorderSize(0);    // No border
    ThresholdsTitle->SetLineColor(0);     // Remove outline
    ThresholdsTitle->SetTextFont(TextFont);
    ThresholdsTitle->SetTextSize(0.4);
    ThresholdsTitle->Draw();

    ThresholdsPages->cd();

    TPad* ThresholdsPad = new TPad("ThresholdsPad", "ThresholdsPad", 0.0, 0.45, 1.0, 0.9);
    ThresholdsPad->Draw();
    ThresholdsPad->cd();
    ThresholdsPad->SetLogz();
    IterThresholds->second->Draw("colz");

    ThresholdsPages->cd();

    TPad* OverflowsPad = new TPad("OverflowsPad", "OverflowsPad", 0.0, 0.0, 1.0, 0.45);
    OverflowsPad->Draw();
    OverflowsPad->cd();
    OverflowsPad->SetLogz();
    IterOverflows->second->Draw("colz");

    ThresholdsPages->cd();
    ThresholdsPages->Update();

    ThresholdsPages->Print(m_ReportFileName);   // middle pages


    // Page 3+ is lines:

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

      LineFitPages->Print(m_ReportFileName);   // middle pages

      ++IterFWHMesInner;
      ++IterResidualsInner;
    }

    ++IterTitles;
    ++IterEnergies;
    ++IterADCs;
    ++IterThresholds;
    ++IterOverflows;
    ++IterFWHMes;
    ++IterResiduals;
  }

  TCanvas* EndPageCanvas = new TCanvas("EndPage", "EndPage", 850, 1100);
  EndPageCanvas->cd();

  TPaveText* TheEnd = new TPaveText(0.1, 0.75, 0.9, 0.9, "NDC");
  TheEnd->AddText("The End");
  TheEnd->SetTextFont(62);
  TheEnd->SetTextSize(0.02);
  TheEnd->SetFillColor(0);
  TheEnd->SetBorderSize(0);
  TheEnd->SetLineColor(0);
  TheEnd->Draw();

  EndPageCanvas->Print(m_ReportFileName + ")");  // close PDF

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
