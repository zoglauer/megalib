/*
 * MBackpropagationMiddleNeuron.cxx
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


#ifdef ___CINT___
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



