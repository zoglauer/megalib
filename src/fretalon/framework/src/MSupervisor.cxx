/*
 * MSupervisor.cxx
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
// MSupervisor
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSupervisor.h"

// Standard libs:

// ROOT libs:
#include "TSystem.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"
#include "MGUIMainFretalon.h"
#include "MGUIExpoCombinedViewer.h"
#include "MModule.h"
#include "MReadOutAssembly.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSupervisor)
#endif


////////////////////////////////////////////////////////////////////////////////


MSupervisor::MSupervisor()
{
  // Construct an instance of MSupervisor

  m_Interrupt = false;
  m_Terminate = false;
  
  m_UseMultiThreading = false;
  m_IsAnalysisRunning = false;

  m_UIProgramName = "Unnamed program";
  m_UIPicturePath = "";
  m_UISubTitle = "No sub title set!";
  m_UILeadAuthor = "No lead author!";
  m_UICoAuthors = "";
  
  m_Geometry = 0;
  
  m_ConfigurationFileName = gSystem->ConcatFileName(gSystem->HomeDirectory(), ".fretalon.cfg");
  
  Clear();

  // All the rest:
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::Clear()
{
  //! Reset all data

  m_Modules.clear();
  
  m_GeometryFileName = "";
  delete m_Geometry;
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


MSupervisor::~MSupervisor()
{
  // Delete this instance of MSupervisor

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    delete m_AvailableModules[m];
  }

  delete m_Geometry;
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetModule(unsigned int i) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  if (i < m_Modules.size()) {
    return m_Modules[i];
  }

  return 0; 
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetAvailableModule(unsigned int i) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  if (i < m_AvailableModules.size()) {
    return m_AvailableModules[i]; 
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetAvailableModule(MString Name) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    if (m_AvailableModules[m]->GetName() == Name) {
      return m_AvailableModules[m];
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


vector<MModule*> MSupervisor::ReturnPossibleVolumes(unsigned int Position)
{
  vector<MModule*> Previous;
  if (Position > 0) {
    for (unsigned int i = 0; i < Position; ++i) {
      Previous.push_back(m_Modules[i]);
    }
  }

  //cout<<"Assembling possbile modules for position: "<<Position<<" --> "<<Previous.size()<<endl;

  return ReturnPossibleVolumes(Previous);
}


////////////////////////////////////////////////////////////////////////////////


vector<MModule*> MSupervisor::ReturnPossibleVolumes(vector<MModule*>& Previous)
{
  vector<MModule*> List;
  
  for (unsigned int i = 0; i < m_AvailableModules.size(); ++i) {
    //cout<<"Checking module: "<<m_AvailableModules[i]->GetName()<<endl;

    bool Passed = true;

    // (1) Check if the requirement on previous modules can be passed 
    for (unsigned int m = 0; m < m_AvailableModules[i]->GetNPreceedingModuleTypes(); ++m) {
      //cout<<"Requires: "<<m_AvailableModules[i]->GetPreceedingModuleType(m)<<endl;
      bool Found = false;
      //cout<<"Checking "<<Previous.size()<<" previous elements"<<endl;
      for (unsigned int p = 0; p < Previous.size(); ++p) {
        //cout<<"Checking: "<<Previous[p]->GetName()<<endl;
        for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
          //cout<<"Provides: "<<Previous[p]->GetModuleType(t)<<endl;
          if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetPreceedingModuleType(m)) {
            Found = true;
            break;
          }
        }
        if (Found == true) break;
      }
      if (Found == false) {
        //mout<<"Failed Previous requirement"<<endl;
        Passed = false;
        break;
      }
    }

    // (2) If any of it is already in the list ignore it:
    if (Passed == true) {
      bool AlreadyInList = false;
      for (unsigned int p = 0; p < m_Modules.size(); ++p) {
        if (m_Modules[p] == m_AvailableModules[i]) {
          AlreadyInList = true;
          break;
        }
      }
      if (AlreadyInList == true) {
        //mout<<"Failed Already in list requirement"<<endl;
        Passed = false;
      }
    }

    // (3) If any of the module types are already in the list, ignore it:
    if (Passed == true) {
      bool AlreadyInList = false;
      for (unsigned int p = 0; p < Previous.size(); ++p) {
        for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
          for (unsigned int at = 0; at < m_AvailableModules[i]->GetNModuleTypes(); ++at) {
            if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetModuleType(at)) {
              AlreadyInList = true;
              break;
            }
          }
        }
      }
      if (AlreadyInList == true) {
        //mout<<"Failed module type already in list requirement"<<endl;
        Passed = false;
      }
    }


    // (4) If any of the modules which can follow are already in the list, also ignore it
//     if (Passed == true) {
//       bool AlreadyInList = false;
//       for (unsigned int at = 0; at < m_AvailableModules[i]->GetNSucceedingModuleTypes(); ++at) {
//         for (unsigned int p = 0; p < Previous.size(); ++p) {
//           for (unsigned int t = 0; t < Previous[p]->GetNModuleTypes(); ++t) {
//             if (Previous[p]->GetModuleType(t) == m_AvailableModules[i]->GetSucceedingModuleType(at)) {
//               AlreadyInList = true;
//               break;
//             }
//           }
//         }
//       }
//       if (AlreadyInList == true) {
//         //mout<<"Failed Successor requirement"<<endl;        
//         Passed = false;
//       }
//     }
      
    if (Passed == true) {
      //mout<<"Passed :)"<<endl;
      List.push_back(m_AvailableModules[i]);
    }
  }

  return List;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::SetModule(MModule* Module, unsigned int i) 
{
  // Set a module at a specific position - return false if other modules had to be eliminated  

  if (m_Modules.size() > i) {
    m_Modules[i] = Module;
  } else if (m_Modules.size() == i) {
    m_Modules.push_back(Module);
  } else {
    merr<<"Unable to add module"<<endl;
  }

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::RemoveModule(unsigned int i)
{
  //! Remove module at a specific position - return false if other modules had to be eliminated  

  if (i < m_Modules.size()) {
    m_Modules.erase(m_Modules.begin() + i);
  } else {
    merr<<"Unable to remove module"<<endl;
  }

  return Validate();
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Validate()
{
  // Validate

  unsigned int ValidUntil = m_Modules.size();

  // (1) Make sure no module appears twice:
  for (unsigned int m1 = 0; m1 < m_Modules.size(); ++m1) {
    for (unsigned int m2 = m1+1; m2 < m_Modules.size(); ++m2) {
      if (m_Modules[m1] == m_Modules[m2]) {
        mout<<"Module: "<<m_Modules[m1]->GetName()<<" appears twice: "<<m1<<" & "<<m2<<endl;
        if (m2 < ValidUntil) {
          ValidUntil = m2;
        }
      }
    }
  }

  // (2) Make sure all predecessor requirements are fulfilled
  vector<int> PredecessorTypes;
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int t = 0; t < m_Modules[m]->GetNPreceedingModuleTypes(); ++t) {
      bool Found = false;
      for (unsigned int p = 0; p < PredecessorTypes.size(); ++p) {
        if (PredecessorTypes[p] == m_Modules[m]->GetPreceedingModuleType(t)) {
          Found = true;
        }
      }
      if (Found == false) {
        mout<<"Predecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
        if (m < ValidUntil) {
          ValidUntil = m;
        }
        break;
      }
    }
    if (ValidUntil == m) break;

    for (unsigned int t = 0; t < m_Modules[m]->GetNModuleTypes(); ++t) {
      PredecessorTypes.push_back(m_Modules[m]->GetModuleType(t));
    }
  }

  // (3) Make sure all succecessor requirements are fulfilled
  /*
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int s = m+1; s < m_Modules.size(); ++s) {
      for (unsigned int st = 0; st < m_Modules[s]->GetNModuleTypes(); ++st) {
        bool Found = false;
        if (
        for (unsigned int mt = 0; mt < m_Modules[m]->GetNSucceedingModuleTypes(); ++mt) {
          if (m_Modules[m]->GetSucceedingModuleType(mt) == m_Modules[s]->GetModuleType(st)) {
            Found = true;
            break;
          }
        }
        if (Found == false) {
          mout<<"Succecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
          if (s < ValidUntil) {
            ValidUntil = s;
          }
          break;
        }
        if (ValidUntil == s) break;
      }
      if (ValidUntil == s) break;
    }
  }
  */

  // Many possible validations are missing


  //cout<<"Valid until: "<<ValidUntil<<endl;
  while (ValidUntil < m_Modules.size()) {
    cout<<"Erasing some modules!"<<endl;
    m_Modules.erase(m_Modules.begin()+ValidUntil);
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Load(MString FileName)
{
  // Load all data from a file

  Clear();

  if (FileName == "") FileName = m_ConfigurationFileName;
  
  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(FileName);

  int Version = 1;
  MXmlNode* VersionNode = Document->GetNode("Version");
  if (VersionNode != 0) {
    Version = VersionNode->GetValueAsInt();
  }
  if (Version != 1) {
    mout<<"Info: While we do have a higher version than 1, everything is still fixed at version 1"<<endl;
  }

  MXmlNode* ModuleSequence = Document->GetNode("ModuleSequence");
  if (ModuleSequence != 0) {
    for (unsigned int m = 0; m < ModuleSequence->GetNNodes(); ++m) {
      MModule* M = GetAvailableModule(ModuleSequence->GetNode(m)->GetValue());
      if (M != 0) {
        m_Modules.push_back(M);
      } else {
        mout<<"Error: Cannot find a module with name: "<<ModuleSequence->GetNode(m)->GetValue()<<endl;
      }
    }
  }

  MXmlNode* GeometryFileName = Document->GetNode("GeometryFileName");
  if (GeometryFileName != 0) {
    m_GeometryFileName = GeometryFileName->GetValue();
  }

  MXmlNode* ModuleOptions = Document->GetNode("ModuleOptions");
  if (ModuleOptions != 0) {
    for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
      MXmlNode* ModuleNode = ModuleOptions->GetNode(m_AvailableModules[a]->GetXmlTag());
      if (ModuleNode != 0) {
        m_AvailableModules[a]->ReadXmlConfiguration(ModuleNode);
      }
    }
  }

  delete Document;

  return true;
} 


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Save(MString FileName)
{
  //! Save all data to a file

  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument("NuclearizerData");

  // Version string: 1
  new MXmlNode(Document, "Version", 1);

  MXmlNode* ModuleSequence = new MXmlNode(Document, "ModuleSequence");
  // Store the module sequence
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    new MXmlNode(ModuleSequence, "ModuleSequenceItem", m_Modules[m]->GetName());
  }

  // Store the file names
  new MXmlNode(Document, "GeometryFileName", m_GeometryFileName);

  MXmlNode* ModuleOptions = new MXmlNode(Document, "ModuleOptions");
  for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
    MXmlNode* ModuleNode = m_AvailableModules[a]->CreateXmlConfiguration();
    ModuleOptions->AddNode(ModuleNode);
  }

  // Store the module content
  Document->Save(FileName);

  delete Document;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::LoadGeometry()
{
  //! Load the geometry and transfer it to all modules

  m_Geometry = new MDGeometryQuest();

  if (m_Geometry->ScanSetupFile(m_GeometryFileName) == true) {
    mlog<<"Geometry "<<m_Geometry->GetName()<<" loaded!"<<endl;
    m_Geometry->ActivateNoising(false);
    m_Geometry->SetGlobalFailureRate(0.0);
  } else {
    mlog<<"Loading of geometry "<<m_GeometryFileName<<" failed!!"<<endl;
    delete m_Geometry;
    m_Geometry = 0;
    return false;
  }  

  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    m_Modules[m]->SetGeometry(m_Geometry);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::SetInterrupt(bool Flag) 
{ 
  // Set the interrupt which will break the analysis
  m_Interrupt = Flag; 
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    GetModule(m)->SetInterrupt(m_Interrupt);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::LaunchUI()
{
  m_Gui = new MGUIMainFretalon(this);
  m_Gui->SetProgramName(m_UIProgramName);
  m_Gui->SetPicturePath(m_UIPicturePath);
  m_Gui->SetSubTitle(m_UISubTitle);
  m_Gui->SetLeadAuthor(m_UILeadAuthor);
  m_Gui->SetCoAuthors(m_UICoAuthors);
  m_Gui->Create();
   
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Analyze()
{
  if (m_UseMultiThreading == true) {
    return AnalyzeMultiThreaded();
  } else {
    return AnalyzeSingleThreaded();
  }
}
 
////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::AnalyzeSingleThreaded()
{
  if (m_IsAnalysisRunning == true) return false;
  m_IsAnalysisRunning = true;

  // Start with saving the data:
  Save(m_ConfigurationFileName);

  m_Interrupt = false;
  
  // Start a global timer:
  MTimer Timer;

  // Load the geometry:
  if (LoadGeometry() == false) {
    m_IsAnalysisRunning = false;
    return false;
  }
  
  // Create a bunch of individual timers
  vector<MTimer> ModuleTimers(GetNModules(), MTimer(false));
  
  // Initialize the modules:
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    ModuleTimers[m].Continue();
    GetModule(m)->SetInterrupt(false);
    if (GetModule(m)->Initialize() == false) {
      ModuleTimers[m].Pause();
      if (m_Interrupt == true) {
        break;
      }
      mout<<"Initialization of module "<<GetModule(m)->GetName()<<" failed"<<endl;
      m_IsAnalysisRunning = false;
      return false;
    }
    ModuleTimers[m].Pause();
  }
  
  // Create the expo viewer:
  if (m_ExpoCombinedViewer == 0) {
    m_ExpoCombinedViewer = new MGUIExpoCombinedViewer();
    m_ExpoCombinedViewer->Create();
  }
  m_ExpoCombinedViewer->RemoveExpos();
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    if (GetModule(m)->HasExpos() == true) {
      m_ExpoCombinedViewer->AddExpos(GetModule(m)->GetExpos());
    }
  }
  m_ExpoCombinedViewer->OnReset();
  m_ExpoCombinedViewer->ShowExpos();
  
  // Do the pipeline
  MReadOutAssembly* Event = new MReadOutAssembly(); // will be loaded on start
  while (m_Interrupt == false) {
    // Reset the event to zero
    Event->Clear();

    // Some modules need to be made ready for the next event, do this here
    bool AllReady = true;
    bool AllOK = true;
    do {
      AllReady = true;
      AllOK = true;
      for (unsigned int m = 0; m < GetNModules(); ++m) {
        ModuleTimers[m].Continue();
        if (GetModule(m)->IsReady() == false) {
          if (m_Interrupt == true) break;
          mout<<"Module \""<<GetModule(m)->GetName()<<"\" is not yet ready..."<<endl;
          AllReady = false;
        }
        if (GetModule(m)->IsOK() == false) {
          mout<<"Module \""<<GetModule(m)->GetName()<<"\" is no longer OK... exiting analysis loop..."<<endl;
          AllOK = false;
        }
        ModuleTimers[m].Pause();
      }
      if (AllReady == false && AllOK == true) {
        cout<<"Not all modules ready (probably waiting for more data)... sleeping 100 ms"<<endl;
        gSystem->Sleep(100);
        gSystem->ProcessEvents();
      }
    } while (AllReady == false && AllOK == true && m_Interrupt == false);
    if (AllOK == false) {
      cout<<"One module had problems, exiting analysis loop"<<endl;
      break;
    }
    
    if (m_Interrupt == true) break;
      
    // Loop over all modules and do the analysis
    for (unsigned int m = 0; m < GetNModules(); ++m) {
      ModuleTimers[m].Continue();
      // Do the analysis
      if (GetModule(m)->AnalyzeEvent(Event) == false) {
        if (Event->GetID() != g_UnsignedIntNotDefined) {
          mout<<"Analysis failed for event "<<Event->GetID()
              <<" in module \""<<GetModule(m)->GetName()<<"\""<<endl;
        } 
        ModuleTimers[m].Pause();
        break;
      }
      ModuleTimers[m].Pause();
      /*
      if (Event->IsDataRead() == false) break;
      // Only analyze non-vetoed, triggered events
      if (Event->GetVeto() == true || Event->GetTrigger() == false) {
        break;
      }
      */
    }
    // if (Event->IsDataRead() == false) break;
    
    gSystem->ProcessEvents();    
  }
  
  // Finalize the modules:
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    ModuleTimers[m].Continue();
    GetModule(m)->Finalize();
    ModuleTimers[m].Pause();
  }
  
  mout<<endl;
  if (m_Interrupt == true) {
    mout<<"Nuclearizer: Analysis INTERRUPTED after "<<Timer.ElapsedTime()<<"s"<<endl;
  } else {
    mout<<"Nuclearizer: Analysis finished in "<<Timer.ElapsedTime()<<"s"<<endl;
  }
  mout<<endl;
  
  if (g_Verbosity >= 2) {
    cout<<"Timings: "<<endl;
    for (unsigned int m = 0; m < GetNModules(); ++m) {
      cout<<"Spent "<<ModuleTimers[m].GetElapsed()<<" sec in module "<<GetModule(m)->GetName()<<endl;
    }
  }
  
  m_IsAnalysisRunning = false;
  
  if (m_Terminate == true) Terminate();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::AnalyzeMultiThreaded()
{
  if (m_IsAnalysisRunning == true) return false;
  m_IsAnalysisRunning = true;
  
  // Start with saving the data:
  Save(m_ConfigurationFileName);

  m_Interrupt = false;
  
  // Start a global timer:
  MTimer Timer;

  // Load the geometry:
  if (LoadGeometry() == false) {
    m_IsAnalysisRunning = false;
    return false;
  }

  // Create a bunch of individual timers
  vector<MTimer> ModuleTimers(GetNModules(), MTimer(false));
  
  
  // Initialize the modules:
  if (GetNModules() == 0) {
    if (g_Verbosity >= c_Error) mout<<"Error: No modules"<<endl;
    return false;
  }
  if (GetNModules() == 1 && GetModule(0)->IsStartModule() == false) {
    if (g_Verbosity >= c_Error) mout<<"Error: The first module must either load or generate the events"<<endl;
    return false;
  }
  
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    ModuleTimers[m].Continue();
    GetModule(m)->SetInterrupt(false);
    GetModule(m)->UseMultiThreading(true);
    if (GetModule(m)->Initialize() == false) {
      ModuleTimers[m].Pause();
      if (m_Interrupt == true) {
        break;
      }
      mout<<"Initialization of module "<<GetModule(m)->GetName()<<" failed"<<endl;
      m_IsAnalysisRunning = false;
      return false;
    }
    ModuleTimers[m].Pause();
  }
  
  
  // Create the expo viewer:
  if (m_ExpoCombinedViewer == 0) {
    m_ExpoCombinedViewer = new MGUIExpoCombinedViewer();
    m_ExpoCombinedViewer->Create();
  }
  m_ExpoCombinedViewer->RemoveExpos();
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    if (GetModule(m)->HasExpos() == true) {
      m_ExpoCombinedViewer->AddExpos(GetModule(m)->GetExpos());
    }
  }
  m_ExpoCombinedViewer->OnReset();
  m_ExpoCombinedViewer->ShowExpos();
  
  
  // Do the analysis pipeline
  bool AllOK = true;
  while (m_Interrupt == false && AllOK == true) {

    for (unsigned int m = 0; m < GetNModules(); ++m) {
      ModuleTimers[m].Continue();
      MModule* M = GetModule(m);
      if (M->IsMultiThreaded() == false) { // We have to do the heavy lifing
        M->DoSingleAnalysis(); 
      }
      if (M->HasAnalyzedEvents() == true) {
        MReadOutAssembly* E = M->GetAnalyzedEvent();
        if (m < GetNModules()-1) {
          GetModule(m+1)->AddEvent(E);
        } else {
          delete E;
        }
      }

      if (M->IsOK() == false) {
        mout<<"Module \""<<GetModule(m)->GetName()<<"\" is no longer OK... exiting analysis loop..."<<endl;
        AllOK = false;
      }
      ModuleTimers[m].Pause();
    }
    
    gSystem->ProcessEvents();    
  }
  
  
  // Finalize the modules:
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    ModuleTimers[m].Continue();
    GetModule(m)->Finalize();
    ModuleTimers[m].Pause();
  }
  
  mout<<endl;
  if (m_Interrupt == true) {
    mout<<"Nuclearizer: Analysis INTERRUPTED after "<<Timer.ElapsedTime()<<"s"<<endl;
  } else {
    mout<<"Nuclearizer: Analysis finished in "<<Timer.ElapsedTime()<<"s"<<endl;
  }
  mout<<endl;
  
  if (g_Verbosity >= 2) {
    cout<<"Timings: "<<endl;
    for (unsigned int m = 0; m < GetNModules(); ++m) {
      cout<<"Spent "<<ModuleTimers[m].GetElapsed()<<" sec in module "<<GetModule(m)->GetName()<<endl;
    }
  }
  
  m_IsAnalysisRunning = false;

  if (m_Terminate == true) Terminate();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::View()
{
  // Show the view
  
  if (m_ExpoCombinedViewer != 0) {
     m_ExpoCombinedViewer->MapWindow();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::Exit()
{
  // Prepare to exit the application
  
  if (m_IsAnalysisRunning == true) {
    m_Interrupt = true;
    m_Terminate = true;
  } else {
    Terminate();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::Terminate()
{
  // Exit the application
  
  Save(m_ConfigurationFileName);
  gApplication->Terminate(0);
}


// MSupervisor.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
