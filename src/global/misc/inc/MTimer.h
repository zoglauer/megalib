/*
 * MTimer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTimer__
#define __MTimer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <chrono>
using namespace std;
using namespace std::chrono;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A timer class
class MTimer
{
  // public interface:
 public:
  // Default constructor
  MTimer(bool Start = true);
  //! Standard constructor with time
  MTimer(double TimeOutSeconds);
  //! Copy constructor
  MTimer(const MTimer& Timer);
  //! Default destructor
  virtual ~MTimer();

  //! Copy operator
  MTimer& operator=(const MTimer& Timer);
  
  //! Pauses the timer and sets all stored values to zero
  void Clear();
  
  //! Start the timer - counting restarts from zero
  void Start();
  //! Reset the timer and start it again
  void Reset();

  //! Pause the timer
  void Pause();
  //! Continue the timer
  void Continue();

  //! Get the elapsed time in seconds
  double GetElapsed();
  //! Get the elapsed time in seconds
  double ElapsedTime() { return GetElapsed(); }
  
  //! Set a time out time
  void SetTimeOut(double TimeOutSeconds = -1);
  //! Get the timeout
  double GetTimeOut();
  //! Check if the timer has timed out -- if we do not have a timeout we will return always false
  bool HasTimedOut(double Seconds = -1);

  // protected methods:
 protected:
  //! Get the time now 
  time_point<steady_clock> Now();  
  //! Get the elapsed time 
  duration<double> GetElapsedTime();

  // protected members:
 protected:


  // private members:
 private:
  //! Time since last start or reset
  time_point<steady_clock> m_StartTime;
  //! Total elapsed time since last start or reset
  duration<double> m_ElapsedTime;
  //! Time which needs to be elapsed until a timeout will be triggered
  duration<double> m_TimeOut;

  //! Flag indicating that this timer is paused
  bool m_IsPaused;

  //! Flag indicating a time out, even when the real time has not yet elapsed 
  bool m_HasTimedOut;


#ifdef ___CINT___
 public:
  ClassDef(MTimer, 0) // no description
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
