/*
 * MReadOutCollection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutCollection__
#define __MReadOutCollection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOut.h"
#include "MReadOutElement.h"
#include "MReadOutData.h"
#include "MReadOutSequence.h"
#include "MReadOutDataGroup.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A collection of read-out groups belonging all to the same read-out element (i.e. channel)
class MReadOutCollection
{
  // public interface:
 public:
  //! Default constructor
  MReadOutCollection();
  //! Default constructor with a reference to a read-out element
  MReadOutCollection(const MReadOutElement& ROE);
  //! The copy constructor 
  MReadOutCollection(const MReadOutCollection& ReadOutCollection);
  //! Default destructor
  virtual ~MReadOutCollection();

  //! Assignment operator
  MReadOutCollection& operator=(const MReadOutCollection& RO);

  //! Set the read-out element
  void SetReadOutElement(const MReadOutElement& ROE);
  //! Check if the read-out elements are identical
  bool HasIdenticalReadOutElement(const MReadOutElement& ROE) const;
  //! Get the read-out element --- constant --- should not be modified
  const MReadOutElement& GetReadOutElement() const { return *m_ROE; }

  //! Add a read-out group and return its ID
  void AddReadOutDataGroup(const MString& Name);
  //! Add a read-out group and return its ID
  void AddReadOutDataGroups(vector<MString> Names);

  //! Add this read-out data to the given read-out group
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  bool Add(const MReadOutData& ROD, unsigned int ROG);
  //! If the read out has the correct ROE and ROG exists add it and return true
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  bool Add(const MReadOut& RO, unsigned int ROG);
  //! Add all read outs of this read-out sequence which have the correct ROE
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  void Add(const MReadOutSequence& RO, unsigned int ROG);
  

  //! Return the number of available read-out data groups 
  unsigned int GetNumberOfReadOutDataGroups() const { return m_ROGs.size(); }
  
  //! Get a read-out data group by its index
  //! Use GetNumberOfReadOutDataGroup first to check if it exists, because
  //! if it doesn't exist an exception MExceptionIndexOutOfBounds is returned
  MReadOutDataGroup& GetReadOutDataGroup(unsigned int i);
      
  
  //! Dump a string
  virtual MString ToString() const;
  
  // protected methods:
 protected:

   
  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The common read-out element
  MReadOutElement* m_ROE;
  //! A list of all read-out data groups
  vector<MReadOutDataGroup> m_ROGs;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MReadOutCollection, 0) // no description
#endif

};

//! Streamify the read-out collection
ostream& operator<<(ostream& os, const MReadOutCollection& R);

#endif


////////////////////////////////////////////////////////////////////////////////
