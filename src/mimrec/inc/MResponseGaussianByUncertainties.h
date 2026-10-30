/*
 * MResponseGaussianByUncertainties.h
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


#ifndef __MResponseGaussianByUncertainties__
#define __MResponseGaussianByUncertainties__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseGaussian.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Calculate the Compton response (cone width) using the known position & energy uncertainties
class MResponseGaussianByUncertainties : public MResponseGaussian
{
  // public interface:
 public:
  //! Default constructor
  MResponseGaussianByUncertainties();
  //! Default destructor
  virtual ~MResponseGaussianByUncertainties();

  //! Set a fixed increase (worsening) of the calculates cone width
  void SetIncrease(double Increase) { m_Increase = Increase; }
  
  //! Set this events data
  virtual bool AnalyzeEvent(MPhysicalEvent* Event);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The fixed worsening of the calculated cone width
  double m_Increase;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MResponseGaussianByUncertainties, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
