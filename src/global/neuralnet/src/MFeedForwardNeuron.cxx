/*
 * MFeedForwardNeuron.cxx
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
// MFeedForwardNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFeedForwardNeuron.h"

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
ClassImp(MFeedForwardNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MFeedForwardNeuron::MFeedForwardNeuron() : 
MNeuron() 
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MFeedForwardNeuron::MFeedForwardNeuron(const MFeedForwardNeuron& N) : 
MNeuron(N) 
{
  // Copy constructor
}


////////////////////////////////////////////////////////////////////////////////


MFeedForwardNeuron::MFeedForwardNeuron(int NValues, int NErrors) : 
MNeuron(NValues, NErrors) 
{
  // Standard constructor
}


////////////////////////////////////////////////////////////////////////////////



double MFeedForwardNeuron::TransferFunction(double Value) 
{
  // Apply the transfer function - default procedure is forwarding
  
  return Value; 
}


////////////////////////////////////////////////////////////////////////////////


void MFeedForwardNeuron::Set(int NValues, int NErrors) 
{
  // Set all parameters
  
  MNeuron::Set(NValues, NErrors);
}


////////////////////////////////////////////////////////////////////////////////


void MFeedForwardNeuron::Run(int Mode) 
{
  // Run - tranfer the values from the input synapses
  
  double TotalWeight = 0.0;
  for (unsigned int i = 0; i < m_InputSynapses.size(); ++i) {
    //cout<<"Weighted in "<<i<<": "<<m_InputSynapses[i]->GetWeightedInValue(Mode)<<" - "<<m_InputSynapses[i]->GetInValue(Mode)<<endl;
    TotalWeight += m_InputSynapses[i]->GetWeightedInValue(Mode);
  }
  if (std::isnan(TransferFunction(TotalWeight)) == true) {
    cout<<"MFeedForwardNeuron::Run: TransferFunction(TotalWeight) == NaN!"<<endl;
    cout<<TotalWeight<<endl;
  }
  m_Values[Mode] = TransferFunction(TotalWeight);
}


// MFeedForwardNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
