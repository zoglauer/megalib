/*
 * MGUIOptionsEventClustering.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsEventClustering__
#define __MGUIOptionsEventClustering__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"
#include "MGUIERBList.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsEventClustering : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsEventClustering(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsEventClustering();


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

  MGUIEEntry* m_DistanceCutOff;
  
  MGUIEFileSelector* m_TMVAFileSelector;
  MGUIERBList* m_TMVAMethods;
  //! Map button IDs to TMVA methods
  map<int, MERCSRTMVAMethod> m_TMVAMethodsMap;
  
  
  enum ButtonIDs { e_ReferencePoint = 200};


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsEventClustering, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
