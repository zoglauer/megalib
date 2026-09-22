/*
 * MResponseGaussianByUncertainties.cxx
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
// MResponseGaussianByUncertainties
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MResponseGaussianByUncertainties.h"

// Standard libs:
#include <cmath>
using namespace std;

// ROOT libs:
#include <TMath.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MComptonEvent.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MResponseGaussianByUncertainties)
#endif


////////////////////////////////////////////////////////////////////////////////


MResponseGaussianByUncertainties::MResponseGaussianByUncertainties() : MResponseGaussian(1, 1, 1, 1)
{
  // default constructor

  m_Increase = 0.0;
  
  // The default threshold is 2.5 sigmas:
  m_Threshold = 2.5;

  // 1.0 sigma :  1.0 - 0.632215; 
  // 1.5 sigma :  1.0 - 0.894141;
  // 2.0 sigma :  1.0 - 0.981645;
  // 2.5 sigma :  1.0 - 0.998066;
  // 3.0 sigma :  1.0 - 0.999873;
  // 3.5 sigma :  1.0 - 0.999971;
}


////////////////////////////////////////////////////////////////////////////////


MResponseGaussianByUncertainties::~MResponseGaussianByUncertainties()
{
  // default destructor

}


////////////////////////////////////////////////////////////////////////////////


bool MResponseGaussianByUncertainties::AnalyzeEvent(MPhysicalEvent* Event)
{
  // Extract all necessary information out of the event:

  // Only the Compton angle is of interest:
  if (Event->GetType() == MPhysicalEvent::c_Compton) {
    m_Phi = dynamic_cast<MComptonEvent*>(Event)->Phi();
    m_HasTrack = dynamic_cast<MComptonEvent*>(Event)->HasTrack();
    
    double dPhi = dynamic_cast<MComptonEvent*>(Event)->dPhi()*c_Deg;
    //if (dPhi > 15) dPhi = 15;
    if (dPhi < 3.0) dPhi = 3.0; // COSI HACK
    if (dPhi == 0.0) {
      cout<<"Error: This Compton event has no cone width!"<<endl;
      return false;
    }
    dPhi += m_Increase;
    SetGaussians(dPhi, 30, 2, 1);
     
    return true;
  } else if (Event->GetType() == MPhysicalEvent::c_Pair) {
    return true;
  }

  return false;
}


// MResponseGaussianByUncertainties.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
