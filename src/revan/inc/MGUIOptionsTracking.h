/*
 * MGUIOptionsTracking.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsTracking__
#define __MGUIOptionsTracking__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIECBList.h"
#include "MGUIEFileSelector.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsTracking : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsTracking(const TGWindow* Parent, const TGWindow* Main, 
                      MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsTracking();


  // protected methods:
 protected:
  virtual void Create();
	virtual bool OnApply();

  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  MSettingsEventReconstruction* m_Data;

  MGUIECBList* m_CBList;
  MGUIEEntry* m_MaxComptonJump;
  MGUIEEntry* m_NSequencesToKeep;
  TGCheckButton* m_RejectPurelyAmbiguousSequences;
  MGUIEFileSelector* m_FileSelector;
  MGUIEEntry* m_NLayersForVertexSearch;
  
  enum ButtonCodes { e_DoTracking = 100, e_RejectPurelyAmbiguousSequences };

#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsTracking, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
