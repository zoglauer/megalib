/*
 * MGUIMGeant.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIMGeant__
#define __MGUIMGeant__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>
#include <TGButton.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsGeomega.h"
#include "MGUIEFileSelector.h"
#include "MGUIECBList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIMGeant : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIMGeant(const TGWindow* Parent, const TGWindow* Main, 
             MSettingsGeomega* Data, bool& OkPressed);
  virtual ~MGUIMGeant();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool ProcessMessage(long Message, long Parameter1, long Parameter2);
  virtual bool OnApply();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsGeomega* m_GUIData;

  TGRadioButton* m_StandardName;
  TGRadioButton* m_SpecialName;

  MGUIEFileSelector* m_FileSelector;

  MGUIECBList* m_MEGAlibExtensionOptions;

  bool& m_OkPressed;

  enum ButtonIDs {
    c_Min = 100, 
    c_StandardName,
    c_SpecialName,
    c_Max }; 


#ifdef ___CINT___
 public:
  ClassDef(MGUIMGeant, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
