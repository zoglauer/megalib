/*
 * MCVisManager.hh
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



/******************************************************************************
 *
 * Class representing a visualization manager
 *
 */


#ifndef ___MCVisManager___
#define ___MCVisManager___

#ifdef G4VIS_USE

// Geant4:
#include "G4VisManager.hh"


/******************************************************************************/


class MCVisManager: public G4VisManager 
{
  // public interface:
public:
  /// Default constructor
  MCVisManager();

  // protected methods:
protected:
  /// Register all graphics systems...
  void RegisterGraphicsSystems();
  

  // protected members:
protected:


  // private members:
private:


};

#endif

#endif
