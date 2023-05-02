/*
 * MJulianDay.cxx
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
// MJulianDay
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MJulianDay.h"

// Standard libs:
#include <ctime>
#include <cmath>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"
#include "MSystem.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MJulianDay)
#endif


////////////////////////////////////////////////////////////////////////////////


MJulianDay::MJulianDay(bool Now)
{
  // Construct an instance of MJulianDay
  // The set time is 0

  if (Now == false) {
    // Initialize with zero:
    SetJulianDay(0.0, 0.0);
  } else {
    // Initialize with the current date and time retrieved from the system:
    
  long int Seconds = 0, NanoSeconds = 0; 
  MSystem::GetTime(Seconds, NanoSeconds);

    time_t tloc = (time_t) Seconds;  // There might be problems on 64 bit systems

    struct tm *tp;
    tp = localtime(&tloc);
    
    CalculateJD(tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec, NanoSeconds);
  }
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay::MJulianDay(double DayPlusFraction)
{
  // ATTENTION: This constructor does not guaratee nanosecond precesion:
  // Only if DayPlusFraction < 10 you do have nanosecond precesion!

  SetJulianDay(DayPlusFraction, 0.0);
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay::MJulianDay(double Day, double Fraction)
{
  // Construct an instance of MJulianDay
  // JD:   initial julian day

  SetJulianDay(Day, Fraction);
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay::MJulianDay(int Year, int Month, int Day, int Hour, 
                       int Minute, int Second, int NanoSecond)
{
  if (CalculateJD(Year, Month, Day, Hour, Minute, Second, NanoSecond) == false) {
    Warning("MJulianDay:::MJulianDay", "Invalid date! Setting date to zero!");
    SetJulianDay(0.0, 0.0);
  }
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay::MJulianDay(const MJulianDay& JD)
{
  // Copy constructor
  
  SetJulianDay(JD.m_Day, JD.m_Fraction);
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay::~MJulianDay()
{
  // Delete this instance of MJulianDay - nothing to do
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::CalculateJD(int Year, int Month, int Day, 
                               int Hour, int Minute, int Second, 
                               int NanoSecond)
{
  // Set the julian day via Coordinated Universal Time (UTC) -
  // Does not check if days are ok!!

  double JDDay, JDFraction;

  // Check if everything is ok:
  if (Month <= 0 || Month >= 12) {
    Fatal("MJulianDay::SetJD", 
            "No valid month: %d (allowed: 0..12)", Month);
    return false;
  }
  if (Day <= 0) {
    Warning("MJulianDay::SetJD", 
            "No valid day: %d (allowed: 0..31)", Day);
    return false;
  }
  if (Day > 28 && Day < 32) {
    Warning("MJulianDay::SetJD", 
            "No maximum allowed day check for days 28, 29, 30, 31!");
  }
  if (Day > 31) {
    Warning("MJulianDay::SetJD", 
            "No valid day: %d (allowed: 0..31)", Day);
  }
  if (Hour < 0 || Hour > 23) {
    Warning("MJulianDay::SetJD", 
            "No valid hour: %d (allowed: 0..23)", Hour);
    return false;
  }
  if (Minute < 0 || Minute > 59) {
    Warning("MJulianDay::SetJD", 
            "No valid minute: %d (allowed: 0..59)", Minute);
    return false;
  }
  if (Second < 0 || Second > 59) {
    Warning("MJulianDay::SetJD", 
            "No valid second: %d (allowed: 0..59)", Second);
    return false;
  }
  if (NanoSecond < 0 || NanoSecond > 999999999) {
    Warning("MJulianDay::SetJD", 
            "No valid microsecond: %d (allowed: 0..999999999)", NanoSecond);
    return false;
  }

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

  SetJulianDay(JDDay, JDFraction);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


double MJulianDay::GetDay()
{
  return m_Day;
}


////////////////////////////////////////////////////////////////////////////////


double MJulianDay::GetFraction()
{
  return m_Fraction;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::CalculateUTC(int& Year, int& Month, int& Day, 
                                int& Hour, int& Minute, int& Second, 
                                int& NanoSecond)
{
  // lokale Konstanten
  // modified Julian Day gemäß XEphem-V3.1-Konventionen (entspricht 0.5 Jan 1900)
  const unsigned long ulMJDOffset = 2415020;

  // dynamische Variablen
  double dMJD0h;              // modified JulianDay der vorangehenden Mitternacht
  double d,f,i,a,b,ce,g;      // Variablen der XEphem-V3.1-Routine
  double dHoursOfDay;         // vergangene Stunden seit Mitternacht;

  // modified Julian Day (gemäß XEphem-Konventionen) der letzten vorangehenden Mitternacht bestimmen
  if (m_Fraction >= 0.5) {
    dMJD0h = ((double)m_Day - (double)ulMJDOffset) + 0.5;
  } else {
    dMJD0h = ((double)m_Day - (double)ulMJDOffset) - 0.5;
  }

  // Beginn der original XEphem-Routine
  d = dMJD0h + 0.5;
  i = floor(d);
  f = d - i;

  if(f == 1) {
    f = 0;
    i += 1;
  }

  if(i > -115860.0) {
    a = floor((i / 36524.25) + 0.9983573) + 14;
    i += 1 + a - floor(a / 4.0);
  }

  b = floor((i / 365.25) + 0.802601);
  ce = i - floor((365.25 * b) + 0.750001) + 416;
  g = floor(ce / 30.6001);
  Day = (unsigned int) (ce - floor(30.6001 * g) + f);

  if(g > 13.5) {
    Month = (unsigned int) (g - 13);
  } else {
    Month = (unsigned int) (g - 1);
  }

  double tYear;
  if(Month < 2.5) {
    tYear = b + 1900;
  } else {
    tYear = b + 1899;
  }

  if (tYear < 1) {
    tYear -= 1;
  }

  Year = (int) tYear; 

  // vergangene Stunden seit letzter Mitternacht bestimmen
  if (m_Fraction >= 0.5) {
    // es ist zwischen 0h und 12h
    dHoursOfDay = (m_Fraction - 0.5) * 24.0;
  } else {
    // es ist zwischen 12h und 24h
    dHoursOfDay = (m_Fraction + 0.5) * 24.0;
  }

  // ganze Stunden absplitten
  Hour = (unsigned short) (dHoursOfDay);

  // ganze Minuten absplitten
  Minute = (unsigned short) ((dHoursOfDay - (double) Hour) * 60.0);

  // verbleidende Sekunden bestimmen
  double tSecond = ((dHoursOfDay - (double) Hour) * 60.0 - (double) Minute) * 60.0;
  Second = (unsigned int) tSecond;

  tSecond -= Second;
  NanoSecond = (unsigned int) (floor(tSecond * 1.0E9 + 0.5));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MJulianDay::SetJulianDay(double Day, double Fraction)
{
  m_Day = Day;
  m_Fraction = Fraction;
  Normalize();
}


////////////////////////////////////////////////////////////////////////////////


void MJulianDay::Normalize()
{
  m_Fraction += m_Day - floor(m_Day);
  m_Day = floor(m_Day);

  if (m_Fraction >= 1.0)
  {
    m_Day += (long) m_Fraction;
    m_Fraction -= (double)(long) m_Fraction;
  }
  // auf Unterlauf prüfen
  else if (m_Fraction < 0.0)
  {
    // Number anpassen
    m_Day += (long) m_Fraction - 1;
    // Fraction anpassen
    m_Fraction += -(double)(long) m_Fraction + 1.0;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MJulianDay::GetUTCString()
{
  // Liefert den aktuellen JulianDay als String zurück

  const int Length = 100;
  char Text[Length];

  int Year, Month, Day, Hour, Minute, Second, NanoSecond;

  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);
  // und in String schreiben
  snprintf(Text, Length, "%02u.%02u.%04d %02u:%02u:%02u:%09u",
          Day, Month, Year, Hour, Minute, Second, NanoSecond);

  //mlog<<m_Day<<":"<<m_Month<<":"<<m_Year<<":"<<m_Hour<<":"<<m_Minute<<":"<<m_Second<<":"<<m_MicroSecond<<endl;

  return Text;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCYear()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Year;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCMonth()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Month;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCDay()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Day;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCHour()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Hour;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCMinute()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Minute;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCSecond()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return Second;
}


////////////////////////////////////////////////////////////////////////////////


int MJulianDay::GetUTCNanoSecond()
{
  int Year, Month, Day, Hour, Minute, Second, NanoSecond;
  CalculateUTC(Year, Month, Day, Hour, Minute, Second, NanoSecond);

  return NanoSecond;
}


////////////////////////////////////////////////////////////////////////////////


double MJulianDay::GetAsDays()
{
  // ATTENTION: The returned value does not have nano-second precesion

  return m_Day + m_Fraction;
}


////////////////////////////////////////////////////////////////////////////////


double MJulianDay::GetAsSeconds()
{
  // Return the Julian Day as seconds
  // It is assumed, that each JD has the same amount of seconds!

  return (m_Day + m_Fraction)*24*60*60;
}


////////////////////////////////////////////////////////////////////////////////


double MJulianDay::GetAsNanoSeconds()
{
  // Warning: Value has to be small enough to suit into one double!
  if (m_Day > 10 || m_Day < -10) {
    Error("double MJulianDay::GetAsNanoSeconds()",
          "GetAsNanoSeconds: Only valid for values < 10 days!"
          "Otherwise you loose your microsecond precision!");
  }
  
  return (m_Day + m_Fraction)*24*60*60*1000000000;

  // Round to nanoseconds
  // OK since no better accurancy needed and actual accuarany is 100x larger
  if ((m_Day + m_Fraction) >= 0) {
    return (double)(int) ((m_Day + m_Fraction)*24*60*60*1000000000 + 0.5);
  } else {
    return (double)(int) ((m_Day + m_Fraction)*24*60*60*1000000000 - 0.5);
  }
}


////////////////////////////////////////////////////////////////////////////////



MJulianDay& MJulianDay::operator=(const MJulianDay& JD)
{
  SetJulianDay(JD.m_Day, JD.m_Fraction);

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay& MJulianDay::operator+=(const MJulianDay& JD)
{
  SetJulianDay(m_Day + JD.m_Day, m_Fraction + JD.m_Fraction);

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay& MJulianDay::operator-=(const MJulianDay& JD)
{
  SetJulianDay(m_Day - JD.m_Day, m_Fraction - JD.m_Fraction);

  return *this;
}


// ////////////////////////////////////////////////////////////////////////////////


// MJulianDay& operator+=(MJulianDay& lhs, const MJulianDay& rhs)
// {
//   lhs.SetJulianDay(lhs.m_Day + rhs.m_Day, lhs.m_Fraction + rhs.m_Fraction);
//   return lhs;

// //   lhs.m_Day += rhs.m_Day;
// //   lhs.m_Fraction += rhs.m_Fraction;
// //   lhs.Normalize();
// }


// ////////////////////////////////////////////////////////////////////////////////


// MJulianDay& operator-=(MJulianDay& lhs, const MJulianDay& rhs)
// {
//   lhs.SetJulianDay(lhs.m_Day - rhs.m_Day, lhs.m_Fraction - rhs.m_Fraction);
//   return lhs;
// }


////////////////////////////////////////////////////////////////////////////////


MJulianDay MJulianDay::operator+(const MJulianDay& JD)
{
  return MJulianDay(m_Day + JD.m_Day, m_Fraction + JD.m_Fraction);
}


////////////////////////////////////////////////////////////////////////////////


MJulianDay MJulianDay::operator-(const MJulianDay& JD)
{
  return MJulianDay(m_Day - JD.m_Day, m_Fraction - JD.m_Fraction);
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator!=(const MJulianDay& JD)
{
  if (m_Day != JD.m_Day || m_Fraction != JD.m_Fraction) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator==(const MJulianDay& JD)
{
  if (m_Day == JD.m_Day && m_Fraction == JD.m_Fraction) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator>=(const MJulianDay& JD)
{
  if (m_Day > JD.m_Day || 
      (m_Day == JD.m_Day && m_Fraction >= JD.m_Fraction)) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator<=(const MJulianDay& JD)
{
  if (m_Day < JD.m_Day || 
      (m_Day == JD.m_Day && m_Fraction <= JD.m_Fraction)) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator>(const MJulianDay& JD)
{
  if (m_Day > JD.m_Day || 
      (m_Day == JD.m_Day && m_Fraction > JD.m_Fraction)) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MJulianDay::operator<(const MJulianDay& JD)
{
  if (m_Day < JD.m_Day || 
      (m_Day == JD.m_Day && m_Fraction < JD.m_Fraction)) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MJulianDay::BusyWait(int musec)
{
  // Do a busy wait (== calling thread is active NOT sleeping!) 
  // for several microseconds
  // Sleep for roughly [musec..musec+1] microseconds

  MSystem::BusyWait(musec);
}


// MJulianDay.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
