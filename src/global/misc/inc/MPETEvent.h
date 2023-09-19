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
#include "MCoordinateSystem.h"

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
  
  //! Stream the content into a tra-file compatible string
  virtual MString ToTraString() const;
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
  
  //! Set the detected energy resolution of the first event
  void SetEnergyResolution1(const double EnergyResolution1) { m_EnergyResolution1 = EnergyResolution1; }
  //! Get the detected energy resolution of the first event
  double GetEnergyResolution1() const { return m_EnergyResolution1; }
  
  //! Set the first detected position of the first event
  void SetPosition1(const MVector& Position1) { m_Position1 = Position1; }
  //! Get the first detected position of the first event
  MVector GetPosition1() const { return m_Position1; }
  
  //! Set the first detected position resolution of the first event
  void SetPositionResolution1(const MVector& PositionResolution1) { m_PositionResolution1 = PositionResolution1; }
  //! Get the first detected position resolution of the first event
  MVector GetPositionResolution1() const { return m_PositionResolution1; }
  
  //! Set the detection timing of the first event
  void SetTiming1(const double& Timing1) { m_Timing1 = Timing1; }
  //! Get the detection timing the first event
  double GetTiming1() const { return m_Timing1; }
  
  //! Set the detection timing resolution of the first event
  void SetTimingResolution1(const double& TimingResolution1) { m_TimingResolution1 = TimingResolution1; }
  //! Get the detection timing resolution the first event
  double GetTimingResolution1() const { return m_TimingResolution1; }
  
  //! Set the detected energy of the second event
  void SetEnergy2(const double Energy2) { m_Energy2 = Energy2; }
  //! Get the detected energy of the second event
  double GetEnergy2() const { return m_Energy2; }
  
  //! Set the detected energy resolution of the second event
  void SetEnergyResolution2(const double EnergyResolution2) { m_EnergyResolution2 = EnergyResolution2; }
  //! Get the detected energy resolution of the second event
  double GetEnergyResolution2() const { return m_EnergyResolution2; }
  
  //! Set the first detected position of the second event
  void SetPosition2(const MVector& Position2) { m_Position2 = Position2; }
  //! Get the first detected position of the second event
  MVector GetPosition2() const { return m_Position2; }
  
  //! Set the first detected position resolution of the second event
  void SetPositionResolution2(const MVector& PositionResolution2) { m_PositionResolution2 = PositionResolution2; }
  //! Get the first detected resolution position of the second event
  MVector GetPositionResolution2() const { return m_PositionResolution2; }
  
  //! Set the detection timing of the second event
  void SetTiming2(const double& Timing2) { m_Timing2 = Timing2; }
  //! Get the detection timing the second event
  double GetTiming2() const { return m_Timing2; }
  
  //! Set the detection timing resolution of the second event
  void SetTimingResolution2(const double& TimingResolution2) { m_TimingResolution2 = TimingResolution2; }
  //! Get the detection timing resolution the second event
  double GetTimingResolution2() const { return m_TimingResolution2; }
  
  //! Return the total energy
  virtual double GetEnergy() const { return m_Energy1 + m_Energy2; }
  
  //! Should be "cast", but... that's bad design...
  MPhysicalEvent* Data(); 
  
  //! Return the Angular Resolution Measure value for the gamma line, i.e. the clostest distance between the line and the origin in cm not degrees as for Compton and pair
  double GetResolutionMeasure(const MVector& Position, const MCoordinateSystem& CS = MCoordinateSystem::c_Cartesian2D);
  
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
  //! Detected energy resolution of the first event
  double m_EnergyResolution1;
  //! First detected position of the first event
  MVector m_Position1;
  //! First detected position resolution of the first event
  MVector m_PositionResolution1;
  //! Detection timing of the first event
  double m_Timing1;
  //! Detection timing resolution of the first event
  double m_TimingResolution1;
  
  //! Detected energy of the second event
  double m_Energy2;
  //! Detected energy resolution of the second event
  double m_EnergyResolution2;
  //! First detected position of the second event
  MVector m_Position2;
  //! First detected position resolution of the second event
  MVector m_PositionResolution2;
  //! Detection timing of the second event
  double m_Timing2;
  //! Detection timing resolution of the second event
  double m_TimingResolution2;
  
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MPETEvent, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
