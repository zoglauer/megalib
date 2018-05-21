/*
 * MBackpropagationNeuron.cxx
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

