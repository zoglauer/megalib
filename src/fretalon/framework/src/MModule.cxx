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
#include "TSystem.h"

// MEGAlib libs:
#include "MModuleReadOutAssemblyQueues.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
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
  
  m_AllowPausing = true;
  m_IsPaused = false;
  
  m_IsOK = true;
  m_IsReady = true;
  m_IsFinished = false;
  m_IsAnalyzing = false;
  
  m_Interrupt = false;
  
  m_AllowMultiThreading = false;
  m_AllowMultipleInstances = false;

  m_UseMultiThreading = false;
  m_Thread = 0;
  m_IsThreadRunning = false;
  
  m_NAnalyzedEvents = 0;
  
  ClearTimer();
  m_SleepTime = 0;
  
  m_Queues = make_shared<MModuleReadOutAssemblyQueues>();
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


//! Return true if this module is a hard predecessor
bool MModule::IsHardPreceedingModule(uint64_t Type) const
{
  for (unsigned int i = 0; i < GetNPreceedingModuleTypes(); ++i) {
    if (GetPreceedingModuleType(i) == Type) {
      if (GetPreceedingModuleHardRequirement(i) == true) {
        return true;
      }
    }
  }
  
  return false;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Return true if this module is a soft predecessor
bool MModule::IsSoftPreceedingModule(uint64_t Type) const
{
  for (unsigned int i = 0; i < GetNPreceedingModuleTypes(); ++i) {
    if (GetPreceedingModuleType(i) == Type) {
      if (GetPreceedingModuleHardRequirement(i) == false) {
        return true;
      }
    }
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MModule::ClearQueues() 
{ 
  //! Clear the queues

  m_Queues->Clear(); 
}


////////////////////////////////////////////////////////////////////////////////


bool MModule::AddReadOutAssembly(MReadOutAssembly* Event)
{
  //! Add an event to the incoming event list

  m_Queues->AddIncoming(Event);
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::HasAddedReadOutAssemblies()
{
  //! Check if there are events in the incoming event list

  return m_Queues->HasIncoming();
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::HasAnalyzedReadOutAssemblies()
{
  //! Check if there are events in the outgoing event list

  return m_Queues->HasOutgoing();
}
  

////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* MModule::GetAnalyzedReadOutAssembly()
{
  //! Check if there are events in the outgoing event list
  
  return m_Queues->GetOutgoing();
}


////////////////////////////////////////////////////////////////////////////////


bool MModule::Initialize()
{
  m_IsPaused = false;
  m_IsOK = true;
  m_IsFinished = false;
  m_NAnalyzedEvents = 0;
  ClearTimer();
  m_SleepTime = 0;
  
  // If we have allow multiple instances of this module, we have to enable sorting 
  // This is safe to be called multiple times, from different Initialize();
  if (m_AllowMultipleInstances == true) {
    m_Queues->EnableSorting(); 
  }
  
  for (auto E: m_Expos) {
    E->Reset(); 
  }
  
  if (m_UseMultiThreading == true && m_AllowMultiThreading > 0) {
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
    
    if (m_IsPaused == true || DoSingleAnalysis() == false) {
      MTimer SleepTimer;
      gSystem->Sleep(20);
      m_SleepTime += SleepTimer.GetElapsed(); // Sleep() is not perfectly accurate...
    }
  }

  m_IsThreadRunning = false;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModule::DoSingleAnalysis()
{
  ContinueTimer();

  // First check if we are ready:
  if (IsReady() == false) {
    PauseTimer();
    return false;
  }
  if (IsOK() == false) {
    PauseTimer();
    return false;
  }
  
  m_IsAnalyzing = true;
  
  MReadOutAssembly* E = 0;
  // If this is a module which does not generate the events, grab one from the incoming list
  if (m_IsStartModule == false) { 
    if (m_Queues->HasIncoming() == true) {
      E = m_Queues->GetIncoming(); // E can still be zero, if another thread did something!
    }
  }
  // If we got one from the incoming list, or if this is a start module which generates them:
  if (E == 0 && m_IsStartModule == true && m_IsFinished == false && m_Interrupt == false) {
    E = new MReadOutAssembly();
  }
  
  if (E != 0) {
    if (FullfillsRequirements(E) == true && E->IsFilteredOut() == false) {
      AnalyzeEvent(E);
    }
    ++m_NAnalyzedEvents; // Analyzed actually just means passed through...
    if (m_IsFinished == true) {
      delete E;
      E = 0;
    }
    if (E != 0) {
      m_Queues->AddOutgoing(E);
    }
    m_IsAnalyzing = false;
    PauseTimer();
    return true;
  } else {
    m_IsAnalyzing = false;
    PauseTimer();
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
  
  // If we have multiple instances, this will be called for the same queue, several
  // time, but that's safe
  m_Queues->Clear();
}


////////////////////////////////////////////////////////////////////////////////


double MModule::GetTimer()
{
  //! Get the value of the processing timer -- thread safe
  
  lock_guard<mutex> lock(m_TimerGuard);
  
  return m_Timer.GetElapsed();
}
 

////////////////////////////////////////////////////////////////////////////////


 
void MModule::ClearTimer()
{
  //! Clear the value of the processing timer -- thread safe
  
  lock_guard<mutex> lock(m_TimerGuard);
  
  m_Timer.Clear();
}


////////////////////////////////////////////////////////////////////////////////


void MModule::ContinueTimer()
{
  //! Continue the processing timer -- thread safe
  
  lock_guard<mutex> lock(m_TimerGuard);
  
  m_Timer.Continue();
} 


////////////////////////////////////////////////////////////////////////////////


void MModule::PauseTimer()
{
  //! Pause the processing timer -- thread safe

  lock_guard<mutex> lock(m_TimerGuard);
  
  m_Timer.Pause();
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
