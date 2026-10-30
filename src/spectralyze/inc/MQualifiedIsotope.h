/*
 * MQualifiedIsotope.h
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


#ifndef __MQualifiedIsotope__
#define __MQualifiedIsotope__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MIsotope.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Isotope with additional qualifiying characteristics, such as a rating
class MQualifiedIsotope : public MIsotope
{
  // public interface:
 public:
  MQualifiedIsotope();
  virtual ~MQualifiedIsotope();

  //! Add line parameters:
  void AddLine(double Energy, double BranchingRatio, const MString& Flags, bool ExcludeExcludedLines = false);
  //! Set line found
  void SetLineFound(unsigned int l, bool Found);
  //! Get line found
  bool GetLineFound(unsigned int l);

  //! Set the rating of the isotope
  void SetRating(double Rating) { m_Rating = Rating; }
  //! Add something to the rating of the isotope
  void AddRating(double Rating) { m_Rating += Rating; }  
  //! Get the rating of the isotope
  double GetRating() const { return m_Rating; }
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The line branching ratio
  vector<bool> m_LineFound;

  //! The rating of the isotope
  double m_Rating;
 

#ifdef ___CLING___
 public:
  ClassDef(MQualifiedIsotope, 0) // no description
#endif

};

//! Stream a nucleon
ostream& operator<<(ostream& os, const MQualifiedIsotope& Isotope);

#endif


////////////////////////////////////////////////////////////////////////////////
