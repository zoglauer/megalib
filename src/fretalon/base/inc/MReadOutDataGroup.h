/*
 * MReadOutDataGroup.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MReadOutDataGroup__
#define __MReadOutDataGroup__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MReadOutData.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A group of read-out datas 
class MReadOutDataGroup
{
  // public interface:
 public:
  //! Default constructor
  MReadOutDataGroup();
  //! Default constructor with name
  MReadOutDataGroup(const MString& Name);
  //! The copy constructor 
  MReadOutDataGroup(const MReadOutDataGroup& ReadOutDataGroup);
  //! Default destructor
  virtual ~MReadOutDataGroup();

  //! Assignment operator
  MReadOutDataGroup& operator=(const MReadOutDataGroup& RO);

  //! Return the name of the group
  MString GetName() const { return m_Name; }
  
  //! Add this read-out data 
  void Add(const MReadOutData& ROD);

  //! Move the content from the given into this read-out data group
  void Move(MReadOutDataGroup& RODG);
    
  //! Return the number of available read-out data's
  unsigned int GetNumberOfReadOutDatas() const { return m_RODs.size(); }
  
  //! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
  MReadOutData& GetReadOutData(unsigned int i);
  //! Return a specific read-out data, in case it doesn't exist throw the exception MExceptionIndexOutOfBounds
  const MReadOutData& GetReadOutData(unsigned int i) const;
  
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
  //! Name of this group
  MString m_Name;
   
  //! A list of all data
  vector<MReadOutData*> m_RODs;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MReadOutDataGroup, 0) // no description
#endif

};

//! Streamify the read-out data group
ostream& operator<<(ostream& os, const MReadOutDataGroup& R);

#endif


////////////////////////////////////////////////////////////////////////////////
