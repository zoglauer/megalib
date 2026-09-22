/*
 * MImageSpheric.h
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


#ifndef __MImageSpheric__
#define __MImageSpheric__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TGaxis.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImageSpheric : public MImage2D
{
  // public interface:
 public:
  //! Default constructor
  MImageSpheric();
  //! Standard constructor
  MImageSpheric(MString Title, double *IA,
                MString xTitle, double xMin, double xMax, int xNBins, 
                MString yTitle, double yMin, double yMax, int yNBins, 
                MString vTitle, int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ);
  //! Standard destructor
  virtual ~MImageSpheric();

  //! Set the image projection
  void SetProjection(MImageProjection Projection) { m_Projection = Projection; }

  //! Clone this image
  virtual MImage* Clone();

  //! Set the image array and redisplay it
  virtual void SetImageArray(double* Array);

  //! Display the histogram in the given canvas
  virtual void Display(TCanvas* Canvas = nullptr);
 
  // protected methods:
 protected:
  //! Display unprojected
  void DisplayProjectionNone();
  //! Display with Hammer projection
  void DisplayProjectionHammer();


  // private methods:
 private:
  //! Convert lat and long (in rad) to x & y in the Hammer histogram
  void HammerConv(double Long, double Lat, double CentralMeridian, double& xHammer, double& yHammer);
  //! Convert x & y in the Hammer histogram to lat and long (in rad)
  bool HammerInvConv(double xHammer, double yHammer, double CentralMeridian, double& Long, double& Lat);



  // protected members:
 protected:

  // private members:
 private:
  //! The projection
  MImageProjection m_Projection;

  //! The new y axis
  TGaxis* m_YAxis;


#ifdef ___CLING___
 public:
  ClassDef(MImageSpheric, 0) // class containing an astropysical image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
