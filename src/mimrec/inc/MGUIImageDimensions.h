/*
 * MGUIImageDimensions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIImageDimensions__
#define __MGUIImageDimensions__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIEMinMaxEntry.h"
#include "MGUIEEntry.h"
#include "MGUIEEntryList.h"
#include "TGComboBox.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIImageDimensions : public MGUIDialog
{
  // Public Interface:
 public:
  MGUIImageDimensions(const TGWindow* Parent, const TGWindow* Main, 
                      MSettingsImaging* Data);
  virtual ~MGUIImageDimensions();

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
  MSettingsImaging* m_GUIData;

  MGUIEEntryList* m_XAxis;
  MGUIEEntryList* m_ZAxis;

  MGUIEMinMaxEntry* m_ThetaDimension;
  MGUIEEntry* m_ThetaBins;
  MGUIEMinMaxEntry* m_PhiDimension;
  MGUIEEntry* m_PhiBins;

  MGUIEMinMaxEntry* m_LatitudeDimension;
  MGUIEEntry* m_LatitudeBins;
  MGUIEMinMaxEntry* m_LongitudeDimension;
  MGUIEEntry* m_LongitudeBins;

  TGComboBox* m_Projection;

  MGUIEMinMaxEntry* m_XDimension;
  MGUIEEntry* m_XBins;
  MGUIEMinMaxEntry* m_YDimension;
  MGUIEEntry* m_YBins;
  MGUIEMinMaxEntry* m_ZDimension;
  MGUIEEntry* m_ZBins;

#ifdef ___CLING___
 public:
  ClassDef(MGUIImageDimensions, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
