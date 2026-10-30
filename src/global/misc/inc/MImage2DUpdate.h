/*
 * MImage2DUpdate.h
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


#ifndef __MImage2DUpdate__
#define __MImage2DUpdate__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MImage2D.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MImage2DUpdate : public MImage2D
{
  // public interface:
 public:
  MImage2DUpdate(MString Title, double *IA, int NEntries, 
                 MString xTitle, double xMin, double xMax, int xNBins, 
                 MString yTitle, double yMin, double yMax, int yNBins, 
                 int Spectrum = c_Viridis, int DrawOption = c_COLCONTZ);
  virtual ~MImage2DUpdate();

  //! Clone this image
  virtual MImage* Clone();

  void Display(TCanvas *Canvas = 0);
  void Add(double *Image);
  void Reset();

  TH1* GetHistogram();
  void DrawCopy();

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  unsigned int m_NAdds;                    // Number of calls to Add(...)

  unsigned int m_DisplayUpdateFrequency;   // After m_DisplayUpdateFrequency-Adds the Display/Canvas is updated
  unsigned int m_StorageUpdateFrequency;   // After m_StorageUpdateFrequency Adds a new summed spectra is started

  TCanvas *m_Canvas;
  TH2D *m_Histogram;


#ifdef ___CLING___
 public:
  ClassDef(MImage2DUpdate, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
