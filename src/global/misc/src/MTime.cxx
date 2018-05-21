/*
 * MTime.cxx
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
// MTime in nanosecond precision - if supported by system 
//
// This class is able to represent a time span and a calender time, depending
// on the construction and executed operation
//
// Examples:
//
// MTime Time; // Contains the current calendar time
// MTime Time(0); // Time span: 0  or  Start of (Linux-) Epoch : 01-01-1970 01:00:00:00
// MTime Time(1243344); // Time span in seconds or a date in 1970...
// MTime Time(2002, 06, 05, 13, 52, 36, 0); // A Calendar time
// time_t MyC++Time;
// MTime Time(MyC++Time); // A Calendar time
//
// The operations behave in the same (ambiguous) way:
//
// Calendar time + Calendar time = ***not useful***
// Calendar time + time span     = calendar time
// Calendar time - time span     = Calendar time
// Calendar time - Calendar time = time span
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTime.h"

// Standard lib:
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstddef>
#include <cstring>
using namespace std;

// ROOT libs:
#include "TMath.h"

// MEGAlib libs:
#include "MStreams.h"
#include "MSystem.h"
#include "MTokenizer.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MTime)
#endif


////////////////////////////////////////////////////////////////////////////////


MTime::MTime()
{
  // Construct an instance of MTime

  Now();
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(MString String, int Format)
{
  // The string has to be in SQL-format
  // If it is not then the time is set to the epoch

  int Result = -1;

  int Years = 0;
  int Months = 0;
  int Days = 0;
  int Hours = 0;
  int Minutes = 0;
  int Seconds = 0;
  int NanoSeconds = 0;

  switch (Format) {
  case UTC:
    Result = sscanf(String, "%02u.%02u.%04d %02u:%02u:%02u:%09u", 
        &Years, &Months, &Days, &Hours, &Minutes, &Seconds, &NanoSeconds);
    if (Result != 6) Result = -1; // Means: we have an error
    break;
  case SQL:
    Result = sscanf(String, "%04d-%02u-%02u %02u:%02u:%02u", 
        &Years, &Months, &Days, &Hours, &Minutes, &Seconds);
    if (Result != 6) Result = -1; // Means: we have an error
    break;
  case SQLU:
    Result = sscanf(String, "%04d-%02u-%02u_%02u:%02u:%02u", 
        &Years, &Months, &Days, &Hours, &Minutes, &Seconds);
    if (Result != 6) Result = -1; // Means: we have an error
    break;
  case Short:
    Result = sscanf(String, "%04d%02u%02u_%02u%02u%02u", 
        &Years, &Months, &Days, &Hours, &Minutes, &Seconds);
    if (Result != 6) Result = -1; // Means: we have an error
    break;
  default:
    break;
  }

  if (Result == -1) {
    mout<<"MTime: Unknown string format! Setting time to Epoch!"<<endl;
    Set(0, 0);
  } else {
    Set(Years, Months, Days, Hours, Minutes, Seconds, NanoSeconds); 
  }
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(long int Seconds, long int NanoSeconds)
{
  // Construct an instance of MTime

  Set(Seconds, NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(int Seconds, int NanoSeconds)
{
  // Construct an instance of MTime

  Set(Seconds, NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(unsigned int Seconds, unsigned int NanoSeconds)
{
  // Construct an instance of MTime

  Set(Seconds, NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(unsigned int Year, unsigned int Month, unsigned int Day, unsigned int Hour, 
             unsigned int Minute, unsigned int Second, unsigned int NanoSecond)
{
  // Construct an instance of MTime
  //
  // Format:
  // Year: 1970...2030 or 70 ... 130 or 70..99, 0, 1, 2, ... 30
  // Month: 1..12
  // Day: 1..31
  // Hour: 0..23
  // Min: 0..59
  // Sec: 0..61 (leap seconds!)
  // NanoSecond: 0..999999999 

  Set(Year, Month, Day, Hour, Minute, Second, NanoSecond);
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(double Time)
{
  // This format is not really accurate... on a 32bit System

  Set(Time);
}


////////////////////////////////////////////////////////////////////////////////


MTime::MTime(const MTime& Time)
{
  m_Seconds = Time.m_Seconds;
  m_NanoSeconds = Time.m_NanoSeconds;
}


////////////////////////////////////////////////////////////////////////////////


MTime::~MTime()
{
  // Delete this instance of MTime
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Now()
{
  // Set the value to the current time
  // The behaviour is encapsulted into the MSystem class, due to differneces
  // between Windows and Linux

  MSystem::GetTime(m_Seconds, m_NanoSeconds);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(unsigned int Year, unsigned int Month, unsigned int Day, 
                unsigned int Hour, unsigned int Minute, unsigned int Second, 
                unsigned int NanoSecond)
{
  // Format:
  // Year: 1970...2030 or 70 ... 130 or 70..99, 0, 1, 2, ... 30
  // Month: 1..12
  // Day: 1..31
  // Hour: 0..23
  // Min: 0..59
  // Sec: 0..61 (leap seconds!)
  // NanoSecond: 0..999999999 

  struct tm tp;

  if (Year < 70) Year += 100;
  if (Year > 1970) Year -= 1900;
  tp.tm_year = Year;
  if (Month <= 0) {
    tp.tm_yday = Day;
  } else {
    tp.tm_mon = Month-1;
    tp.tm_mday = Day;
  }
  tp.tm_hour = Hour;
  tp.tm_min = Minute;
  tp.tm_sec = Second;
  tp.tm_isdst = 0; // GMT: no daylight savings!

  m_Seconds = mktime(&tp);

  if (m_Seconds == -1) {
    merr<<"Invalid time!!"<<endl;
    m_Seconds = 0;
    m_NanoSeconds = 0;
    return false;
  }
  
  m_NanoSeconds = NanoSecond;

  Normalize();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const MTime& Time)
{
  //

  *this = Time;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const long int Seconds, const long int NanoSeconds)
{
  //

  m_Seconds = Seconds;
  m_NanoSeconds = NanoSeconds;

  Normalize();

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const int Seconds, const int NanoSeconds)
{
  //

  m_Seconds = Seconds;
  m_NanoSeconds = NanoSeconds;

  Normalize();

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const unsigned int Seconds, const unsigned int NanoSeconds)
{
  //

  m_Seconds = int(Seconds);
  m_NanoSeconds = int(NanoSeconds);

  Normalize();

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const double Seconds, const double NanoSeconds)
{
  //

  Set(Seconds);
  MTime Temp(NanoSeconds/1000000000);
  *this += Temp;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(double Time)
{
  //

  m_Seconds = (long int) Time;
  m_NanoSeconds = (long int) TMath::FloorNint((Time - m_Seconds) * 1000000000);

  Normalize();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const char* Line)
{
  // This is the fast, unsafe, no-error checks version:
  // If you want it save use the MString version

  char* Text = strdup(Line);
  size_t Size = strlen(Text);

  size_t Start = 0;
  size_t Stop = 0;
  while (Start < Size && !isdigit(Text[Start])) ++Start;
  if (Start == Size) { 
    m_Seconds = 0;
    m_NanoSeconds = 0;
  } else {
    Stop = Start;

    while (Stop < Size && isdigit(Text[Stop])) ++Stop;
    if (Stop < Size) Text[Stop] = '\0';
    m_Seconds = atoi(Text+Start);
    if (m_Seconds == numeric_limits<int>::max()) {
      mout<<"Seconds in MTime are maxed out... time is probably wrong: "<<Line<<endl;
    }

    Start = Stop+1;
    while (Start < Size && !isdigit(Text[Start])) ++Start;
    if (Start >= Size) { 
      m_NanoSeconds = 0;
    } else {
      Stop = Start;
      while (isdigit(Text[Stop])) ++Stop;
      Text[Stop] = '\0';

      // Count digits:
      int Digits = Stop - Start;
      if (Digits > 9) Digits = 9;
      Text[Start+Digits] = '\0';
      m_NanoSeconds = atoi(Text+Start);
      if (Digits < 9) {
        m_NanoSeconds *= int(pow(10.0, 9.0 - Digits));
      } else if (Digits > 9) {
        m_NanoSeconds /= int(pow(10.0, Digits - 9));   
      }
    }
  } 

  free(Text);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::Set(const MString& String, unsigned int I)
{
  // Slow but safe version of setting a time
  
  int Seconds = 0;
  int NanoSeconds = 0;
  MString NewString = String.GetSubString(I);
  NewString.ReplaceAll("TI", "");
  NewString.ReplaceAll("sec", "");
  NewString.Strip(); // remove white spaces from beginning and end
  
  // Check if what we have now is a number
  if (NewString.IsNumber() == false) {
    mout<<"MTime: Cannot set time correctly via MString: String is no number: "<<String<<endl;
    return false;
  }
  
  // Split and 
  MTokenizer T('.', true);
  T.Analyze(NewString);
  if (T.GetNTokens() == 1) {
    Seconds = T.GetTokenAtAsLong(0);
  } else if (T.GetNTokens() == 2) {
    Seconds = T.GetTokenAtAsLong(0);
    NanoSeconds = T.GetTokenAtAsLong(1);
    // Find digits:
    int Digits = T.GetTokenAt(1).Length();
    if (Digits < 9) {
      NanoSeconds *= int(pow(10.0, 9.0 - Digits));
    } else if (Digits > 9) {
      NanoSeconds /= int(pow(10.0, Digits - 9.0));   
    }
  } else {
    mout<<"MTime: Cannot set time not correctly via MString ("<<T.GetNTokens()<<" tokens): "<<String<<endl;
    return false;       
  }
  
  Set(Seconds, NanoSeconds);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MTime::Normalize()
{
  while (m_NanoSeconds >= 1000000000) {
    m_Seconds += m_NanoSeconds/1000000000;
    m_NanoSeconds %= 1000000000;
  }
  while (m_NanoSeconds <= -1000000000) {
    m_Seconds += m_NanoSeconds/1000000000;
    m_NanoSeconds %= 1000000000;
  }

  while (m_Seconds > 0 && m_NanoSeconds < 0) {
    m_NanoSeconds += 1000000000;
    m_Seconds -= 1;
  }
  while (m_Seconds < 0 && m_NanoSeconds > 0) {
    m_NanoSeconds -= 1000000000;
    m_Seconds += 1;
  }
}


////////////////////////////////////////////////////////////////////////////////


double MTime::GetElapsedSeconds()
{
  // Return the number of seconds which are ellapsed since MTime

  MTime Now;

  return Now.GetAsSeconds() - GetAsSeconds();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetNanoSeconds()
{
  // Return the seconds

  return m_NanoSeconds;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetSeconds()
{
  // Return the seconds

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_sec;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetMinutes()
{
  // Return the minutes

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_min;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetHours()
{
  // Return the hours

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_hour;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetDaysSinceEpoch()
{
  // Return the days since the epoch 1970-01-01
  // Very stupid approach, but might work --- never really tested...

  mimp<<"This function has not been thoroughly tested!!!"<<show;

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  unsigned int Days = tp.tm_yday;

  unsigned int Year = GetYears();
  while (Year > 1970) {
    Year--;
    MTime NewTime(Year, 12, 31, 23, 59, 30);
    time_t Seconds = (long int) NewTime.GetAsSeconds();
    tp = *localtime(&Seconds);
    Days += tp.tm_yday;
  }

  return Days;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetDays()
{
  // Return the day

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_mday;
}

////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetMonths()
{
  // Return the month

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_mon+1;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTime::GetYears()
{
  // Return the Year

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  return tp.tm_year+1900;
}


////////////////////////////////////////////////////////////////////////////////


double MTime::GetAsSeconds() const
{
  // Return the seconds

  return (double) m_Seconds + m_NanoSeconds/1000000000.0;
}


////////////////////////////////////////////////////////////////////////////////


double MTime::GetAsYears()
{
  // Return as year in form 1984.478657

  double Year = GetYears();

  MTime Start(GetYears(), 1, 1, 0, 0, 0, 0);
  MTime Stop(GetYears(), 12, 31, 23, 59, 59, 999999999);

  return Year += (GetAsSeconds() - Start.GetAsSeconds())/(Stop.GetAsSeconds() - Start.GetAsSeconds());
}


////////////////////////////////////////////////////////////////////////////////


long int MTime::GetAsSystemSeconds()
{
  // Return the system time seconds

  return m_Seconds;
}


////////////////////////////////////////////////////////////////////////////////


double MTime::GetAsJulianDay()
{
  // Set the julian day via Coordinated Universal Time (UTC) -
  // Does not check if days are ok!!

  time_t Time = m_Seconds;
  struct tm tp;
  tp = *localtime(&Time);

  int Year = tp.tm_year+1900;
  int Month = tp.tm_mon+1;
  int Day = tp.tm_mday; 
  int Hour = tp.tm_hour;
  int Minute = tp.tm_min;
  int Second = tp.tm_sec;
  int NanoSecond = m_NanoSeconds;

  double JDDay, JDFraction;

  // Now transform to julian day:
  bool reform;
  long a, b=0, c, d;
  double x1;

  if (Month < 3) {
    Year--;
    Month += 12;
  }

  reform = ((Year == 1582) && (((Month == 10) && (Day > 15)) || (Month > 10))) || (Year > 1582);
  
  if (reform) {
    a = Year / 100;
    c = a/4;
    b = 2 - a + c;
  }

  x1 = 365.25 * Year;
  if (Year < 0) {
    x1 -= 0.75;
  }
  c = (long) x1;
  d = (long) (30.6001 * (Month + 1));
  JDDay = c + d + Day + 1720994;

  if (reform == true) {
    JDDay += b;
  }

  //mlog.precision(40);
  //mlog<<JDDay<<endl;

  JDDay += 0.5;
  //mlog<<JDDay<<endl;

  // Now the fraction of the day
  JDFraction = ((double) Hour / 24.0);
  //mlog<<JDFraction<<"Diff: "<<((double) Hour / 24.0)<<endl;
  JDFraction += ((double) Minute / 1440.0 );
  //mlog<<JDFraction<<"Diff: "<<((double) Minute / 1440.0)<<endl;
  JDFraction += ((double) Second / 86400.0);
  //mlog<<JDFraction<<"Diff: "<<((double) Second / 86400.0)<<endl;
  JDFraction += ((double) NanoSecond / (86400.0*1000000000.0));
  //mlog<<JDFraction<<"Diff: "<<((double) NanoSecond / (86400.0*1000000000.0))<<endl;

  return JDDay + JDFraction;
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetString()
{
  // Return in Format: 76751347.238477

  char Text[100];
  double Time = GetAsSeconds();
  sprintf(Text, "Seconds since epoch: %10.6f", Time);
  //cout<<GetAsSeconds()<<"!"<<GetNanoSeconds()<<endl;

  return Text; 
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetLongIntsString() const
{
  // Return in Format: 76751347.238477213

  int Precision = 9;
  
  long Nanos = m_NanoSeconds;
  Nanos /= (long) pow(10.0, 9-Precision);
  
  ostringstream out;
  out<<((Nanos < 0 && m_Seconds == 0) ? "-" : "")<<m_Seconds;
  if (Precision > 0) {
    out<<"."<<setprecision(Precision)<<setw(Precision)<<setfill('0')<<abs(Nanos);
  }
  //out<<" sec";
  
  return out.str().c_str(); 
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetUTCString()
{
  // Return in Format: 15.05.2002 13:15:23:123456789

  char Text[100];
  sprintf(Text, "%02u.%02u.%04d %02u:%02u:%02u:%09u", 
          GetDays(), GetMonths(), GetYears(), GetHours(), GetMinutes(), GetSeconds(), GetNanoSeconds());

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetSQLString()
{
  // Return in Format: 1997-01-15 20:16:28

  char Text[100];
  sprintf(Text, "%04d-%02u-%02u %02u:%02u:%02u", 
          GetYears(), GetMonths(), GetDays(), GetHours(), GetMinutes(), GetSeconds());

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetSQLUString()
{
  // Return in Format: 1997-01-15_20:16:28

  char Text[100];
  sprintf(Text, "%04d-%02u-%02u_%02u:%02u:%02u", 
          GetYears(), GetMonths(), GetDays(), GetHours(), GetMinutes(), GetSeconds());

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


MString MTime::GetShortString()
{
  // Return in Format: 19970115_201628

  char Text[100];
  sprintf(Text, "%04d%02u%02u_%02u%02u%02u", 
          GetYears(), GetMonths(), GetDays(), GetHours(), GetMinutes(), GetSeconds());

  return Text;
}

////////////////////////////////////////////////////////////////////////////////


int MTime::BusyWait(int musec)
{
  // Sleep for roughly [musec..musec+1] microseconds
  // Do not call this function at midnight...
  // Otherwise some ghost will send this function into an endless loop

  MSystem::BusyWait(musec);

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MTime& MTime::operator=(const MTime& T)
{
  Set(T.m_Seconds, T.m_NanoSeconds);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MTime& MTime::operator+=(const MTime& T)
{
  Set(m_Seconds + T.m_Seconds, m_NanoSeconds + T.m_NanoSeconds);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MTime& MTime::operator*=(const double& T)
{
  if (numeric_limits<long>::max() < double(m_NanoSeconds)*T) {
    merr<<"Overflow in MTime..."<<endl;
  } 
  
  Set(m_Seconds*T, m_NanoSeconds*T);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MTime& MTime::operator-=(const MTime& T)
{
  Set(m_Seconds - T.m_Seconds, m_NanoSeconds - T.m_NanoSeconds);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MTime MTime::operator+(const MTime& T)
{
  return MTime(m_Seconds + T.m_Seconds, m_NanoSeconds + T.m_NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


MTime MTime::operator*(const double& v)
{
  if (numeric_limits<long>::max() < double(m_NanoSeconds)*v) {
    merr<<"Overflow in MTime..."<<endl;
  } 
  
  MTime Time;
  Time.Set(m_Seconds*v, m_NanoSeconds*v);

  return Time;
}

////////////////////////////////////////////////////////////////////////////////


MTime MTime::operator/(const double& v)
{
  MTime Time;
  Time.Set(double(m_Seconds)/v, double(m_NanoSeconds)/v);
  
  return Time;
}


////////////////////////////////////////////////////////////////////////////////


MTime MTime::operator-(const MTime& T)
{
  return MTime(m_Seconds - T.m_Seconds, m_NanoSeconds - T.m_NanoSeconds);
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator!=(const MTime& T) const
{
  return ((m_Seconds != T.m_Seconds) && (m_NanoSeconds != T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator==(const MTime& T) const
{
  return ((m_Seconds == T.m_Seconds) && (m_NanoSeconds == T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator>=(const MTime& T) const
{
  return (m_Seconds > T.m_Seconds || (m_Seconds == T.m_Seconds && m_NanoSeconds >= T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator<=(const MTime& T) const
{
  return (m_Seconds < T.m_Seconds || (m_Seconds == T.m_Seconds && m_NanoSeconds <= T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator>(const MTime& T) const
{
  return (m_Seconds > T.m_Seconds || (m_Seconds == T.m_Seconds && m_NanoSeconds > T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


bool MTime::operator<(const MTime& T) const
{
  return (m_Seconds < T.m_Seconds || (m_Seconds == T.m_Seconds && m_NanoSeconds < T.m_NanoSeconds));
}


////////////////////////////////////////////////////////////////////////////////


double MTime::GetAsDouble() const
{
  return (double) m_Seconds + m_NanoSeconds/1000000000.0;
}


////////////////////////////////////////////////////////////////////////////////


std::ostream& operator<<(std::ostream& os, const MTime& Time)
{
  os<<Time.GetLongIntsString();
  return os;
}

// MTime.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
