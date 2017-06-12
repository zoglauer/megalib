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
#include <algorithm>
#include <chrono>
#include <thread>
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

  m_NSetSubSets = 1;
  m_NUsedSubSets = m_NSetSubSets;
}


////////////////////////////////////////////////////////////////////////////////


MLMLOSEM::~MLMLOSEM()
{
  // Destruct this instance of MLMLOSEM - most work is done in base class
}


////////////////////////////////////////////////////////////////////////////////


//! Set the number of subsets:
void MLMLOSEM::SetNSubSets(unsigned int NSubSets)
{
  m_NSetSubSets = NSubSets;
  if (m_NSetSubSets < 1) {
    m_NSetSubSets = 1;
  }
  m_NUsedSubSets = m_NSetSubSets;
}


////////////////////////////////////////////////////////////////////////////////


//! Randomly shuffle the events
void MLMLOSEM::Shuffle()
{
  random_shuffle(m_Storage.begin(), m_Storage.end());
}


////////////////////////////////////////////////////////////////////////////////


void MLMLOSEM::CalculateEventApportionment()
{
  // Split the events by used bins

  m_EventApportionment.clear();

  int MinNEventsPerSubset = 5000;
  m_NUsedSubSets = m_NSetSubSets;
  if (m_NEvents < MinNEventsPerSubset*m_NUsedSubSets && m_NUsedSubSets > 1) {
    cout<<"OS-EM: You need at least "<<MinNEventsPerSubset<<" events per subset for the OS-EM algorithm."<<endl;
    m_NUsedSubSets = m_NEvents/MinNEventsPerSubset;
    if (m_NUsedSubSets == 0) m_NUsedSubSets = 1;
  }
  cout<<"OS-EM: You have "<<m_NEvents<<" events, thus I use "<<m_NUsedSubSets<<" subsets."<<endl;

  unsigned int Split = m_Storage.size() / m_NThreads / m_NUsedSubSets;
  if (Split == 0) {
    m_NUsedThreads = 1;
    Split = m_Storage.size() / m_NUsedThreads / m_NUsedSubSets;
  } else {
    m_NUsedThreads = m_NThreads; 
  }

  for (unsigned int i = 0; i < m_NUsedThreads*m_NUsedSubSets; ++i) {
    unsigned int Start = 0;
    if (m_EventApportionment.size() != 0) {
      Start = m_EventApportionment.back().second + 1;
    }
    unsigned int Stop = Start + Split - 1;
    if (i == m_NUsedThreads*m_NUsedSubSets - 1) {
      Stop = m_Storage.size() - 1;
    }
    //cout<<"Split: "<<Start<<":"<<Stop<<endl;
    m_EventApportionment.push_back(pair<unsigned int, unsigned int>(Start, Stop));
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MLMLOSEM::DoOneIteration()
{
  // Do only one iteration, i.e. convole and deconvole the data

  for (unsigned int i = 0; i < m_NEvents; i++) m_Yi[i] = 0.0;

  // Everything happens in the main thread
  if (m_NUsedThreads == 1) { 
    for (unsigned int s = 0; s < m_NUsedSubSets; ++s) {
      //cout<<"Subset: "<<s+1<<"/"<<m_NUsedSubSets<<endl;
      // Convolve:
      Convolve(m_EventApportionment[s].first, m_EventApportionment[s].second);

      // Deconvolve:
      ResetExpectation();
      Deconvolve(m_EventApportionment[s].first, m_EventApportionment[s].second);
      CorrectImage();
    }
  }
  // Now multi-threaded
  else {

    for (unsigned int s = 0; s < m_NUsedSubSets; ++s) {
      //cout<<"Subset: "<<s+1<<"/"<<m_NUsedSubSets<<endl;
      // Convolution:
      vector<thread> Threads(m_NUsedThreads);
      m_ThreadRunning.resize(m_NUsedThreads, true);
      for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
        m_ThreadRunning[t] = true;
        Threads[t] = thread(&MLMLOSEM::ConvolveThreadEntry, this, t, m_EventApportionment[t + s*m_NUsedThreads].first, m_EventApportionment[t + s*m_NUsedThreads].second);
      }
      while (true) {
        bool Finished = true;
        for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
          if (m_ThreadRunning[t] == true) {
            Finished = false;
            break;
          }
        }
        if (Finished == false) {
          this_thread::sleep_for(chrono::milliseconds(1));
        } else {
          for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
            Threads[t].join();
          }
          break;
        }
      }

      // Deconvolution:
      
      // Reset the expectation:
      if (m_tEj.size() != m_NUsedThreads) {
        m_tEj.resize(m_NUsedThreads, vector<double>(m_NBins));
      }
      for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
        for (unsigned int i = 0; i < m_NBins; ++i) {
          m_tEj[t][i] = 0;
        }
      }
      
      // Deconvolve
      Threads.clear();
      for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
        m_ThreadRunning[t] = true;
        Threads[t] = thread(&MLMLOSEM::DeconvolveThreadEntry, this, t, m_EventApportionment[t + s*m_NUsedThreads].first, m_EventApportionment[t + s*m_NUsedThreads].second);
      }
      while (true) {
        bool Finished = true;
        for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
          if (m_ThreadRunning[t] == true) {
            Finished = false;
            break;
          }
        }
        if (Finished == false) {
          this_thread::sleep_for(chrono::milliseconds(1));
        } else {
          for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
            Threads[t].join();
          }
          break;
        }
      }
      
      ResetExpectation();
      for (unsigned int t = 0; t < m_NUsedThreads; ++t) {
        for (unsigned int i = 0; i < m_NBins; i++) {
          m_Ej[i] += m_tEj[t][i];
        }
      }      
      
      CorrectImage();
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


////////////////////////////////////////////////////////////////////////////////


void MLMLOSEM::ConvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop)
{
  MLMLClassicEM::ConvolveThreadEntry(ThreadID, Start, Stop);
}


////////////////////////////////////////////////////////////////////////////////


void MLMLOSEM::DeconvolveThreadEntry(unsigned int ThreadID, unsigned int Start, unsigned int Stop)
{
  MLMLClassicEM::DeconvolveThreadEntry(ThreadID, Start, Stop);
}


// MLMLOSEM.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
