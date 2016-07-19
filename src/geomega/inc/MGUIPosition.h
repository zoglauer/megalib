/*
 * MGUIPosition.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIPosition__
#define __MGUIPosition__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsGeomega.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIPosition : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIPosition(const TGWindow* Parent, const TGWindow* Main, 
               MSettingsGeomega* Data, bool& OkPressed);
  virtual ~MGUIPosition();


  // protected methods:
 protected:
  void Create();
  bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsGeomega* m_Data;

  bool& m_OkPressed;

  TGLayoutHints* m_PositionLayout;
  MGUIEEntryList* m_Position;
  MGUIEEntry* m_PositionString;


#ifdef ___CINT___
 public:
  ClassDef(MGUIPosition, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
