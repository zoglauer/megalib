/*
 * MJulianDay.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


#ifndef __MJulianDay__
#define __MJulianDay__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MJulianDay 
{
  // public interface:
 public:
  //MJulianDay();
  MJulianDay(bool Now = false);
  MJulianDay(double DayPlusFraction);
  MJulianDay(double Day, double Fraction);
  MJulianDay(int UTCYear, int UTCMonth, int UTCDay, int UTCHour, 
             int UTCMinute, int UTCSecond, int UTCNanoSecond);
  MJulianDay(const MJulianDay& JD);
  virtual ~MJulianDay();


  MJulianDay& operator=(const MJulianDay& JD);
  MJulianDay& operator+=(const MJulianDay& JD);
  MJulianDay& operator-=(const MJulianDay& JD);
  MJulianDay operator+(const MJulianDay& JD);
  MJulianDay operator-(const MJulianDay& JD);
  bool      operator!=(const MJulianDay& JD);
  bool      operator==(const MJulianDay& JD);
  bool      operator>=(const MJulianDay& JD);
  bool      operator<=(const MJulianDay& JD);
  bool      operator>(const MJulianDay& JD);
  bool      operator<(const MJulianDay& JD);

  int GetUTCYear();
  int GetUTCMonth();
  int GetUTCDay();
  int GetUTCHour();
  int GetUTCMinute();
  int GetUTCSecond();
  int GetUTCNanoSecond();

  double GetDay();
  double GetFraction();

  double GetAsDays();
  double GetAsSeconds();
  double GetAsNanoSeconds();

  MString GetUTCString();

  static void BusyWait(int Microseconds);


  // protected methods:
 protected:
  bool CalculateUTC(int& UTCYear, int& UTCMonth, int& UTCDay, 
                      int& UTCHour, int& UTCMinute, int& UTCSecond, 
                      int& UTCNanoSecond);
  bool CalculateJD(int UTCYear, int UTCMonth, int UTCDay, 
                     int UTCHour, int UTCMinute, int UTCSecond, 
                     int UTCNanoSecond);

  void SetJulianDay(double Day, double Fraction);

  void Normalize();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  double m_Day;
  double m_Fraction;

#ifdef ___CLING___
 public:
  ClassDef(MJulianDay, 1) // Representation of Julian Day as two doubles (days, and fraction of the day)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
