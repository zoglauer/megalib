/*
 * MTimer.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MTimer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTimer.h"

// Standard libs:
#include <iostream>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MSystem.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MTimer)
#endif


////////////////////////////////////////////////////////////////////////////////


MTimer::MTimer(bool Start)
{
  // Default constructor

  m_StartTime = Now();
  m_ElapsedTime = duration<double>(0);

  m_HasTimedOut = false;
  m_TimeOut = duration<double>(0);

  m_IsPaused = !Start;
}


////////////////////////////////////////////////////////////////////////////////


MTimer::MTimer(double TimeOut)
{
  // Standard constrcutor

  m_StartTime = Now();
  m_ElapsedTime = duration<double>(0);

  m_HasTimedOut = false;
  m_TimeOut = duration<double>(TimeOut);

  m_IsPaused = false;
}


////////////////////////////////////////////////////////////////////////////////


MTimer::MTimer(const MTimer& Timer)
{
  // Copy constructor
 
  m_StartTime = Timer.m_StartTime;
  m_HasTimedOut = Timer.m_HasTimedOut;
  m_TimeOut = Timer.m_TimeOut;
  m_ElapsedTime = Timer.m_ElapsedTime;
  m_IsPaused = Timer.m_IsPaused;
}


////////////////////////////////////////////////////////////////////////////////


MTimer::~MTimer()
{
  // Default destructor
}


////////////////////////////////////////////////////////////////////////////////


MTimer& MTimer::operator=(const MTimer& Timer)
{
  // Copy constructor
 
  if (this != &Timer) {
    m_StartTime = Timer.m_StartTime;
    m_HasTimedOut = Timer.m_HasTimedOut;
    m_TimeOut = Timer.m_TimeOut;
    m_ElapsedTime = Timer.m_ElapsedTime;
    m_IsPaused = Timer.m_IsPaused;
  }
  
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::Clear()
{
  // (Re-) Start the timer
  
  m_IsPaused = true;

  m_StartTime = time_point<steady_clock>(duration<long int>(0));
  m_HasTimedOut = false;
  m_TimeOut = duration<double>(0);
  m_ElapsedTime = duration<double>(0);
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::Start()
{
  // (Re-) Start the timer
  
  m_StartTime = Now();
  m_HasTimedOut = false;
  m_ElapsedTime = duration<double>(0);
  m_IsPaused = false;
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::Reset()
{
  // (Re-) Start the timer --- same as start

  Start();
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::Pause()
{
  // (Re-) Start the timer
  
  if (m_IsPaused == true) return;
  
  m_ElapsedTime = GetElapsedTime();
  m_IsPaused = true;
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::Continue()
{
  // Start the timer after it has been paused - does not reset the time

  if (m_IsPaused == false) return;
  
  m_StartTime = Now();
  m_IsPaused = false;
}


////////////////////////////////////////////////////////////////////////////////


time_point<steady_clock> MTimer::Now()
{
  // Return the time now
  
  return steady_clock::now();
}


////////////////////////////////////////////////////////////////////////////////


void MTimer::SetTimeOut(double TimeOut) 
{ 
  // Set the timeout
  // If timeout <= 0 then we never timeout
  
  m_TimeOut = duration<double>(TimeOut);
}
 

////////////////////////////////////////////////////////////////////////////////


double MTimer::GetTimeOut()
{
  // Return the timeout
  // If timeout <= 0 then never timeout

  return m_TimeOut.count();
}


////////////////////////////////////////////////////////////////////////////////


double MTimer::GetElapsed()
{
  // Convert the time to seconds and return as double

  return GetElapsedTime().count();
}


////////////////////////////////////////////////////////////////////////////////


duration<double> MTimer::GetElapsedTime()
{
  // Convert the time to seconds and return as double

  if (m_IsPaused == true) {
    return m_ElapsedTime;
  } else {
    return Now() - m_StartTime + m_ElapsedTime;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MTimer::HasTimedOut(double Seconds)
{
  // Convert the time to seconds and return as double

  if (m_HasTimedOut == true) return true;

  if (m_TimeOut <= duration<double>(0)) return false;

  if (Seconds == -1) Seconds = m_TimeOut.count();

  if ((Now() - m_StartTime).count() > Seconds) return true;

  return false;
}


// MTimer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
