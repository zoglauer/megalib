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
#include <iomanip>
#include <thread>
using namespace std;

// ROOT libs:
#include "TSystem.h"
#include "TApplication.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"
#include "MGUIMainFretalon.h"
#include "MGUIExpoCombinedViewer.h"
#include "MGUIExpoSupervisor.h"
#include "MModule.h"
#include "MReadOutAssembly.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MSupervisor)
#endif


////////////////////////////////////////////////////////////////////////////////


MSupervisor* MSupervisor::GetSupervisor()
{
  // Get the singleton
  
  static MSupervisor Instance;
  return &Instance;
}


////////////////////////////////////////////////////////////////////////////////


MSupervisor::MSupervisor()
{
  // Construct an instance of MSupervisor

  m_SoftInterrupt = false;
  m_HardInterrupt = false;
  m_Terminate = false;
  
  m_UseMultiThreading = false;
  m_IsAnalysisRunning = false;

  m_UIUse = true;
  m_UIProgramName = "Unnamed program";
  m_UIPicturePath = "";
  m_UISubTitle = "No sub title set!";
  m_UILeadAuthor = "No lead author!";
  m_UICoAuthors = "";
  
  m_Geometry = nullptr;
  m_Gui = nullptr;
  m_ExpoCombinedViewer = nullptr;
  m_ExpoSupervisor = nullptr;
  
  m_ConfigurationFileName = "~/.fretalon.cfg";

  // All the rest:
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::Clear()
{
  //! Reset all data

  m_Modules.clear();
  
  // Don't delete those! Otherwise we have to add improved handling of when 
  // the canvases within the UI's get deleted...
  // delete m_Gui;
  // delete m_ExpoCombinedViewer;
  // m_ExpoCombinedViewer = nullptr;
  if (m_ExpoCombinedViewer != nullptr) {
    m_ExpoCombinedViewer->CloseWindow();
    m_ExpoCombinedViewer->RemoveExpos();
  }
  
  delete m_ExpoSupervisor;
  m_ExpoSupervisor = nullptr;
  
  m_GeometryFileName = "";
  delete m_Geometry;
  m_Geometry = nullptr;
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

  return nullptr; 
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetAvailableModule(unsigned int i) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  if (i < m_AvailableModules.size()) {
    return m_AvailableModules[i]; 
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetAvailableModuleByName(MString Name) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    if (m_AvailableModules[m]->GetName() == Name) {
      return m_AvailableModules[m];
    }
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MModule* MSupervisor::GetAvailableModuleByXmlTag(MString XmlTag) 
{ 
  //! Return the modules at position i in the current sequence --- no error checks are performed  

  for (unsigned int m = 0; m < m_AvailableModules.size(); ++m) {
    if (m_AvailableModules[m]->GetXmlTag() == XmlTag) {
      return m_AvailableModules[m];
    }
  }

  return nullptr;
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

  //cout<<"Assembling possible modules for position: "<<Position<<" --> "<<Previous.size()<<endl;

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


    // If any of the existing modules has it is as sort predecessor requirement, ignore it.
    if (Passed == true) {
      bool SoftPredecessor = false;
      for (unsigned int p = 0; p < Previous.size(); ++p) {
        for (unsigned int at = 0; at < m_AvailableModules[i]->GetNModuleTypes(); ++at) {
          if (Previous[p]->IsSoftPreceedingModule(m_AvailableModules[i]->GetModuleType(at)) == true) {
            SoftPredecessor = true;
            break;
          }
        }
      }
      if (SoftPredecessor == true) {
        //mout<<"Failed module type already is soft predecessor requirement for another module"<<endl;
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
  vector<uint64_t> PredecessorTypes; // This stores the modules which came before m
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    for (unsigned int t = 0; t < m_Modules[m]->GetNPreceedingModuleTypes(); ++t) {
      // (a) Check if a required predecessor is in the existing predecessor list 
      bool Found = false;
      for (unsigned int p = 0; p < PredecessorTypes.size(); ++p) {
        if (PredecessorTypes[p] == m_Modules[m]->GetPreceedingModuleType(t)) {
          Found = true;
        }
      }
      // (b) is not check if it is a hard or soft requirement
      if (Found == false) {
        // if it is a hard requirement than we have to quit
        if (m_Modules[m]->GetPreceedingModuleHardRequirement(t) == true) {
          mout<<"Hard predecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
          if (m < ValidUntil) {
            ValidUntil = m;
          }
          break;
        }
        // if it is just a soft requirement, make sure it does not show up in the rest of the list
        else {
          bool FoundAfter = false;
          for (unsigned int m2 = m+1; m2 < m_Modules.size(); ++m2) {
            if (m_Modules[m2]->ProvidesModuleType(m_Modules[m]->GetPreceedingModuleType(t)) == true) {
              FoundAfter = true;
              break;
            }
          }
          if (FoundAfter == true) {
            mout<<"Soft predecessor requirements for module "<<m_Modules[m]->GetName()<<" are not fullfilled"<<endl;
            if (m < ValidUntil) {
              ValidUntil = m;
            }
            break;            
          }
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


bool MSupervisor::ChangeConfiguration(MString NewField)
{
  // Change one settings field
  
  // Parse the data
  MString Nodes = NewField.GetSubString(0, NewField.First('='));
  vector<MString> NodeNames = Nodes.Tokenize(".");
  MString Value = NewField.GetSubString(NewField.First('=') + 1);
  
  //for (MString S: NodeNames) cout<<S<<endl;
  //cout<<"Value: "<<Value<<endl;
  
  // Find the correct field
  MXmlDocument* Master = CreateXmlConfiguration();
  
  MXmlNode* Iter = Master;
  for (MString S: NodeNames) {
    MXmlNode* Node = Iter->GetNode(S);
    if (Node == nullptr) {
      cout<<"Error: Unable to find node "<<S<<" under node "<<Iter->GetName()<<endl;
      return false;
    }
    Iter = Node;
  }
  
  // Set the data
  Iter->SetValue(Value);
  
  // Read everything back
  ReadXmlConfiguration(Master);
  
  // Cleanup
  delete Master;
  
  return true;  
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Load(MString FileName)
{
  // Load all data from a file

  Clear();

  if (FileName == "") FileName = m_ConfigurationFileName;
  MFile::ExpandFileName(FileName);
  
  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument();
  Document->Load(FileName);

  ReadXmlConfiguration(Document);

  delete Document;

  return true;
} 


////////////////////////////////////////////////////////////////////////////////


//! Read the configuration data from an XML node
bool MSupervisor::ReadXmlConfiguration(MXmlDocument* Document)
{
  int Version = 1;
  MXmlNode* VersionNode = Document->GetNode("Version");
  if (VersionNode != nullptr) {
    Version = VersionNode->GetValueAsInt();
  }
  if (Version != 1) {
    mout<<"Info: While we do have a higher version than 1, everything is still fixed at version 1"<<endl;
  }

  MXmlNode* ModuleSequence = Document->GetNode("ModuleSequence");
  if (ModuleSequence != 0) {
    m_Modules.clear();
    for (unsigned int m = 0; m < ModuleSequence->GetNNodes(); ++m) {
      MModule* M = GetAvailableModuleByXmlTag(ModuleSequence->GetNode(m)->GetValue());
      if (M == nullptr) M = GetAvailableModuleByName(ModuleSequence->GetNode(m)->GetValue());  // for compatibility with old format
      if (M != nullptr) {
        m_Modules.push_back(M);
      } else {
        mout<<"Error: Cannot find a module with name: "<<ModuleSequence->GetNode(m)->GetValue()<<endl;
      }
    }
  }

  MXmlNode* GeometryFileName = Document->GetNode("GeometryFileName");
  if (GeometryFileName != nullptr) {
    m_GeometryFileName = GeometryFileName->GetValue();
  }

  MXmlNode* ModuleOptions = Document->GetNode("ModuleOptions");
  if (ModuleOptions != nullptr) {
    for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
      MXmlNode* ModuleNode = ModuleOptions->GetNode(m_AvailableModules[a]->GetXmlTag());
      if (ModuleNode != nullptr) {
        m_AvailableModules[a]->ReadXmlConfiguration(ModuleNode);
      }
    }
  }
  
  return true;
}
 

////////////////////////////////////////////////////////////////////////////////


//! Create an XML node tree from the configuration
MXmlDocument* MSupervisor::CreateXmlConfiguration()
{
  // Create a XML document describing the data:
  MXmlDocument* Document = new MXmlDocument("NuclearizerData");

  // Version string: 1
  new MXmlNode(Document, "Version", 1);

  MXmlNode* ModuleSequence = new MXmlNode(Document, "ModuleSequence");
  // Store the module sequence
  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    new MXmlNode(ModuleSequence, "ModuleSequenceItem", m_Modules[m]->GetXmlTag());
  }

  // Store the file names
  new MXmlNode(Document, "GeometryFileName", m_GeometryFileName);

  MXmlNode* ModuleOptions = new MXmlNode(Document, "ModuleOptions");
  for (unsigned int a = 0; a < m_AvailableModules.size(); ++a) {
    MXmlNode* ModuleNode = m_AvailableModules[a]->CreateXmlConfiguration();
    ModuleOptions->AddNode(ModuleNode);
  }  
  
  return Document;
}


////////////////////////////////////////////////////////////////////////////////


bool MSupervisor::Save(MString FileName)
{
  //! Save all data to a file

  // Create the XML tree
  MXmlDocument* Document = CreateXmlConfiguration();

  // Store the module content
  MFile::ExpandFileName(FileName);
  Document->Save(FileName);

  // Clean up
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
    m_Geometry = nullptr;
    return false;
  }  

  for (unsigned int m = 0; m < m_Modules.size(); ++m) {
    m_Modules[m]->SetGeometry(m_Geometry);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::SetSoftInterrupt(bool Flag) 
{ 
  // Set the interrupt which will break the analysis
  m_SoftInterrupt = Flag; 
  if (m_SoftInterrupt == false) m_HardInterrupt = false;
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::SetHardInterrupt(bool Flag) 
{ 
  // Set the interrupt which will break the analysis
  m_HardInterrupt = Flag; 
  if (m_HardInterrupt == true) m_SoftInterrupt = true;
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
  if (m_IsAnalysisRunning == true) return false;
  m_IsAnalysisRunning = true;
  
  // Start with saving the data:
  Save(m_ConfigurationFileName);

  m_SoftInterrupt = false;
  m_HardInterrupt = false;
  
  // Start a global timer:
  MTimer Timer;

  // Load the geometry:
  if (LoadGeometry() == false) {
    m_IsAnalysisRunning = false;
    return false;
  }

  
  // Initialize the modules:
  if (GetNModules() == 0) {
    if (g_Verbosity >= c_Error) mout<<"Error: No modules"<<endl;
    m_IsAnalysisRunning = false;
    return false;
  }
  if (GetNModules() == 1 && GetModule(0)->IsStartModule() == false) {
    if (g_Verbosity >= c_Error) mout<<"Error: The first module must either load or generate the events"<<endl;
    m_IsAnalysisRunning = false;
    return false;
  }
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    if (g_Verbosity >= c_Info) cout<<"Initializing: "<<GetModule(m)->GetName()<<": "<<long(GetModule(m))<<endl;
    GetModule(m)->SetInterrupt(false);
    GetModule(m)->UseMultiThreading(m_UseMultiThreading);
    GetModule(m)->ClearQueues(); // Just in case a module did not call Finalize...
    if (GetModule(m)->Initialize() == false) {
      if (m_SoftInterrupt == true) {
        break;
      }
      mout<<"Initialization of module "<<GetModule(m)->GetName()<<" failed"<<endl;
      m_IsAnalysisRunning = false;
      return false;
    }
  }
  
  
  // Create the expo viewer:
  int NExpos = 0;
  if (m_UIUse == true) {
    if (m_ExpoCombinedViewer == nullptr) {
      m_ExpoCombinedViewer = new MGUIExpoCombinedViewer();
      m_ExpoCombinedViewer->Create();
    }
    m_ExpoCombinedViewer->RemoveExpos();


    delete m_ExpoSupervisor;
    m_ExpoSupervisor = new MGUIExpoSupervisor();   
    m_ExpoCombinedViewer->AddExpo(m_ExpoSupervisor);
    ++NExpos;

    for (unsigned int m = 0; m < GetNModules(); ++m) {
      if (GetModule(m)->HasExpos() == true) {
        m_ExpoCombinedViewer->AddExpos(GetModule(m)->GetExpos());
        ++NExpos;
      }
    }
    m_ExpoCombinedViewer->OnReset();
  
    m_ExpoSupervisor->Reset();
    m_ExpoSupervisor->SetNModules(GetNModules());
    for (unsigned int m = 0; m < GetNModules(); ++m) {
      m_ExpoSupervisor->SetModuleName(m, GetModule(m)->GetName());
    }
    
    m_ExpoCombinedViewer->ShowExpos();
    if (NExpos == 0) {
      gSystem->ProcessEvents();
      m_ExpoCombinedViewer->CloseWindow(); 
      gSystem->ProcessEvents();
    }
  }
  
  
  // Do the analysis pipeline
  
  // We have events 
  bool HasMoreEvents = false;
  bool DoShutdown = false;
  
  // Timing stuff
  long NPasses = 0;
  MTimer LastUIUpdate;
  long NumberOfUIUpdates = 0;
  MTime TimeAssemblyExitedStartModule(0);
  
  // Number of instances per module
  unsigned int MaxInstances = thread::hardware_concurrency() / 2;
  if (MaxInstances < 2) MaxInstances = 2;
  
  // Create a local list of the modules
  vector<vector<MModule*>> Modules;
  for (unsigned int m = 0; m < GetNModules(); ++m) {
    vector<MModule*> List;
    List.push_back(GetModule(m));
    Modules.push_back(List);
  }
  
  while (true) {

    if (m_HardInterrupt == true) break;
    
    if (m_SoftInterrupt == true) {
      DoShutdown = true;
      // Each start module gets the interrupt signal:
      for (unsigned int m = 0; m < Modules.size(); ++m) {
        for (unsigned int s = 0; s < Modules[m].size(); ++s) {
          if (Modules[m][s]->IsStartModule() == true) {
            Modules[m][s]->SetInterrupt(true);
          }
        }
      }
    }
   
    // We ensured before that "0" exists 
    if (GetModule(0)->IsStartModule() == true && GetModule(0)->IsFinished() == true) {
      DoShutdown = true; 
    }
    
    HasMoreEvents = false;
    for (unsigned int m = 0; m < Modules.size(); ++m) {
      for (unsigned int s = 0; s < Modules[m].size(); ++s) {
      
        MModule* M = Modules[m][s];

        if (HasMoreEvents == false && (M->HasAddedReadOutAssemblies() == true || M->HasAnalyzedReadOutAssemblies() == true)) {
          HasMoreEvents = true;
          //cout<<"Events for module "<<m<<endl; 
        } else {
          //cout<<"No events for module "<<m<<endl; 
        }

        if (M->IsMultiThreaded() == false) { // We have to do the heavy lifting in this thread
          //cout<<"Doing analysis for module "<<M->GetName()<<endl;
          M->DoSingleAnalysis(); 
        }
        if (M->HasAnalyzedReadOutAssemblies() == true) {
          if (m == 0) {
            TimeAssemblyExitedStartModule.Now();
          }
          MReadOutAssembly* ROA = M->GetAnalyzedReadOutAssembly();
          if (ROA->IsFilteredOut() == true) {
            //cout<<"ROA "<<ROA->GetID()<<" has been filterered out"<<endl;
            delete ROA;
          } else {
            if (m < Modules.size()-1) {
              //cout<<"Adding ROA "<<ROA->GetID()<<" to module "<<Modules[m+1][0]->GetName()<<endl;
              Modules[m+1][0]->AddReadOutAssembly(ROA); 
              // Remark: the modules share their queue, so it does not matter to which we add the event
            } else {
              if (m == Modules.size()-1) {
                //cout<<"ROA "<<ROA->GetID()<<" passed through all modules"<<endl;
                delete ROA;
              }
            }
          }
        }

        if (M->IsOK() == false && DoShutdown == false) {
          mout<<"Module \""<<GetModule(m)->GetName()<<"\" is no longer OK... exiting analysis loop..."<<endl;
          DoShutdown = true;
        }
      }
    }
    
    if (DoShutdown == true && HasMoreEvents == false) break;

    // Update the GUI infrequently and check for spawn even less frequently
    if (++NPasses % 100 == 0) {
             
      // Check & create spawns:
      if (NPasses % 10000 == 0) {
        double Last = 0;
        for (unsigned int m = 0; m < Modules.size(); ++m) {
          double Now = 0;
          for (unsigned int s = 0; s < Modules[m].size(); ++s) {
            Now += Modules[m][s]->GetNumberOfAnalyzedEvents();
          }
          if (Now + 40000*Modules[m].size() < Last) { // I know...
            if (Modules[m][0]->AllowsMultipleInstances() == true && Modules[m].size() < MaxInstances) {
              MModule* M = Modules[m][0]->Clone();
              MXmlNode* Node = Modules[m][0]->CreateXmlConfiguration();
              M->ReadXmlConfiguration(Node);
              delete Node;
              M->SetInterrupt(false);
              M->UseMultiThreading(m_UseMultiThreading);
              Modules[m][0]->ShareQueues(M);
              if (M->Initialize() == false) {
                delete M;
              } else {
                Modules[m].push_back(M);
              }
              cout<<"Spawned module: "<<M->GetName()<<endl;
            }
          }
          Last = Now;
        }
      }

      // Update the processing time:
      if (m_UIUse == true) {
        for (unsigned int m = 0; m < Modules.size(); ++m) {
          long ProcessedEvents = 0;
          double ProcessingTime = 0;
          for (unsigned int s = 0; s < Modules[m].size(); ++s) {
            ProcessedEvents += Modules[m][s]->GetNumberOfAnalyzedEvents();
            ProcessingTime += Modules[m][s]->GetProcessingTime();
          }
          m_ExpoSupervisor->SetProcessedEvents(m, ProcessedEvents);
          m_ExpoSupervisor->SetProcessingTime(m, ProcessingTime);
          m_ExpoSupervisor->SetInstances(m, Modules[m].size());
        }
        m_ExpoSupervisor->SetLastProcessingTime(TimeAssemblyExitedStartModule);
        
        // Update the UI
        // Little trick: Start with frequent updates after 1 sec, 2 sec, 3 sec, 4 sec
        // and then after each 5 seconds have past
        double UIElapsed = LastUIUpdate.GetElapsed();
        if (UIElapsed > 5.0 || UIElapsed > NumberOfUIUpdates) { 
          m_ExpoCombinedViewer->OnUpdate();
          LastUIUpdate.Reset();
          ++NumberOfUIUpdates;
        }
      }
    } // UI updates and spwan checks
    
    
    // If any module is behind more than MaxBehind the one before and has reached its maximum number of
    // instances, we slow the first modules down
    int MaxBehind = 200000;
    if (m_UseMultiThreading == true) {
      bool Pause = false;
      for (unsigned int m = 0; m < Modules.size()-1; ++m) {
        long Behind = 0;
        for (unsigned int s = 0; s < Modules[m+1].size(); ++s) {
          Behind -= Modules[m+1][s]->GetNumberOfAnalyzedEvents();
        }
        for (unsigned int s = 0; s < Modules[m].size(); ++s) {
          Behind += Modules[m][s]->GetNumberOfAnalyzedEvents();
        }
        if ((Modules[m+1].size() == MaxInstances || Modules[m+1][0]->AllowsMultipleInstances() == false) && Behind > MaxBehind) {
          Pause = true;
        }
      }
      if (Pause == true) {
        for (unsigned int s = 0; s < Modules.front().size(); ++s) {
          if (Modules.front()[s]->AllowPausing() == true) {
            Modules.front()[s]->Pause(true);
          }
        }
      } else {
        for (unsigned int s = 0; s < Modules.front().size(); ++s) {
          if (Modules.front()[s]->IsPaused() == true) {
            Modules.front()[s]->Pause(false);
          }
        }
      }      
    }
    
    
    // If there were no events --- sleep a bit
    if (m_UseMultiThreading == true && HasMoreEvents == false) {
      gSystem->Sleep(10); 
    }

    gSystem->ProcessEvents();    
  } // big while loop
  
  
  // Finalize the modules:
  for (unsigned int m = 0; m < Modules.size(); ++m) {
    for (unsigned int s = 0; s < Modules[m].size(); ++s) {
      Modules[m][s]->Finalize();
    }
  }

  if (m_UIUse == true) {
    m_ExpoCombinedViewer->OnUpdate();
  }
  
  cout<<endl;
  if (m_SoftInterrupt == true) {
    cout<<"Nuclearizer: Analysis INTERRUPTED after "<<Timer.ElapsedTime()<<"s"<<endl;
  } else {
    cout<<"Nuclearizer: Analysis finished in "<<Timer.ElapsedTime()<<"s"<<endl;
  }
  
  //if (g_Verbosity >= c_Error) {
  ios::fmtflags SavedFlags(cout.flags());
  cout.setf(ios::fixed);
  cout.precision(1);
  cout<<endl;
  cout<<"Summary: "<<endl;
  for (unsigned int m = 0; m < Modules.size(); ++m) {
    long ProcessedEvents = 0;
    double ProcessingTime = 0;
    double SleepingTime = 0;
    for (unsigned int s = 0; s < Modules[m].size(); ++s) {
      ProcessedEvents += Modules[m][s]->GetNumberOfAnalyzedEvents();
      ProcessingTime += Modules[m][s]->GetProcessingTime();
      SleepingTime += Modules[m][s]->GetSleepingTime();
    }
    if (m_UIUse == true) {
      m_ExpoSupervisor->SetProcessedEvents(m, ProcessedEvents);
      m_ExpoSupervisor->SetProcessingTime(m, ProcessingTime);
      m_ExpoSupervisor->SetInstances(m, Modules[m].size());
    }
    
    cout<<"Spent "<<ProcessingTime<<" sec analyzing ";
    cout<<"(vs. "<<SleepingTime<<" sec sleeping) ";
    cout<<"in module \""<<Modules[m][0]->GetName()<<"\" ";
    cout<<"utilizing "<<Modules[m].size()<<" instance"<<(Modules[m].size() > 1 ? "s " : " ");
    cout<<"and processed "<<ProcessedEvents<<" events."<<endl;
  }
  cout.flags(SavedFlags);
  //}

  // A final UI update:
  if (m_UIUse == true) {
    m_ExpoCombinedViewer->OnUpdate();
  }
  
  m_IsAnalysisRunning = false;

  if (m_Terminate == true) Terminate();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::View()
{
  // Show the view
  
  if (m_ExpoCombinedViewer != nullptr && m_ExpoCombinedViewer->GetNExpos() > 0) {
     m_ExpoCombinedViewer->MapWindow();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MSupervisor::Exit()
{
  // Prepare to exit the application
  
  if (m_IsAnalysisRunning == true) {
    m_SoftInterrupt = true;
    m_HardInterrupt = true;
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
