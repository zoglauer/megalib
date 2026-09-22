/*
 * MHitData.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MHitData
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MHitData.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MHitData)
#endif


////////////////////////////////////////////////////////////////////////////////


MHitData::MHitData()
{
  // Construct an empty hit:

  SetDetector(0);
  SetPosition(MVector(0.0, 0.0, 0.0));
  SetEnergy(0.0);
}


////////////////////////////////////////////////////////////////////////////////


MHitData::MHitData(UInt_t Detector, MVector Position, Double_t Energy)
{
  // default constructor

  SetDetector(Detector);
  SetPosition(Position);
  SetEnergy(Energy);
}


////////////////////////////////////////////////////////////////////////////////


MHitData::~MHitData()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////


void MHitData::SetDetector(UInt_t Detector)
{
  //

  m_Detector = Detector;
}


////////////////////////////////////////////////////////////////////////////////


void MHitData::SetPosition(MVector Position)
{
  //

  m_Position = Position;
}


////////////////////////////////////////////////////////////////////////////////


void MHitData::SetEnergy(Double_t Energy)
{
  // 

  m_Energy = Energy;
}


////////////////////////////////////////////////////////////////////////////////


UInt_t MHitData::GetDetector()
{
  // 

  return m_Detector;
}


////////////////////////////////////////////////////////////////////////////////


MVector MHitData::GetPosition()
{
  // 

  return m_Position;
}


////////////////////////////////////////////////////////////////////////////////


Double_t MHitData::GetEnergy()
{
  //

  return m_Energy;
}


// MHitData.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
