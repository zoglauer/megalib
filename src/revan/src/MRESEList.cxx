/*
 * MRESEList.cxx
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
// MRESEList.cxx
//
// This is a list of RESE objects.
// The destructor does not delete the objects, but you have to call RemoveAll()
// instead.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MRESEList.h"

// Standard libs:
#include <iostream>
using namespace std;

// ROOT libs:
#include <TObjArray.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MRESE.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MRESEList)
#endif


////////////////////////////////////////////////////////////////////////////////


MRESEList::MRESEList()
{
  // default constructor
}


////////////////////////////////////////////////////////////////////////////////


MRESEList::MRESEList(MRESEList* RESEList)
{
  // copy-constructor
  
  for (int i = 0; i < RESEList->GetNRESEs(); ++i) {
    if (RESEList->GetRESEAt(i) == 0) {
      AddRESE(0);
    } else {
      AddRESE(RESEList->GetRESEAt(i)->Duplicate());
    }
  }

  // restore all links within the list:
  MRESE* RESE;
  int IDRESE, IDLink;
  // for all RESEs in RESEList!
  for (int i = 0; i < RESEList->GetNRESEs(); i++) {
    RESE = RESEList->GetRESEAt(i);
    if (RESE == 0) continue;
    IDRESE = RESE->GetID();
    for (int l = 0; l < RESE->GetNLinks(); l++) {
      IDLink = RESE->GetLinkAt(l)->GetID();
      // the RESE i in m_RESEList should have the same link, so search for a 
      // RESE with ID id in m_RESEList:
      if (RESEList->ContainsRESE(RESE->GetLinkAt(l)) == true) {
        GetRESE(IDRESE)->AddLink(GetRESE(IDLink));
      }
    }

    if (RESE->GetNLinks() > 2 || RESE->GetNLinks() != GetRESE(IDRESE)->GetNLinks()) {
      merr<<RESE->GetNLinks()<<"=?="<<GetRESE(IDRESE)->GetNLinks()<<endl;
      massert(RESE->GetNLinks() > 2);
      massert(RESE->GetNLinks() != GetRESE(IDRESE)->GetNLinks());
    }
  }   
}


////////////////////////////////////////////////////////////////////////////////


MRESEList::~MRESEList()
{
  // the destructor does not delete the objects stored in the list.
  // Call RemoveAll() before calling the destructor .
}


////////////////////////////////////////////////////////////////////////////////


void MRESEList::DeleteAll()
{
  // Remove all objects from the list

  massert(this != 0);

  //m_RESEList->Delete();

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) != 0) {
      (*Iter)->DeleteAll();
      // delete GetRESEAt(i);
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


int MRESEList::GetNRESEs()
{
  // return number of raw event sub elements in the list

  massert(this != 0);

  return int(m_RESEList.size());
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEList::GetRESEAt(const int i)
{
  // return the RESE at position i in the list. Counting starts with zero!
  // This is an extremely  time critical function! 

  massert(this != 0);

  if (i < int(m_RESEList.size())) {
    return m_RESEList[i];
  } else {
    merr<<"Index ("<<i<<") out of bounds (0, "<<GetNRESEs()-1<<")"<<endl;
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEList::GetRESE(int ID)
{
  //
  
  massert(this != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) != 0) {
      if ((*Iter)->GetID() == ID) {
        return (*Iter);
      }
    }
  }

  merr<<"No RESE with ID="<<ID<<" found!"<<endl;

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEList::GetRESEContainingRESE(int ID)
{
  //
  
  massert(this != 0);

  // Top level check:
  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) != 0) {
      if ((*Iter)->GetID() == ID) {
        return (*Iter);
      }
    }
  }
  
  // Deeper check:
  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) != 0) {
      if ((*Iter)->GetNRESEs() != 0) {
        if ((*Iter)->GetRESEContainingRESE(ID) != 0) {
          return (*Iter);
        }
      }
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MRESEList::ContainsRESE(int ID)
{
  //
  
  massert(this != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) != 0) {
      if ((*Iter)->GetID() == ID) {
        return true;
      }
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MRESEList::ContainsRESE(MRESE* RESE)
{
  // return true if RESE (the pointer!) is on the list

  massert(this != 0);
  massert(RESE != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) == RESE) return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MRESEList::AddRESE(MRESE* RESE)
{
  // Add a RESE to the end of the list

  massert(this != 0);
  massert(RESE != 0);

  m_RESEList.push_back(RESE);
}


////////////////////////////////////////////////////////////////////////////////


void MRESEList::AddRESEFirst(MRESE* RESE)
{
  // Add a RESE to the beginning of the list

  massert(this != 0);
  massert(RESE != 0);

  m_RESEList.insert(m_RESEList.begin(), RESE);
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEList::RemoveRESE(MRESE* RESE)
{
  // Remove RESE from the list. Afterwards the list contains an empty slot!

  massert(this != 0);
  massert(RESE != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    if ((*Iter) == RESE) {
      (*Iter) = 0;
      return RESE;
    }
  }

  merr<<"Can't remove a RESE not contained in the list!"<<endl;;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MRESE* MRESEList::RemoveRESEAt(int i)
{
  // Remove and return the RESE at position i in the list. 
  // An empty slot remains!

  massert(this != 0);

  MRESE* RESE = 0;

  if (i < int(m_RESEList.size())) {
    RESE = m_RESEList[i];
    m_RESEList[i] = 0;
    return RESE;
  }

  merr<<"Index "<<i<<" out of bounds (0, "<<m_RESEList.size()<<")"<<endl;
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


void MRESEList::RemoveAllRESEs()
{
  // Remove all RESEs from the list

  massert(this != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    (*Iter) = 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MRESEList::Compress()
{
  // Remove empty slots from the list - time consuming!

  massert(this != 0);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); /* ++Iter */) {
    if ((*Iter) == 0) {
      Iter = m_RESEList.erase(Iter);
    } else {
      ++Iter;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


MRESEList* MRESEList::Duplicate()
{
  // Duplicate this object

  massert(this != 0);

  return new MRESEList(this);
}


////////////////////////////////////////////////////////////////////////////////


MString MRESEList::ToString(bool WithLink, int Level)
{
  // Returns a MString containing the relevant data of this object
  //
  // WithLink: Display the links
  // Level:    A level of N displays 3*N blancs before the text

  massert(this != 0);

  char Text[1000];

  MString String("");
  sprintf(Text, "Contents of this RESEList:\n");
  for (int i = 0; i < Level; i++) {
    String += MString("   ");
  }
  String += MString(Text);

  for (vector<MRESE*>::iterator Iter = m_RESEList.begin(); 
       Iter != m_RESEList.end(); ++Iter) {
    String += (*Iter)->ToString(WithLink, Level+1);
  }

  return String;
}


// MRESEList.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
