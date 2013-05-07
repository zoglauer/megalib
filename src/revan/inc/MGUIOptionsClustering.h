/*
 * MGUIOptionsClustering.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIOptionsClustering__
#define __MGUIOptionsClustering__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsEventReconstruction.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIOptionsClustering : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIOptionsClustering(const TGWindow* Parent, const TGWindow* Main, 
                        MSettingsEventReconstruction* Data);
  virtual ~MGUIOptionsClustering();


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

  enum ButtonIDs { e_ReferencePoint = 200};


#ifdef ___CINT___
 public:
  ClassDef(MGUIOptionsClustering, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
