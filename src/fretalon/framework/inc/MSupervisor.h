/*
 * MSupervisor.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSupervisor__
#define __MSupervisor__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MDGeometryQuest.h"

// Forward declarations:
class MModule;
class MGUIMainFretalon;
class MGUIExpoCombinedViewer;
class MGUIExpoSupervisor;


////////////////////////////////////////////////////////////////////////////////


//! The dsupervisor class implemented as a singleton 
class MSupervisor
{
  //! public interface:
 public:
  //! Default constructor
  static MSupervisor* GetSupervisor();

  //! Reset all data
  void Clear();

  //! Set the name of the default configuration file
  //! e.g. ~/.fretalon.cfg
  void SetConfigurationFileName(const MString& FileName) { m_ConfigurationFileName = FileName; }

  //! Load all data from a file
  bool Load(MString FileName = "");
  //! Save all data to a file
  bool Save(MString FileName);

  //! Add an available module
  void AddAvailableModule(MModule* Module) { m_AvailableModules.push_back(Module); }
  //! Return the number of available modules
  unsigned int GetNAvailableModules() { return m_AvailableModules.size(); }
  //! Return the available modules at position i --- no error checks are performed  
  MModule* GetAvailableModule(unsigned int i);
  //! Return the module with the given name OR XML tag or a nullptr in case there is none --- no error checks are performed  
  MModule* GetAvailableModuleByName(MString Name);
  //! Return the module with the given XML tag or a nullptr in case there is none --- no error checks are performed  
  MModule* GetAvailableModuleByXmlTag(MString XmlTag);

  //! Return the number of modules in the current sequence
  unsigned int GetNModules() { return m_Modules.size(); }
  //! Return the modules at position i in the current sequence --- no error checks are performed  
  MModule* GetModule(unsigned int i);
  //! Set a module at a specific position - return false if other modules had to be eliminated  
  bool SetModule(MModule* Module, unsigned int i);
  //! Remove module at a specific position - return false if other modules had to be eliminated  
  bool RemoveModule(unsigned int i);

  //! Return a list of possible volumes, which might to follow
  vector<MModule*> ReturnPossibleVolumes(unsigned int Position);
  //! Return a list of possible volumes, which might to follow
  vector<MModule*> ReturnPossibleVolumes(vector<MModule*>& Previous);

  //! Set the name of the geometry file
  void SetGeometryFileName(MString GeometryFileName) { m_GeometryFileName = GeometryFileName; }
  //! Return the name of the geometry file
  MString GetGeometryFileName() const { return m_GeometryFileName; }

  //! Return the geometry itself
  MDGeometryQuest* GetGeometry() const { return m_Geometry; }

  //! Load the geometry and transfer it to all modules
  bool LoadGeometry();

  //! Launch the UI
  bool LaunchUI();
  
  //! Analyze the data - single-threaded or multi-threaded mode
  bool Analyze();
  //! Return true if the analysis is running
  bool IsAnalysisisRunning() const { return m_IsAnalysisRunning; }
  
  //! Exit the application - if multi-threaded prepare to exit after all threads have exited
  void Exit();

  //! Show the expo view
  void View();
  
  //! Set the soft interrupt status for ending the analysis
  void SetSoftInterrupt(bool Flag = true);
  //! Get the soft interrupt status for ending the analysis
  bool GetSoftInterrupt() { return m_SoftInterrupt; }
  //! Set the hard interrupt status for ending the analysis
  void SetHardInterrupt(bool Flag = true);
  //! Get the hard interrupt status for ending the analysis
  bool GetHardInterrupt() { return m_HardInterrupt; }
  
  //! Choose if to use multi-threading
  void UseMultiThreading(bool Flag = true) { m_UseMultiThreading = Flag; }

  //! Use the UI
  void UseUI(bool UIUse) { m_UIUse = UIUse; }
  //! Set the program name
  void SetUIProgramName(const MString& ProgramName) { m_UIProgramName = ProgramName; }
  //! Set the UI Picture file name, and an alternative text if the picture could not be loaded
  void SetUIPicturePath(const MString& Path) { m_UIPicturePath = Path; }
  //! Set the sub title below the picture
  void SetUISubTitle(const MString& SubTitle) { m_UISubTitle = SubTitle; }
  //! Set the lead author
  void SetUILeadAuthor(const MString& LeadAuthor) { m_UILeadAuthor = LeadAuthor; }
  //! Set the co-author list
  void SetUICoAuthors(const MString& CoAuthors) { m_UICoAuthors = CoAuthors; }

  
  // protected methods:
 protected:
  //! Analyze the data - single-threaded mode
  //bool AnalyzeSingleThreaded();
  //! Analyze the data - multi-threaded mode
  //bool AnalyzeMultiThreaded();
 
  //! End the program (and saves the GUI data)
  void Terminate();


  //! Validate the sequence of possible modules
  bool Validate();

  
  // private methods:
 private:
  //! Default constructor
  MSupervisor();
  //! Default destructor
  ~MSupervisor();
  //! No copy constructor
  MSupervisor(MSupervisor const&) = delete;
  //! No assignment operator
  void operator=(MSupervisor const&) = delete;

  // protected members:
 protected:


  // private members:
 private:
  //! Name of the configuration file
  MString m_ConfigurationFileName; 
   
  //! List of all available modules
  vector<MModule*> m_AvailableModules;

  //! Sequence of currently used modules
  vector<MModule*> m_Modules;

  //! The geometry file name
  MString m_GeometryFileName;
  //! The geometry
  MDGeometryQuest* m_Geometry;

  //! The main graphical user interface
  MGUIMainFretalon* m_Gui;
  //! The expos - main combined viewer
  MGUIExpoCombinedViewer* m_ExpoCombinedViewer;
  //! The supervisor's own expo
  MGUIExpoSupervisor* m_ExpoSupervisor;
  

  //! The soft interrupt flag --- graciously stops the analysis, by finishing all started events
  bool m_SoftInterrupt;
  //! The hard interrupt flag --- kills the analysis immediately
  bool m_HardInterrupt;
  //! The terminate flag - should always be set together with the interrupt flag
  //! After the analysis is stopped by the interupt flag, this flag will terminate the 
  //! program
  bool m_Terminate;
  
  //! True if multi-threading is enabled
  bool m_UseMultiThreading;
  
  //! True if the analysis is currently underway
  bool m_IsAnalysisRunning;
  
  //! Use the UI
  bool m_UIUse;
  //! The program name
  MString m_UIProgramName;
  //! The path to the picture in the GUI
  MString m_UIPicturePath;
  //! The sub title below the picture in the GUI
  MString m_UISubTitle;
  //! The lead author for the Info GUI
  MString m_UILeadAuthor;
  //! The co-authors for the Info GUI
  MString m_UICoAuthors;

  
#ifdef ___CINT___
 public:
  ClassDef(MSupervisor, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
