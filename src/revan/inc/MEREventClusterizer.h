/*
 * MEREventClusterizer.h
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


#ifndef __MEREventClusterizer__
#define __MEREventClusterizer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MERConstruction.h"
#include "MRawEventIncarnations.h"
#include "MRawEventIncarnationList.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Find clusters for hits originating from the same gamma ray 
class MEREventClusterizer : public MERConstruction
{
  // public interface:
 public:
  //! Default constructor
  MEREventClusterizer();
  //! Default destructor
  virtual ~MEREventClusterizer();

  //! Do the analysis - not used here
  virtual bool Analyze(MRawEventIncarnations* List) { return false; }
  //! Do the analysis - this will add events to the whole incarnation list
  virtual bool Analyze(MRawEventIncarnationList* List) { return false; }

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
  ClassDef(MEREventClusterizer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
