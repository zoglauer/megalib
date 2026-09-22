/*
 * MBinnerSpherical.h
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


#ifndef __MBinnerSpherical__
#define __MBinnerSpherical__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <sstream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MVector.h"
#include "MString.h"

// Forward declarations:

// Other libs:


////////////////////////////////////////////////////////////////////////////////


//! Base class for the spherical binners
//! All member functions are abstrcat
class MBinnerSpherical
{
  // public interface:
  public :
    //! Default constructor
    MBinnerSpherical();
    //! Default destructor
    virtual ~MBinnerSpherical();
    
    //! Find a bin
    //! Theta (= latitude) and phi (= longitude) are in (mathematical) spherical coordinates
    //! The bins are arranged along the iso-latitude lines starting at the north pole (theta = latitude = 0)
    //! Theta and phi are in radians!
    virtual unsigned int FindBin (double Theta, double Phi) const = 0;

    //! Get number of bins
    virtual unsigned int GetNBins() const = 0;

    //! Returns all bin centers as vector
    virtual vector<double> GetBinCenters(unsigned int Bin) const = 0;

    //! Returns all bin centers as vector
    virtual vector<MVector> GetAllBinCenters() const = 0;

    //! Return the minimum axis values [min theta, min phi]
    virtual vector<double> GetMinima() const = 0;

    //! Return the minimum axis values [max theta, max phi]
    virtual vector<double> GetMaxima() const = 0;

    //! Return axis bins edges for external drawing (1st array: longitude/phi, 2nd array: latitude/theta)
    virtual vector<vector<double> > GetDrawingAxisBinEdges() const = 0;

    // //! Check if we have equal bins
    // virtual bool operator ==(const MBinnerSpherical&) const = 0;
    // //! ... or not
    // virtual bool operator !=(const MBinnerSpherical&) const = 0;
  
    //! Write the content to a stream
    virtual void Write(MString name, ostringstream& out) const = 0;


#ifdef ___CLING___
 public:
  ClassDef(MBinnerSpherical, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////


