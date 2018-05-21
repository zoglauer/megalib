/*
 * MResponseImagingARM.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseImagingARM__
#define __MResponseImagingARM__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseBuilder.h"
#include "MResponseMatrixO5.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingARM : public MResponseBuilder
{
  // public interface:
 public:
  //! Default constructor
  MResponseImagingARM();
  //! Default destructor
  virtual ~MResponseImagingARM();

  //! Initialize the response matrices and their generation
  virtual bool Initialize();

  //! Analyze th events (all if in file mode, one if in event-by-event mode)
  virtual bool Analyze();
    
  //! Finalize the response generation (i.e. save the data a final time )
  virtual bool Finalize();

  // protected methods:
 protected:

  //! Save the response matrices
  virtual bool Save();

  // private methods:
 private:
   //! The ARM shape as a function of a few things
   MResponseMatrixO5 m_Arm;
   //! The ARM shape of the photo-peak events as a function of a few things
   MResponseMatrixO5 m_ArmPhotoPeak;
   
   //! The SPD shape as a function of a few things
   MResponseMatrixO3 m_Spd;
   //! The SPD shape as a function of a few things
   MResponseMatrixO3 m_SpdPhotoPeak;
   
   
  //! The number of events which can be analyzed
  unsigned long m_NMatchedEvents;
  //! The number of reconstructed events
  unsigned long m_NOptimumEvents;
  //! The number of Compton events after selections
  unsigned long m_NQualifiedComptonEvents;
  //! The number of Compton events in photo peak after selections
  unsigned long m_NPhotoPeakEvents;
  //! The number of Compton events after selections with track
  unsigned long m_NQualifiedComptonEventsWithTrack = 0;
  //! The number of Compton events in photo peak after selections with track
  unsigned long m_NPhotoPeakEventsWithTrack = 0;
  
  
  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseImagingARM, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
