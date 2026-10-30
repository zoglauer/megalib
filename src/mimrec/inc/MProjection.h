/*
 * MProjection.h
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


#ifndef __MProjection__
#define __MProjection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MCoordinateSystem.h"
#include "MPhysicalEvent.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MPETEvent.h"
#include "MMultiEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MProjection
{
  // Public Interface:
 public:
  //! Default constructor
  MProjection(MCoordinateSystem CoordianteSystem);
  //! Default destructor
  virtual ~MProjection();

  //! Choose if you want to use maths approximations
  virtual void SetApproximatedMaths(bool Approximated = true) { m_ApproximatedMaths = Approximated; }

  //! Assimilate the data of this event
  virtual bool Assimilate(MPhysicalEvent* Event);


  // protected methods:
 protected:


  // private methods:
 private:


  // protected members:
 protected:
  //! The actual used coordinate system
  MCoordinateSystem m_CoordinateSystem;
   
   //! The current event
  MPhysicalEvent* m_Event;
  //! For faster access: Compton event
  MComptonEvent* m_C;
  //! For faster access: Pair event
  MPairEvent* m_P;
  //! For faster access: Photo event
  MPhotoEvent* m_Photo;
  //! For faster access: PET event
  MPETEvent* m_PET;
  //! For faster access: Multi event
  MMultiEvent* m_Multi;
  
  //! True if we should used approxiamted maths
  bool m_ApproximatedMaths;

  
  
  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MProjection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
