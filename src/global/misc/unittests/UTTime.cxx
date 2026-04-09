/*
 * UTTime.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


// MEGAlib:
#include "MTime.h"
#include "MUnitTest.h"
#include "MStreams.h"

// Standard lib:
#include <sstream>
using namespace std;


//! Unit test class for the MTime helper
class UTTime : public MUnitTest
{
public:
  //! Default constructor
  UTTime() : MUnitTest("UTTime") {}
  //! Default destructor
  virtual ~UTTime() {}

  //! Run all tests
  virtual bool Run();

private:
  //! Test construction and direct setters
  bool TestConstructionAndSetters();
  //! Test normalization and arithmetic
  bool TestArithmeticAndNormalization();
  //! Test comparisons and scalar operators
  bool TestComparisonsAndScaling();
  //! Test string parsing and formatting
  bool TestParsingAndFormatting();
  //! Test calendar and conversion helpers
  bool TestCalendarAndConversions();
};


////////////////////////////////////////////////////////////////////////////////


//! Run all tests
bool UTTime::Run()
{
  bool AllPassed = true;

  AllPassed = TestConstructionAndSetters() && AllPassed;
  AllPassed = TestArithmeticAndNormalization() && AllPassed;
  AllPassed = TestComparisonsAndScaling() && AllPassed;
  AllPassed = TestParsingAndFormatting() && AllPassed;
  AllPassed = TestCalendarAndConversions() && AllPassed;

  Summarize();

  return AllPassed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test construction and direct setters
bool UTTime::TestConstructionAndSetters()
{
  bool Passed = true;

  MTime EpochLong(0L, 0L);
  Passed = EvaluateNear("MTime(long, long)", "epoch seconds", "The long constructor stores the seconds component", EpochLong.GetAsSeconds(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetInternalNanoSeconds()", "epoch nanoseconds", "The long constructor stores the nanoseconds component", EpochLong.GetInternalNanoSeconds(), 0.0, 1e-12) && Passed;

  MTime EpochInt(0, 500000000);
  Passed = EvaluateNear("MTime(int, int)", "half second", "The int constructor stores the given nanoseconds", EpochInt.GetAsSeconds(), 0.5, 1e-12) && Passed;

  MTime EpochUnsigned(1U, 250000000U);
  Passed = EvaluateNear("MTime(unsigned int, unsigned int)", "one and a quarter seconds", "The unsigned constructor stores the given values", EpochUnsigned.GetAsSeconds(), 1.25, 1e-12) && Passed;

  MTime FromDouble(12.75);
  Passed = EvaluateNear("MTime(double)", "12.75", "The double constructor splits seconds and nanoseconds correctly", FromDouble.GetAsSeconds(), 12.75, 1e-12) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "12.75", "The double constructor stores the expected nanosecond precision",
                        FromDouble.GetLongIntsString() == "12.750000000") && Passed;

  MTime Calendar(1970, 1, 1, 0, 0, 0, 123456789);
  Passed = EvaluateNear("MTime(year, month, day, ...)", "epoch calendar", "The calendar constructor builds the expected epoch time", Calendar.GetAsSeconds(), 0.123456789, 1e-12) && Passed;
  Passed = EvaluateTrue("GetUTCString()", "epoch calendar", "The calendar constructor exposes the expected UTC string",
                        Calendar.GetUTCString() == "01.01.1970 00:00:00:123456789") && Passed;

  MTime Copy(Calendar);
  Passed = EvaluateTrue("MTime(const MTime&)", "copy", "Copy construction preserves the stored time", Copy == Calendar) && Passed;

  MTime Assigned(5L, 6L);
  Assigned = Calendar;
  Passed = EvaluateTrue("operator=", "assignment", "Assignment preserves the stored time", Assigned == Calendar) && Passed;

  MTime SetByTime;
  Passed = EvaluateFalse("Set(const MTime&)", "return value", "Set(const MTime&) currently returns false even though it updates the time", SetByTime.Set(Calendar)) && Passed;
  Passed = EvaluateTrue("Set(const MTime&)", "state update", "Set(const MTime&) copies the source time", SetByTime == Calendar) && Passed;

  MTime SetByLong;
  Passed = EvaluateFalse("Set(long, long)", "return value", "Set(long, long) currently returns false after updating the time", SetByLong.Set(3L, 400000000L)) && Passed;
  Passed = EvaluateNear("Set(long, long)", "state update", "Set(long, long) stores the given values", SetByLong.GetAsSeconds(), 3.4, 1e-12) && Passed;

  MTime SetByInt;
  Passed = EvaluateFalse("Set(int, int)", "return value", "Set(int, int) currently returns false after updating the time", SetByInt.Set(4, 500000000)) && Passed;
  Passed = EvaluateNear("Set(int, int)", "state update", "Set(int, int) stores the given values", SetByInt.GetAsSeconds(), 4.5, 1e-12) && Passed;

  MTime SetByUnsigned;
  Passed = EvaluateFalse("Set(unsigned int, unsigned int)", "return value", "Set(unsigned int, unsigned int) currently returns false after updating the time", SetByUnsigned.Set(5U, 600000000U)) && Passed;
  Passed = EvaluateNear("Set(unsigned int, unsigned int)", "state update", "Set(unsigned int, unsigned int) stores the given values", SetByUnsigned.GetAsSeconds(), 5.6, 1e-12) && Passed;

  MTime SetByDouble;
  Passed = EvaluateTrue("Set(double)", "return value", "Set(double) reports success", SetByDouble.Set(8.125)) && Passed;
  Passed = EvaluateNear("Set(double)", "state update", "Set(double) stores the given time", SetByDouble.GetAsSeconds(), 8.125, 1e-12) && Passed;

  MTime SetByDoubleParts;
  Passed = EvaluateFalse("Set(double, double)", "return value", "Set(double, double) currently returns false after updating the time", SetByDoubleParts.Set(1.0, 500000000.0)) && Passed;
  Passed = EvaluateNear("Set(double, double)", "state update", "Set(double, double) combines the two parts into one time", SetByDoubleParts.GetAsSeconds(), 1.5, 1e-12) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test normalization and arithmetic
bool UTTime::TestArithmeticAndNormalization()
{
  bool Passed = true;

  MTime Carry(1L, 1500000000L);
  Passed = EvaluateTrue("Normalize()", "positive carry", "Nanoseconds above one second are normalized into the seconds field",
                        Carry.GetLongIntsString() == "2.500000000") && Passed;

  MTime Borrow(1L, -500000000L);
  Passed = EvaluateTrue("Normalize()", "positive seconds, negative nanos", "Mixed-sign representations are normalized for positive seconds",
                        Borrow.GetLongIntsString() == "0.500000000") && Passed;

  MTime NegativeCarry(-1L, -1500000000L);
  Passed = EvaluateTrue("Normalize()", "negative carry", "Large negative nanoseconds are normalized into the seconds field",
                        NegativeCarry.GetLongIntsString() == "-2.500000000") && Passed;

  MTime NegativeBorrow(-1L, 500000000L);
  Passed = EvaluateTrue("Normalize()", "negative seconds, positive nanos", "Mixed-sign representations are normalized into the historical negative-fraction format",
                        NegativeBorrow.GetLongIntsString() == "-0.500000000") && Passed;

  MTime Sum = MTime(1L, 750000000L) + MTime(2L, 500000000L);
  Passed = EvaluateTrue("operator+", "sum", "operator+ adds and normalizes seconds and nanoseconds", Sum.GetLongIntsString() == "4.250000000") && Passed;

  MTime Difference = MTime(5L, 250000000L) - MTime(2L, 500000000L);
  Passed = EvaluateTrue("operator-", "difference", "operator- subtracts and normalizes seconds and nanoseconds", Difference.GetLongIntsString() == "2.750000000") && Passed;

  MTime InPlaceSum(1L, 750000000L);
  InPlaceSum += MTime(2L, 500000000L);
  Passed = EvaluateTrue("operator+=", "sum", "operator+= matches operator+", InPlaceSum == Sum) && Passed;

  MTime InPlaceDifference(5L, 250000000L);
  InPlaceDifference -= MTime(2L, 500000000L);
  Passed = EvaluateTrue("operator-=", "difference", "operator-= matches operator-", InPlaceDifference == Difference) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test comparisons and scalar operators
bool UTTime::TestComparisonsAndScaling()
{
  bool Passed = true;

  MTime A(1L, 250000000L);
  MTime B(2L, 0L);
  MTime C(1L, 250000000L);
  MTime D(1L, 500000000L);

  Passed = EvaluateTrue("operator==", "equal", "operator== recognizes identical times", A == C) && Passed;
  Passed = EvaluateFalse("operator!=", "equal", "operator!= is false for identical times", A != C) && Passed;
  Passed = EvaluateTrue("operator!=", "different nanos", "operator!= is true when the nanoseconds differ", A != D) && Passed;
  Passed = EvaluateTrue("operator<", "ordering", "operator< compares seconds before nanoseconds", A < B) && Passed;
  Passed = EvaluateTrue("operator>", "ordering", "operator> compares seconds before nanoseconds", B > A) && Passed;
  Passed = EvaluateTrue("operator<=", "equal", "operator<= accepts equality", A <= C) && Passed;
  Passed = EvaluateTrue("operator>=", "equal", "operator>= accepts equality", A >= C) && Passed;

  MTime Scaled = A * 2.0;
  Passed = EvaluateTrue("operator*", "scale by 2", "operator* scales the stored time", Scaled.GetLongIntsString() == "2.500000000") && Passed;

  MTime InPlaceScaled = A;
  InPlaceScaled *= 2.0;
  Passed = EvaluateTrue("operator*=", "scale by 2", "operator*= matches operator*", InPlaceScaled == Scaled) && Passed;

  MTime Divided = MTime(5L, 0L) / 2.0;
  Passed = EvaluateTrue("operator/", "divide by 2", "operator/ divides the stored time", Divided.GetLongIntsString() == "2.500000000") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test string parsing and formatting
bool UTTime::TestParsingAndFormatting()
{
  bool Passed = true;

  MTime FromString;
  Passed = EvaluateTrue("Set(MString)", "plain TI format", "Set(MString) parses the standard TI format",
                        FromString.Set("TI 12.3456789")) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "plain TI format", "Set(MString) pads fractional digits to nanoseconds",
                        FromString.GetLongIntsString() == "12.345678900") && Passed;

  Passed = EvaluateTrue("Set(MString)", "with sec suffix", "Set(MString) ignores the optional sec suffix",
                        FromString.Set("TI 7.25 sec")) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "with sec suffix", "Set(MString) stores the parsed seconds and nanoseconds",
                        FromString.GetLongIntsString() == "7.250000000") && Passed;

  mout.Enable(false);
  Passed = EvaluateFalse("Set(MString)", "not a number", "Set(MString) rejects invalid input", FromString.Set(MString("TI nonsense"))) && Passed;
  Passed = EvaluateFalse("Set(MString)", "too many separators", "Set(MString) rejects more than one fractional separator", FromString.Set(MString("1.2.3"))) && Passed;
  mout.Enable(true);

  MTime FromChar;
  Passed = EvaluateTrue("Set(const char*)", "plain digits", "Set(const char*) parses seconds and fractional digits", FromChar.Set("TI 9.5")) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "plain digits", "Set(const char*) pads fractional digits to nanoseconds",
                        FromChar.GetLongIntsString() == "9.500000000") && Passed;

  Passed = EvaluateTrue("Set(const char*)", "no fractional part", "Set(const char*) accepts integer-only values", FromChar.Set("TI 11")) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "no fractional part", "Set(const char*) sets the nanoseconds field to zero when omitted",
                        FromChar.GetLongIntsString() == "11.000000000") && Passed;

  MTime NoDigits;
  Passed = EvaluateTrue("Set(const char*)", "no digits", "Set(const char*) accepts digit-free input and resets to zero", NoDigits.Set("invalid")) && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "no digits", "Set(const char*) resets to the epoch when no digits are present",
                        NoDigits.GetLongIntsString() == "0.000000000") && Passed;

  MTime UTC("01.01.1970 00:00:00:000000123", MTime::UTC);
  Passed = EvaluateTrue("MTime(MString, UTC)", "UTC string", "The deprecated UTC parser still constructs the expected time",
                        UTC.GetLongIntsString() == "0.000000123") && Passed;

  MTime SQL("1970-01-01 00:00:00", MTime::SQL);
  Passed = EvaluateTrue("MTime(MString, SQL)", "SQL string", "The deprecated SQL parser still constructs the expected time",
                        SQL.GetLongIntsString() == "0.000000000") && Passed;

  MTime SQLU("1970-01-01_00:00:00", MTime::SQLU);
  Passed = EvaluateTrue("MTime(MString, SQLU)", "SQLU string", "The deprecated SQLU parser still constructs the expected time",
                        SQLU.GetLongIntsString() == "0.000000000") && Passed;

  MTime Short("19700101_000000", MTime::Short);
  Passed = EvaluateTrue("MTime(MString, Short)", "short string", "The deprecated short parser still constructs the expected time",
                        Short.GetLongIntsString() == "0.000000000") && Passed;

  mout.Enable(false);
  MTime Invalid("wrong", MTime::SQL);
  mout.Enable(true);
  Passed = EvaluateTrue("MTime(MString, Format)", "invalid format", "Invalid formatted input falls back to the epoch",
                        Invalid.GetLongIntsString() == "0.000000000") && Passed;

  MTime FormatTime(0L, 123456789L);
  Passed = EvaluateTrue("GetString()", "human readable", "GetString returns the documented human-readable format",
                        FormatTime.GetString() == "Seconds since epoch:   0.123457") && Passed;
  Passed = EvaluateTrue("GetLongIntsString()", "long ints", "GetLongIntsString returns the seconds.nanoseconds layout",
                        FormatTime.GetLongIntsString() == "0.123456789") && Passed;
  Passed = EvaluateTrue("GetUTCString()", "UTC", "GetUTCString returns the documented UTC layout",
                        FormatTime.GetUTCString() == "01.01.1970 00:00:00:123456789") && Passed;
  Passed = EvaluateTrue("GetSQLString()", "SQL", "GetSQLString returns the documented SQL layout",
                        FormatTime.GetSQLString() == "1970-01-01 00:00:00") && Passed;
  Passed = EvaluateTrue("GetSQLUString()", "SQLU", "GetSQLUString returns the documented SQLU layout",
                        FormatTime.GetSQLUString() == "1970-01-01_00:00:00") && Passed;
  Passed = EvaluateTrue("GetShortString()", "short", "GetShortString returns the compact timestamp layout",
                        FormatTime.GetShortString() == "19700101_000000") && Passed;
  Passed = EvaluateTrue("GetFitsDateString()", "FITS date", "GetFitsDateString returns the FITS-style date layout",
                        FormatTime.GetFitsDateString() == "01/01/70") && Passed;
  Passed = EvaluateTrue("GetFitsTimeString()", "FITS time", "GetFitsTimeString returns the FITS-style time layout",
                        FormatTime.GetFitsTimeString() == "00:00:00") && Passed;

  ostringstream Out;
  Out << FormatTime;
  Passed = EvaluateTrue("operator<<", "stream formatting", "The stream operator forwards to GetLongIntsString()",
                        Out.str() == "0.123456789") && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


//! Test calendar and conversion helpers
bool UTTime::TestCalendarAndConversions()
{
  bool Passed = true;

  MTime Epoch(1970, 1, 1, 0, 0, 0, 123456789);
  Passed = EvaluateNear("GetAsDouble()", "epoch", "GetAsDouble matches GetAsSeconds()", Epoch.GetAsDouble(), 0.123456789, 1e-12) && Passed;
  Passed = EvaluateNear("GetAsSeconds()", "epoch", "GetAsSeconds includes fractional nanoseconds", Epoch.GetAsSeconds(), 0.123456789, 1e-12) && Passed;
  Passed = EvaluateNear("GetAsSystemSeconds()", "epoch", "GetAsSystemSeconds returns the stored whole seconds", Epoch.GetAsSystemSeconds(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetAsJulianDay()", "epoch", "GetAsJulianDay returns the documented Julian day for the Unix epoch",
                        Epoch.GetAsJulianDay(), 2440587.500001429, 1e-9) && Passed;

  Passed = EvaluateNear("GetYears()", "epoch", "GetYears returns the calendar year", Epoch.GetYears(), 1970.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMonths()", "epoch", "GetMonths returns the calendar month", Epoch.GetMonths(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetDays()", "epoch", "GetDays returns the calendar day", Epoch.GetDays(), 1.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetHours()", "epoch", "GetHours returns the calendar hour", Epoch.GetHours(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetMinutes()", "epoch", "GetMinutes returns the calendar minute", Epoch.GetMinutes(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetSeconds()", "epoch", "GetSeconds returns the calendar second", Epoch.GetSeconds(), 0.0, 1e-12) && Passed;
  Passed = EvaluateNear("GetNanoSeconds()", "epoch", "GetNanoSeconds returns the stored nanoseconds", Epoch.GetNanoSeconds(), 123456789.0, 1e-12) && Passed;

  MTime LeapYearMiddle(2000, 7, 2, 12, 0, 0, 0);
  Passed = EvaluateTrue("GetAsYears()", "middle of leap year", "GetAsYears stays within the expected calendar year bounds",
                        LeapYearMiddle.GetAsYears() > 2000.49 && LeapYearMiddle.GetAsYears() < 2000.51) && Passed;

  mout.Enable(false);
  unsigned int DaysSinceEpoch = Epoch.GetDaysSinceEpoch();
  mout.Enable(true);
  Passed = EvaluateNear("GetDaysSinceEpoch()", "epoch", "GetDaysSinceEpoch returns zero for the epoch", DaysSinceEpoch, 0.0, 1e-12) && Passed;

  Passed = EvaluateNear("BusyWait()", "zero microseconds", "BusyWait returns zero", MTime::BusyWait(0), 0.0, 1e-12) && Passed;

  MTime BeforeNow;
  BeforeNow.Set(0.0);
  Passed = EvaluateTrue("GetElapsedSeconds()", "epoch", "GetElapsedSeconds is positive for a time in the past", BeforeNow.GetElapsedSeconds() > 0.0) && Passed;

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTTime Test;
  return Test.Run() == true ? 0 : 1;
}


////////////////////////////////////////////////////////////////////////////////
