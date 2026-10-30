/*
 * MEREventClusterizerDistance.h
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


#ifndef __MEREventClusterizerDistance__
#define __MEREventClusterizerDistance__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnationList.h"
#include "MEREventClusterizer.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Find clusters for hits originating from the same gamma ray 
class MEREventClusterizerDistance : public MEREventClusterizer
{
  // public interface:
 public:
  //! Default constructor
  MEREventClusterizerDistance();
  //! Default destructor
  virtual ~MEREventClusterizerDistance();

  //! Set the distance cut off
  virtual bool SetDistanceCutOff(double DistanceCutOff);  
                             
  //! Do the analysis - this will add events to the whole incarnation list
  virtual bool Analyze(MRawEventIncarnationList* List);
  
  //! Dump the reconstruction options into a string
  virtual MString ToString(bool CoreOnly = false) const;
  
  
  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The distance cutoff below which we assume we have individual events
  double m_DistanceCutOff;

  
#ifdef ___CLING___
 public:
  ClassDef(MEREventClusterizerDistance, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
