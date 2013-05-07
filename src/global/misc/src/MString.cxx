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

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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


vector<MString> MString::Tokenize(const MString& Delimeter)  const
{ 
  MString S;
  vector<MString> T;
  size_t OldPos = 0; 
  size_t NewPos = 0; 
  while ((NewPos = m_String.find(Delimeter.m_String, OldPos)) != string::npos) {
    S = MString(m_String.substr(OldPos, NewPos-OldPos));
    T.push_back(S);
    OldPos = NewPos + Delimeter.Length();
  }
  if (Length() > OldPos) {
    S = MString(m_String.substr(OldPos, Length() - OldPos));
    T.push_back(S);
  }

  return T; 
}


////////////////////////////////////////////////////////////////////////////////


MString& MString::ReplaceAll(const MString& From, const MString& To)
{
  if (From.Length() == 0) return *this;
  
  size_t OldPos = 0; 
  size_t NewPos = 0; 
  
  while ((NewPos = m_String.find(From.m_String, OldPos)) != string::npos) {
    m_String.replace(NewPos, From.Length(), To.m_String);
    OldPos = NewPos + To.Length();
  }
  
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
