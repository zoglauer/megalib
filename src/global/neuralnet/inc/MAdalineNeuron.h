/*
 * MAdalineNeuron.h
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


#ifndef __MAdalineNeuron__
#define __MAdalineNeuron__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFeedForwardNeuron.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MAdalineNeuron : public MFeedForwardNeuron
{
  // public interface:
public:
  //! Default constructor
  MAdalineNeuron();
  //! Standard constructor
  MAdalineNeuron(double LearningRate);
  
  //! The Run routine:
  virtual void Learn(int Mode = 0);
  
  
  // protected methods:
protected:
  virtual double TransferFunction(double Value);
  
  // private methods:
private:
  
  
  // protected members:
protected:
  
  
  // private members:
private:
  static const int c_ValueIndex = 0;
  static const int c_LearningRateIndex = 1;
  static const int c_ErrorIndex = 0;
  
  #ifdef ___CLING___
public:
  ClassDef(MAdalineNeuron, 0) // no description
  #endif
  
};

#endif


////////////////////////////////////////////////////////////////////////////////
