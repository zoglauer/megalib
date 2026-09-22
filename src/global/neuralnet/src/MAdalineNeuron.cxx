/*
 * MAdalineNeuron.cxx
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
// MAdalineNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAdalineNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MSynapse.h"
#include "MFeedForwardNeuron.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAdalineNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MAdalineNeuron::MAdalineNeuron() : 
MFeedForwardNeuron(2, 1) 
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MAdalineNeuron::MAdalineNeuron(double LearningRate) : MFeedForwardNeuron(2, 1) 
{
  // Standard constructor
  
  m_Values[c_LearningRateIndex] = LearningRate;
}


////////////////////////////////////////////////////////////////////////////////


double MAdalineNeuron::TransferFunction(double Value)
{
  // The transfer function
  
  if (Value < 0) {
    return -1.0;
  } else {
    return 1.0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MAdalineNeuron::Learn(int Mode) 
{
  // do the learning
  
  m_Errors[c_ErrorIndex] = m_Values[c_ValueIndex] - 2.0;
  
  double Delta = 0.0;
  for (unsigned int i = 0; i < m_InputSynapses.size(); ++i) {
    Delta = m_Values[c_LearningRateIndex]*m_InputSynapses[i]->GetInValue()*m_Errors[c_ErrorIndex];
    m_InputSynapses[i]->SetWeight(Delta);
  }
}


// MAdalineNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
