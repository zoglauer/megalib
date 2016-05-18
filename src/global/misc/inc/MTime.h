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


class MTime
{
  // public interface:
 public:
  //! Default constructor, set the time to NOW
  MTime();
  //! Extracts the time from a string -- depreciated! -- don't use since there vis no error catching done! 
	explicit MTime(MString SQLString, int Format);
  //! Set the time as two long integers -- the time is counted since Epoch
  MTime(long int LinuxTime, long int NanoSeconds = 0);
  //! Set the time as two intergers -- the time is counted since Epoch
  MTime(int LinuxTime, int NanoSeconds = 0);
  //! Set the time as two unsigned intergers -- the time is counted since Epoch
  MTime(unsigned int LinuxTime, unsigned int NanoSeconds = 0);
  //! Set the time elements (years, days, etc) individually
  MTime(unsigned int m_Year, unsigned int m_Month, unsigned int m_Day, unsigned int m_Hour = 0, 
        unsigned int m_Minute = 0, unsigned int m_Second = 0, unsigned int m_NanoSecond = 0);
  //! Set the time as a double -- the time is counted since Epoch
  MTime(double Time);
  //! Copy constructor
  MTime(const MTime& Time);
  
  //! Default destructor
  virtual ~MTime();


  // TODO: Some of these function return false by default! Before next alpha release switch it!
  
  bool Now();
  bool Set(unsigned int m_Year, unsigned int m_Month, unsigned int m_Day, unsigned int m_Hour = 0, 
             unsigned int m_Minute = 0, unsigned int m_Second = 0, unsigned int m_NanoSecond = 0);
  bool Set(const long int LinuxTime, const long int NanoSeconds = 0);
  bool Set(const int LinuxTime, const int NanoSeconds = 0);
  bool Set(const unsigned int LinuxTime, const unsigned int NanoSeconds = 0);
  bool Set(const double Seconds, const double NanoSeconds);
  bool Set(double Time);
  bool Set(const MTime& Time);
  //! Set a string in MEGAlib TI format: TI 123456789.123456789
  //! Starts the conversion at character I
  bool Set(const MString& String, unsigned int I = 0);
  //! Set a string in MEGAlib TI format: TI 123456789.123456789
  //! Fast version with minimal error checks and rather unsafe...
  bool Set(const char* Line);

  // Access the fields:
  
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


  // The operators
  
  MTime& operator=(const MTime& Time);
  MTime& operator*=(const double& Const);
  MTime& operator+=(const MTime& Time);
  MTime& operator-=(const MTime& Time);
  MTime operator+(const MTime& Time);
  MTime operator-(const MTime& Time);
  MTime operator*(const double& v);
  MTime operator/(const double& v);
  bool operator!=(const MTime& Time) const;
  bool operator==(const MTime& Time) const;
  bool operator>=(const MTime& Time) const;
  bool operator<=(const MTime& Time) const;
  bool operator>(const MTime& Time) const;
  bool operator<(const MTime& Time) const;

  
  // Conversions

  // Convert into a double 
  double GetAsDouble() const;
  //! Return the days since the epoch 1970-01-01
  unsigned int GetDaysSinceEpoch();
  //! Get the seconds since epoch in double format (should be renamed: GetSecondsSinceEpoch())
  double GetAsSeconds() const;
  //! Get as years in the form 2008.45345 
  double GetAsYears();
  //! Return the seconds since epoch
  long int GetAsSystemSeconds();
  //! Convert into Julian days
  double GetAsJulianDay();

  // Return in Format: 76751347.238477
  MString GetString();
  // Return in Format: 15.05.2002 13:15:23:123456789
  MString GetUTCString();
  // Return in Format: 1997-01-15 20:16:28
  MString GetSQLString();
  // Return in Format: 1997-01-15_20:16:28
  MString GetSQLUString();
  // Return in Format: 19970115_201628
  MString GetShortString();
  // Return in Format: 1164864276.623883519
  MString GetLongIntsString() const;

  
  //! Get the time between NOW and the stored time in seconds as double 
  double GetElapsedSeconds();

	//! Busy wait loop --- Historic remnant from MEGAlyze --- should not be really here...
  static int BusyWait(int musec);

  enum Format { FormatLowerLimit = 0, Short, UTC, SQL, SQLU, LongInts, MEGAlib, FormatUpperLimit };


  // protected methods:
 protected:
  void Normalize();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  long int m_Seconds;
  long int m_NanoSeconds; 


#ifdef ___CINT___
 public:
  ClassDef(MTime, 0) // no description
#endif

};

std::ostream& operator<<(std::ostream& os, const MTime& Time);


#endif


////////////////////////////////////////////////////////////////////////////////
