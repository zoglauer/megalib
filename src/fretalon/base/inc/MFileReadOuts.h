/*
 * MFileReadOuts.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileReadOuts__
#define __MFileReadOuts__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MTime.h"
#include "MReadOutSequence.h"
#include "MReadOutData.h"
#include "MReadOutElement.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! Read the measured data from a file
class MFileReadOuts : public MFile
{
  // public interface:
 public:
  //! Default constructor reading uncalibrated detector data
  MFileReadOuts();
  //! Default destructor
  virtual ~MFileReadOuts();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);

  //! Return the next event
  //! If SelectedDetectorID >= 0 then restrict yourself to SelectedDetectorID
  bool ReadNext(MReadOutSequence& Sequence, int SelectedDetectorID = -1);

  // protected methods:
 protected:
  //MFileReadOuts() {};
  //MFileReadOuts(const MFileReadOuts& FileReadOuts) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! Name of the detector which we are reading
  MString m_Detector;
  //! The number of events in the file
  unsigned int m_NEventsInFile;
  //! The number of good events in file
  unsigned int m_NGoodEventsInFile;
  //! Start of the observation time
  MTime m_StartObservationTime;
  //! Clock time belonging to the start of the observation time
  unsigned long m_StartClock; 
  //! End of the observation time
  MTime m_EndObservationTime;
  //! Clock time belonging to the end of the observation time
  unsigned long m_EndClock;

  //! The read-out element
  MReadOutElement* m_ROE;
  //! The read-out data
  MReadOutData* m_ROD;
  
  

#ifdef ___CINT___
 public:
  ClassDef(MFileReadOuts, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
