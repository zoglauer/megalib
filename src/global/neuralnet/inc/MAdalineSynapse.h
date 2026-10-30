/*
 * MAdalineSynapse.h
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


#ifndef __MAdalineSynapse__
#define __MAdalineSynapse__


////////////////////////////////////////////////////////////////////////////////



// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSynapse.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MAdalineSynapse : public MSynapse
{
  // public interface:
public:
  MAdalineSynapse();
  virtual ~MAdalineSynapse() {}
  
  
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
  ClassDef(MAdalineSynapse, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
