/*
 * MCalibrationCollection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCalibrationCollection__
#define __MCalibrationCollection__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:
#include "TH1.h"
#include "TF1.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MCalibration.h"
#include "MCalibrationElement.h"
#include "MCalibrationData.h"
#include "MCalibrationSequence.h"
#include "MCalibrationDataGroup.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A collection of read-out groups belonging all to the same read-out element (i.e. channel)
class MCalibrationCollection
{
  // public interface:
 public:
  //! Default constructor
  MCalibrationCollection();
  //! Default constructor with a reference to a read-out element
  MCalibrationCollection(const MReadOutElement& ROE);
  //! The copy constructor 
  MCalibrationCollection(const MCalibrationCollection& CalibrationCollection);
  //! Default destructor
  virtual ~MCalibrationCollection();

  //! Assignment operator
  MCalibrationCollection& operator=(const MCalibrationCollection& RO);

  //! Set the read-out element
  void SetReadOutElement(const MReadOutElement& ROE);
  //! Check if the read-out elements are identical
  bool HasIdenticalReadOutElement(const MReadOutElement& ROE) const;
  //! Get the read-out element --- constant --- should not be modified
  const MReadOutElement& GetReadOutElement() const { return *m_ROE; }

  //! Add a read-out group and return its ID
  void AddCalibrationDataGroup(const MString& Name);
  //! Add a read-out group and return its ID
  void AddCalibrationDataGroups(vector<MString> Names);

  //! Add this read-out data to the given read-out group
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  bool Add(const MCalibrationData& ROD, unsigned int ROG);
  //! If the read out has the correct ROE and ROG exists add it and return true
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  bool Add(const MCalibration& RO, unsigned int ROG);
  //! Add all read outs of this read-out sequence which have the correct ROE
  //! If ROG does not exist return an MExceptionIndexOutOfBounds exception
  void Add(const MCalibrationSequence& RO, unsigned int ROG);
  

  //! Return the number of available read-out data groups 
  unsigned int GetNumberOfCalibrationDataGroups() const { return m_ROGs.size(); }
  
  //! Get a read-out data group by its index
  //! Use GetNumberOfCalibrationDataGroup first to check if it exists, because
  //! if it doesn't exist an exception MExceptionIndexOutOfBounds is returned
  MCalibrationDataGroup& GetCalibrationDataGroup(unsigned int i);
      
  
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
  vector<MCalibrationSpectrum> m_CSs;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MCalibrationCollection, 0) // no description
#endif

};

//! Streamify the read-out collection
ostream& operator<<(ostream& os, const MCalibrationCollection& R);

#endif


////////////////////////////////////////////////////////////////////////////////
