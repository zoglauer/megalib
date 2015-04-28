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
#include <mutex>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A thread-safe timer
class MTimer
{
  // public interface:
 public:
  // Default constructor
  MTimer(bool Start = true);
  //! Standard constructor with time
  MTimer(double TimeOutSeconds);
  //! Copy constructor
  //! Side remark: Since we have to lock Timer's mutex, we cannot have a const here
  MTimer(const MTimer& Timer);
  //! Default destructor
  virtual ~MTimer();

  //! Copy operator
  //! Side remark: Since we have to lock Timer's mutex, we cannot have a const here
  MTimer& operator=(const MTimer& Timer);
  
  //! Pauses the time and sets all stored values to zero
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
  // Get the relative time in seconds
  double GetRelativeTime();
  //! Get the elapsed time in seconds -- not thread safe version!
  double GetElapsedTime();

  // protected members:
 protected:


  // private members:
 private:
  //! Time since last start or reset
  double m_StartTime;
  //! Total elapsed time since last start or reset
  double m_ElapsedTime;
  //! Time which needs to be elapsed until a timeout will be triggered
  double m_TimeOut;

  //! Flag indicating that this timer is paused
  bool m_IsPaused;

  //! Flag indicating a time out, even when the real time has not yet elapsed 
  bool m_HasTimedOut;

  //! A mutex ensuring that this class is completely reentrant
  //! Needs to be mutable for copy constructor and copy operator
  mutable mutex m_Guard;


#ifdef ___CINT___
 public:
  ClassDef(MTimer, 0) // no description
#endif
};

#endif


////////////////////////////////////////////////////////////////////////////////
