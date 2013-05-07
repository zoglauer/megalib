/*
 * MHitEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MHitEvent__
#define __MHitEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TObjArray.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:
class MHitData;

////////////////////////////////////////////////////////////////////////////////


class MHitEvent : public TObject
{
  // public interface:
 public:
  MHitEvent();
  ~MHitEvent();

  void AddHit(MHitData* Hit);
  Int_t GetNHits();
  MHitData* GetHitAt(Int_t i);

  void SetID(UInt_t ID);
  UInt_t GetID();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  TObjArray* m_Hits;

  Int_t m_ID;


#ifdef ___CINT___
 public:
  ClassDef(MHitEvent, 1) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
