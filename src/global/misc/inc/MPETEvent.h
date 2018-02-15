/*
 * MPETEvent.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MPETEvent__
#define __MPETEvent__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MPhysicalEvent.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A event measured by a typical Positron Emission tomography instrument
//! consisting of two positions and measured energies which in the ideal case are 
//! both 511 keV 
class MPETEvent : public MPhysicalEvent
{
  // Public Interface:
 public:
  //! Default constructor
  MPETEvent();
  //! Default destructor
  virtual ~MPETEvent();

  //! Copy the data of the given event into this one
  bool Assimilate(MPETEvent* PETEvent);
  //! Copy the data of the given event into this one
  bool Assimilate(MPhysicalEvent* Event);
  //! Assimilate the basic event data
  bool Assimilate(MVector Position1, double Energy1, MVector Position2, double Energy2);

  
  //! Stream the data from a file
  virtual bool Stream(MFile& File, int Version, bool Read, bool Fast = false, bool ReadDelayed = false);
  //! Parse a single line from the file
  virtual int ParseLine(const char* Line, bool Fast = false);
  
  //! Create a copy of this event
  virtual MPhysicalEvent* Duplicate();

  //! Validate the event and calculate all high level data...
  virtual bool Validate();
  
  //! Set the detected energy of the first event
  void SetEnergy1(const double Energy1) { m_Energy1 = Energy1; }
  //! Get the detected energy of the first event
  double GetEnergy1() const { return m_Energy1; }
  
  //! Set the first detected position of the first event
  void SetPosition1(const MVector& Position1) { m_Position1 = Position1; }
  //! Get the first detected position of the first event
  MVector GetPosition1() const { return m_Position1; }
  
  //! Set the detected energy of the second event
  void SetEnergy2(const double Energy2) { m_Energy2 = Energy2; }
  //! Get the detected energy of the second event
  double GetEnergy2() const { return m_Energy2; }
  
  //! Set the first detected position of the second event
  void SetPosition2(const MVector& Position2) { m_Position2 = Position2; }
  //! Get the first detected position of the second event
  MVector GetPosition2() const { return m_Position2; }
  
  
  //! Return the total energy
  virtual double GetEnergy() const { return m_Energy1 + m_Energy2; }
  
  //! Should be "cast", but... that's bad design...
  MPhysicalEvent* Data(); 

  //! Reset the event data to default values
  virtual void Reset();
  //! Create a string with the data of the event
  virtual MString ToString() const;


  // protected methods:
 protected:


  // private methods:
 private:
 


  // protected members:
 protected:
  //! Detected energy of the first event
  double m_Energy1;
  //! First detected position of the first event
  MVector m_Position1;
  //! Detected energy of the second event
  double m_Energy2;
  //! First detected position of the second event
  MVector m_Position2;
   
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MPETEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
