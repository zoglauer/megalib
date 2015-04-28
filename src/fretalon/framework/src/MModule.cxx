/*
 * MModule.cxx
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
// MModule
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModule.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MModule)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Global function to start the thread
void* MModuleKickstartThread(void* ClassDerivedFromMModule)
{
  // dynamic_cast<MModule*>(ClassDerivedFromMModule)->AnalysisLoop();
  ((MModule*) ClassDerivedFromMModule)->AnalysisLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MModule::MModule()
{
  // Construct an instance of MModule

  m_Name = "Base class...";
  m_XmlTag = "BaseClass"; // No spaces allowed

  m_HasOptionsGUI = false;
  
  m_IsStartModule = false;
  
  m_IsOK = true;
  m_IsReady = true;
  m_IsFinished = false;
  
  m_Interrupt = false;
  
  m_UseMultiThreading = false;
  m_NAllowedWorkerThreads = 0;
  m_Thread = 0;
  m_IsThreadRunning = false;
  
  m_NAnalyzedEvents = 0;
  
  m_Timer.Clear();
}


////////////////////////////////////////////////////////////////////////////////


MModule::~MModule()
{
  // Delete this instance of MModule
}


////////////////////////////////////////////////////////////////////////////////


bool MModule::FullfillsRequirements(MReadOutAssembly* Event)
{
  //! Return true, if the read-out assembly fullfills the preeceding modules requirements

  for (unsigned int i = 0; i < GetNPreceedingModuleTypes(); ++i) {
    if (Event->HasAnalysisProgress(GetPreceedingModuleType(i)) == false) {
      return false; 
    }
  }
    
  return true;
}

////////////////////////////////////////////////////////////////////////////////


bool MModule::AddEvent(MReadOutAssembly* Event)
{
  //! Add an event to the incoming event list

  m_IncomingEventsMutex.Lock();
  m_IncomingEvents.push_back(Event);
  m_IncomingEventsMutex.UnLock();
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::HasAddedEvents()
{
  //! Check if there are events in the incoming event list

  bool HasEvents = false;

  m_IncomingEventsMutex.Lock();
  HasEvents = m_IncomingEvents.begin() != m_IncomingEvents.end(); // faster for deque than size if filled!
  m_IncomingEventsMutex.UnLock();

  return HasEvents;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::HasAnalyzedEvents()
{
  //! Check if there are events in the outgoing event list

  bool HasEvents = false;

  m_OutgoingEventsMutex.Lock();
  HasEvents = m_OutgoingEvents.begin() != m_OutgoingEvents.end(); // faster for deque than size if filled!
  m_OutgoingEventsMutex.UnLock();

  return HasEvents;
}
  

////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* MModule::GetAnalyzedEvent()
{
  //! Check if there are events in the outgoing event list

  MReadOutAssembly* E = 0;
  
  m_OutgoingEventsMutex.Lock();
  if (m_OutgoingEvents.begin() != m_OutgoingEvents.end()) {
    E = m_OutgoingEvents.front();
    m_OutgoingEvents.pop_front();
  }
  m_OutgoingEventsMutex.UnLock();
  
  return E;
}


////////////////////////////////////////////////////////////////////////////////


bool MModule::Initialize()
{
  m_IsOK = true;
  m_IsFinished = false;
  m_NAnalyzedEvents = 0;
  m_Timer.Clear();
  
  for (auto E: m_Expos) {
    E->Reset(); 
  }
  
  if (m_UseMultiThreading == true && m_NAllowedWorkerThreads > 0) {
    m_IsThreadRunning = false;
  
    delete m_Thread;
    m_Thread = new TThread(m_XmlTag + "-Thread ", 
                           (void(*) (void *)) &MModuleKickstartThread, 
                           (void*) this);
    m_Thread->SetPriority(TThread::kHighPriority);
    m_Thread->Run();
    
    while (m_IsThreadRunning == false) {
      gSystem->Sleep(10);
    }
  }
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


void MModule::AnalysisLoop()
{
  m_IsThreadRunning = true;
  
  while (m_Interrupt == false) {
    
    if (DoSingleAnalysis() == false) {
      gSystem->Sleep(20);
    }
  }

  m_IsThreadRunning = false;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::DoSingleAnalysis()
{
  // First check if we are ready:
  if (IsReady() == false) return false;
  if (IsOK() == false) return false;
  
  m_Timer.Continue();
  
  MReadOutAssembly* E = 0;
  // If this is a module which does not generate the events, grab one from the incoming list
  if (m_IsStartModule == false) { 
    m_IncomingEventsMutex.Lock();
    if (m_IncomingEvents.begin() != m_IncomingEvents.end()) {
      E = m_IncomingEvents.front();
      m_IncomingEvents.pop_front();
    }
    m_IncomingEventsMutex.UnLock();
  }
  // If we got one from the incoming list, or if this is a start module which generates them:
  if (E == 0 && m_IsStartModule == true && m_IsFinished == false && m_Interrupt == false) {
    E = new MReadOutAssembly();
  }
  
  if (E != 0) {
    if (FullfillsRequirements(E) == true) {
      AnalyzeEvent(E);
      if (m_IsFinished == true) {
        delete E;
        E = 0;
      } else {
        ++m_NAnalyzedEvents;
      }
    }
    if (E != 0) {
      m_OutgoingEventsMutex.Lock();
      m_OutgoingEvents.push_back(E);
      m_OutgoingEventsMutex.UnLock();
    }
    m_Timer.Pause();
    return true;
  } else {
    m_Timer.Pause();
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MModule::Finalize()
{
  if (m_Thread != 0) {
    m_Interrupt = true;
    while (m_IsThreadRunning == true) {
      gSystem->Sleep(20);
    }
    if (m_Thread != 0) m_Thread->Kill();
    m_Thread = 0;
  }
  
  if (HasAddedEvents() > 0) {
    for (auto E: m_IncomingEvents) {
      delete E; 
    }
    m_IncomingEvents.clear();
  }
  if (HasAnalyzedEvents() > 0) {
    for (auto E: m_OutgoingEvents) {
      delete E; 
    }
    m_OutgoingEvents.clear();
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModule::ReadXmlConfiguration(MXmlNode* Node)
{
  //! Read the configuration data from an XML node

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MXmlNode* MModule::CreateXmlConfiguration() 
{
  //! Create an XML node tree from the configuration

  MXmlNode* Node = new MXmlNode(0, m_XmlTag);

  return Node;
}


// MModule.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
