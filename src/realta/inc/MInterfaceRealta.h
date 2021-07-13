/*
 * MInterfaceRealta.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MInterfaceRealta__
#define __MInterfaceRealta__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MInterface.h"
#include "MSettingsRealta.h"
#include "MSettingsMimrec.h"
#include "MSettingsRevan.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MInterfaceRealta : public MInterface
{
  // public interface:
 public:
  MInterfaceRealta(MSettingsRealta *GUIData);
  ~MInterfaceRealta();

  void AnalyzeEvents();



  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsRealta *m_GUIData; // All the information of the GUI

  MSettingsMimrec *m_GUIDataImaging;
  MSettingsRevan *m_GUIDataRevan;


#ifdef ___CLING___
 public:
  ClassDef(MInterfaceRealta, 0) // interface to the Realta-part of MEGAlib
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
