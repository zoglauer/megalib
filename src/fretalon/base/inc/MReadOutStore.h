/*
 * MReadOutStore.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutStore__
#define __MReadOutStore__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MIsotope.h"
#include "MReadOutElement.h"
#include "MReadOutCollection.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Store a sequence of read outs in read-out collections (read outs stored by their read out element, i.e. channel)
//! Provide methods to retrieve the data for calibration and to store it
class MReadOutStore
{
  // public interface:
 public:
  //! Default constructor for the storage of all read outs
  MReadOutStore();
  //! Default destructor
  virtual ~MReadOutStore();

  //! Remove all collected data
  void Clear();
  
  //! Add a group to all collections and return its ID
  unsigned int AddReadOutDataGroup(const MString& Name);
  
  //! Move the content of the read-out data store to this ones GroupID
  //! Group ID must exist, and the store must only have one own group at position 0
  bool Move(MReadOutStore& Store, unsigned int GroupID);
  
  //! Add a group to all collections and return its ID
  unsigned int GetNumberOfReadOutDataGroups() const { return m_ReadOutDataGroups.size(); }
  
  //! Add the data of a read-out sequence to the store and the given group
  //! If the data can not be added return false
  bool Add(const MReadOutSequence& Sequence, unsigned int Group);

  //! Return the number of collections in store
  unsigned int GetNumberOfReadOutCollections() const { return m_Collection.size(); }

  //! Check if the given read-out element is in the collection
  //! If yes return its position, if not return g_UnsignedIntNotDefined;
  unsigned int FindReadOutCollection(const MReadOutElement& ROE) const;

  //! Get a read-out collection by its read-out element
  //! Use HasReadOutCollection first to check if it exists, because
  //! if it doesn't exist an empty one is returned, which might cause a crash if the collection is empty.
  MReadOutCollection& GetReadOutCollection(unsigned int i);
  
  //! Dump a string
  virtual MString ToString() const;
  
  
  // protected methods:
 protected:
  //MReadOutStore() {};
  //MReadOutStore(const MReadOutStore& ReadOutStore) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The actual read-out collections
  vector<MReadOutCollection> m_Collection;

  //! The read-out group names
  vector<MString> m_ReadOutDataGroups;
  
#ifdef ___CLING___
 public:
  ClassDef(MReadOutStore, 0) // no description
#endif

};

//! Streamify the read-out store
ostream& operator<<(ostream& os, const MReadOutStore& R);

#endif


////////////////////////////////////////////////////////////////////////////////
