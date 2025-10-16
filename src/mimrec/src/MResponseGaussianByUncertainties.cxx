/*
 * MResponseGaussianByUncertainties.cxx
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
  if (Event->GetType() == c_ComptonEvent) {
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
  } else if (Event->GetType() == c_PairEvent) {
    return true;
  }

  return false;
}


// MResponseGaussianByUncertainties.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
