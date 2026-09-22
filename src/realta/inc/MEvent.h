/*
 * MEvent.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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



#ifdef ___CLING___
 public:
  ClassDef(MEvent, 0)   // base class for compton and pair events
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
