/*
 * UTJulianDay.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MExceptions.h"
#include "MJulianDay.h"
#include "MStreams.h"
#include "MUnitTest.h"


//! Unit test class for MJulianDay
class UTJulianDay : public MUnitTest
{
public:
  //! Default constructor
  UTJulianDay() : MUnitTest("UTJulianDay") {}
  //! Default destructor
  virtual ~UTJulianDay() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test constructors and UTC accessors
  bool TestConstructionAndAccess();
  //! Test normalization, arithmetic, and comparisons
  bool TestArithmeticAndComparison();
  //! Test edge cases and boundary behavior
  bool TestEdgeCases();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTJulianDay::Run()
{
  bool Passed = true;

  Passed = TestConstructionAndAccess() && Passed;
  Passed = TestArithmeticAndComparison() && Passed;
  Passed = TestEdgeCases() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test constructors and UTC accessors
bool UTJulianDay::TestConstructionAndAccess()
{
  bool Passed = true;

  {
    MJulianDay Zero;
    Passed = EvaluateNear("MJulianDay()", "day", "The default constructor initializes the day to zero", Zero.GetDay(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("MJulianDay()", "fraction", "The default constructor initializes the fraction to zero", Zero.GetFraction(), 0.0, 1e-12) && Passed;
  }

  {
    MJulianDay Combined(2.25);
    Passed = EvaluateNear("MJulianDay(double)", "day", "The combined day constructor normalizes the integral day part", Combined.GetDay(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("MJulianDay(double)", "fraction", "The combined day constructor keeps the fractional day part", Combined.GetFraction(), 0.25, 1e-12) && Passed;
  }

  {
    MJulianDay Epoch(1970, 1, 1, 0, 0, 0, 0);
    Passed = EvaluateNear("MJulianDay(UTC)", "day", "Unix epoch corresponds to Julian day 2440587.5", Epoch.GetAsDays(), 2440587.5, 1e-9) && Passed;
    Passed = Evaluate("GetUTCString()", "epoch", "The UTC string formatter returns the expected epoch string", Epoch.GetUTCString(), MString("01.01.1970 00:00:00:000000000")) && Passed;
    Passed = Evaluate("GetUTCYear()", "epoch", "The UTC year accessor returns the expected year", Epoch.GetUTCYear(), 1970) && Passed;
    Passed = Evaluate("GetUTCMonth()", "epoch", "The UTC month accessor returns the expected month", Epoch.GetUTCMonth(), 1) && Passed;
    Passed = Evaluate("GetUTCDay()", "epoch", "The UTC day accessor returns the expected day", Epoch.GetUTCDay(), 1) && Passed;
    Passed = Evaluate("GetUTCHour()", "epoch", "The UTC hour accessor returns the expected hour", Epoch.GetUTCHour(), 0) && Passed;
    Passed = Evaluate("GetUTCMinute()", "epoch", "The UTC minute accessor returns the expected minute", Epoch.GetUTCMinute(), 0) && Passed;
    Passed = Evaluate("GetUTCSecond()", "epoch", "The UTC second accessor returns the expected second", Epoch.GetUTCSecond(), 0) && Passed;
    Passed = Evaluate("GetUTCNanoSecond()", "epoch", "The UTC nanosecond accessor returns the expected nanoseconds", Epoch.GetUTCNanoSecond(), 0) && Passed;
    Passed = EvaluateNear("GetAsSeconds()", "epoch", "Unix epoch converted to seconds matches the Julian-day definition", Epoch.GetAsSeconds(), 2440587.5 * 86400.0, 1e-3) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay Precise(2000, 2, 29, 12, 34, 56, 123456789);
    EnableDefaultStreams();
    Passed = Evaluate("GetUTCString()", "leap day", "Leap-day construction round-trips through the UTC string", Precise.GetUTCString(), MString("29.02.2000 12:34:56:123456789")) && Passed;
    Passed = Evaluate("GetUTCYear()", "leap day", "The leap-day year accessor returns the expected year", Precise.GetUTCYear(), 2000) && Passed;
    Passed = Evaluate("GetUTCMonth()", "leap day", "The leap-day month accessor returns the expected month", Precise.GetUTCMonth(), 2) && Passed;
    Passed = Evaluate("GetUTCDay()", "leap day", "The leap-day accessor returns the expected day", Precise.GetUTCDay(), 29) && Passed;
    Passed = Evaluate("GetUTCHour()", "leap day", "The leap-day hour accessor returns the expected hour", Precise.GetUTCHour(), 12) && Passed;
    Passed = Evaluate("GetUTCMinute()", "leap day", "The leap-day minute accessor returns the expected minute", Precise.GetUTCMinute(), 34) && Passed;
    Passed = Evaluate("GetUTCSecond()", "leap day", "The leap-day second accessor returns the expected second", Precise.GetUTCSecond(), 56) && Passed;
    Passed = Evaluate("GetUTCNanoSecond()", "leap day", "The leap-day nanosecond accessor returns the expected nanoseconds", Precise.GetUTCNanoSecond(), 123456789) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay Interior(2011, 7, 8, 9, 10, 11, 345678901);
    EnableDefaultStreams();
    Passed = Evaluate("GetUTCString()", "interior date", "A representative interior date round-trips through the UTC string", Interior.GetUTCString(), MString("08.07.2011 09:10:11:345678901")) && Passed;
    Passed = Evaluate("GetUTCYear()", "interior date", "The representative interior year accessor returns the expected year", Interior.GetUTCYear(), 2011) && Passed;
    Passed = Evaluate("GetUTCMonth()", "interior date", "The representative interior month accessor returns the expected month", Interior.GetUTCMonth(), 7) && Passed;
    Passed = Evaluate("GetUTCDay()", "interior date", "The representative interior day accessor returns the expected day", Interior.GetUTCDay(), 8) && Passed;
    Passed = Evaluate("GetUTCHour()", "interior date", "The representative interior hour accessor returns the expected hour", Interior.GetUTCHour(), 9) && Passed;
    Passed = Evaluate("GetUTCMinute()", "interior date", "The representative interior minute accessor returns the expected minute", Interior.GetUTCMinute(), 10) && Passed;
    Passed = Evaluate("GetUTCSecond()", "interior date", "The representative interior second accessor returns the expected second", Interior.GetUTCSecond(), 11) && Passed;
    Passed = Evaluate("GetUTCNanoSecond()", "interior date", "The representative interior nanosecond accessor returns the expected nanoseconds", Interior.GetUTCNanoSecond(), 345678901) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay Original(2020, 5, 6, 7, 8, 9, 10);
    EnableDefaultStreams();
    MJulianDay Copy(Original);
    Passed = Evaluate("MJulianDay(const MJulianDay&)", "copy equality", "The copy constructor preserves the full Julian day", Copy == Original, true) && Passed;

    MJulianDay Assigned;
    Assigned = Original;
    Passed = Evaluate("operator=", "assignment equality", "Assignment preserves the full Julian day", Assigned == Original, true) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay Now(true);
    EnableDefaultStreams();
    Passed = EvaluateTrue("MJulianDay(true)", "current date", "The 'now' constructor creates a positive Julian day", Now.GetAsDays() > 2400000.0) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test normalization, arithmetic, and comparisons
bool UTJulianDay::TestArithmeticAndComparison()
{
  bool Passed = true;

  {
    MJulianDay DayA(1.25);
    MJulianDay DayB(0.75);

    MJulianDay Sum = DayA + DayB;
    Passed = EvaluateNear("operator+", "sum day", "Adding two Julian days normalizes the resulting day", Sum.GetDay(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator+", "sum fraction", "Adding two Julian days normalizes the resulting fraction", Sum.GetFraction(), 0.0, 1e-12) && Passed;

    MJulianDay Difference = DayA - DayB;
    Passed = EvaluateNear("operator-", "difference day", "Subtracting two Julian days normalizes the resulting day", Difference.GetDay(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator-", "difference fraction", "Subtracting two Julian days normalizes the resulting fraction", Difference.GetFraction(), 0.5, 1e-12) && Passed;

    DayA += DayB;
    Passed = EvaluateNear("operator+=", "in-place day", "In-place addition updates the day correctly", DayA.GetDay(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator+=", "in-place fraction", "In-place addition updates the fraction correctly", DayA.GetFraction(), 0.0, 1e-12) && Passed;

    DayA -= MJulianDay(0.5);
    Passed = EvaluateNear("operator-=", "in-place subtraction day", "In-place subtraction updates the day correctly", DayA.GetDay(), 1.0, 1e-12) && Passed;
    Passed = EvaluateNear("operator-=", "in-place subtraction fraction", "In-place subtraction updates the fraction correctly", DayA.GetFraction(), 0.5, 1e-12) && Passed;
  }

  {
    MJulianDay Smaller(10.2);
    MJulianDay Equal(10.2);
    MJulianDay Larger(10.2001);

    Passed = Evaluate("operator==", "equal", "Equality returns true for identical day and fraction", Smaller == Equal, true) && Passed;
    Passed = Evaluate("operator!=", "different", "Inequality returns true for non-identical day or fraction", Smaller != Larger, true) && Passed;
    Passed = Evaluate("operator<", "less", "Strict less-than compares day then fraction", Smaller < Larger, true) && Passed;
    Passed = Evaluate("operator>", "greater", "Strict greater-than compares day then fraction", Larger > Smaller, true) && Passed;
    Passed = Evaluate("operator<=", "equal", "Less-than-or-equal returns true for identical values", Smaller <= Equal, true) && Passed;
    Passed = Evaluate("operator>=", "equal", "Greater-than-or-equal returns true for identical values", Smaller >= Equal, true) && Passed;
  }

  {
    MJulianDay Fractional(3.125);
    Passed = EvaluateNear("GetAsDays()", "fractional", "GetAsDays returns the combined normalized day and fraction", Fractional.GetAsDays(), 3.125, 1e-12) && Passed;
    Passed = EvaluateNear("GetAsSeconds()", "fractional", "GetAsSeconds converts a Julian day into uniform seconds", Fractional.GetAsSeconds(), 3.125 * 86400.0, 1e-3) && Passed;
    Passed = EvaluateNear("GetAsNanoSeconds()", "small value", "GetAsNanoSeconds converts short Julian-day intervals into nanoseconds", Fractional.GetAsNanoSeconds(), 3.125 * 86400.0 * 1.0E9, 1.0) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay Interior(2011, 7, 8, 9, 10, 11, 345678901);
    EnableDefaultStreams();
    MJulianDay InteriorPlusHalfDay = Interior + MJulianDay(0.5);
    Passed = Evaluate("operator+", "interior half day UTC", "Adding half a day to a representative interior Julian day advances by twelve hours", InteriorPlusHalfDay.GetUTCString(), MString("08.07.2011 21:10:11:345678901")) && Passed;

    MJulianDay InteriorMinusQuarterDay = Interior - MJulianDay(0.25);
    Passed = Evaluate("operator-", "interior quarter day UTC", "Subtracting a quarter day from a representative interior Julian day rewinds by six hours", InteriorMinusQuarterDay.GetUTCString(), MString("08.07.2011 03:10:11:345678901")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test edge cases and boundary behavior
bool UTJulianDay::TestEdgeCases()
{
  bool Passed = true;

  {
    MJulianDay Normalized(2.75, 0.5);
    Passed = EvaluateNear("MJulianDay(day,fraction)", "overflow day", "Fractions larger than one day are normalized into the day part", Normalized.GetDay(), 3.0, 1e-12) && Passed;
    Passed = EvaluateNear("MJulianDay(day,fraction)", "overflow fraction", "Fractions larger than one day are normalized into the fraction part", Normalized.GetFraction(), 0.25, 1e-12) && Passed;
  }

  {
    MJulianDay Negative(5.25, -0.5);
    Passed = EvaluateNear("MJulianDay(day,fraction)", "negative normalization day", "Negative fractions are normalized by borrowing from the day part", Negative.GetDay(), 4.0, 1e-12) && Passed;
    Passed = EvaluateNear("MJulianDay(day,fraction)", "negative normalization fraction", "Negative fractions are normalized into a positive stored fraction", Negative.GetFraction(), 0.75, 1e-12) && Passed;
  }

  {
    Passed = EvaluateException<MExceptionParameterOutOfRange>("MJulianDay(UTC)", "invalid month", "An invalid month now throws a range exception instead of aborting the process", []() { MJulianDay InvalidMonth(2020, 13, 1, 0, 0, 0, 0); }) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay InvalidDay(2020, 1, 0, 0, 0, 0, 0);
    EnableDefaultStreams();
    Passed = EvaluateNear("MJulianDay(UTC)", "invalid day", "An invalid day falls back to the zero Julian day", InvalidDay.GetAsDays(), 0.0, 1e-12) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay InvalidHour(2020, 1, 1, 24, 0, 0, 0);
    EnableDefaultStreams();
    Passed = EvaluateNear("MJulianDay(UTC)", "invalid hour", "An invalid hour falls back to the zero Julian day", InvalidHour.GetAsDays(), 0.0, 1e-12) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay InvalidMinute(2020, 1, 1, 0, 60, 0, 0);
    EnableDefaultStreams();
    Passed = EvaluateNear("MJulianDay(UTC)", "invalid minute", "An invalid minute falls back to the zero Julian day", InvalidMinute.GetAsDays(), 0.0, 1e-12) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay InvalidSecond(2020, 1, 1, 0, 0, 60, 0);
    EnableDefaultStreams();
    Passed = EvaluateNear("MJulianDay(UTC)", "invalid second", "An invalid second falls back to the zero Julian day", InvalidSecond.GetAsDays(), 0.0, 1e-12) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay InvalidNanoSecond(2020, 1, 1, 0, 0, 0, 1000000000);
    EnableDefaultStreams();
    Passed = EvaluateNear("MJulianDay(UTC)", "invalid nanosecond", "An invalid nanosecond value falls back to the zero Julian day", InvalidNanoSecond.GetAsDays(), 0.0, 1e-12) && Passed;
  }

  {
    MJulianDay WaitStart(true);
    MJulianDay::BusyWait(2000);
    MJulianDay WaitStop(true);
    Passed = EvaluateTrue("BusyWait()", "time advances", "BusyWait delays long enough for the current Julian day to advance measurably", WaitStop > WaitStart) && Passed;
  }

  {
    DisableDefaultStreams();
    MJulianDay GregorianSwitch(1582, 10, 16, 0, 0, 0, 0);
    EnableDefaultStreams();
    Passed = Evaluate("GetUTCString()", "gregorian switch", "A post-reform date round-trips through the UTC formatter", GregorianSwitch.GetUTCString(), MString("16.10.1582 00:00:00:000000000")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTJulianDay Test;
  return Test.Run() == true ? 0 : 1;
}
