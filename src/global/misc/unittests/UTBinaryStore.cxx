/*
 * UTBinaryStore.cxx
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

// Standard libs:
#include <cmath>
#include <limits>
#include <sstream>
using namespace std;

// MEGAlib:
#include "MBinaryStore.h"
#include "MExceptions.h"
#include "MTime.h"
#include "MUnitTest.h"


//! Unit test class for MBinaryStore
class UTBinaryStore : public MUnitTest
{
public:
  UTBinaryStore() : MUnitTest("UTBinaryStore") {}
  virtual ~UTBinaryStore() {}

  virtual bool Run();

private:
  bool TestPrimitiveRoundTrips();
  bool TestVectorsAndTimes();
  bool TestStringsAndCharacters();
  bool TestPositionAndBounds();
};


////////////////////////////////////////////////////////////////////////////////


bool UTBinaryStore::Run()
{
  bool Passed = true;

  Passed = TestPrimitiveRoundTrips() && Passed;
  Passed = TestVectorsAndTimes() && Passed;
  Passed = TestStringsAndCharacters() && Passed;
  Passed = TestPositionAndBounds() && Passed;

  Summarize();

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinaryStore::TestPrimitiveRoundTrips()
{
  bool Passed = true;

  {
    MBinaryStore Store;
    Store.AddInt8(-12);
    Store.AddUInt8(250);
    Store.AddInt16(-12345);
    Store.AddUInt16(54321);
    Store.AddInt32(-123456789);
    Store.AddUInt32(3456789012u);
    Store.AddInt64(-1234567890123456789ll);
    Store.AddUInt64(12345678901234567890ull);
    Store.AddFloat(1.25f);
    Store.AddDouble(-3.141592653589793);

    Passed = Evaluate("GetInt8()", "representative signed byte", "Signed 8-bit integers round-trip through the binary store", static_cast<int>(Store.GetInt8()), -12) && Passed;
    Passed = Evaluate("GetUInt8()", "representative unsigned byte", "Unsigned 8-bit integers round-trip through the binary store", static_cast<unsigned int>(Store.GetUInt8()), 250U) && Passed;
    Passed = Evaluate("GetInt16()", "representative signed short", "Signed 16-bit integers round-trip through the binary store", static_cast<int>(Store.GetInt16()), -12345) && Passed;
    Passed = Evaluate("GetUInt16()", "representative unsigned short", "Unsigned 16-bit integers round-trip through the binary store", static_cast<unsigned int>(Store.GetUInt16()), 54321U) && Passed;
    Passed = Evaluate("GetInt32()", "representative signed int", "Signed 32-bit integers round-trip through the binary store", Store.GetInt32(), -123456789) && Passed;
    Passed = Evaluate("GetUInt32()", "representative unsigned int", "Unsigned 32-bit integers round-trip through the binary store", Store.GetUInt32(), 3456789012u) && Passed;
    Passed = Evaluate("GetInt64()", "representative signed long long", "Signed 64-bit integers round-trip through the binary store", Store.GetInt64(), static_cast<int64_t>(-1234567890123456789ll)) && Passed;
    Passed = Evaluate("GetUInt64()", "representative unsigned long long", "Unsigned 64-bit integers round-trip through the binary store", Store.GetUInt64(), static_cast<uint64_t>(12345678901234567890ull)) && Passed;
    Passed = EvaluateNear("GetFloat()", "representative float", "Floats round-trip through the binary store", Store.GetFloat(), 1.25, 1e-7) && Passed;
    Passed = EvaluateNear("GetDouble()", "representative double", "Doubles round-trip through the binary store", Store.GetDouble(), -3.141592653589793, 1e-12) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "all primitives consumed", "Reading all representative primitive values advances to the end of the store", Store.GetArraySizeUnread(), 0.0, 1e-12) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddInt8(numeric_limits<int8_t>::min());
    Store.AddInt8(0);
    Store.AddInt8(numeric_limits<int8_t>::max());
    Store.AddUInt8(numeric_limits<uint8_t>::max());
    Store.AddInt16(numeric_limits<int16_t>::min());
    Store.AddInt16(0);
    Store.AddInt16(numeric_limits<int16_t>::max());
    Store.AddUInt16(numeric_limits<uint16_t>::max());
    Store.AddInt32(numeric_limits<int32_t>::min());
    Store.AddInt32(0);
    Store.AddInt32(numeric_limits<int32_t>::max());
    Store.AddUInt32(numeric_limits<uint32_t>::max());
    Store.AddInt64(numeric_limits<int64_t>::min());
    Store.AddInt64(0);
    Store.AddInt64(numeric_limits<int64_t>::max());
    Store.AddUInt64(numeric_limits<uint64_t>::max());
    Store.AddFloat(0.0f);
    Store.AddFloat(-0.0f);
    Store.AddFloat(numeric_limits<float>::infinity());
    Store.AddFloat(-numeric_limits<float>::infinity());
    Store.AddFloat(numeric_limits<float>::quiet_NaN());
    Store.AddDouble(0.0);
    Store.AddDouble(-0.0);
    Store.AddDouble(numeric_limits<double>::infinity());
    Store.AddDouble(-numeric_limits<double>::infinity());
    Store.AddDouble(numeric_limits<double>::quiet_NaN());

    Passed = Evaluate("GetInt8()", "minimum signed byte", "Signed 8-bit integers preserve the minimum boundary value", static_cast<int>(Store.GetInt8()), static_cast<int>(numeric_limits<int8_t>::min())) && Passed;
    Passed = Evaluate("GetInt8()", "zero signed byte", "Signed 8-bit integers preserve zero exactly", static_cast<int>(Store.GetInt8()), 0) && Passed;
    Passed = Evaluate("GetInt8()", "maximum signed byte", "Signed 8-bit integers preserve the maximum boundary value", static_cast<int>(Store.GetInt8()), static_cast<int>(numeric_limits<int8_t>::max())) && Passed;
    Passed = Evaluate("GetUInt8()", "maximum unsigned byte", "Unsigned 8-bit integers preserve the maximum boundary value", static_cast<unsigned int>(Store.GetUInt8()), static_cast<unsigned int>(numeric_limits<uint8_t>::max())) && Passed;
    Passed = Evaluate("GetInt16()", "minimum signed short", "Signed 16-bit integers preserve the minimum boundary value", static_cast<int>(Store.GetInt16()), static_cast<int>(numeric_limits<int16_t>::min())) && Passed;
    Passed = Evaluate("GetInt16()", "zero signed short", "Signed 16-bit integers preserve zero exactly", static_cast<int>(Store.GetInt16()), 0) && Passed;
    Passed = Evaluate("GetInt16()", "maximum signed short", "Signed 16-bit integers preserve the maximum boundary value", static_cast<int>(Store.GetInt16()), static_cast<int>(numeric_limits<int16_t>::max())) && Passed;
    Passed = Evaluate("GetUInt16()", "maximum unsigned short", "Unsigned 16-bit integers preserve the maximum boundary value", static_cast<unsigned int>(Store.GetUInt16()), static_cast<unsigned int>(numeric_limits<uint16_t>::max())) && Passed;
    Passed = Evaluate("GetInt32()", "minimum signed int", "Signed 32-bit integers preserve the minimum boundary value", Store.GetInt32(), numeric_limits<int32_t>::min()) && Passed;
    Passed = Evaluate("GetInt32()", "zero signed int", "Signed 32-bit integers preserve zero exactly", Store.GetInt32(), 0) && Passed;
    Passed = Evaluate("GetInt32()", "maximum signed int", "Signed 32-bit integers preserve the maximum boundary value", Store.GetInt32(), numeric_limits<int32_t>::max()) && Passed;
    Passed = Evaluate("GetUInt32()", "maximum unsigned int", "Unsigned 32-bit integers preserve the maximum boundary value", Store.GetUInt32(), numeric_limits<uint32_t>::max()) && Passed;
    Passed = Evaluate("GetInt64()", "minimum signed long long", "Signed 64-bit integers preserve the minimum boundary value", Store.GetInt64(), numeric_limits<int64_t>::min()) && Passed;
    Passed = Evaluate("GetInt64()", "zero signed long long", "Signed 64-bit integers preserve zero exactly", Store.GetInt64(), static_cast<int64_t>(0)) && Passed;
    Passed = Evaluate("GetInt64()", "maximum signed long long", "Signed 64-bit integers preserve the maximum boundary value", Store.GetInt64(), numeric_limits<int64_t>::max()) && Passed;
    Passed = Evaluate("GetUInt64()", "maximum unsigned long long", "Unsigned 64-bit integers preserve the maximum boundary value", Store.GetUInt64(), numeric_limits<uint64_t>::max()) && Passed;

    float FloatZero = Store.GetFloat();
    float FloatNegativeZero = Store.GetFloat();
    float FloatPositiveInfinity = Store.GetFloat();
    float FloatNegativeInfinity = Store.GetFloat();
    float FloatNaN = Store.GetFloat();
    double DoubleZero = Store.GetDouble();
    double DoubleNegativeZero = Store.GetDouble();
    double DoublePositiveInfinity = Store.GetDouble();
    double DoubleNegativeInfinity = Store.GetDouble();
    double DoubleNaN = Store.GetDouble();

    Passed = EvaluateNear("GetFloat()", "positive zero float", "Float round-trips preserve positive zero", FloatZero, 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetFloat()", "negative zero float sign", "Float round-trips preserve the sign bit of representative negative zero", signbit(FloatNegativeZero) != 0) && Passed;
    Passed = EvaluateTrue("GetFloat()", "positive infinity float", "Float round-trips preserve representative positive infinity", isinf(FloatPositiveInfinity) != 0 && FloatPositiveInfinity > 0) && Passed;
    Passed = EvaluateTrue("GetFloat()", "negative infinity float", "Float round-trips preserve representative negative infinity", isinf(FloatNegativeInfinity) != 0 && FloatNegativeInfinity < 0) && Passed;
    Passed = EvaluateTrue("GetFloat()", "nan float", "Float round-trips preserve representative NaN payload semantics", isnan(FloatNaN) != 0) && Passed;
    Passed = EvaluateNear("GetDouble()", "positive zero double", "Double round-trips preserve positive zero", DoubleZero, 0.0, 1e-12) && Passed;
    Passed = EvaluateTrue("GetDouble()", "negative zero double sign", "Double round-trips preserve the sign bit of representative negative zero", signbit(DoubleNegativeZero) != 0) && Passed;
    Passed = EvaluateTrue("GetDouble()", "positive infinity double", "Double round-trips preserve representative positive infinity", isinf(DoublePositiveInfinity) != 0 && DoublePositiveInfinity > 0) && Passed;
    Passed = EvaluateTrue("GetDouble()", "negative infinity double", "Double round-trips preserve representative negative infinity", isinf(DoubleNegativeInfinity) != 0 && DoubleNegativeInfinity < 0) && Passed;
    Passed = EvaluateTrue("GetDouble()", "nan double", "Double round-trips preserve representative NaN payload semantics", isnan(DoubleNaN) != 0) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddFloats(1.25f, -2.5f);
    Passed = EvaluateNear("AddFloats()", "representative first float", "The AddFloats convenience helper stores the first representative float in order", Store.GetFloat(), 1.25, 1e-7) && Passed;
    Passed = EvaluateNear("AddFloats()", "representative second float", "The AddFloats convenience helper stores the second representative float in order", Store.GetFloat(), -2.5, 1e-7) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddDoubles(1.25, -2.5);
    Passed = EvaluateNear("AddDoubles()", "representative first double", "The AddDoubles convenience helper stores the first representative double in order", Store.GetDouble(), 1.25, 1e-12) && Passed;
    Passed = EvaluateNear("AddDoubles()", "representative second double", "The AddDoubles convenience helper stores the second representative double in order", Store.GetDouble(), -2.5, 1e-12) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinaryStore::TestVectorsAndTimes()
{
  bool Passed = true;

  {
    MBinaryStore Store;
    MVector Normalized(0.1234, -0.5, 1.0);
    Store.AddNormalizedVectorInt16(Normalized);
    MVector ReadBack = Store.GetNormalizedVectorInt16();
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "representative x", "Normalized vectors stored as int16 preserve a representative x component", ReadBack.X(), 0.1234, 1e-4) && Passed;
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "representative y", "Normalized vectors stored as int16 preserve a representative y component", ReadBack.Y(), -0.5, 1e-4) && Passed;
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "representative z", "Normalized vectors stored as int16 preserve a representative z component", ReadBack.Z(), 1.0, 1e-4) && Passed;
  }

  {
    MBinaryStore Store;
    MVector Normalized(-1.0, 0.0, 0.0);
    Store.AddNormalizedVectorInt16(Normalized);
    MVector ReadBack = Store.GetNormalizedVectorInt16();
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "negative boundary x", "Normalized vectors stored as int16 preserve the representative negative boundary value", ReadBack.X(), -1.0, 1e-4) && Passed;
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "negative boundary y", "Normalized vectors stored as int16 preserve a representative zero y component at the negative boundary", ReadBack.Y(), 0.0, 1e-4) && Passed;
    Passed = EvaluateNear("GetNormalizedVectorInt16()", "negative boundary z", "Normalized vectors stored as int16 preserve a representative zero z component at the negative boundary", ReadBack.Z(), 0.0, 1e-4) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddInt16(123);
    Store.AddInt16(456);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetNormalizedVectorInt16()", "representative short buffer", "Normalized int16 vectors reject representative stores with fewer than three components", [&]() { Store.GetNormalizedVectorInt16(); }) && Passed;
  }

  {
    MBinaryStore Store;
    MVector Value(1.25, -2.5, 3.75);
    Store.AddVectorFloat(Value);
    MVector ReadBack = Store.GetVectorFloat();
    Passed = EvaluateNear("GetVectorFloat()", "representative x", "Vectors stored as floats preserve a representative x component", ReadBack.X(), 1.25, 1e-6) && Passed;
    Passed = EvaluateNear("GetVectorFloat()", "representative y", "Vectors stored as floats preserve a representative y component", ReadBack.Y(), -2.5, 1e-6) && Passed;
    Passed = EvaluateNear("GetVectorFloat()", "representative z", "Vectors stored as floats preserve a representative z component", ReadBack.Z(), 3.75, 1e-6) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddFloat(1.25f);
    Store.AddFloat(-2.5f);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetVectorFloat()", "representative short buffer", "Float vectors reject representative stores with fewer than three components", [&]() { Store.GetVectorFloat(); }) && Passed;
  }

  {
    MBinaryStore Store;
    MVector Value(1.25, -2.5, 3.75);
    Store.AddVectorDouble(Value);
    MVector ReadBack = Store.GetVectorDouble();
    Passed = EvaluateNear("GetVectorDouble()", "representative x", "Vectors stored as doubles preserve a representative x component", ReadBack.X(), 1.25, 1e-12) && Passed;
    Passed = EvaluateNear("GetVectorDouble()", "representative y", "Vectors stored as doubles preserve a representative y component", ReadBack.Y(), -2.5, 1e-12) && Passed;
    Passed = EvaluateNear("GetVectorDouble()", "representative z", "Vectors stored as doubles preserve a representative z component", ReadBack.Z(), 3.75, 1e-12) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddDouble(1.25);
    Store.AddDouble(-2.5);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetVectorDouble()", "representative short buffer", "Double vectors reject representative stores with fewer than three components", [&]() { Store.GetVectorDouble(); }) && Passed;
  }

  {
    MBinaryStore Store;
    MTime Time(123456789L, 987654321L);
    Store.AddTime(Time);
    MTime ReadBack = Store.GetTime();
    Passed = Evaluate("GetTime()", "representative seconds", "Times stored as int64 plus int32 preserve representative seconds", ReadBack.GetInternalSeconds(), 123456789L) && Passed;
    Passed = Evaluate("GetTime()", "representative nanoseconds", "Times stored as int64 plus int32 preserve representative nanoseconds", ReadBack.GetInternalNanoSeconds(), 987654321L) && Passed;
  }

  {
    MBinaryStore Store;
    MTime Time(12L, 345678901L);
    Store.AddTimeUInt64(Time);
    MTime ReadBack = Store.GetTimeUInt64();
    Passed = Evaluate("GetTimeUInt64()", "representative seconds", "Times stored as compact uint64 nanoseconds preserve representative seconds", ReadBack.GetInternalSeconds(), 12L) && Passed;
    Passed = Evaluate("GetTimeUInt64()", "representative nanoseconds", "Times stored as compact uint64 nanoseconds preserve representative nanoseconds", ReadBack.GetInternalNanoSeconds(), 345678901L) && Passed;
  }

  {
    MBinaryStore Store;
    MTime TooLarge(static_cast<long int>(numeric_limits<uint64_t>::max() / 1000000000ull), 0L);
    Passed = EvaluateException<MExceptionArbitrary>("AddTimeUInt64()", "representative overflow", "Compact uint64 time storage rejects representative times whose seconds would overflow nanosecond packing", [&]() { Store.AddTimeUInt64(TooLarge); }) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinaryStore::TestStringsAndCharacters()
{
  bool Passed = true;

  {
    MBinaryStore Store;
    Store.AddString("abc", 5);
    MString Value = Store.GetString(5);
    Passed = EvaluateNear("GetString()", "representative fixed width length", "Fixed-width strings preserve the requested representative length including padding", Value.Length(), 5.0, 1e-12) && Passed;
    Passed = Evaluate("GetString()", "representative fixed width char 0", "Fixed-width strings preserve the first representative payload character", Value[0], 'a') && Passed;
    Passed = Evaluate("GetString()", "representative fixed width char 1", "Fixed-width strings preserve the second representative payload character", Value[1], 'b') && Passed;
    Passed = Evaluate("GetString()", "representative fixed width char 2", "Fixed-width strings preserve the third representative payload character", Value[2], 'c') && Passed;
    Passed = Evaluate("GetString()", "representative fixed width char 3", "Fixed-width strings pad unused representative characters with null bytes", Value[3], '\0') && Passed;
    Passed = Evaluate("GetString()", "representative fixed width char 4", "Fixed-width strings pad later representative characters with null bytes", Value[4], '\0') && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddString("abcde", 3);
    MString Value = Store.GetString(3);
    Passed = EvaluateNear("AddString()", "representative truncation length", "Fixed-width strings truncate representative longer payloads to the requested width", Value.Length(), 3.0, 1e-12) && Passed;
    Passed = Evaluate("AddString()", "representative truncation char 0", "Fixed-width strings keep the first representative truncated character", Value[0], 'a') && Passed;
    Passed = Evaluate("AddString()", "representative truncation char 1", "Fixed-width strings keep the second representative truncated character", Value[1], 'b') && Passed;
    Passed = Evaluate("AddString()", "representative truncation char 2", "Fixed-width strings keep the third representative truncated character", Value[2], 'c') && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddString("abc", 0);
    Passed = EvaluateNear("AddString()", "zero-width representative string", "Adding a representative fixed-width string with zero characters writes no bytes", Store.GetArraySize(), 0.0, 1e-12) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = Evaluate("GetString()", "zero-width representative string", "Reading a representative fixed-width string with zero characters returns an empty string", Store.GetString(0), MString("")) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddChar('a');
    Store.AddChar('b');
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetString()", "representative short buffer", "Fixed-width strings reject representative stores that are shorter than the requested field width", [&]() { Store.GetString(3); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddChar('x');
    char Text[] = {'y', 'z'};
    Store.AddChars(Text, 2);
    Passed = Evaluate("GetChar()", "representative first char", "Individual representative characters round-trip through the binary store", Store.GetChar(), 'x') && Passed;
    Passed = Evaluate("GetChar()", "representative second char", "Representative character arrays preserve the second byte", Store.GetChar(), 'y') && Passed;
    Passed = Evaluate("GetChar()", "representative third char", "Representative character arrays preserve the third byte", Store.GetChar(), 'z') && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddChar('a');
    Store.AddChar('b');
    Store.AddChar('c');
    ostringstream Stream;
    Stream << Store;
    Passed = Evaluate("operator<<", "representative byte stream", "Streaming a representative binary store prints representative stored characters in order", MString(Stream.str().c_str()), MString("a b c ")) && Passed;
  }

  {
    MBinaryStore Store;
    ostringstream Stream;
    Stream << Store;
    Passed = Evaluate("operator<<", "empty representative byte stream", "Streaming an empty representative binary store prints nothing", MString(Stream.str().c_str()), MString("")) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


bool UTBinaryStore::TestPositionAndBounds()
{
  bool Passed = true;

  {
    MBinaryStore Store;
    Store.AddUInt8(10);
    Store.AddUInt8(20);
    Store.AddUInt8(30);
    Passed = Evaluate("GetArrayValue()", "representative in-bounds access", "Array-value access returns the representative stored byte at a valid index", static_cast<unsigned int>(Store.GetArrayValue(1)), 20U) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "initial representative unread size", "Unread size starts at the full representative array length", Store.GetArraySizeUnread(), 3.0, 1e-12) && Passed;
    Passed = Evaluate("GetUInt8()", "representative first byte", "Reading advances through representative bytes in insertion order", static_cast<unsigned int>(Store.GetUInt8()), 10U) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "after representative first read", "Unread size shrinks after a representative read", Store.GetArraySizeUnread(), 2.0, 1e-12) && Passed;
    Passed = Evaluate("GetUInt8()", "representative second byte", "Reading a representative second byte advances the position", static_cast<unsigned int>(Store.GetUInt8()), 20U) && Passed;
    Store.ProgressPosition(-1);
    Passed = Evaluate("ProgressPosition()", "representative backward step", "ProgressPosition can move a representative read position backward within bounds", static_cast<unsigned int>(Store.GetUInt8()), 20U) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(1);
    Store.AddUInt8(2);
    Store.AddUInt8(3);
    Passed = Evaluate("GetUInt8()", "truncate representative first byte", "Truncate is exercised after a representative first read", static_cast<unsigned int>(Store.GetUInt8()), 1U) && Passed;
    Store.Truncate();
    Passed = EvaluateNear("Truncate()", "representative remaining size", "Truncate removes already-read representative bytes from the front", Store.GetArraySize(), 2.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "representative remaining unread size", "Truncate resets the unread representative size to the remaining bytes", Store.GetArraySizeUnread(), 2.0, 1e-12) && Passed;
    Passed = Evaluate("GetUInt8()", "representative byte after truncate", "Truncate keeps the next representative unread byte at the front", static_cast<unsigned int>(Store.GetUInt8()), 2U) && Passed;
  }

  {
    MBinaryStore Store;
    Store.Truncate();
    Passed = EvaluateNear("Truncate()", "empty representative store", "Truncate is a no-op on a representative store that was never written to", Store.GetArraySize(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "empty representative store", "Truncate keeps the unread size at zero on a representative empty store", Store.GetArraySizeUnread(), 0.0, 1e-12) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(1);
    Store.AddUInt8(2);
    Passed = Evaluate("GetUInt8()", "truncate fully-consumed representative first byte", "A representative fully-consumed truncate case reads the first byte before truncation", static_cast<unsigned int>(Store.GetUInt8()), 1U) && Passed;
    Passed = Evaluate("GetUInt8()", "truncate fully-consumed representative second byte", "A representative fully-consumed truncate case reads the second byte before truncation", static_cast<unsigned int>(Store.GetUInt8()), 2U) && Passed;
    Store.Truncate();
    Passed = EvaluateNear("Truncate()", "fully-consumed representative store", "Truncate removes all bytes once a representative store is fully consumed", Store.GetArraySize(), 0.0, 1e-12) && Passed;
    Passed = EvaluateNear("GetArraySizeUnread()", "fully-consumed representative store", "Truncate resets the unread size to zero on a representative fully-consumed store", Store.GetArraySizeUnread(), 0.0, 1e-12) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetUInt16()", "representative short read past end", "Reading a representative 16-bit value past the available bytes throws", [&]() { Store.GetUInt16(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetInt8()", "representative read from empty store", "Reading a representative signed byte from an empty store throws", [&]() { Store.GetInt8(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetUInt8()", "representative read from empty store", "Reading a representative unsigned byte from an empty store throws", [&]() { Store.GetUInt8(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetInt16()", "representative short signed short read past end", "Reading a representative signed 16-bit value past the available bytes throws", [&]() { Store.GetInt16(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(1);
    Store.AddUInt8(2);
    Store.AddUInt8(3);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetInt32()", "representative short signed int read past end", "Reading a representative signed 32-bit value past the available bytes throws", [&]() { Store.GetInt32(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(1);
    Store.AddUInt8(2);
    Store.AddUInt8(3);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetUInt32()", "representative short unsigned int read past end", "Reading a representative unsigned 32-bit value past the available bytes throws", [&]() { Store.GetUInt32(); }) && Passed;
  }

  {
    MBinaryStore Store;
    for (unsigned int i = 0; i < 7; ++i) Store.AddUInt8(i);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetInt64()", "representative short signed long long read past end", "Reading a representative signed 64-bit value past the available bytes throws", [&]() { Store.GetInt64(); }) && Passed;
  }

  {
    MBinaryStore Store;
    for (unsigned int i = 0; i < 7; ++i) Store.AddUInt8(i);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetUInt64()", "representative short unsigned long long read past end", "Reading a representative unsigned 64-bit value past the available bytes throws", [&]() { Store.GetUInt64(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(1);
    Store.AddUInt8(2);
    Store.AddUInt8(3);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetFloat()", "representative short float read past end", "Reading a representative float past the available bytes throws", [&]() { Store.GetFloat(); }) && Passed;
  }

  {
    MBinaryStore Store;
    for (unsigned int i = 0; i < 7; ++i) Store.AddUInt8(i);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetDouble()", "representative short double read past end", "Reading a representative double past the available bytes throws", [&]() { Store.GetDouble(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetChar()", "representative char read from empty store", "Reading a representative char from an empty store throws", [&]() { Store.GetChar(); }) && Passed;
  }

  {
    MBinaryStore Store;
    for (unsigned int i = 0; i < 11; ++i) Store.AddUInt8(i);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetTime()", "representative short time read past end", "Reading a representative int64-plus-int32 time past the available bytes throws", [&]() { Store.GetTime(); }) && Passed;
  }

  {
    MBinaryStore Store;
    for (unsigned int i = 0; i < 7; ++i) Store.AddUInt8(i);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("GetTimeUInt64()", "representative short compact time read past end", "Reading a representative compact uint64 time past the available bytes throws", [&]() { Store.GetTimeUInt64(); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Passed = EvaluateException<out_of_range>("GetArrayValue()", "representative out-of-bounds access", "Array-value access rejects representative indices outside the stored byte range", [&]() { Store.GetArrayValue(1); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("ProgressPosition()", "representative negative step past start", "ProgressPosition rejects representative backward moves before the start of the store", [&]() { Store.ProgressPosition(-1); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Store.ProgressPosition(0);
    Passed = Evaluate("ProgressPosition()", "representative zero step on non-empty store", "ProgressPosition accepts a representative zero move on a non-empty store without changing the read position", static_cast<unsigned int>(Store.GetUInt8()), 5U) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("ProgressPosition()", "representative zero step on empty store", "ProgressPosition rejects a representative zero move on an empty store because the current position is already at end", [&]() { Store.ProgressPosition(0); }) && Passed;
  }

  {
    MBinaryStore Store;
    Store.AddUInt8(5);
    Passed = EvaluateException<MExceptionIndexOutOfBounds>("ProgressPosition()", "representative forward step past end", "ProgressPosition rejects representative forward moves beyond the end of the store", [&]() { Store.ProgressPosition(1); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionValueOutOfBounds>("AddNormalizedVectorInt16()", "representative component outside [-1,1]", "Normalized int16 vectors reject representative components outside the allowed range", [&]() { Store.AddNormalizedVectorInt16(MVector(1.1, 0.0, 0.0)); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionValueOutOfBounds>("AddNormalizedVectorInt16()", "representative y component outside [-1,1]", "Normalized int16 vectors reject representative y components outside the allowed range", [&]() { Store.AddNormalizedVectorInt16(MVector(0.0, -1.1, 0.0)); }) && Passed;
  }

  {
    MBinaryStore Store;
    Passed = EvaluateException<MExceptionValueOutOfBounds>("AddNormalizedVectorInt16()", "representative z component outside [-1,1]", "Normalized int16 vectors reject representative z components outside the allowed range", [&]() { Store.AddNormalizedVectorInt16(MVector(0.0, 0.0, 1.1)); }) && Passed;
  }

  return Passed;
}


////////////////////////////////////////////////////////////////////////////////


int main()
{
  UTBinaryStore Test;
  return !Test.Run();
}
