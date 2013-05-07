/*
 * MGUISessionRealta.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUISessionRealta__
#define __MGUISessionRealta__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"
#include <TGMenu.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUISession.h"
#include "MSettingsRealta.h"
#include "MInterfaceRealta.h"
#include "MSystem.h"
#include "MGUIControlCenter.h"

// Forward declarations:




////////////////////////////////////////////////////////////////////////////////


enum RealtaIdentifier 
{
  M_REALTA_NETWORK = 500,
  M_REALTA_STORAGE = 501
};


////////////////////////////////////////////////////////////////////////////////


class MGUISessionRealta : public MGUISession
{
  // public Session:
 public:
  MGUISessionRealta(MGUIMainFrame *Parent);
  ~MGUISessionRealta();

  virtual Bool_t ProcessMessage(Long_t msg, Long_t param1, Long_t);

  virtual void CreateMenuBar();
  virtual Char_t** GetFileTypes();
  virtual void CreateCenterFrame();
  virtual void Launch();
  virtual void SetFileName(MString FileName);

  virtual void Activate();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  MSettingsRealta *m_GUIData;
  MInterfaceRealta *m_Realta;

  MGUIControlCenter *m_ControlCenter;

  // private members:
 private:
  TGPopupMenu *m_MenuAnalysis;

  TGLayoutHints *m_PictureLayout;
  TGPictureButton *m_PictureButton;



#ifdef ___CINT___
 public:
  ClassDef(MGUISessionRealta, 0) // Realta main GUI
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
