/*
 * MGUIOptions.h
 *
 * Copyright (C) 2008-2008 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptions__
#define __MGUIOptions__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TVirtualX.h>
#include <TGWindow.h>
#include <TObjArray.h>
#include <TGFrame.h>
#include <TGButton.h>
#include <MString.h>
#include <TGClient.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIERBList.h"
#include "MModule.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptions : public TGTransientFrame
{
  // public Session:
 public:
  //! Default constructor
  MGUIOptions(MModule* Module);
  //! Default destructor
  virtual ~MGUIOptions();

  //! Close the window
  void CloseWindow();
  //! Process all button, etc. messages
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  
  //! The creation part which gets overwritten
  virtual void Create() = 0;


  // protected methods:
 protected:
  //! Create the Gui -- initial step
  void PreCreate();
  //! Create the Gui -- final step
  void PostCreate();

  //! Actions after the OK button has been pressed
  virtual bool OnOk();
  //! Actions after the Cancel button has been pressed
  virtual bool OnCancel();
  //! Actions after the Apply or OK button has been pressed
  virtual bool OnApply();


  // protected members:
 protected:
  //! The module
  MModule* m_Module;

  //! The font scaler (normal font size is 12 - if we have anything larger we need to scale some GUIs)
  double m_FontScaler;

  //! The main frame to which to add all other content
  TGVerticalFrame* m_OptionsFrame;
  
  enum BasicButtonIDs { e_Ok = 1, e_Cancel, e_Apply };

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptions, 1) // basic class for dialog windows
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
