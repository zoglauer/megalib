/*
 * MString.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MString
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MString.h"

// Standard libs:
#include <algorithm>
#include <cmath>
#include <limits>
#include <locale>
#include <stdexcept>
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MExceptions.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MString)
#endif


////////////////////////////////////////////////////////////////////////////////


const size_t MString::npos = string::npos;


////////////////////////////////////////////////////////////////////////////////


//! Read one white-space separated word from the stream
istream& operator>>(istream& in, MString& S)
{
  in >> S.GetStringRef();
  return in;
}


////////////////////////////////////////////////////////////////////////////////


//! Write the string to the stream
ostream& operator<<(ostream& out, const MString& S)
{
  out << S.GetString();
  return out;
}


////////////////////////////////////////////////////////////////////////////////


//! Construct with a double of the given number of significant digits
MString::MString(const double D, unsigned int Precision)
{
  // Construct with double of given precision

  ostringstream in;
  in << setprecision(Precision) << D;

  m_String = in.str();
}


////////////////////////////////////////////////////////////////////////////////


//! Construct from value, uncertainty, and unit using scientific rounding
//! Will be something like (12.345, 1.872, "mm") -> "(12.3 +- 1.9) mm"
MString::MString(double Value, double Uncertainty, MString Units, bool Latex)
{
  // Without a usable uncertainty (zero, negative, or not finite) the numbers are printed unrounded
  bool Round = true;
  if (Uncertainty <= 0 || isfinite(Uncertainty) == false || isfinite(Value) == false) {
    Round = false;
  }

  int RoundOffDigit = 0;
  double ValueInDigits = 0;
  double UncertaintyInDigits = 0;
  if (Round == true) {
    // Find at which position the first digit is, e.g.:
    // 187.2 = 2, 3.23 = 0, 0.348 = -1, etc.
    RoundOffDigit = static_cast<int>(log10(Uncertainty));
    // A value below 1 needs one more digit, since int() rounds towards zero
    if (Uncertainty < 1) {
      RoundOffDigit -= 1;
    }

    // If the uncertainty starts with 1 & 2, we have one more significant digit.
    if (round(Uncertainty * pow(10, -RoundOffDigit)) < 3) {
      RoundOffDigit -= 1;
    }

    // Round - unless the scale or a rounded number overflows (tiny uncertainties, huge values)
    const double Scale = pow(10, -RoundOffDigit);
    // The numbers in units of the round-off digit
    ValueInDigits = round(Value * Scale);
    UncertaintyInDigits = round(Uncertainty * Scale);
    // Above 2^53 in units of the round-off digit there is nothing left to round, and the division would only add errors
    double RoundedValue = Value;
    if (fabs(Value * Scale) < 9007199254740992.0) {
      // Adding 0.0 turns a negative zero into a zero
      RoundedValue = round(Value * Scale) / Scale + 0.0;
    }
    const double RoundedUncertainty = round(Uncertainty * Scale) / Scale;
    if (isfinite(Scale) == false || isfinite(Value * Scale) == false || isfinite(RoundedValue) == false || isfinite(RoundedUncertainty) == false) {
      Round = false;
    } else {
      Value = RoundedValue;
      Uncertainty = RoundedUncertainty;
    }
  }

  // Rounded numbers are printed with the decimals of the round-off digit, so that no digit above it is lost
  MString ValueText;
  MString UncertaintyText;
  if (Round == true) {
    int Decimals = 0;
    if (RoundOffDigit < 0) {
      Decimals = -RoundOffDigit;
    }
    ostringstream ValueStream;
    ostringstream UncertaintyStream;
    ValueStream << fixed << setprecision(Decimals) << Value;
    UncertaintyStream << fixed << setprecision(Decimals) << Uncertainty;
    // Above the decimal point the rounded number is the digits followed by zeros - the scaled-back double would show its binary value
    if (RoundOffDigit > 0 && fabs(ValueInDigits) < 9007199254740992.0) {
      ValueStream.str("");
      ValueStream << fixed << setprecision(0) << ValueInDigits + 0.0;
      if (ValueInDigits != 0) {
        ValueStream << string(RoundOffDigit, '0');
      }
    }
    if (RoundOffDigit > 0 && fabs(UncertaintyInDigits) < 9007199254740992.0) {
      UncertaintyStream.str("");
      UncertaintyStream << fixed << setprecision(0) << UncertaintyInDigits;
      UncertaintyStream << string(RoundOffDigit, '0');
    }
    ValueText = ValueStream.str();
    UncertaintyText = UncertaintyStream.str();
  } else {
    // Unrounded numbers are printed with 15 digits, or with 17 if 15 do not read back as the same number (e.g. the largest double)
    auto GetUnroundedText = [](double Number) {
      ostringstream Text;
      Text << setprecision(numeric_limits<double>::digits10) << Number;
      if (strtod(Text.str().c_str(), nullptr) != Number) {
        ostringstream Longer;
        Longer << setprecision(numeric_limits<double>::max_digits10) << Number;
        return Longer.str();
      }
      return Text.str();
    };
    ValueText = GetUnroundedText(Value);
    UncertaintyText = GetUnroundedText(Uncertainty);
  }

  MString Separator = " ± ";
  if (Latex == true) {
    Separator = " #pm ";
  }

  // The rounded form is put in parentheses if there is a unit
  if (Units != "" && Round == true) {
    m_String = "(" + ValueText + Separator + UncertaintyText + ") " + Units;
  } else if (Units != "") {
    m_String = ValueText + Separator + UncertaintyText + " " + Units;
  } else {
    m_String = ValueText + Separator + UncertaintyText;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to a long integer
long MString::ToLong() const
{
  try {
    return stol(m_String.c_str());
  } catch (const std::invalid_argument&) {
    throw MExceptionArbitrary(MString("ToLong: \"") + m_String + "\" is not a number");
  } catch (const std::out_of_range&) {
    throw MExceptionParameterOutOfRange(ToDouble(), static_cast<double>(numeric_limits<long>::min()), static_cast<double>(numeric_limits<long>::max()), "ToLong");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to an unsigned integer
unsigned int MString::ToUnsignedInt() const
{
  // The value must neither be negative nor silently truncated
  if (HasLeadingMinus() == true) {
    throw MExceptionParameterOutOfRange(ToDouble(), 0, static_cast<double>(numeric_limits<unsigned int>::max()), "ToUnsignedInt");
  }

  unsigned long Value = 0;
  try {
    Value = stoul(m_String.c_str());
  } catch (const std::invalid_argument&) {
    throw MExceptionArbitrary(MString("ToUnsignedInt: \"") + m_String + "\" is not a number");
  } catch (const std::out_of_range&) {
    throw MExceptionParameterOutOfRange(ToDouble(), 0, static_cast<double>(numeric_limits<unsigned int>::max()), "ToUnsignedInt");
  }
  if (Value > numeric_limits<unsigned int>::max()) {
    throw MExceptionParameterOutOfRange(static_cast<double>(Value), 0, static_cast<double>(numeric_limits<unsigned int>::max()), "ToUnsignedInt");
  }

  return static_cast<unsigned int>(Value);
}


////////////////////////////////////////////////////////////////////////////////


//! Convert to an unsigned long integer
unsigned long MString::ToUnsignedLong() const
{
  // stoul wraps a negative number around, so reject it
  if (HasLeadingMinus() == true) {
    throw MExceptionParameterOutOfRange(ToDouble(), 0, static_cast<double>(numeric_limits<unsigned long>::max()), "ToUnsignedLong");
  }

  try {
    return stoul(m_String.c_str());
  } catch (const std::invalid_argument&) {
    throw MExceptionArbitrary(MString("ToUnsignedLong: \"") + m_String + "\" is not a number");
  } catch (const std::out_of_range&) {
    throw MExceptionParameterOutOfRange(ToDouble(), 0, static_cast<double>(numeric_limits<unsigned long>::max()), "ToUnsignedLong");
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Lexicographic order, used for sorting
bool MString::operator<(const MString& N) const
{
  // Is lower operator for sorting by name

  if (m_String.compare(N.m_String) < 0) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Compare two strings, optionally ignoring the case
bool MString::AreIdentical(const MString& S, bool IgnoreCase) const
{
  // Fast comparison especially in case of ignoring the case

  if (m_String.size() != S.Length()) {
    return false;
  }

  // The strings are compared from back for speed reason in Geomega
  // Do not change!
  if (IgnoreCase == true) {
    size_t Size = m_String.size();
    for (size_t l = Size-1; l < Size; --l) {
      if (tolower(static_cast<unsigned char>(m_String[l])) != tolower(static_cast<unsigned char>(S[l]))) {
        return false;
      }
    }
    return true;
  } else {
    size_t Size = m_String.size();
    for (size_t l = Size-1; l < Size; --l) {
      if (m_String[l] != S[l]) {
        return false;
      }
    }
    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Split the string at the delimiter
vector<MString> MString::Tokenize(const MString& Delimiter, bool IgnoreEmpty)  const
{
  if (Delimiter.Length() == 0) {
    vector<MString> Tokens;
    if (IgnoreEmpty == false || IsEmpty() == false) {
      Tokens.push_back(*this);
    }
    return Tokens;
  }

  MString S;
  vector<MString> T;
  size_t OldPos = 0;
  size_t NewPos = 0;
  while ((NewPos = m_String.find(Delimiter.m_String, OldPos)) != string::npos) {
    S = MString(m_String.substr(OldPos, NewPos-OldPos));
    if (IgnoreEmpty == false || (IgnoreEmpty == true && S != "")) {
      T.push_back(S);
    }
    OldPos = NewPos + Delimiter.Length();
  }
  if (Length() > OldPos || (IgnoreEmpty == false && OldPos == Length())) {
    S = MString(m_String.substr(OldPos, Length() - OldPos));
    if (IgnoreEmpty == false || (IgnoreEmpty == true && S != "")) {
      T.push_back(S);
    }
  }

  return T;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the string between two strings
MString MString::Extract(MString Before, MString After)
{
  size_t BeforeStart = Index(Before);
  if (BeforeStart == string::npos) {
    return "";
  }

  size_t PosBefore = BeforeStart + Before.Length();
  size_t PosAfter = Index(After, PosBefore);
  if (PosAfter == string::npos) {
    return "";
  }

  return GetSubString(PosBefore, PosAfter-PosBefore);
}


////////////////////////////////////////////////////////////////////////////////


//! Replace all occurrences of From with To in place
void MString::ReplaceAllInPlace(const MString& From, const MString& To)
{
  if (From.Length() == 0) {
    return;
  }

  // The arguments can be this string itself, so work on copies
  const string Pattern = From.m_String;
  const string Replacement = To.m_String;

  size_t OldPos = 0;
  size_t NewPos = 0;

  while ((NewPos = m_String.find(Pattern, OldPos)) != string::npos) {
    m_String.replace(NewPos, Pattern.size(), Replacement);
    OldPos = NewPos + Replacement.size();
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Replace all occurrences of From with To and return the new string
MString& MString::ReplaceAll(const MString& From, const MString& To)
{
  ReplaceAllInPlace(From, To);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string
void MString::ReplaceAtEndInPlace(const MString& From, const MString& To)
{
  if (EndsWith(From) == false) {
    return;
  }

  // The arguments can be this string itself, so work on copies
  const size_t PatternLength = From.Length();
  const string Replacement = To.m_String;

  m_String.erase(Length() - PatternLength, PatternLength);
  m_String += Replacement;
}


////////////////////////////////////////////////////////////////////////////////


//! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string and return the new string
MString& MString::ReplaceAtEnd(const MString& From, const MString& To)
{
  ReplaceAtEndInPlace(From, To);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Remove all occurrences of From in place
void MString::RemoveAllInPlace(const MString& From)
{
  if (From.Length() == 0) {
    return;
  }

  // The pattern can be this string itself, so work on a copy
  const string Pattern = From.m_String;

  size_t OldPos = 0;
  size_t NewPos = 0;

  while ((NewPos = m_String.find(Pattern, OldPos)) != string::npos) {
    m_String.erase(NewPos, Pattern.size());
    OldPos = NewPos;
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Remove all occurrences of From and return the new string
MString& MString::RemoveAll(const MString& S)
{
  RemoveAllInPlace(S);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string begins with the given string
bool MString::BeginsWith(const MString& S) const
{
  if (Length() < S.Length()) {
    return false;
  }
  if (S != GetSubString(0, S.Length())) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string ends with the given string
bool MString::EndsWith(const MString& S) const
{
  if (Length() < S.Length()) {
    return false;
  }
  if (S != GetSubString(Length()-S.Length(), S.Length())) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string is a non-negative base-10 integer
bool MString::IsPositiveInteger() const
{
  // Accept a non-negative base-10 integer with optional surrounding
  // whitespace and an optional leading plus sign. Reject empty strings,
  // whitespace-only strings, minus signs, decimal points, and trailing text.

  // Strip leading spaces.
  size_t Begin = 0;
  while (Begin < m_String.size() && isspace(static_cast<unsigned char>(m_String[Begin])) != 0) {
    ++Begin;
  }

  // Strip trailing spaces.
  size_t End = m_String.size();
  while (End > Begin && isspace(static_cast<unsigned char>(m_String[End-1])) != 0) {
    --End;
  }

  // Reject empty or whitespace-only strings.
  if (Begin == End) {
    return false;
  }

  // Allow one optional leading plus sign.
  if (m_String[Begin] == '+') {
    ++Begin;
    if (Begin == End) {
      return false;
    }
  }

  // The remaining content must be decimal digits only.
  for (size_t i = Begin; i < End; ++i) {
    if (isdigit(static_cast<unsigned char>(m_String[i])) == 0) {
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Read one line into this string and replace all previous content
istream& MString::ReadLine(istream& in)
{

  Clear();
  getline(in, m_String);

  return in;
}


////////////////////////////////////////////////////////////////////////////////


//! Read all lines of the stream into this string and replace all previous content
void MString::Read(istream& in)
{

  Clear();

  string Line;
  while (getline(in, Line)) {
    m_String += Line;
    m_String += "\n";
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Return a hash value of the string
long MString::GetHash() const
{

  locale loc;
  const collate<char>& coll = use_facet<collate<char> >(loc);

  return coll.hash(m_String.data(), m_String.data() + m_String.length());
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string is zero or a finite normal double
bool MString::IsNumber() const
{
  // Test if the string is zero or a finite normal double - ignores all leading and trailing whitespace (spaces, tabs, newlines, etc.)

  istringstream In(m_String);

  double Number;
  In>>skipws>>Number; // ignore white spaces at the beginning

  if (In.fail() == true) {
    return false;
  }

  // Reject values which underflow to zero, e.g. 1e-400: libc++ flags them as failure, but libstdc++ silently returns 0
  if (Number == 0.0) {
    string Mantissa = m_String.substr(0, m_String.find_first_of("eE"));
    if (Mantissa.find_first_of("123456789") != string::npos) {
      return false;
    }
  }

  // Reject nonzero subnormal values so only the fully supported normal double range and zero are accepted
  if (Number != 0.0 && fabs(Number) < numeric_limits<double>::min()) {
    return false;
  }

  // Ignore white spaces at the end
  In>>ws;

  // If nothing is left, then the complete string is a number
  if (In.eof() == false) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the string is a floating-point number
bool MString::IsFloatingPointNumber() const
{
  // Test if the string is a floating-point number - ignores all leading and trailing whitespace (spaces, tabs, newlines, etc.)

  // Floating-point numbers must contain a decimal point or an exponent
  if (m_String.find('.') == string::npos &&
      m_String.find('e') == string::npos &&
      m_String.find('E') == string::npos) {
    return false;
  }

  // Check if the complete string is a number
  return IsNumber();
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if both floating-point numbers match within the tolerance given in units of the last printed digit
bool MString::AreNumbersNumericallyMatching(const MString& Number, unsigned int MaximumLastDigitDifference) const
{
  // Test if two floating-point numbers match within a tolerance based on their printed precision

  // Both strings must be floating-point representations
  if (IsFloatingPointNumber() == false) {
    return false;
  }
  if (Number.IsFloatingPointNumber() == false) {
    return false;
  }

  // Identical floating-point representations always match
  if (*this == Number) {
    return true;
  }

  // Return the value represented by the last printed digit
  // For example, 0.87370 returns 0.00001, 0.8737 returns 0.0001, and 1.234e-4 returns 0.0000001
  // Thus, the smaller returned unit belongs to the number with the higher printed precision
  auto GetPrintedFloatingPointUnit = [](const MString& Value) {
    // Extract the single numeric token while discarding all leading and trailing stream whitespace
    istringstream TokenStream(Value.ToString());
    string Token;
    TokenStream >> Token;

    // Separate mantissa and exponent so that the exponent can shift the last printed digit
    // For example, the last digit of 1.234e-4 represents 10^(-4-3) = 10^-7
    const size_t ExponentPosition = Token.find_first_of("eE");
    string Mantissa = Token;
    if (ExponentPosition != string::npos) {
      Mantissa = Token.substr(0, ExponentPosition);
    }
    int Exponent = 0;
    if (ExponentPosition != string::npos) {
      const string ExponentToken = Token.substr(ExponentPosition + 1);
      try {
        Exponent = stoi(ExponentToken);
      } catch (const out_of_range&) {
        // Exponents outside the integer range already force a double power to zero or infinity
        if (ExponentToken[0] == '-') {
          return 0.0;
        }
        return numeric_limits<double>::infinity();
      }
    }

    // Without a decimal point, the mantissa has no printed fractional digits
    const size_t DecimalPosition = Mantissa.find('.');
    if (DecimalPosition == string::npos) {
      return pow(10.0, Exponent);
    }

    // Each printed fractional digit reduces the unit by one power of ten
    const double FractionDigits = static_cast<double>(Mantissa.size() - DecimalPosition - 1);
    return pow(10.0, static_cast<double>(Exponent) - FractionDigits);
  };

  // The numbers need to be finite (not NaN or infinite)
  const double ThisValue = ToDouble();
  const double OtherValue = Number.ToDouble();
  if (isfinite(ThisValue) == false) {
    return false;
  }
  if (isfinite(OtherValue) == false) {
    return false;
  }

  // Use the smaller unit so both numbers are compared at the higher printed precision
  // For example, 0.8737 is treated as 0.87370 when compared with 0.87365
  const double ThisPrintedUnit = GetPrintedFloatingPointUnit(*this);
  const double OtherPrintedUnit = GetPrintedFloatingPointUnit(Number);
  const double SmallerPrintedUnit = min(ThisPrintedUnit, OtherPrintedUnit);
  const double MaximumDifference = MaximumLastDigitDifference * SmallerPrintedUnit;

  const double Difference = fabs(ThisValue - OtherValue);
  // Subtracting finite values with large opposite signs can overflow to infinity
  if (isfinite(Difference) == false) {
    return false;
  }

  // Account for round-off introduced by parsing, subtraction, and tolerance calculation
  // Scale the allowance to the values and tolerance involved in this comparison
  // Four machine epsilons cover the small number of floating-point operations without imposing an absolute floor near zero
  const double ComparisonScale = max({fabs(ThisValue), fabs(OtherValue), MaximumDifference});
  const double RoundOff = 4 * numeric_limits<double>::epsilon() * ComparisonScale;
  if (Difference > MaximumDifference + RoundOff) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return true if the first character which is not white space is a minus sign
bool MString::HasLeadingMinus() const
{
  const size_t First = m_String.find_first_not_of(" \t\n\r\f\v");
  if (First != string::npos && m_String[First] == '-') {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


/*
MString& MString::Format(const char* Format, ...) 
{ 
  va_list ap;
  va_start(ap, Format);
  
  int TextSize = 1024;
  char* Text = new char[TextSize];
  
  TextSize = vsnprintf(Text, TextSize, Format, ap);
  if (TextSize > 0) {
    delete [] Text;
    TextSize += 1;
    Text = new char[TextSize];
    TextSize = vsnprintf(Text, TextSize, Format, ap);
  }  
   
  m_String = Text;
  delete [] Text;
  
  va_end(ap);

  return *this;
}
*/


////////////////////////////////////////////////////////////////////////////////


// MString.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
