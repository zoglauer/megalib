/*
 * MGUIDataRealta.h
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDataRealta__
#define __MGUIDataRealta__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDataRealta : public MGUIData
{
  // public Session:
 public:
  MGUIDataRealta();
  virtual ~MGUIDataRealta();

  // protected members:
 protected:
  virtual bool ReadDataLocal(MXmlDocument* Node);
  virtual bool SaveDataLocal(MXmlDocument* Node);


  // private members:
 private:
  //! Bla bla
  //int m_Realism;
  //! Number of events which are are used to initialize the initial size of dimensional hitsograms
  //int m_NInitializationEvents;


#ifdef ___CINT___
 public:
  ClassDef(MGUIDataRealta, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////

