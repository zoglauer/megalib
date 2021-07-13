/*
 * MInputNeuron.cxx
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

// Include the header:
#include "MInputNeuron.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:

// MEGAlib:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MInputNeuron)
#endif


////////////////////////////////////////////////////////////////////////////////


MInputNeuron::MInputNeuron(int NValues) : MNeuron(NValues, NValues) 
{
  // Standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MInputNeuron::MInputNeuron(const MInputNeuron& N) : MNeuron(N) 
{
  // Copy constructor
}


// MInputNeuron.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

