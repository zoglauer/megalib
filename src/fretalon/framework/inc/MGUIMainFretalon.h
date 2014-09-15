/*
* MGUIMainFretalon.h
*
* Copyright (C) by Andreas Zoglauer.
* All rights reserved.
*
* Please see the source-file for the copyright-notice.
*
*/


#ifndef __MGUIMainFretalon__
#define __MGUIMainFretalon__


////////////////////////////////////////////////////////////////////////////////


// Standard libs

// ROOT libs
#include <TGMenu.h>
#include <TGButton.h>
#include <TGFrame.h>
#include <TGFileDialog.h>
#include <TGIcon.h>
#include <TGPicture.h>

// MEGAlib libs
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSupervisor.h"
#include "MGUIEModule.h"


////////////////////////////////////////////////////////////////////////////////


class MGUIMainFretalon : public TGMainFrame
{
  // Public members:
public:
  //! Default constructor
  MGUIMainFretalon(MSupervisor* Supervisor);
  //! Default destructor
  virtual ~MGUIMainFretalon();

  //! Set the program name
  void SetProgramName(const MString& ProgramName) { m_ProgramName = ProgramName; }
  //! Set the UI Picture file name, and an alternative text if the picture could not be loaded
  void SetPicturePath(const MString& Path) { m_PicturePath = Path; }
  //! Set the sub title below the picture
  void SetSubTitle(const MString& SubTitle) { m_SubTitle = SubTitle; }
  //! Set the lead author
  void SetLeadAuthor(const MString& LeadAuthor) { m_LeadAuthor = LeadAuthor; }
  //! Set the co-author list
  void SetCoAuthors(const MString& CoAuthors) { m_CoAuthors = CoAuthors; }
  
  //! Create the GUI
  virtual void Create();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  //! Called when the "x" is pressed
  virtual void CloseWindow();

  //! Handle some keys
  bool HandleKey(Event_t* Event);

  // protected members:
protected:
  //! Update the module section
  void UpdateModules();

  //! Actions when the change button has been pressed
  virtual bool OnChange(unsigned int ModuleID);
  //! Actions when the remove button has been pressed
  virtual bool OnRemove(unsigned int ModuleID);
  //! Actions when the options button has been pressed
  virtual bool OnOptions(unsigned int ModuleID);
  //! Actions when the apply button has been pressed
  virtual bool OnApply();
  //! Actions when the start button has been pressed
  virtual bool OnStart();
  //! Actions when the exit button has been pressed
  virtual bool OnExit();
  //! Actions when the stop button has been pressed
  virtual bool OnStop();
  //! Actions when the view button has been pressed
  virtual bool OnView();
  //! Actions when the save key has been pressed
  virtual bool OnSaveConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnLoadConfiguration();
  //! Actions when the load key has been pressed
  virtual bool OnGeometry();
  //! Actions when the about button has been pressed
  virtual bool OnAbout();

  // private members:
private:
  //! Reference to the supervisor with all the relevant data
  MSupervisor* m_Supervisor;

  // Some common used GUI elements
  
  //! The program name
  MString m_ProgramName;
  //! The path to the picture
  MString m_PicturePath;
  //! The sub title below the picture
  MString m_SubTitle;
  //! The lead author
  MString m_LeadAuthor;
  //! The co-authors
  MString m_CoAuthors;
  
  //! The frame for the modules
  TGVerticalFrame* m_ModuleFrame;
  //! The layout of an module
  TGLayoutHints* m_ModuleLayout;
  
  //! List of the modules
  vector<MGUIEModule*> m_Modules;


  // IDs:
  static const int c_Start      =   1;
  static const int c_Exit       =   2;
  static const int c_LoadConfig =   3;
  static const int c_SaveConfig =   4;
  static const int c_Geometry   =   5;
  static const int c_About      =   6;
  static const int c_Stop       =   7;
  static const int c_View       =   8;
  static const int c_Remove     = 400;
  static const int c_Options    = 500;
  static const int c_Change     = 600;

#ifdef ___CINT___
public:
  ClassDef(MGUIMainFretalon, 0) // main window of the Nuclearizer GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
