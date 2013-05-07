/*
 * MRawEventAnalyzerMultiThreaded.cxx
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
// MRawEventAnalyzerMultiThreaded
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRawEventAnalyzerMultiThreaded.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRawEventAnalyzerMultiThreaded)
#endif



////////////////////////////////////////////////////////////////////////////////


void SplitCalculation(void *adr)
{
  MRawEventAnalyzerMultiThreaded* M = 
    ((MRawEventAnalyzerMultiThreadedContainer*) adr)->GetClass();
  M->ThreadedCalculation(((MRawEventAnalyzerMultiThreadedContainer*) adr)->GetID());
}


////////////////////////////////////////////////////////////////////////////////


MRawEventAnalyzerMultiThreaded::MRawEventAnalyzerMultiThreaded(MString FileNameIn, 
                                                               MString FileNameOut, 
                                                               MGeometryRevan* Geometry, 
                                                               unsigned int NJobs)
{
  // Construct an instance of MRawEventAnalyzerMultiThreaded

  m_FileNameIn = FileNameIn;
  m_FileNameOut = FileNameOut;
  m_Geometry = Geometry;
  m_NJobs = NJobs;
}


////////////////////////////////////////////////////////////////////////////////


MRawEventAnalyzerMultiThreaded::~MRawEventAnalyzerMultiThreaded()
{
  // Delete this instance of MRawEventAnalyzerMultiThreaded
}


////////////////////////////////////////////////////////////////////////////////

  
bool MRawEventAnalyzerMultiThreaded::PreAnalysis()
{
  m_FileIn = new MFileEventsEvta(m_Geometry);
  if (m_FileIn->Open(m_FileNameIn) == false) {
    mout<<"MRawEventAnalyzerMultiThreaded: Unable to open input file!"<<endl;
    delete m_FileIn;
    m_FileIn = 0;
    return false;
  }

  m_FileOut = new MFileEventsTra();
  if (m_FileOut->Open(m_FileNameOut, MFile::c_Write) == false) {
    mout<<"MRawEventAnalyzerMultiThreaded: Unable to open output file!"<<endl;
    delete m_FileOut;
    m_FileOut = 0;
    return false;
  }

  for (unsigned int i = 0; i < m_NJobs; ++i) {
    m_Analyzers.push_back(new MRawEventAnalyzer("", "", m_Geometry));
    m_ThreadStatus.push_back(-1);
    m_Threads.push_back(0);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////

  
bool MRawEventAnalyzerMultiThreaded::PostAnalysis()
{
  // We have to join the MRawEventAnalyzers und dump the statistics of the last 
  // one

  for (unsigned int i = 1; i < m_NJobs; ++i) {
    m_Analyzers[0]->JoinStatistics(*m_Analyzers[i]);
  }

  m_Analyzers[0]->PostAnalysis();

  return true;
}


////////////////////////////////////////////////////////////////////////////////

  
MRawEventAnalyzer* MRawEventAnalyzerMultiThreaded::GetAnalyzer(unsigned int i)
{
  return m_Analyzers.at(i);
}


////////////////////////////////////////////////////////////////////////////////


void MRawEventAnalyzerMultiThreaded::AnalyzeAllEvents()
{
  cout<<"Let's get loud!"<<endl;

  int j;
  TThread *Thread;

  // Start the threads...
  for (unsigned int i = 0; i < m_NJobs; i++) {
    Thread = new TThread("acz", 
                         (void(*) (void *)) &SplitCalculation, 
                         (void*) new MRawEventAnalyzerMultiThreadedContainer(i, this));
    Thread->Run();
    j = Thread->GetId();
    m_ThreadStatus[i] = j;
  }

  // Wait for their termination
  for (unsigned int i = 0; i < m_NJobs; i++) {
    while (m_ThreadStatus[i] != -1) {
      for (j = 0; j < 5; j++) {
        gSystem->ProcessEvents();
      }
      gSystem->Sleep(100);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void* MRawEventAnalyzerMultiThreaded::ThreadedCalculation(unsigned int ThreadCounter)
{
  TThread::Self()->Ps();

  // Do the analysis:
  MRERawEvent* RE = 0;
  MRawEventAnalyzer* Analyzer = m_Analyzers[ThreadCounter];
  bool MoreEvents = true;
  while (MoreEvents == true) { 
    TThread::Lock();
    RE = m_FileIn->GetNextEvent();
    TThread::UnLock();

    if (RE == 0) {
      break;
    }
    Analyzer->AnalyzeOneEvent(RE);
    
    if (Analyzer->GetOptimumEvent() != 0) {
      MPhysicalEvent* Event = Analyzer->GetOptimumEvent()->GetPhysicalEvent();
      if (m_FileOut != 0 && Event != 0) {
        TThread::Lock();
        m_FileOut->AddEvent(Event);
        TThread::UnLock();
      }
    }

    //cout<<"Reading event: "<<RE->GetEventId()<<" in thread "<<ThreadCounter<<endl;
    //delete RE;
  }

  // "Der Letzte macht das Licht aus..." - no good translation found...
  
  // Check if there are other threads running:
  // ---> Lock damit nicht ein anderer fertig wird in dem Moment wo hier noch getestet wird ...
  bool LastRemaining = true;
  for (unsigned int i = 0; i < m_NJobs; i++) {
    if (i != ThreadCounter) {
      if (m_ThreadStatus[i] != -1) {
        LastRemaining = false;
      }
    }
  }
  m_ThreadStatus[ThreadCounter] = -1;


  //TThread* T = TThread::GetThread(m_ThreadStatus[ThreadCounter]);
  if (LastRemaining == true) {
    cout<<"Last thread... "<<endl;
  }

  // I am not sure what kind of clean up we need here...

  return 0;
}


// MRawEventAnalyzerMultiThreaded.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
