/*
 * MInputNeuron.cxx
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

