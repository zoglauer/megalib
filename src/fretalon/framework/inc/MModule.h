/*
 * MModule.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModule__
#define __MModule__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <deque>
#include <memory>
#include <mutex>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlNode.h"
#include "MDGeometryQuest.h"
#include "MGUIExpo.h"
#include "MAssembly.h"
#include "MReadOutAssembly.h"
#include "MTimer.h"

// Forward declarations:
class MModuleReadOutAssemblyQueues;

////////////////////////////////////////////////////////////////////////////////


//! Global function to start the thread
void* MModuleKickstartThread(void* ClassDerivedFromMModule);


////////////////////////////////////////////////////////////////////////////////


class MModule
{
  // public interface:
 public:
  //! Default constructor
  MModule();
  //! Default destructor
  virtual ~MModule();

  //! Create a new object of this class 
  virtual MModule* Clone() = 0;
  
  //! Return the name of this module:
  MString GetName() { return m_Name; }

  //! Return the XML tag of this module:
  MString GetXmlTag() { return m_XmlTag; }

  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry) { m_Geometry = Geometry; }
  
  
  //! Return the number of the preceeding modules
  unsigned int GetNPreceedingModuleTypes() { return m_PreceedingModules.size(); }
  //! Return the preceeding module at position i (no error checks performed)
  uint64_t GetPreceedingModuleType(unsigned int i) { return m_PreceedingModules.at(i); }

  //! Return the number of module types
  unsigned int GetNModuleTypes() { return m_Modules.size(); }
  //! Return the module type at position i (no error checks performed)
  uint64_t GetModuleType(unsigned int i) { return m_Modules.at(i); }

  //! Return the number of the succeeding modules
  unsigned int GetNSucceedingModuleTypes() { return m_SucceedingModules.size(); }
  //! Return the succeeding module at position i (no error checks performed)
  uint64_t GetSucceedingModuleType(unsigned int i) { return m_SucceedingModules.at(i); }

  //! Return true, if the read-out assembly fullfills the preeceding modules requirements
  bool FullfillsRequirements(MReadOutAssembly* Event);
  
  //! Raise an interrupt
  void SetInterrupt(bool Flag = true) { m_Interrupt = Flag; }

  //! Return true, if this module allows multi-threading
  bool AllowsMultiThreading() const { return m_AllowMultiThreading; }
  //! Return true, if this module allows to be used in multiple instances
  bool AllowsMultipleInstances() const { return m_AllowMultipleInstances; }

  //! Use multi threading -- but it is only really used if the module allows it
  void UseMultiThreading(bool UseMultiThreading = true) { m_UseMultiThreading = UseMultiThreading; }

  //! Return true if this module is using multiple-treads
  bool IsMultiThreaded() { return m_IsThreadRunning; }

  //! Is this a start module which generates its own events
  bool IsStartModule() const { return m_IsStartModule; }
  
  //! Initialize the module
  //! When overwritten, the base-class'es Initialize() has to be called *at the very end* of the 
  //! Initilize() function of the derived class, since in multi-threaded mode it 
  //! starts the thraeds
  virtual bool Initialize();

  //! Finalize the module
  //! When overwritten, the base-class'es Finalize() has to be called *at the very beginning* of the 
  //! Finalize() function of the derived class, since in multi-threaded mode it 
  //! ends the threads
  virtual void Finalize();

  //! Main data analysis routine, which updates the event to a new level 
  //! Has to be overwritten in derived class
  virtual bool AnalyzeEvent(MReadOutAssembly* Event) = 0;

  //! Clear the queues
  void ClearQueues();

  //! Share the queue between modules
  void ShareQueues(MModule* M) { M->m_Queues = m_Queues; }
  
  //! Add an read-out assembly to the incoming read-out assembly list - only used by the supervisor
  virtual bool AddReadOutAssembly(MReadOutAssembly* Assembly);
  
  //! Check if there are read-out assemblies in the incoming read-out assembly list - only used by the supervisor
  virtual bool HasAddedReadOutAssemblies();
  
  //! Check if there are read-out assemblies in the outgoing read-out assembly list - only used by the supervisor
  virtual bool HasAnalyzedReadOutAssemblies();
  
  //! Check if there are read-out assemblies in the outgoing read-out assembly list - only used by the supervisor
  virtual MReadOutAssembly* GetAnalyzedReadOutAssembly();

  //! The number of events passing through the analysis routine
  long GetNumberOfAnalyzedEvents() const { return m_NAnalyzedEvents; }

  //! The analysis loop in multi-threaded mode
  virtual void AnalysisLoop();
  //! Analyze a single event in multi-threaded mode
  //! Returns true if an event passed through all stages
  bool DoSingleAnalysis();

  //! True if this module has an associated options GUI
  bool HasOptionsGUI() { return m_HasOptionsGUI; }
  //! Show the options GUI --- has to be overwritten!
  virtual void ShowOptionsGUI() {};

  //! True if this module has associated expo GUI(s)
  bool HasExpos() { return m_Expos.size() > 0 ? true : false; }
  //! Return the associated expo GUI(s). If there are none return an empty vector
  vector<MGUIExpo*> GetExpos() { return m_Expos; }

  //! Read the configuration data from an XML node
  virtual bool ReadXmlConfiguration(MXmlNode* Node);
  //! Create an XML node tree from the configuration
  virtual MXmlNode* CreateXmlConfiguration();

  //! Return if the module is ready to analyze events
  virtual bool IsReady() { return m_IsReady; }
  //! Return if the status of the module is OK
  virtual bool IsOK() { return m_IsOK; }
  //! Return if the module has finished all possible analyses
  virtual bool IsFinished() { return m_IsFinished; }

  //! Return the processing time in seconds - thread safe!
  double GetProcessingTime() { return GetTimer(); } 
  //! Return the processing time in seconds - thread safe!
  double GetSleepingTime() { return m_SleepTime; } 
  
  
  // protected methods:
 protected:
  //! Set the name of this module
  void SetName(MString Name) { m_Name = Name; }

  //! Set which modules are assumed to be already performed
  void AddPreceedingModuleType(int Type) { m_PreceedingModules.push_back(Type); }
  //! Add which type of module this is, e.g. c_EnergyCalibration
  //! This option ca be called twice to set two tasks of this modules!
  void AddModuleType(int Type) { m_Modules.push_back(Type); };
  //! Set which modules are expected to follow this one
  void AddSucceedingModuleType(int Type) { m_SucceedingModules.push_back(Type); };

  //! Get the value of the processing timer -- thread safe
  double GetTimer();
  //! Clear the value of the processing timer -- thread safe
  void ClearTimer();
  //! Continue the processing timer -- thread safe
  void ContinueTimer();
  //! Pause the processing timer -- thread safe
  void PauseTimer();
  

  // private methods:
 private:
  //! No Copy constructor
  MModule(const MModule&) = delete;
  //! No copying whatsoever
  MModule& operator=(const MModule&) = delete;

  
  // protected members:
 protected:
  //! Name of this module
  MString m_Name;
  //! Name of the XML tag --- has to be unique
  MString m_XmlTag;

  //! List of preceeding modules
  vector<uint64_t> m_PreceedingModules;
  //! List of succeeding modules
  vector<uint64_t> m_SucceedingModules;
  //! List of types of this modules
  vector<uint64_t> m_Modules;
  
  //! The Geometry description
  MDGeometryQuest* m_Geometry;

  //! True, if this module has an options GUI
  bool m_HasOptionsGUI;

  //! A vector of associated expo GUIs
  vector<MGUIExpo*> m_Expos;
  
  //! True if this is a start module which generates the events by itself, i.e. reads them from file
  //! Usually the first module and only the first module is a start module
  bool m_IsStartModule;
  
  //! True, if the module is ready to analyze events
  bool m_IsReady;
  //! True, if the status of the module is OK
  bool m_IsOK;
  //! True, if the module is finished (e.g. cannot read any more events)
  bool m_IsFinished;
  
  //! Interrupt whatever it is doing and break
  bool m_Interrupt;

  //! Flag indicating that this module allows multi-threading
  bool m_AllowMultiThreading;
  //! Flag indicating that this module allows multiple instances
  bool m_AllowMultipleInstances;
  
  //! Flag indicating if we should use multithreading if available
  bool m_UseMultiThreading;
  
  //! The thread where the analysis happens
  TThread* m_Thread;     
  //! True if the analysis thread is in its execution loop
  bool m_IsThreadRunning;

  //! The number of events passing through the Analysis function
  long m_NAnalyzedEvents;
  
  
  // private members:
 private:
  //! The internal analysis timer
  MTimer m_Timer;
  //! The mutex protecting the analysis timer
  mutex m_TimerGuard;
  //! The internal sleep time
  double m_SleepTime;

  //! The incoming and outgoing event queues
  shared_ptr<MModuleReadOutAssemblyQueues> m_Queues;


#ifdef ___CINT___
 public:
  ClassDef(MModule, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
