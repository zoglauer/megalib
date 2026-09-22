/*
 * MResponse.h
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


#ifndef __MResponse__
#define __MResponse__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TObject.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponse
{
  // public interface:
 public:
  MResponse();
  virtual ~MResponse();

  //! Choose if you want to use maths approximations
  void SetApproximatedMaths(bool Approximated = true) { m_ApproximatedMaths = Approximated; }

  virtual void SetThreshold(const double Threshold);
  virtual double GetThreshold() const;

  virtual bool AnalyzeEvent(MPhysicalEvent* Event); 

  //! Compton response for untracked events
  virtual double GetComptonResponse(const double t);
  //! Compton response for tracked events
  virtual double GetComptonResponse(const double t, const double l);
  //! Maximum of the response
  virtual double GetComptonMaximum();
  //! Integral of the response
  virtual double GetComptonIntegral(const double Radius) const;

  virtual double GetComptonTransversalMax();
  virtual double GetComptonTransversalMin();

  //! Get pair response
  virtual double GetPairResponse(const double t);
  //! Integral of the response
  virtual double GetPairIntegral() const;
 
  
  //! Get PET response
  virtual double GetPETResponse(const double t);
  

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! The threshold out to which the response should be calculated
  double m_Threshold;
  //! Flag indiaction whether or not approxiamte4d maths should be used:
  bool m_ApproximatedMaths;


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponse, 0) // basic response class
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
