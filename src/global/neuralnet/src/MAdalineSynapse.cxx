/*
 * MAdalineSynapse.cxx
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
// MAdalineSynapse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MAdalineSynapse.h"

// Standard libs:
#include <sstream>
#include <iostream>
#include <list>
using namespace std;

// ROOT libs:
#include "TRandom.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MAdalineSynapse)
#endif


////////////////////////////////////////////////////////////////////////////////


MAdalineSynapse::MAdalineSynapse() : 
MSynapse() 
{
  // Default constructor
  
  m_Weight=2*gRandom->Rndm()-1;
}


// MAdalineSynapse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

