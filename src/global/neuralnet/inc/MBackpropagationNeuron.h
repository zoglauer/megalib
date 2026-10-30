/*
 * MBackpropagationNeuron.h
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


#ifndef __MBackpropagationNeuron__
#define __MBackpropagationNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFeedForwardNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationNeuron : public MFeedForwardNeuron
{
  // public interface:
public:
  //! Standard constructor
  MBackpropagationNeuron();
  
  //! Copy constructor
  MBackpropagationNeuron(const MBackpropagationNeuron& N);
  
  //! Standard constructor
  MBackpropagationNeuron(int NValues, int NErrors);
  
  //! Set all parameters
  virtual void Set(int NValues = 1, int NErrors = 1);
  
  // protected methods:
protected:
  //! The transfer function
  virtual double TransferFunction(double Value);
  
  
  // private methods:
private:
  
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  
  
  
  #ifdef ___CLING___
public:
  ClassDef(MBackpropagationNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
