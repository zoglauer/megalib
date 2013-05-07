/*
 * MGUINetwork.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUINetwork__
#define __MGUINetwork__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIDialog.h"
#include "MGUIERBList.h"
#include "MSettingsRealta.h"

// Forward declarations:
class MGUIEEntry;

////////////////////////////////////////////////////////////////////////////////


class MGUINetwork : public MGUIDialog
{
  // Public Interface:
 public:
  MGUINetwork(const TGWindow* p, const TGWindow* main, MSettingsRealta* Settings);
  virtual ~MGUINetwork();


  // protected methods:
 protected:
  virtual void Create();
  virtual bool OnApply();
  virtual bool ProcessMessage(Long_t Message, Long_t Parameter1, Long_t Parameter2);


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  MSettingsRealta* m_Settings;

  MGUIEEntry* m_Host;
  MGUIEEntry* m_Port;

  MGUIERBList* m_TransceiverMode;
  
  TGCheckButton *m_ConnectOnStart;


#ifdef ___CINT___
 public:
  ClassDef(MGUINetwork, 0) // GUI window: ...
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
