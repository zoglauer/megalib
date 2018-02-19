/*
 * MGUIOptionsHitClustering.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsHitClustering__
#define __MGUIOptionsHitClustering__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsHitClustering : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsHitClustering(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsHitClustering();


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

  MGUIEEntryList* m_MinDistance;

  TGComboBox* m_AdjacentLevel;
  MGUIEEntry* m_AdjacentSigma;

  TGCheckButton* m_ReferencePoint;

  MGUIEFileSelector* m_PDFClusterizerBaseFileName;
  
  
  enum ButtonIDs { e_ReferencePoint = 200};


#ifdef ___CLING___
 public:
  ClassDef(MGUIOptionsHitClustering, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
