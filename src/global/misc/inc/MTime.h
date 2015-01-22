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
  MTime();
	explicit MTime(MString SQLString, int Format);
  MTime(long int LinuxTime, long int NanoSeconds = 0);
  MTime(int LinuxTime, int NanoSeconds = 0);
  MTime(unsigned int LinuxTime, unsigned int NanoSeconds = 0);
  MTime(unsigned int m_Year, unsigned int m_Month, unsigned int m_Day, unsigned int m_Hour = 0, 
        unsigned int m_Minute = 0, unsigned int m_Second = 0, unsigned int m_NanoSecond = 0);
  MTime(double Time);
  virtual ~MTime();


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

  unsigned int GetNanoSeconds();
  unsigned int GetSeconds();
  unsigned int GetMinutes();
  unsigned int GetHours();
  double GetAsDouble() const;
  
  //! Return the days since the epoch 1970-01-01
  //! Quick emergency hack (absolute number was unnecessary) --- thus never really tested...
  unsigned int GetDaysSinceEpoch();

  unsigned int GetDays();
  unsigned int GetMonths();
  unsigned int GetYears();

  MTime& operator=(const MTime& Time);
  MTime& operator*=(const double& Const);
  MTime& operator+=(const MTime& Time);
  MTime& operator-=(const MTime& Time);
  MTime operator+(const MTime& Time);
  MTime operator-(const MTime& Time);
  MTime operator*(const double& v);
  MTime operator/(const double& v);
  bool operator!=(const MTime& Time);
  bool operator==(const MTime& Time);
  bool operator>=(const MTime& Time);
  bool operator<=(const MTime& Time);
  bool operator>(const MTime& Time);
  bool operator<(const MTime& Time);

  double GetElapsedSeconds();

  //! Get the seconds since epoch (should be renamed: GetSecondsSinceEpoch())
  double GetAsSeconds() const;
  //! Get as years in the form 2008.45345 
  double GetAsYears();
	long int GetAsSystemSeconds();
  double GetAsJulianDay();

  MString GetString();
  MString GetUTCString();
  MString GetSQLString();
  MString GetSQLUString();
  MString GetShortString();
  MString GetLongIntsString() const;

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
