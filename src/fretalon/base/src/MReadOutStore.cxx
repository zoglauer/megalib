/*
 * MReadOutStore.cxx
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
// MReadOutStore
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutStore.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MExceptions.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MReadOutStore)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutStore::MReadOutStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutStore::~MReadOutStore()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Remove all collected data
void MReadOutStore::Clear()
{
  m_Collection.clear();
  m_ReadOutDataGroups.clear();
}


////////////////////////////////////////////////////////////////////////////////


//! Add a group to all collections and return its ID
unsigned int MReadOutStore::AddReadOutDataGroup(const MString& Name)
{
  m_ReadOutDataGroups.push_back(Name);
  for (unsigned int c = 0; c < m_Collection.size(); ++c) {
    m_Collection[c].AddReadOutDataGroup(Name);
  }
  return m_ReadOutDataGroups.size() - 1;
}

  
////////////////////////////////////////////////////////////////////////////////

  
//! Move the content of the read-out data store to this ones GroupID
//! Group ID must exist, and the store must only have one own group at position 0
bool MReadOutStore::Move(MReadOutStore& Store, unsigned int GroupID)
{
  if (GroupID >= m_ReadOutDataGroups.size()) {
    merr<<"Read out data group does not exist: "<<GroupID<<endl;
    return false;
  }
  if (Store.m_ReadOutDataGroups.size() != 1) {
    merr<<"Store is only allowed to have exactly one read-out-data group and not "<<Store.m_ReadOutDataGroups.size()<<endl;
    return false;
  }
  
  for (unsigned int c = 0; c < Store.m_Collection.size(); ++c) {
    MReadOutCollection C(Store.m_Collection[c].GetReadOutElement());
    vector<MReadOutCollection>::iterator I = lower_bound(m_Collection.begin(), m_Collection.end(), C);
    // We have a lower bound, i.e. the first which is not "<", so we might not yet have found it and we have to check a final time:
    if (I == m_Collection.end() || !((*I).GetReadOutElement() == Store.m_Collection[c].GetReadOutElement())) {
      vector<MReadOutCollection>::iterator New = m_Collection.insert(I, C);
      (*New).AddReadOutDataGroups(m_ReadOutDataGroups);
      (*New).Move(Store.m_Collection[c].GetReadOutDataGroup(0), GroupID);
    } else {
      (*I).Move(Store.m_Collection[c].GetReadOutDataGroup(0), GroupID);
    }
  }
  
  return true;
}

  
////////////////////////////////////////////////////////////////////////////////


//! Add the data of a read-out sequence to the store 
bool MReadOutStore::Add(const MReadOutSequence& Sequence, unsigned int Group)
{
  if (Group < m_ReadOutDataGroups.size()) {
    // Loop over all collections and try to add the sequence data
    for (unsigned int r = 0; r < Sequence.GetNumberOfReadOuts(); ++r) { 
      /*
      bool Found = false;
      for (unsigned int c = 0; c < m_Collection.size(); ++c) {
        Found = m_Collection[c].Add(Sequence.GetReadOut(r), Group);
        if (Found == true) break;
      }
      */
      
      MReadOutCollection C(Sequence.GetReadOut(r).GetReadOutElement());
      vector<MReadOutCollection>::iterator I = lower_bound(m_Collection.begin(), m_Collection.end(), C);
      // We have a lower bound, i.e. the first which is not "<", so we might not yet have found it and we have to check a final time:
      if (I == m_Collection.end() || (*I).Add(Sequence.GetReadOut(r), Group) == false) {
        C.AddReadOutDataGroups(m_ReadOutDataGroups);
        C.Add(Sequence, Group);
        m_Collection.insert(I, C);
        //cout<<ToString()<<endl;
      }
    }
    return true;
  }
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Check if the given read-out element is in the collection
unsigned int MReadOutStore::FindReadOutCollection(const MReadOutElement& ROE) const
{
  // Loop over all collection and try to add the sequence data
  for (unsigned int s = 0; s < m_Collection.size(); ++s) {
    if (m_Collection[s].HasIdenticalReadOutElement(ROE) == true) {
      return s;
    }
  }
  
  return g_UnsignedIntNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


//! Get a read-out collection
MReadOutCollection& MReadOutStore::GetReadOutCollection(unsigned int s)
{
  if (s < m_Collection.size()) return m_Collection[s]; 
  
  throw MExceptionIndexOutOfBounds(0, m_Collection.size(), s);
  
  // We still need a return value...
  return m_Collection[0];
}

/*
////////////////////////////////////////////////////////////////////////////////

bool IsNotPositive(const MReadOutCollection& R) { return R.IsPositive() ? false : true; }

//! Remove all collection which don't have positive data entries
void MReadOutStore::RemoveCollectionsWithNoPositiveEntries()
{
  // The remove-erase idiom speeds this up:
  m_Collection.erase(remove_if(m_Collection.begin(), m_Collection.end(), IsNotPositive), m_Collection.end());
  
  //for (vector<MReadOutCollection>::iterator I = m_Collection.begin(); I != m_Collection.end(); ) {
  //  if ((*I).IsPositive() == false) {
  //    cout<<"Removing: "<<(*I).ToString()<<endl;
  //    I = m_Collection.erase(I);
  //  } else {
  //    ++I;
  //  }
  //}
  
}
*/


////////////////////////////////////////////////////////////////////////////////


//! Convert to a string
MString MReadOutStore::ToString() const
{
  ostringstream os;
  os<<"ROS - number of collections = "<<m_Collection.size()<<endl;
  for (unsigned int c = 0; c < m_Collection.size(); ++c) {
    os<<m_Collection[c]<<endl;  
  }
  return os.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Append the context as text 
ostream& operator<<(ostream& os, const MReadOutStore& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutStore.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
