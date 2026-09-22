/*
 * MResponseEarthHorizon.h
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


#ifndef __MResponseEarthHorizon__
#define __MResponseEarthHorizon__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBase.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseEarthHorizon : public MResponseBase
{
  // public interface:
 public:
  MResponseEarthHorizon();
  virtual ~MResponseEarthHorizon();

  //! Do all the response creation
  virtual bool CreateResponse();


  // protected methods:
 protected:
  //MResponseEarthHorizon() {};
  //MResponseEarthHorizon(const MResponseEarthHorizon& ResponseEarthHorizon) {};

  //! Load the simulation file in revan and mimrec as well as the configuration files:
  virtual bool OpenFiles();


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MResponseEarthHorizon, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
