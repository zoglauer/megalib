/*
 * MResponseConeShapes.h
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


#ifndef __MResponseConeShapes__
#define __MResponseConeShapes__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"
#include "MResponse.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseConeShapes : public MResponse
{
  // public interface:
 public:
  MResponseConeShapes();
  virtual ~MResponseConeShapes();

  bool LoadResponseFile(const MString& ResponseFileName);

  virtual double GetComptonResponse(const double t);
  virtual double GetComptonMaximum();
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  virtual bool AnalyzeEvent(MPhysicalEvent* Event); 


  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:

  //! The transversal Compton response
  MResponseMatrixO5 m_ARMShape;

  //! Compton scatter angle of current Compton event:
  double m_Phi;
  //! Total energy of current Compton event:
  double m_Ei;
  //! Distance between the first two interactions
  double m_Distance;
  //! Numober of interactions
  int m_NumberOfInteractions;

  //! Data splice of the Compton Transversal Response 
  MResponseMatrixO1 m_Slice;

  //! Integral (Sum) over the Compton response
  double m_ComptonIntegral;


#ifdef ___CLING___
 public:
  ClassDef(MResponseConeShapes, 0) // basic response class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
