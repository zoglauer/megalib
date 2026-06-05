/*
 * MTime.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTime__
#define __MTime__


////////////////////////////////////////////////////////////////////////////////

// Standard libs:
#include <iostream>

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! A time class with nanosecond precision relative to the Unix epoch
class MTime
{
  // public interface:
 public:
  //! Default constructor, set the time to NOW
  MTime();
  //! Extract the time from a formatted string
  //! Deprecated: this parser has limited error handling
  explicit MTime(MString String, int Format);
  //! Set the time as two long integers -- the time is counted since Epoch
  MTime(long int LinuxTime, long int NanoSeconds = 0);
  //! Set the time as two integers -- the time is counted since Epoch
  MTime(int LinuxTime, int NanoSeconds = 0);
  //! Set the time as two unsigned integers -- the time is counted since Epoch
  MTime(unsigned int LinuxTime, unsigned int NanoSeconds = 0);
  //! Set the time elements (years, days, etc) individually
  MTime(unsigned int Year, unsigned int Month, unsigned int Day, unsigned int Hour = 0,
        unsigned int Minute = 0, unsigned int Second = 0, unsigned int NanoSecond = 0);
  //! Set the time as a double -- the time is counted since Epoch
  MTime(double Time);
  //! Copy constructor
  MTime(const MTime& Time);
  
  //! Default destructor
  virtual ~MTime();


  // TODO: Some of these function return false by default! Before next alpha release switch it!
  
  //! Set the time to the current system time
  bool Now();
  //! Set the time from calendar fields
  bool Set(unsigned int Year, unsigned int Month, unsigned int Day, unsigned int Hour = 0,
           unsigned int Minute = 0, unsigned int Second = 0, unsigned int NanoSecond = 0);
  //! Set the time from seconds and nanoseconds since the epoch
  bool Set(const long int LinuxTime, const long int NanoSeconds = 0);
  //! Set the time from seconds and nanoseconds since the epoch
  bool Set(const int LinuxTime, const int NanoSeconds = 0);
  //! Set the time from seconds and nanoseconds since the epoch
  bool Set(const unsigned int LinuxTime, const unsigned int NanoSeconds = 0);
  //! Set the time from separate seconds and nanoseconds given as doubles
  bool Set(const double Seconds, const double NanoSeconds);
  //! Set the time from seconds since the epoch
  bool Set(double Time);
  //! Set the time from another MTime instance
  bool Set(const MTime& Time);
  //! Set a string in MEGAlib TI format: TI 123456789.123456789
  //! Starts the conversion at character I
  bool Set(const MString& String, unsigned int I = 0);
  //! Set a string in MEGAlib TI format: TI 123456789.123456789
  //! Fast version with minimal error checks and rather unsafe...
  bool Set(const char* Line);

  // Access the fields:
  
  //! Get the internal seconds
  long int GetInternalSeconds() const { return m_Seconds; }
  //! Get the internal nanoseconds
  long int GetInternalNanoSeconds() const { return m_NanoSeconds; }
  
  //! Get the nanoseconds in the time
  unsigned int GetNanoSeconds();
  //! Get the seconds in the time
  unsigned int GetSeconds();
  //! Get the minutes in the time
  unsigned int GetMinutes();
  //! Get the hours in the time
  unsigned int GetHours();
  //! Get the days in the time
  unsigned int GetDays();
  //! Get the months in the time
  unsigned int GetMonths();
  //! Get the years in the time
  unsigned int GetYears();

  //! Assignment operator
  MTime& operator=(const MTime& Time);
  //! Scale the time by a constant in place
  MTime& operator*=(const double& Constant);
  //! Add another time in place
  MTime& operator+=(const MTime& Time);
  //! Subtract another time in place
  MTime& operator-=(const MTime& Time);
  //! Return the sum of two times
  MTime operator+(const MTime& Time);
  //! Return the difference of two times
  MTime operator-(const MTime& Time);
  //! Return the time scaled by a constant
  MTime operator*(const double& Value);
  //! Return the time divided by a constant
  MTime operator/(const double& Value);
  //! Return true if two times differ
  bool operator!=(const MTime& Time) const;
  //! Return true if two times are identical
  bool operator==(const MTime& Time) const;
  //! Return true if this time is later than or equal to Time
  bool operator>=(const MTime& Time) const;
  //! Return true if this time is earlier than or equal to Time
  bool operator<=(const MTime& Time) const;
  //! Return true if this time is later than Time
  bool operator>(const MTime& Time) const;
  //! Return true if this time is earlier than Time
  bool operator<(const MTime& Time) const;

  
  //! Convert the time into seconds since the epoch as a double
  double GetAsDouble() const;
  //! Return the days since the epoch 1970-01-01
  unsigned int GetDaysSinceEpoch();
  //! Get the seconds since epoch in double format (should be renamed: GetSecondsSinceEpoch())
  double GetAsSeconds() const;
  //! Get the time as a fractional year, e.g. 2008.45345
  double GetAsYears();
  //! Return the seconds since epoch
  long int GetAsSystemSeconds();
  //! Convert into Julian days
  double GetAsJulianDay();

  //! Return as a human-readable "Seconds since epoch" string
  MString GetString();
  //! Return in format: 15.05.2002 13:15:23:123456789
  MString GetUTCString();
  //! Return in format: 1997-01-15 20:16:28
  MString GetSQLString();
  //! Return in format: 1997-01-15_20:16:28
  MString GetSQLUString();
  //! Return in format: 19970115_201628
  MString GetShortString();
  //! Return in format: 1164864276.623883519
  MString GetLongIntsString() const;
  //! Return as FITS date string: 31/12/94
  MString GetFitsDateString();
  //! Return as FITS time string: 15:45:57
  MString GetFitsTimeString();
  
  
  //! Get the time between NOW and the stored time in seconds as double 
  double GetElapsedSeconds();

  //! Busy wait loop --- Historic remnant from MEGAlyze --- should not be really here...
  static int BusyWait(int musec);

  enum Format { FormatLowerLimit = 0, Short, UTC, SQL, SQLU, LongInts, MEGAlib, FormatUpperLimit };

protected:
  //! Normalize the seconds and nanoseconds representation
  void Normalize();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
private:
  //! Seconds since the Unix epoch
  long int m_Seconds;
  //! Nanoseconds relative to m_Seconds
  long int m_NanoSeconds;


#ifdef ___CLING___
 public:
  ClassDef(MTime, 0) // no description
#endif

};

//! Stream a time to an output stream
std::ostream& operator<<(std::ostream& os, const MTime& Time);


#endif


////////////////////////////////////////////////////////////////////////////////
