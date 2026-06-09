/*
 * MString.cxx
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
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MString)
#endif


////////////////////////////////////////////////////////////////////////////////


const size_t MString::npos = string::npos;


////////////////////////////////////////////////////////////////////////////////


istream& operator>>(istream& in, MString& S) 
{ 
  in>>S.GetStringRef(); 
  return in; 
}


////////////////////////////////////////////////////////////////////////////////


ostream& operator<<(ostream& out, const MString& S) 
{ 
  out<<S.GetString(); 
  return out; 
}


////////////////////////////////////////////////////////////////////////////////


MString::MString(const double D, unsigned int Precision)
{
  //! Construct with double of given precision

  ostringstream in;
  in<<setprecision(Precision)<<D;

  m_String = in.str();
}


////////////////////////////////////////////////////////////////////////////////


MString::MString(double Value, double Uncertainty, MString Units, bool Latex)
{
  //! Construct from value, uncertainty, unit using scientific rounding
  //! Will be something like (12.345, 1.872, "mm") -> "(12.3 +- 1.9) mm"

  // Find at which position the first digit is, e.g.:
  // 187.2 = 2, 3.23 = 0, 0.348 = -1, etc.
  int RoundOffDigit = int(log10(Uncertainty));
  if (Uncertainty < 1) RoundOffDigit -= 1; // Due to how rounding using int() works

  // If the uncertainty starts with 1 & 2, we have one more significant digit.
  if (round(Uncertainty * pow(10, -RoundOffDigit)) < 3) RoundOffDigit -= 1;

  // Now round
  double Scale = pow(10, -RoundOffDigit);
  Value = round(Value * Scale) / Scale;
  Uncertainty = round(Uncertainty * Scale) / Scale;

  // Print the result
  ostringstream out;
  if (Units != "") {
    if (Latex == true) {
      out<<"("<<Value<<" #pm "<<Uncertainty<<") "<<Units;
    } else {
      out<<"("<<Value<<" ± "<<Uncertainty<<") "<<Units;
    }
  } else {
    if (Latex == true) {
      out<<Value<<" #pm "<<Uncertainty;
    } else {
      out<<Value<<" ± "<<Uncertainty;
    }
  }

  m_String = out.str();
}

////////////////////////////////////////////////////////////////////////////////


bool MString::operator<(const MString& N) const
{
  // Is lower operator for sorting by name

  if (m_String.compare(N.m_String) < 0) return true; 
  
  return false;
}

  
////////////////////////////////////////////////////////////////////////////////


bool MString::AreIdentical(const MString& S, bool IgnoreCase) const
{
  // Fast comparison especially in case of ignoring the case

  if (m_String.size() != S.Length()) return false;

  // The strings are compared from back for speed reason in Geomega
  // Do not change!
  if (IgnoreCase == true) {
    size_t Size = m_String.size();
    for (unsigned int l = Size-1; l < Size; --l) {
      if (tolower(m_String[l]) != tolower(S[l])) return false;
    }
    return true;
  } else {
    size_t Size = m_String.size();
    for (unsigned int l = Size-1; l < Size; --l) {
      if (m_String[l] != S[l]) return false;
    }
    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MString> MString::Tokenize(const MString& Delimeter, bool IgnoreEmpty)  const
{ 
  MString S;
  vector<MString> T;
  size_t OldPos = 0; 
  size_t NewPos = 0; 
  while ((NewPos = m_String.find(Delimeter.m_String, OldPos)) != string::npos) {
    S = MString(m_String.substr(OldPos, NewPos-OldPos));
    if (IgnoreEmpty == false || (IgnoreEmpty == true && S != "")) { 
      T.push_back(S);
    }
    OldPos = NewPos + Delimeter.Length();
  }
  if (Length() > OldPos) {
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
  size_t PosBefore = Index(Before) + Before.Length();
  size_t PosAfter = Index(After, PosBefore);
  
  return GetSubString(PosBefore, PosAfter-PosBefore);
}


////////////////////////////////////////////////////////////////////////////////


void MString::ReplaceAllInPlace(const MString& From, const MString& To)
{
  if (From.Length() == 0) return;
  
  size_t OldPos = 0; 
  size_t NewPos = 0; 
  
  while ((NewPos = m_String.find(From.m_String, OldPos)) != string::npos) {
    m_String.replace(NewPos, From.Length(), To.m_String);
    OldPos = NewPos + To.Length();
  }
}


////////////////////////////////////////////////////////////////////////////////


MString& MString::ReplaceAll(const MString& From, const MString& To)
{
  ReplaceAllInPlace(From, To);
  return *this;
} 

////////////////////////////////////////////////////////////////////////////////


//! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string
void MString::ReplaceAtEndInPlace(const MString& From, const MString& To)
{
  if (EndsWith(From) == false) return;
  
  m_String.erase(Length() - From.Length(), From.Length());
  m_String += To.GetString();
}


////////////////////////////////////////////////////////////////////////////////


//! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string and return the new string
MString& MString::ReplaceAtEnd(const MString& From, const MString& To)
{
  ReplaceAtEndInPlace(From, To);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MString::RemoveAllInPlace(const MString& From)
{
  if (From.Length() == 0) return;
  
  size_t OldPos = 0; 
  size_t NewPos = 0; 
  
  while ((NewPos = m_String.find(From.m_String, OldPos)) != string::npos) {
    m_String.erase(NewPos, From.Length());
    OldPos = NewPos;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString& MString::RemoveAll(const MString& S)
{
  RemoveAllInPlace(S);
  return *this;
}


////////////////////////////////////////////////////////////////////////////////


bool MString::BeginsWith(const MString& S) const
{
  if (Length() < S.Length()) return false;
  return (S == GetSubString(0, S.Length()));
}
 

////////////////////////////////////////////////////////////////////////////////


bool MString::EndsWith(const MString& S) const
{
  if (Length() < S.Length()) return false;
  return (S == GetSubString(Length()-S.Length(), S.Length()));
}


////////////////////////////////////////////////////////////////////////////////


istream& MString::ReadLine(istream& in)
{
  //! Read one line into this string replace all previous content

  Clear();
  getline(in, m_String);

  return in;
}


////////////////////////////////////////////////////////////////////////////////


void MString::Read(istream& in)
{
  //! Read one line into this string replace all previous content

  Clear();
  
  string Line;
  while (getline(in, Line)) {
    m_String += Line;
    m_String += "\n";
  }
}


////////////////////////////////////////////////////////////////////////////////


long MString::GetHash() const
{
  //! Return a hash value

  locale loc;
  const collate<char>& coll = use_facet<collate<char> >(loc);
  
  return coll.hash(m_String.data(), m_String.data() + m_String.length());
}


////////////////////////////////////////////////////////////////////////////////


bool MString::IsNumber() const
{
  //! Test if the string is zero or a finite normal double - ignores all leading and trailing whitespace (spaces, tabs, newlines, etc.)

  istringstream In(m_String);

  double Number;
  In>>skipws>>Number; // ignore white spaces at the beginning
  if (In.fail() == true) return false;

  //! Reject nonzero subnormal values so only the fully supported normal double range and zero are accepted
  if (Number != 0.0 && fabs(Number) < numeric_limits<double>::min()) return false;

  //! Ignore white spaces at the end
  In>>ws;

  //! If nothing is left, then the complete string is a number
  if (In.eof() == false) return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MString::IsFloatingPointNumber() const
{
  //! Test if the string is a floating-point number - ignores all leading and trailing whitespace (spaces, tabs, newlines, etc.)

  //! Floating-point numbers must contain a decimal point or an exponent
  if (m_String.find('.') == string::npos &&
      m_String.find('e') == string::npos &&
      m_String.find('E') == string::npos) {
    return false;
  }

  //! Check if the complete string is a number
  return IsNumber();
}


////////////////////////////////////////////////////////////////////////////////


bool MString::AreNumbersNumericallyMatching(const MString& Number, unsigned int MaximumLastDigitDifference) const
{
  //! Test if two floating-point numbers match within a tolerance based on their printed precision

  //! Both strings must be floating-point representations
  if (IsFloatingPointNumber() == false) return false;
  if (Number.IsFloatingPointNumber() == false) return false;

  //! Identical floating-point representations always match
  if (*this == Number) return true;

  //! Return the value represented by the last printed digit
  //! For example, 0.87370 returns 0.00001, 0.8737 returns 0.0001, and 1.234e-4 returns 0.0000001
  //! Thus, the smaller returned unit belongs to the number with the higher printed precision
  auto GetPrintedFloatingPointUnit = [](const MString& Value) {
    //! Extract the single numeric token while discarding all leading and trailing stream whitespace
    istringstream TokenStream(Value.ToString());
    string Token;
    TokenStream >> Token;

    //! Separate mantissa and exponent so that the exponent can shift the last printed digit
    //! For example, the last digit of 1.234e-4 represents 10^(-4-3) = 10^-7
    const size_t ExponentPosition = Token.find_first_of("eE");
    const string Mantissa = ExponentPosition == string::npos ? Token : Token.substr(0, ExponentPosition);
    int Exponent = 0;
    if (ExponentPosition != string::npos) {
      const string ExponentToken = Token.substr(ExponentPosition + 1);
      try {
        Exponent = stoi(ExponentToken);
      } catch (const out_of_range&) {
        //! Exponents outside the integer range already force a double power to zero or infinity
        return ExponentToken[0] == '-' ? 0.0 : numeric_limits<double>::infinity();
      }
    }

    //! Without a decimal point, the mantissa has no printed fractional digits
    const size_t DecimalPosition = Mantissa.find('.');
    if (DecimalPosition == string::npos) return pow(10.0, Exponent);

    //! Each printed fractional digit reduces the unit by one power of ten
    const double FractionDigits = static_cast<double>(Mantissa.size() - DecimalPosition - 1);
    return pow(10.0, static_cast<double>(Exponent) - FractionDigits);
  };

  //! The numbers need to be finite (not NaN or infinite)
  const double ThisValue = ToDouble();
  const double OtherValue = Number.ToDouble();
  if (isfinite(ThisValue) == false) return false;
  if (isfinite(OtherValue) == false) return false;

  //! Use the smaller unit so both numbers are compared at the higher printed precision
  //! For example, 0.8737 is treated as 0.87370 when compared with 0.87365
  const double ThisPrintedUnit = GetPrintedFloatingPointUnit(*this);
  const double OtherPrintedUnit = GetPrintedFloatingPointUnit(Number);
  const double SmallerPrintedUnit = min(ThisPrintedUnit, OtherPrintedUnit);
  const double MaximumDifference = MaximumLastDigitDifference * SmallerPrintedUnit;

  const double Difference = fabs(ThisValue - OtherValue);
  //! Subtracting finite values with large opposite signs can overflow to infinity
  if (isfinite(Difference) == false) return false;

  //! Account for round-off introduced by parsing, subtraction, and tolerance calculation
  //! Scale the allowance to the values and tolerance involved in this comparison
  //! Four machine epsilons cover the small number of floating-point operations without imposing an absolute floor near zero
  const double ComparisonScale = max({fabs(ThisValue), fabs(OtherValue), MaximumDifference});
  const double RoundOff = 4 * numeric_limits<double>::epsilon() * ComparisonScale;
  if (Difference > MaximumDifference + RoundOff) return false;

  return true;
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
