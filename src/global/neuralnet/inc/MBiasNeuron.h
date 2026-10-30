/*
 * MBiasNeuron.h
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


#ifndef __MBiasNeuron__
#define __MBiasNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MInputNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBiasNeuron : public MInputNeuron
{
  // public interface:
public:
  //! Default and standard constructor
  MBiasNeuron(double Bias = 1.0);
  
  //! No SetValue allowed:
  virtual void SetValue(double Value, int Number);
  //! Retrieve only the one and only value:
  virtual double GetValue(int Number);
  
  
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
  ClassDef(MBiasNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
