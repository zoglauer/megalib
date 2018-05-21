/*
 * MBiasNeuron.cxx
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
// MBiasNeuron
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBiasNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBiasNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MBiasNeuron::MBiasNeuron(double Bias) : MInputNeuron(1) 
{
  // Standard constructor
  
  m_Values[0] = Bias;
}


////////////////////////////////////////////////////////////////////////////////


void MBiasNeuron::SetValue(double Value, int Number) 
{
  // We don not set anything!!
}


////////////////////////////////////////////////////////////////////////////////


double MBiasNeuron::GetValue(int Number) 
{ 
  // Retrieve only the one and only value:
  
  return m_Values[0]; 
}


// MBiasNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

