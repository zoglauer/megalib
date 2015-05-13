/*
 * MModuleReadOutAssemblyQueues.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MModuleReadOutAssemblyQueues__
#define __MModuleReadOutAssemblyQueues__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
#include <deque>
#include <mutex>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MReadOutAssembly.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////




//! This class manages the two read-out assembly queues of a module
//! It ensures that the out-sequence is the same as the in sequence
class MModuleReadOutAssemblyQueues
{
  // public interface:
 public:
  //! Default constructor
  MModuleReadOutAssemblyQueues();
  //! Default destructor
  ~MModuleReadOutAssemblyQueues();

  //! Clear the queue
  void Clear();
  
  //! Enable sorting
  void EnableSorting(bool Sorted = true);
  
  //! Add an read-out assembly to the incoming read-out assembly list
  bool AddIncoming(MReadOutAssembly* Event);
  //! Check if there are read-out assemblies in the incoming read-out assembly list
  bool HasIncoming();
  //! Get an input read-out assembly (FIFO) - returns nullptr if there is none
  MReadOutAssembly* GetIncoming();
  
  
  //! Add on output read-out assembly --- they do not need to be sorted!
  bool AddOutgoing(MReadOutAssembly* Event);
  //! Check if there are read-out assemblies in the outgoing read-out assembly list 
  bool HasOutgoing();
  //! Get an outgoing read-out assembly --- ordering is the same as in the incoming list - returns nullptr if there is none
  MReadOutAssembly* GetOutgoing();
  
  
  // protected methods:
 protected:

  

  // private methods:
 private:
  //! No Copy constructor
  MModuleReadOutAssemblyQueues(const MModuleReadOutAssemblyQueues&) = delete;
  //! No copying whatsoever
  MModuleReadOutAssemblyQueues& operator=(const MModuleReadOutAssemblyQueues&) = delete;



  // protected members:
 protected:
  //! True if we have a sorted queue
  bool m_SortedQueue;
   
  //! The incoming event list
  deque<MReadOutAssembly*> m_IncomingEvents;
  //! A mutex protecting the incoming event list
  mutex m_IncomingEventsMutex;
  
  //! The sorting order for the outgoing events -- by event ID!
  deque<unsigned long> m_SortingOrder;  
  //! The outgoing event list
  deque<MReadOutAssembly*> m_OutgoingEvents; 
  //! A mutex protecting the outgoing event list
  mutex m_OutgoingEventsMutex;
    
  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MModuleReadOutAssemblyQueues, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
