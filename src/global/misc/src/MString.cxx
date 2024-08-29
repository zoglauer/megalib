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
#include <locale>
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
  //! Test if the string is a number - ignores whitespaces at beginning and end
  
  
  istringstream In(m_String);
  
  double Number;
  In>>skipws>>Number; // ignore white spaces at the beginning
  
  //! Check for whitespaces at the end
  char c;
  while (!In.eof() && !In.fail()) {
    c = In.get();
    if (In.eof()) return true;
    if (c != ' ') return false;
  }
  
  // If nothing is left and the string is still good, then we are good
  return In.eof() && !In.fail(); 
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
