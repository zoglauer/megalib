/*
 * MFunction3DSpherical.h
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


#ifndef __MFunction3DSpherical__
#define __MFunction3DSpherical__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>
#include <TSpline.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MFunction3D.h"

// Standard libs:
#include <vector>
using std::vector;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFunction3DSpherical : public MFunction3D
{
  // public interface:
 public:
  //! Default constructor (x:phi, y:theta, z:energy)
  MFunction3DSpherical();
  //! Copy constructor
  MFunction3DSpherical(const MFunction3DSpherical& Function3DSpherical);
  //! Default destructor
  virtual ~MFunction3DSpherical();

  //! Default assignment constructor
  const MFunction3DSpherical& operator=(const MFunction3DSpherical& F);

  //! Set the basic data, load the file and parse it
  bool Set(const MString FileName,
           const MString KeyWord,
           const unsigned int InterpolationType = c_InterpolationLinear);

  bool Set(const vector<double>& X, const vector<double>& Y, const vector<double>& Z, const vector<double>& Values, 
           unsigned int InterpolationType = c_InterpolationLinear) { return MFunction3D::Set(X, Y, Z, Values, InterpolationType); }

  //! Return the total content of the function
  //! This is a simplified integration which uses the value at the bin centers and the (3D) bin size
  double Integrate() const;

  //! Return random numbers x, y, z distributed as the underlying function
  void GetRandom(double& x, double& y, double& z);

  //! Plot the function in a Canvas (diagnostics only)
  void Plot(bool Random = false);

  //! Save as a file
  virtual bool Save(const MString FileName, const MString Keyword = "AP");

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:
  

#ifdef ___CLING___
 public:
  ClassDef(MFunction3DSpherical, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
