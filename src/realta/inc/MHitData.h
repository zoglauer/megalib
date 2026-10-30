/*
 * MHitData.h
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


#ifndef __MHitData__
#define __MHitData__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"

// Forward declarations:

////////////////////////////////////////////////////////////////////////////////


class MHitData : public TObject
{
  // public interface:
 public:
  MHitData();
  MHitData(UInt_t Detector, MVector Position, Double_t Energy);
  ~MHitData();

  void SetDetector(UInt_t Detector);
  void SetPosition(MVector Position);
  void SetEnergy(Double_t Energy);

  UInt_t GetDetector();
  MVector GetPosition();
  Double_t GetEnergy();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  UInt_t m_Detector;
  MVector m_Position;
  Double_t m_Energy;


#ifdef ___CLING___
 public:
  ClassDef(MHitData, 1) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
