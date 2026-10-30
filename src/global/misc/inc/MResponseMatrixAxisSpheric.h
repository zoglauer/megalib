/*
 * MResponseMatrixAxisSpheric.h
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


#ifndef __MResponseMatrixAxisSpheric__
#define __MResponseMatrixAxisSpheric__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMatrixAxis.h"
#include "MBinnerFISBEL.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! This defines a response matrix axis
class MResponseMatrixAxisSpheric : public MResponseMatrixAxis
{
  // public interface:
 public:
  //! Default constructor
  MResponseMatrixAxisSpheric(const MString& Theta, const MString& Phi);
  //! Default destuctor 
  virtual ~MResponseMatrixAxisSpheric();
  
  //! Equality operator
  bool operator==(const MResponseMatrixAxisSpheric& Axis) const;
  //! Inequality operator
  bool operator!=(const MResponseMatrixAxisSpheric& Axis) const { return !(operator==(Axis)); }

  //! Clone this axis
  virtual MResponseMatrixAxisSpheric* Clone() const;
  
  //! Set the axis in FISBEL mode with a longitude shift in degrees
  void SetFISBEL(unsigned long NBins, double LongitudeShift = 0);
  

  //! Return the axis bin, given theta=latitude and phi=longitude in degrees
  virtual unsigned long GetAxisBin(double Theta, double Phi) const;
  
  //! Test if theta and phi (in degree) are within the range of the axis - the second value is required
  virtual bool InRange(double Theta, double Phi) const;
  
  //! True if the axis has 1D bin edges
  virtual bool Has1DBinEdges() const { return false; }
  //! Get the 1D bin edges
  //! Check with Has1DBinEdges first, because this is not guaranteed
  virtual vector<double> Get1DBinEdges() const { return vector<double>(); }
  
  //! Return the area of the given axis bin
  virtual double GetArea(unsigned long Bin) const;
  
  //! Return the axis bin edges for drawing --- those might be narrower than the real bins
  virtual vector<vector<double>> GetDrawingAxisBinEdges() const;
  //! Return the minimum axis values
  virtual vector<double> GetMinima() const;
  //! Return the minimum axis values
  virtual vector<double> GetMaxima() const;
  
  //! Return the bin center(s) of the given axis bin, theta & phi 
  //! Can throw: MExceptionIndexOutOfBounds
  virtual vector<double> GetBinCenters(unsigned long Bin) const;
  
  //! Return the bin center of all axis bins as vectors 
  //! Can throw: MExceptionIndexOutOfBounds
  virtual vector<MVector> GetAllBinCenters() const;
  
  //! Write the content to a stream
  virtual void Write(ostringstream& out);
  
  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:


  // private members:
 private:
  //! The binner
  MBinnerFISBEL m_Binner;
  

#ifdef ___CLING___
 public:
  ClassDef(MResponseMatrixAxisSpheric, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
