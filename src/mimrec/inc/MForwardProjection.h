/*
 * MForwardProjection.h
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


#ifndef __MForwardProjection__
#define __MForwardProjection__


////////////////////////////////////////////////////////////////////////////////


// standard libs
#include <iostream>

// ROOT libs
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MProjection.h"
#include "MPhysicalEvent.h"
#include "MFPDataPoint.h"


////////////////////////////////////////////////////////////////////////////////


class MForwardProjection : public MProjection
{
  // Public Interface:
 public:
  MForwardProjection();
  ~MForwardProjection();


  void SetSpaceBins(int NBinsPsi, int NBinsEta, int NBinsPhi, int NBinsTheta);

  bool ForwardProject(MPhysicalEvent *Event, MFPDataPoint *Point);
  bool Assimilate(MPhysicalEvent *Event);


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  int m_NBinsPsi;
  int m_NBinsEta; 
  int m_NBinsPhi;
  int m_NBinsTheta;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MForwardProjection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
