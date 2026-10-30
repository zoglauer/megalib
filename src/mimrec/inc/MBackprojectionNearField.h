/*
 * MBackprojectionNearField.h
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


#ifndef __MBackprojectionNearField__
#define __MBackprojectionNearField__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackprojection.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackprojectionNearField : public MBackprojection
{
  // public interface:
 public:
  //! Default constructor
  MBackprojectionNearField(MCoordinateSystem CoordinateSystem = MCoordinateSystem::c_Cartesian2D);
  //! Default destructor
  virtual ~MBackprojectionNearField();

  //! Prepare all backprojections - must be called before the backprojections
  virtual void PrepareBackprojection();
  //! The entry point into the backprojection
  virtual bool Backproject(MPhysicalEvent* Event, double* Image, int* Bins, int& NUsedBins, double& Maximum);

  //! Set the viewport, i.e. image dimensions
  virtual bool SetDimensions(double x1Min, double x1Max, unsigned int x1NBins, 
                             double x2Min, double x2Max, unsigned int x2NBins,
                             double x3Min = 0, double x3Max = 0, unsigned int x3NBins = 1,
                             MVector x1Axis = MVector(1.0, 0.0, 0.0), 
                             MVector x3Axis = MVector(0.0, 0.0, 1.0));

  // protected methods:
 protected:
  //! Assimilate the event data - stores parts of the event data in the class for acceleration pruposes 
  virtual bool Assimilate(MPhysicalEvent* Event);

  //! Calculate the origin probabilities for a Compton event
  bool BackprojectionCompton(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a Pair event
  bool BackprojectionPair(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a PET event
  bool BackprojectionPET(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  //! Calculate the origin probabilities for a multi-event event
  bool BackprojectionMulti(double* Image, int* Bins, int& NUsedBins, double& Maximum);
  

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MBackprojectionNearField, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
