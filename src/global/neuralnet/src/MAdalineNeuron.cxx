/*
 * MAdalineNeuron.cxx
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
