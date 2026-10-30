/*
 * MDShapeSubtraction.h
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


#ifndef __MDShapeSubtraction__
#define __MDShapeSubtraction__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MVector.h"
#include "MStreams.h"
#include "MDShape.h"
#include "MDOrientation.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing a boolean shape constructed by a subtraction
class MDShapeSubtraction : public MDShape
{
  // public interface:
 public:
  //! Standard constructor
  MDShapeSubtraction(const MString& Name);
  //! Default destructor
  virtual ~MDShapeSubtraction();

  //! Set the all parameters of the shape
  bool Set(MDShape* Minuend, MDShape* Subtrahend, MDOrientation* Orientation);

  //! Validate the data and create the shape 
  bool Validate();  
  
  //! Parse some tokenized text
  bool Parse(const MTokenizer& Tokenizer, const MDDebugInfo& Info);

  //!
  MVector GetSize() { return MVector(); }

  //! Dump the content as a string
  MString ToString();
  //! Dump the content in geomega format
  MString GetGeomega() const { return ""; };

  
  
  //! Return the minuend (first number)
  MDShape* GetMinuend() const { return m_SubShapes[0]; }
  //! Return the subtrahend (second number)
  MDShape* GetSubtrahend() const { return m_SubShapes[1]; }
  //! Return the orientation of the subtrahend
  MDOrientation* GetOrientationSubtrahend() const { return m_Orientation; }

  //! Get the volume of this shape
  //! Attention: For boolean volumes it is sampled thus only correct to >1%
  double GetVolume();

  //! Scale the axes given in Axes by a factor Scaler
  virtual bool Scale(const double Scaler, const MString Axes = "XYZ");

  //! Return a unique position within the volume of the detector (center if possible)
  virtual MVector GetUniquePosition() const;

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The orientation of the subtrahend
  MDOrientation* m_Orientation;

  //! Almost unique position
  MVector m_AlmostUniquePosition;
  
#ifdef ___CLING___
 public:
  ClassDef(MDShapeSubtraction, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
