/*
 * MRESEIterator.cxx
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
// MRESEIterator
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRESEIterator.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MRESEIterator)
#endif


////////////////////////////////////////////////////////////////////////////////


MRESEIterator::MRESEIterator()
{
  // Construct an instance of MRESEIterator

  m_Start = 0;
  m_Previous = 0;
  m_Current = 0;
  m_IsStart = true;
}


////////////////////////////////////////////////////////////////////////////////


MRESEIterator::MRESEIterator(MRESE* RESE)
{
  // Construct an instance of MRESEIterator
  // This constructor ignores errors, which occur during "Start"!!

  m_Start = 0;
  m_Previous = 0;
  m_Current = 0;
  m_IsStart = true;

  Start(RESE);
}


////////////////////////////////////////////////////////////////////////////////


MRESEIterator::~MRESEIterator()
{
  // Nothing needs to be deleted:

  m_Start = 0;
  m_Previous = 0;
  m_Current = 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MRESEIterator::Start(MRESE* RESE)
{
  // Reset the iterator
  // RESE is the startpoint of the iteration. 
  // It must have one and only one link! 
  // Use GetNextRESE() for the iteration

  massert(RESE != 0);

  m_Start = RESE;
  m_Previous = 0;
  m_Current = RESE;

//   if (RESE->GetNLinks() != 1) {
//     merr<<"The iteration startpoint has "<<RESE->GetNLinks()
//         <<" links instead of one!"<<endl;
//     m_Current = 0;
//     return false;
//   }

//   m_Next = m_Current->GetOtherLink(m_Previous);
//   m_IsStart = true;

  if (RESE->GetNLinks() != 1) {
    m_Next = 0;
  } else {
    m_Next = m_Current->GetOtherLink(m_Previous);
  }
  m_IsStart = true;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEIterator::GetNextRESE()
{
  // Loop iterative over all RESE in the track
  // If NULL is returned then there is no next RESE
  //
  // Example:
  // Track->ResetIterator();
  // if (Track->IsValid() == true) {
  //   while ((RESE = Track->GetNextRESE()) != 0) {
  //   ...
  //   }
  // }

  massert(m_Start != 0);

  if (m_IsStart == false) {
    if (m_Next != 0) {
      m_Previous = m_Current;
      m_Current = m_Next;
      m_Next = m_Current->GetOtherLink(m_Previous);
    } else if (m_Current != 0) {
      m_Previous = m_Current;
      m_Current = m_Next;
      m_Next = 0;      
    }
  } else {
    m_IsStart = false;
  }

  //cout<<"next: "<<int(m_Previous)<<"!"<<int(m_Current)<<"!"<<int(m_Next)<<endl;

  return m_Current;
}


////////////////////////////////////////////////////////////////////////////////


int MRESEIterator::GetNRESEs()
{
  // Does not modify anything!

  massert(m_Start != 0);

  MRESEIterator I;
  I.Start(m_Start);

  int i = 0;
  MRESE* RESE;
  while ((RESE = I.GetNextRESE()) != 0) {
    //cout<<i<<": "<<int(RESE)<<endl;
    ++i;
  }

  return i;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEIterator::GetRESEAt(int i)
{
  // The returned RESE is the new current RESE!

  massert(m_Start != 0);

  Start(m_Start);
  MRESE* RESE = 0;
  while ((RESE = GetNextRESE()) != 0) {
    if (--i < 0) {
      //cout<<"at: "<<int(m_Previous)<<"!"<<int(m_Current)<<"!"<<int(m_Next)<<endl;
      return RESE;
    }
  }

  //cout<<"at: "<<int(m_Previous)<<"!"<<int(m_Current)<<"!"<<int(m_Next)<<endl;

  return 0;
}


// MRESEIterator.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
