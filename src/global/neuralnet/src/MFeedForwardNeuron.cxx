/*
 * MFeedForwardNeuron.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
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


#ifdef ___CINT___
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
