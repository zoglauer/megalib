/*
 * MBackpropagationMiddleNeuron.cxx
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
// MBackpropagationMiddleNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBackpropagationMiddleNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MSynapse.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBackpropagationMiddleNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MBackpropagationMiddleNeuron::MBackpropagationMiddleNeuron() : 
MBackpropagationOutputNeuron() 
{
  // Default constructor
}

////////////////////////////////////////////////////////////////////////////////


MBackpropagationMiddleNeuron::MBackpropagationMiddleNeuron(const MBackpropagationMiddleNeuron& N) :
MBackpropagationOutputNeuron(N) 
{
  // Copy constructor
}


////////////////////////////////////////////////////////////////////////////////


MBackpropagationMiddleNeuron::MBackpropagationMiddleNeuron(double LearningRate, double Momentum, int NValues, int NErrors) : 
MBackpropagationOutputNeuron(LearningRate, Momentum, NValues, NErrors) 
{
  // Standard constructor
}


////////////////////////////////////////////////////////////////////////////////


void MBackpropagationMiddleNeuron::Set(double LearningRate, double Momentum, int NValues = 1, int NErrors = 1)
{
  // Set all parameters
  
  MBackpropagationOutputNeuron::Set(LearningRate, Momentum, NValues, NErrors);
}


////////////////////////////////////////////////////////////////////////////////


double MBackpropagationMiddleNeuron::ComputeError(int Mode)
{
  // Copmpute the error
  
  double Total = 0.0;
  for (unsigned int i = 0; i < m_OutputSynapses.size(); ++i) {
    Total += m_OutputSynapses[i]->GetWeightedOutError();
  }
  return m_Values[0]*(1.0-m_Values[0])*Total;
}


// MBackpropagationMiddleNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////



