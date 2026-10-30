/*
 * MBackpropagationMiddleNeuron.h
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


#ifndef __MBackpropagationMiddleNeuron__
#define __MBackpropagationMiddleNeuron__


////////////////////////////////////////////////////////////////////////////////



// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBackpropagationOutputNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MBackpropagationMiddleNeuron : public MBackpropagationOutputNeuron
{
  // public interface:
public:
  //! Default constructor
  MBackpropagationMiddleNeuron();
  //! Copy constructor
  MBackpropagationMiddleNeuron(const MBackpropagationMiddleNeuron& N);
  //! Standard constructor
  MBackpropagationMiddleNeuron(double LearningRate, double Momentum, int NValues, int NErrors);
  
  //! Sety all parameters
  virtual void Set(double LearningRate, double Momentum, int NValues, int NErrors);
  //! Compute the error
  virtual double ComputeError(int Mode = 0);
  
  
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
  ClassDef(MBackpropagationMiddleNeuron, 0) // no description
  #endif
  
};


#endif


////////////////////////////////////////////////////////////////////////////////

