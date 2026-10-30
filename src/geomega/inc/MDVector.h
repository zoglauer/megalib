/*
 * MDVector.h
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


#ifndef __MDVector__
#define __MDVector__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include <vector>
using namespace std; 

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDVector
{
  // This is a named vector of positions and associated values in space
  // The positions do not need to follow any specific order

  // public interface:
 public:
  //! Standard constructor with the name of the vector
  MDVector(const MString Name = "");
  //! Standard copy constructor
  MDVector(const MDVector& V);
  //! Default destructor
  virtual ~MDVector();

  //! Return the name of the vector
  MString GetName() const { return m_Name; }
  //! Set the name of the vector
  void SetName(const MString& Name) { m_Name = Name; }

  //! Add a position to the vector
  void Add(MVector Pos, double Value) { m_Positions.push_back(Pos); m_Values.push_back(Value); }
  //! Return the number of stored positions
  unsigned int GetSize() const { return m_Positions.size(); }
  //! Return the position at pos i - g_VectorNotDefined if out of bounds
  MVector GetPosition(unsigned int i);
  //! Return the value at pos i - g_DoubleNotDefined if out of bounds
  double GetValue(unsigned int i);


  // protected methods:
 protected:
  //MDVector() {};
  //MDVector(const MDVector& DVector) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of the vector
  MString m_Name;
  //! The position vector
  vector<MVector> m_Positions;
  //! The value vector
  vector<double> m_Values;


#ifdef ___CLING___
 public:
  ClassDef(MDVector, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
