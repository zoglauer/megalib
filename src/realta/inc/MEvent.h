/*
 * MEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEvent__
#define __MEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TRotMatrix.h>
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////////////////////////


class MEvent : public TObject
{
  // public interface:
 public:
  MEvent();
  ~MEvent();

  Bool_t Assimilate(MEvent *E);

  Int_t GetEventType();

  void SetRotationXAxis(MVector Rot);
  void SetRotationZAxis(MVector Rot);
  MVector GetRotationXAxis();
  MVector GetRotationZAxis();

  TMatrix GetRotation();

  void SetTime(Double_t Time);
  Double_t GetTime();

  virtual MString ToString();

  // protected methods:
 protected:


  // private methods:
 private:


  // public members:
 public:
  enum EMEventType {
    c_Unkown = -1,
    c_Compton = 0,
    c_Pair = 1
  };

  // protected members:
 protected:
  Int_t m_EventType;

  MVector m_RotationXAxis;
  MVector m_RotationZAxis;
  Double_t m_Time;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MEvent, 0)   // base class for compton and pair events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
