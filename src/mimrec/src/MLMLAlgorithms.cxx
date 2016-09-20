/*
 * MLMLAlgorithms.cxx
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
// MLMLAlgorithms
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MLMLAlgorithms.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MLMLAlgorithms)
#endif


////////////////////////////////////////////////////////////////////////////////


// Don't modify the numbers, they are stored in the mimrec configuration file
const unsigned int MLMLAlgorithms::c_ClassicEM = 0;
const unsigned int MLMLAlgorithms::c_OSEM      = 1;

const unsigned int MLMLAlgorithms::c_StopAfterIterations = 0;
const unsigned int MLMLAlgorithms::c_StopAfterLikelihoodIncrease = 0;


////////////////////////////////////////////////////////////////////////////////


MLMLAlgorithms::MLMLAlgorithms()
{
  // default constructor

  m_UsedStopCriterion = c_StopAfterIterations;
  m_MaxNIterations = 10;

  //m_NIterations = 0;
  m_NPerformedIterations = 0;

  m_NBins = 0;
  m_NEvents = 0;

  m_Lj = nullptr;
  m_Sj = nullptr;
  m_Ri = nullptr;
  m_Yi = nullptr;
  m_InvYi = nullptr;

  m_Exposure = nullptr;
  m_Background = nullptr;
  
  m_EnableGUIInteractions = true;
}


////////////////////////////////////////////////////////////////////////////////


MLMLAlgorithms::~MLMLAlgorithms()
{
  // default destructor

  delete [] m_Lj;
  delete [] m_Sj;
  delete [] m_Ri;
  delete [] m_Yi;
  delete [] m_InvYi;
}



////////////////////////////////////////////////////////////////////////////////


void MLMLAlgorithms::UseStopCriterionByIterations(unsigned int NIterations)
{
  //! Use the stop criterion 

  m_UsedStopCriterion = c_StopAfterIterations;
  m_MaxNIterations = NIterations;
}


////////////////////////////////////////////////////////////////////////////////


bool MLMLAlgorithms::IsStopCriterionFullfilled()
{
  //! Return true if the stop criterion is fullfilled

  if (m_UsedStopCriterion == c_StopAfterIterations) {
    if (m_NPerformedIterations >= m_MaxNIterations) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////


void MLMLAlgorithms::ResetStopCriterion()
{
  //! Reset data used by the stop criterion
  
  m_NPerformedIterations = 0;
}


////////////////////////////////////////////////////////////////////////////////


// void MLMLAlgorithms::SetNIterations(int NIterations)
// {
//   // Set the number of iterations

//   m_NIterations = NIterations;
// }


////////////////////////////////////////////////////////////////////////////////


bool MLMLAlgorithms::SetResponseSlices(vector<MBPData*>& Storage, int NImageBins)
{
  // Set the backprojection storage (PSFs!)

  m_NEvents = Storage.size();
  m_NBins = NImageBins;

  m_Storage = Storage;

  // ... and we can allocate some of the important arrays
  if (m_Lj != 0) delete [] m_Lj;
  m_Lj = new(nothrow) double[m_NBins];
  if (m_Lj == 0) {
    merr<<"Out of memory"<<show;
    return false;
  }

  if (m_Sj != 0) delete [] m_Sj;
  m_Sj = new(nothrow) double[m_NBins];
  if (m_Sj == 0) {
    merr<<"Out of memory"<<show;
    return false;
  }

  if (m_Ri != 0) delete [] m_Ri;
  m_Ri = new(nothrow) double[m_NEvents];
  if (m_Ri == 0) {
    merr<<"Out of memory"<<show;
    return false;
  }

  if (m_Yi != 0) delete [] m_Yi;
  m_Yi = new(nothrow) double[m_NEvents];
  if (m_Yi == 0) {
    merr<<"Out of memory"<<show;
    return false;
  }

  if (m_InvYi != 0) delete [] m_InvYi;
  m_InvYi = new(nothrow) double[m_NEvents];
  if (m_InvYi == 0) {
    merr<<"Out of memory"<<show;
    return false;
  }

  // ... and initialize them to default values:
  
  // a flat image (O.K., because the only necessary requirement is > 0)
  for (unsigned int i = 0; i < m_NBins; i++) m_Lj[i] = 1.0;

  // a flat sensitivity (O.K. as long as the detector has a wide field of view)
  for (unsigned int i = 0; i < m_NBins; i++) m_Sj[i] = 1.0;

  // no background (somewhat too ideal for COMPTEL..., but ok...)
  for (unsigned int i = 0; i < m_NEvents; i++) m_Ri[i] = 0.0;

  // All events are totally within the image space (will not work for 
  // Compton-cameras, unless the reconstructed object is not much smaller 
  // than the image...)
  for (unsigned int i = 0; i < m_NEvents; i++) m_Yi[i] = 1.0;
  for (unsigned int i = 0; i < m_NEvents; i++) m_InvYi[i] = 1.0;

  // Initial Background scaling factor
  m_BgdS = 0.0;
  // Initial total background:
  m_BgdT = 1.0;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MLMLAlgorithms::SetExposure(MExposure* Exposure)
{
  // Set the sensitivity

  massert(Exposure != nullptr);

  m_Exposure = Exposure;

  if (m_Sj != 0) delete [] m_Sj;
  m_Sj = Exposure->GetExposure(); // Array must be deleted here 
}


////////////////////////////////////////////////////////////////////////////////


void MLMLAlgorithms::SetBackground(MBackground *Background)
{
  // Set the background model

  massert(Background != nullptr);

  m_Background = Background;

  if (m_Ri != 0) delete [] m_Ri;
  m_Ri = new(nothrow) double[m_NEvents];
  //m_Ri = m_Background->GetBackground(m_BPStorage);

  // Total Background
  m_BgdT = 0;
  for (unsigned int i = 0; i < m_NEvents; ++i) m_BgdT += m_Ri[i];

  // Initial Background scaling factor
  m_BgdS = 1.0;
}


////////////////////////////////////////////////////////////////////////////////


double* MLMLAlgorithms::GetImage()
{
  // Return the current estimation of the image

  return m_Lj;
}


// MLMLAlgorithms.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
