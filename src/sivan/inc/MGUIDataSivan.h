/*
 * MGUIDataSivan.h
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIDataSivan__
#define __MGUIDataSivan__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIDataSivan : public MGUIData
{
  // public Session:
 public:
  MGUIDataSivan();
  ~MGUIDataSivan();

  void SetRealism(const int Realism);
  int GetRealism() const;

  void SetNInitializationEvents(const int NEvents) { m_NInitializationEvents = NEvents; }
  unsigned int GetNInitializationEvents() const{ return m_NInitializationEvents; }


  // protected members:
 protected:
  virtual void ReadDataLocal(FILE* File);
  virtual bool ReadDataLocal(MXmlDocument* Node);
  virtual bool SaveDataLocal(MXmlDocument* Node);


  // private members:
 private:
  int m_Realism;
  //! Number of events which are are used to initialize the initial size of dimensional hitsograms
  int m_NInitializationEvents;


#ifdef ___CINT___
 public:
  ClassDef(MGUIDataSivan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
