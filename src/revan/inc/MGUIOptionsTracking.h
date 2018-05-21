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
#include "TGListBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MDGeometryQuest.h"
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
                      MSettingsEventReconstruction* Data,
                      MDGeometryQuest* Geometry);
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
  MDGeometryQuest* m_Geometry;
  
  MGUIECBList* m_CBList;
  MGUIEEntry* m_MaxComptonJump;
  MGUIEEntry* m_NSequencesToKeep;
  TGCheckButton* m_RejectPurelyAmbiguousSequences;
  MGUIEFileSelector* m_FileSelector;
  MGUIEEntry* m_NLayersForVertexSearch;

  TGListBox* m_DetectorList;
  
  enum ButtonCodes { e_DoTracking = 100, e_RejectPurelyAmbiguousSequences, e_Detectors };

#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsTracking, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
