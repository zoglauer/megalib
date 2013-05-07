/*
 * MLMLOSEM.cxx
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
// MLMLOSEM
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MLMLOSEM.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TSystem.h"

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MLMLOSEM)
#endif


////////////////////////////////////////////////////////////////////////////////


MLMLOSEM::MLMLOSEM() : MLMLClassicEM()
{
  // Construct an instance of MLMLOSEM

  m_NSubSets = 4;
}


////////////////////////////////////////////////////////////////////////////////


MLMLOSEM::~MLMLOSEM()
{
  // Destruct this instance of MLMLOSEM - most work is done in base class
}


////////////////////////////////////////////////////////////////////////////////


bool MLMLOSEM::DoOneIteration()
{
  // Do only one iteration, i.e. convole and deconvole the data

  int UpdateInterval = 0;

  int MinNEventsPerSubset = 5000;
  unsigned int NSubSets = m_NSubSets;
  if (m_NEvents < MinNEventsPerSubset*NSubSets && NSubSets > 1) {
    cout<<"You need at least "<<MinNEventsPerSubset<<" events per subset for the OS-EM algorithm."<<endl;
    NSubSets = m_NEvents/MinNEventsPerSubset;
    if (NSubSets == 0) NSubSets = 1;
    cout<<"You have "<<m_NEvents<<" events, thus I use "<<NSubSets<<" subsets."<<endl;
  }

  for (unsigned int i = 0; i < m_NEvents; i++) m_Yi[i] = 0.0;

  for (unsigned int s = 0; s < NSubSets; ++s) {
    UpdateInterval = 200*NSubSets;

    // Do the convolution:
    // y_i_bar = Sum_j (t_ij l_j)/v_i + rs*r_i

    // This is very simple for the classic algorithm:
    for (unsigned int i = s; i < m_NEvents; i += NSubSets) {
      // All the convolution-work is done within the MBPImage... classes, 
      // called by m_BPStorage->GetResponseSlice(i): Sum_j (t_ij l_j)
      m_Storage[i]->Convolve(m_Yi, i, m_Lj, m_NBins);
      
      // Normalize and add background
      if (m_Vi[i] != 0) {
        m_Yi[i] = m_Yi[i]/m_Vi[i];
      } else {
        //Warning("void MLMLOSEM::Convolve(double *Ci)",
        //"m_V[i] = 0 not allowed --> The event is not within the image so useless...");
      }
      
      if (m_EnableGUIInteractions == true && (i-s)%UpdateInterval == 0) {
        gSystem->ProcessEvents();
        gSystem->ProcessEvents();
      }
    }

    // The inv is just for improved performance"
    for (unsigned int i = s; i < m_NEvents; i += NSubSets) {
      if (m_Yi[i] == 0) {
        cout<<"LM-ML-OS-EM: We have an empty event. Eliminating event "<<i<<"..."<<endl;
        m_InvYi[i] = 0.0;
      } else {
        m_InvYi[i] = 1.0/m_Yi[i];
      }  
    }

    // Do the de-convolution:
    // In other words: compute the correction image
    //             or: compute the expectation
    // e_j = Sum_i t_ij / y_i_bar for the image pixels
        
    for (unsigned int j = 0; j < m_NBins; ++j) m_Ej[j] = 0.0;

    for (unsigned int i = s; i < m_NEvents; i += NSubSets) {
      // All the deconvolution-work is done within the MBPImage... classes, 
      // called by m_BPStorage->GetResponseSlice(i)
      m_Storage[i]->Deconvolve(m_Ej, m_InvYi, i);
      
      if (m_EnableGUIInteractions == true && (i-s)%UpdateInterval == 0) {
        gSystem->ProcessEvents();
        gSystem->ProcessEvents();
      }
    }
    

    // Correct the image we the data we have so far:
    for (unsigned int j = 0; j < m_NBins; ++j) {
      m_Lj[j] *= m_Ej[j]/m_Sj[j];
    }
  }

  m_NPerformedIterations++;

	if (m_NPerformedIterations == 1) {
    m_InitialLikelihood = 0;
		for (unsigned int i = 0; i < m_NBins; ++i) m_InitialLikelihood += m_Lj[i];
		m_LastLikelihood = 1;
		m_CurrentLikelihood = m_InitialLikelihood;
	} else {
		m_LastLikelihood = m_CurrentLikelihood;
		m_CurrentLikelihood = 0;
		for (unsigned int i = 0; i < m_NBins; ++i) m_CurrentLikelihood += m_Lj[i];
	}

  return true;
}


// MLMLOSEM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
