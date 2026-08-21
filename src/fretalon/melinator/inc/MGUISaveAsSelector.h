/*
 * MGUISaveAsSelector.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISaveAsSelector__
#define __MGUISaveAsSelector__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsMelinator.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"
#include "MGUIEMinMaxEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! GUI dialog to select the default file name for ecal, fits, and report
class MGUISaveAsSelector : public MGUIDialog
{
  // Public Interface:
 public:
  //! Default constructor
  MGUISaveAsSelector(const TGWindow* Parent, const TGWindow* Main, MSettingsMelinator* Settings, bool& OKPressed);
  //! Default destructor
  virtual ~MGUISaveAsSelector();

  
  // protected methods:
 protected:
  //! Create the UI
  virtual void Create();
  //! Apply changes
  virtual bool OnApply();
  //! Process all messages
  bool ProcessMessage(long Message, long Parameter1, long Parameter2);



  // protected members:
 protected:


  // private members:
 private:
  //! The return message that OK was pressed
  bool& m_OkPressed;

  //! The settings file
  MSettingsMelinator* m_Settings;

  //! The report file name
  MGUIEFileSelector* m_FileName;


#ifdef ___CLING___
 public:
  ClassDef(MGUISaveAsSelector, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
