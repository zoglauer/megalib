/*
 * MGUIImageOptions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIImageOptions__
#define __MGUIImageOptions__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TGFrame.h"
#include "TGComboBox.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"
#include "MGUIEFileSelector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIImageOptions : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIImageOptions(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data);
  //! default destructor
  virtual ~MGUIImageOptions();

  // protected methods:
 protected:
  //! Create the GUI
  virtual void Create();
  //! Apply the data
  virtual bool OnApply();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The GUI data
  MSettingsImaging* m_GUIData;

  //! GUI element for the palette
  TGComboBox* m_Palette;
  //! GUI element for the drawing mode
  TGComboBox* m_DrawMode;
  //! GUI element for the source catalog
  MGUIEFileSelector* m_SourceCatalog;


#ifdef ___CLING___
 public:
  ClassDef(MGUIImageOptions, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
