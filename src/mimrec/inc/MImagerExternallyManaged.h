/*
 * MImagerExternallyManaged.h
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


#ifndef __MImagerExternallyManaged__
#define __MImagerExternallyManaged__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMatrix.h>
#include <TThread.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MBPDataImage.h"
#include "MBackprojection.h"
#include "MEventSelector.h"
#include "MFileEventsTra.h"
#include "MExposure.h"
#include "MImage.h"
#include "MImager.h"

// Forward declarations:
class MBPData;
class MLMLAlgorithms;


////////////////////////////////////////////////////////////////////////////////


class MImagerExternallyManaged : public MImager
{
  // Public Interface:
 public:
  //! Standard constructor
  MImagerExternallyManaged(MCoordinateSystem CoordinateSystem);
  //! Default destructor
  virtual ~MImagerExternallyManaged();

  //! Enable the use of GUI features, i.e. progress bar, call ProcessEvents, etc.
  void UseGUI(bool UseGUI = true) { m_UseGUI = UseGUI; }

  //! Call before the response slice calculation after all options are set
  bool Initialize();
  
  //! Calculate the response slice for the given event
  //! Return the data or zero in case the event is not within the event selection, not within the image or we are out of events
  MBPData* CalculateResponseSlice(MPhysicalEvent* Event);

  //! Deconvolve a set of response slices 
  vector<MImage*> Deconvolve(vector<MBPData*> ResponseSlices);
  
 // Addition from Christian Lang
  //---------------------------------------------------------
  //MBPData* CalculateResponseSliceLine(MPhysicalEvent* Event, double X1Position, double Y1Position, double Z1Position, double X2Position, double Y2Position, double Z2Position);
//------------------------------------------------------------

  // protected methods:
 protected:


  // private members:
 private:
  //! True if GUI features are used, i.e. progress bar, call ProcessEvents, etc. (default: true)
  bool m_UseGUI;

#ifdef ___CLING___
 public:
  ClassDef(MImagerExternallyManaged, 0) // Computes and stores system matrix
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
