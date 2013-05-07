/*
 * MGUIResponseParameterPRM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIResponseParameterPRM__
#define __MGUIResponseParameterPRM__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIResponseParameterPRM : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIResponseParameterPRM(const TGWindow* Parent, const TGWindow* Main, 
                           MSettingsImaging* Data);
  virtual ~MGUIResponseParameterPRM();

  // protected methods:
 protected:
  virtual void Create();
	virtual bool OnApply();


  // protected members:
 protected:


  // private members:
 private:
  TGWindow* m_Parent;

  MSettingsImaging* m_GUIData;

  MGUIEFileSelector* m_ComptonLongitudinalFile;
  MGUIEFileSelector* m_ComptonTransversalFile;



#ifdef ___CINT___
 public:
  ClassDef(MGUIResponseParameterPRM, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
