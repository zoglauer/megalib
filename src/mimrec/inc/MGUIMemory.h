/*
 * MGUIMemory.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIMemory__
#define __MGUIMemory__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGFrame.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MSettingsImaging.h"
#include "MGUIESlider.h"
#include "MGUIERBList.h"
#include "MGUIEEntryList.h"
#include "MGUIEEntry.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIMemory : public MGUIDialog
{
  // Public Interface:
 public:
  //! Standard constructor
  MGUIMemory(const TGWindow* Parent, const TGWindow* Main, MSettingsImaging* Data);
  //! default destructor
  virtual ~MGUIMemory();

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

  //! GUI element for the maximum amount of used RAM
  MGUIEEntryList* m_MaxRAM;
  //! GUI element to select the number of bytes in the response
  MGUIERBList* m_Bytes;
  //! GUI element to select the maths
  MGUIERBList* m_Maths;
  //! GUI element to select the fast file parsing
  MGUIERBList* m_Parsing;
  //! GUI element to select the number of threads
  MGUIEEntryList* m_Threads;


#ifdef ___CINT___
 public:
  ClassDef(MGUIMemory, 0)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
