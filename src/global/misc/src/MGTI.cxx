/*
 * MGTI.cxx
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
#include "MGTI.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MParser.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGTI)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MGTI::MGTI()
{
  m_GoodStart.push_back(MTime(0));
  m_GoodStop.push_back(MTime(2000000000));
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MGTI::~MGTI()
{
}


////////////////////////////////////////////////////////////////////////////////


void MGTI::Reset(bool AllOpen)
{
  //! Reset - Create an open GTI

  m_GoodStart.clear();
  m_GoodStop.clear();
  m_BadStart.clear();
  m_BadStop.clear();

  if (AllOpen == true) {
    m_GoodStart.push_back(MTime(0));
    m_GoodStop.push_back(MTime(2000000000));
  }
}

////////////////////////////////////////////////////////////////////////////////


//! Check if the time is withing a good interval 
bool MGTI::IsGood(const MTime& Time) const
{
  bool InGood = false;
  for (unsigned int i = 0; i < m_GoodStart.size(); ++i) {
    //cout<<"GTI test: "<<m_GoodStart[i]<<"-"<<m_GoodStop[i]<<endl;
    if (Time >= m_GoodStart[i] && Time <= m_GoodStop[i]) {
      InGood = true;
      break;
    }
  }
  if (InGood == false) return false;
  
  for (unsigned int i = 0; i < m_BadStart.size(); ++i) {
    //cout<<"BTI test: "<<m_BadStart[i]<<"-"<<m_BadStop[i]<<endl;
    if (Time >= m_BadStart[i] && Time <= m_BadStop[i]) {
      return false;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Load the good time interval data
bool MGTI::Load(const MString& FileName)
{
  Reset(false);
  
  MParser P;
  P.Open(FileName);
  if (P.IsOpen() == false) {
    cout<<"Error: Unable to open GTI file: "<<FileName<<endl;
    Reset(true);
    return false;
  }
  
  
  for (unsigned int l = 0; l < P.GetNLines(); ++l) {
    if (P.GetTokenizerAt(l)->GetNTokens() == 1 && P.GetTokenizerAt(l)->IsTokenAt(0, "EN") == true) break;
    if (P.GetTokenizerAt(l)->GetNTokens() != 3) continue;
    if (P.GetTokenizerAt(l)->IsTokenAt(0, "GT") == true) {
      m_GoodStart.push_back(P.GetTokenizerAt(l)->GetTokenAtAsTime(1));
      m_GoodStop.push_back(P.GetTokenizerAt(l)->GetTokenAtAsTime(2));
    }
    if (P.GetTokenizerAt(l)->IsTokenAt(0, "BT") == true) {
      m_BadStart.push_back(P.GetTokenizerAt(l)->GetTokenAtAsTime(1));
      m_BadStop.push_back(P.GetTokenizerAt(l)->GetTokenAtAsTime(2));
    }
  }
 
  return true;
}


// MGTI.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
