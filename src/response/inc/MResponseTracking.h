/*
 * MResponseTracking.h
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


#ifndef __MResponseTracking__
#define __MResponseTracking__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseMultipleCompton.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseTracking : public MResponseMultipleCompton
{
  // public interface:
 public:
  //! Default constructor
  MResponseTracking();
  //! Default destructor
  virtual ~MResponseTracking();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();

  // protected methods:
 protected:

  //! Save the response matrices
  virtual bool Save();

   
   
  double CalculateAngleIn(MRESE& Start, MRESE& Central);
  double CalculateAngleOutPhi(MRESE& Start, MRESE& Central, MRESE& Stop);
  double CalculateAngleOutTheta(MRESE& Start, MRESE& Central, MRESE& Stop);

  // private methods:
 private:



  // protected members:
 protected:
   
  MResponseMatrixO1 m_GoodBadTable;

  MResponseMatrixO3 m_PdfDualGood;
  MResponseMatrixO3 m_PdfDualBad;
   
  MResponseMatrixO3 m_PdfStartGood;
  MResponseMatrixO3 m_PdfStartBad;
  
  MResponseMatrixO5 m_PdfGood;
  MResponseMatrixO5 m_PdfBad;
   
  MResponseMatrixO2 m_PdfStopGood;
  MResponseMatrixO2 m_PdfStopBad;

  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseTracking, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
