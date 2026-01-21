/*
 * MModuleReadOutAssemblyQueues.cxx
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
// MModuleReadOutAssemblyQueues
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MModuleReadOutAssemblyQueues.h"

// Standard libs:
#include <algorithm>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MModuleReadOutAssemblyQueues)
#endif


////////////////////////////////////////////////////////////////////////////////


MModuleReadOutAssemblyQueues::MModuleReadOutAssemblyQueues()
{
  // Construct an instance of MModuleReadOutAssemblyQueues
  
  m_SortedQueue = false;
}


////////////////////////////////////////////////////////////////////////////////


MModuleReadOutAssemblyQueues::~MModuleReadOutAssemblyQueues()
{
  // Delete this instance of MModuleReadOutAssemblyQueues
  
  Clear();
}


////////////////////////////////////////////////////////////////////////////////


void MModuleReadOutAssemblyQueues::Clear()
{
  // Clear the queues

  lock_guard<mutex> IncomingLock(m_IncomingEventsMutex);
  lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);

  for (MReadOutAssembly* ROA: m_IncomingEvents) delete ROA;
  m_IncomingEvents.clear();
  
  for (MReadOutAssembly* ROA: m_OutgoingEvents) delete ROA;
  m_OutgoingEvents.clear();

  m_SortedQueue = false;
}


////////////////////////////////////////////////////////////////////////////////


void MModuleReadOutAssemblyQueues::EnableSorting(bool Sorted) 
{ 
  //! Enable sorting

  lock_guard<mutex> IncomingLock(m_IncomingEventsMutex);
  lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);

  if (m_SortedQueue != Sorted) {
    if (m_IncomingEvents.begin() != m_IncomingEvents.end() ||
        m_OutgoingEvents.begin() != m_OutgoingEvents.end()) {
      if (g_Verbosity >= c_Error) {
        cout<<"Error in MModuleReadOutAssemblyQueues::EnableSorting:"<<endl;
        cout<<"You cannot change the sorting mode while events are in the queue!"<<endl;
      }
    } else {
      m_SortedQueue = Sorted;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleReadOutAssemblyQueues::AddIncoming(MReadOutAssembly* Event)
{
  //! Add an event to the incoming event list

  lock_guard<mutex> IncomingLock(m_IncomingEventsMutex);
  
  m_IncomingEvents.push_back(Event);
  
  return true;
}
  

////////////////////////////////////////////////////////////////////////////////


bool MModuleReadOutAssemblyQueues::HasIncoming()
{
  //! Check if there are events in the incoming event list

  lock_guard<mutex> IncomingLock(m_IncomingEventsMutex);

  return (m_IncomingEvents.begin() != m_IncomingEvents.end()); // faster for deque than size if filled!
}
  

////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* MModuleReadOutAssemblyQueues::GetIncoming()
{
  //! Get one incoming event

  lock_guard<mutex> IncomingLock(m_IncomingEventsMutex);

  MReadOutAssembly* E = nullptr;
  
  if (m_IncomingEvents.begin() != m_IncomingEvents.end()) {
    E = m_IncomingEvents.front();
    m_IncomingEvents.pop_front();
  
    if (m_SortedQueue == true) {
      lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);
  
      m_OutgoingEvents.push_back(nullptr);
      m_SortingOrder.push_back(E->GetID());
    }
  }
  
  return E;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleReadOutAssemblyQueues::AddOutgoing(MReadOutAssembly* Event)
{
  //! Add an event to the incoming event list

  lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);
  
  if (m_SortedQueue == true) {
    // Find the ID in the sorted list...
    unsigned long ID = Event->GetID();
    deque<unsigned long>::reverse_iterator Iter = find(m_SortingOrder.rbegin(), m_SortingOrder.rend(), ID);
    if (Iter == m_SortingOrder.rend()) {
      if (g_Verbosity >= c_Error) {
        cout<<"Serious error: The event ID "<<ID<<" was not previously part of the deque!"<<endl;
      }
      return false;
    }
    // ... and add the event at the right place
    deque<MReadOutAssembly*>::reverse_iterator ROAIter = m_OutgoingEvents.rbegin() + (Iter - m_SortingOrder.rbegin());
    *ROAIter = Event; 
  } else {
    // and add the event at the right place
    m_OutgoingEvents.push_back(Event);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MModuleReadOutAssemblyQueues::HasOutgoing()
{
  //! Check if there are events in the outgoing event list

  lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);

  if (m_OutgoingEvents.begin() != m_OutgoingEvents.end() &&
      m_OutgoingEvents.front() != 0) {
    return true;
  }

  return false;
}
  

////////////////////////////////////////////////////////////////////////////////


MReadOutAssembly* MModuleReadOutAssemblyQueues::GetOutgoing()
{
  //! Check if there are events in the outgoing event list

  lock_guard<mutex> OutgoingLock(m_OutgoingEventsMutex);

  MReadOutAssembly* E = nullptr;
  
  if (m_OutgoingEvents.begin() != m_OutgoingEvents.end()) {
    E = m_OutgoingEvents.front();
    if (E != nullptr) {
      m_OutgoingEvents.pop_front();
      if (m_SortedQueue == true) {
        m_SortingOrder.pop_front(); 
      }
    }
  }
  
  return E;
}


// MModuleReadOutAssemblyQueues.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
