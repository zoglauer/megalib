/*
 * MBackpropagationNeuron.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MBackpropagationNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackpropagationNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackpropagationNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackpropagationNeuron::MBackpropagationNeuron() : MFeedForwardNeuron() 
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationNeuron::MBackpropagationNeuron(const MBackpropagationNeuron& N) : 
MFeedForwardNeuron(N) 
{
  // Copy constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationNeuron::MBackpropagationNeuron(int NValues, int NErrors) : 
MFeedForwardNeuron(NValues, NErrors) 
{
  // Standard constructor
}


////////////////////////////////////////////////////////////////////////////////


void MBackpropagationNeuron::Set(int NValues, int NErrors) 
{ 
  // Set all parameters
  
  MFeedForwardNeuron::Set(NValues, NErrors);
}


////////////////////////////////////////////////////////////////////////////////


double MBackpropagationNeuron::TransferFunction(double Value)
{
  // Usa a sigmoidal transfer function 
  
  return 1.0/(1.0+exp(-Value));
}


// MBackpropagationNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

