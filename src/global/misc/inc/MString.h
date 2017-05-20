/*
 * MString.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MString__
#define __MString__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <string>
#include <cstdlib>
#include <vector>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <sstream>
#include <iostream>
using namespace std;

// ROOT libs:

// MEGAlib libs:

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MString
{
  // public interface:
 public:
  //! Deafult constructor
  MString() {};
  // Copy constructor
  MString(const MString& String) : m_String(String.m_String) {}
  // Move constructor
  MString(MString&& String) : m_String(move(String.m_String)) {} 
  MString(const string& S) { m_String = S; }
  MString(const ostringstream& S) { m_String = S.str(); }
  MString(const char* S) { if (S!= nullptr) m_String = S; }
  //! Copy first n characters of S
  MString(const char* S, unsigned int N) { if (S != nullptr) m_String = string(S, N); /* slower: m_String.assign(S, N); */ }
  MString(const char S) { m_String = S; }
  MString(const short i) { operator+=(i); }
  MString(const unsigned short i) { operator+=(i); }
  MString(const int i) { operator+=(i); }
  MString(const unsigned int i) { operator+=(i); }
  MString(const long i) { operator+=(i); }
  MString(const unsigned long i) { operator+=(i); }
  MString(const float i) { operator+=(i); }
  MString(const double i) { operator+=(i); }
  virtual ~MString() {}

  // Move
  MString& operator=(MString&& String) { m_String = move(String.m_String); return *this; }
  
  // Assignment: 
  MString& operator=(const MString& S) { m_String = S.m_String; return *this; }
  MString& operator=(char S) { m_String = S; return *this; }
  MString& operator=(const char* S) { if (S != nullptr) m_String = S; return *this; }
  //MString& operator=(const TString& S) { m_String = S; return *this; }
  MString& operator=(const string& S) { m_String = S; return *this; }

  //MString& Format(const char* Format, ...);

  //char& operator[](size_t i) { return m_String[i]; }  
  //char operator[](size_t i) const { return m_String[i]; } 

  char& operator[](int i) { return m_String[i]; } 
  char operator[](int i) const { return m_String[i]; } 

  char& operator[](unsigned int i) { return m_String[i]; } 
  char operator[](unsigned int i) const { return m_String[i]; } 

  char& operator[](long i) { return m_String[i]; } 
  char operator[](long i) const { return m_String[i]; } 

  char& operator[](unsigned long i) { return m_String[i]; } 
  char operator[](unsigned long i) const { return m_String[i]; } 


  // Access:
  
  //! Access to the internal c-string -- do not delete the array since it is the MString internal data 
  const char* Data() const { return m_String.c_str(); }
  //! Return a copy of the internal c++-string
  string GetString() const { return m_String; }
  //! Access to the internal c++-string -- do not delete the array since it is the MString internal data 
  string& GetStringRef() { return m_String; }
  //! Return true if the string is empty
  bool IsEmpty() const { return m_String.empty(); }
  //! Return the length of the string
  size_t Length() const { return m_String.size(); }
  
  //! Return a substring
  MString GetSubString(size_t Start, size_t Size) const { if (Start >= Length()) return ""; return m_String.substr(Start, Size); }
  //! Return a substring starting at start with the remaining length of the string
  MString GetSubString(size_t Start) const { if (Start >= Length()) return ""; return m_String.substr(Start, m_String.size() - Start); }

  //! Get position of last character or string::npos if none is found
  size_t Last(char c) const { return m_String.find_last_of(c); }
  //! Get position of last character or string::npos if none is found
  size_t First(char c) const { return m_String.find(c); }

  size_t Index(const MString& S, size_t Start = 0) const { return m_String.find(S.m_String, Start); }

  // Conversion:

  operator const char*() const { return m_String.c_str(); }
  // operator string() const { return m_String; } // VC++ 10 has problem with this
  // operator TString() const { return TString(m_String.c_str()); } // ROOT has problems with this
  string ToString() const { return m_String; } 
  int ToInt() const { return atoi(m_String.c_str()); } 
  double ToDouble() const { return atof(m_String.c_str()); }


  // Comparison


  //! Is lower operator for sorting by name
  bool operator<(const MString& N) const;  
  
  //! Fast comparison especially in case of ignoring the case
  bool AreIdentical(const MString& S, bool IgnoreCase = false) const;

  // Modify

  //! Remove all content and thus set the size to zero
  void Clear() { m_String.clear(); }

  void AppendInPlace(const char* S) { m_String += S; }
  MString& Append(const char* S) { m_String += S; return *this; }
  void AppendInPlace(MString& S) { m_String += S.m_String; }
  MString& Append(MString& S) { m_String += S.m_String; return *this; }
  void PrependInPlace(const char* S) { m_String.insert(0, S); }
  MString& Prepend(const char* S) { m_String.insert(0, S); return *this; }
  void PrependInPlace(MString& S) { m_String.insert(0, S.m_String); }
  MString& Prepend(MString& S) { m_String.insert(0, S.m_String); return *this; }

  MString& operator+=(char S) { m_String += S; return *this; }
  MString& operator+=(const char* S) { m_String += S; return *this; }
  MString& operator+=(const MString& S) { m_String += S.m_String; return *this; }
  MString& operator+=(const string& S) { m_String += S; return *this; }
  MString& operator+=(short N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(unsigned short N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(int N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(unsigned int N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(long N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(unsigned long N) { ostringstream out; out<<N; m_String += out.str(); return *this; }
  MString& operator+=(float N) { ostringstream out; out.precision(8); out<<N; m_String += out.str(); return *this; }
  MString& operator+=(double N) { ostringstream out; out.precision(15); out<<N; m_String += out.str(); return *this; }


  //! Remove all characters from Start to the End
  void RemoveInPlace(size_t Start) { if (Start >= Length()) return; m_String.erase(Start, Length() - Start); }
  //! Remove all characters from Start to the End and return the new string
  MString& Remove(size_t Start) { if (Start >= Length()) return *this; m_String.erase(Start, Length() - Start); return *this; }
  //! Remove all characters from Start with the given length
  void RemoveInPlace(size_t Start, size_t Size) { if (Start >= Length()) return; m_String.erase(Start, Size); }
  //! Remove all characters from Start with the given length and return the new string
  MString& Remove(size_t Start, size_t Size) { if (Start >= Length()) return *this; m_String.erase(Start, Size); return *this; }
  //! Remove the last N characters from the string and return the new string
  //! If the string is smaller than Last, remove everything
  void RemoveLastInPlace(size_t Size) { if (Length() < Size) Size = Length(); m_String.erase(Length() - Size, Size); }
  //! Remove the last N characters from the string and return the new string
  //! If the string is smaller than Last, remove everything
  MString& RemoveLast(size_t Size) { if (Length() < Size) Size = Length(); m_String.erase(Length() - Size, Size); return *this; }

  //! Replace the characters starting at Start with Size with the string S
  void ReplaceInPlace(size_t Start, size_t Size, const MString& S) { if (Start >= Length()) return; m_String.replace(Start, Size, S.m_String); }
  //! Replace the characters starting at Start with Size with the string S and return the new string
  MString& Replace(size_t Start, size_t Size, const MString& S) { if (Start >= Length()) return *this; m_String.replace(Start, Size, S.m_String); return *this; }

  //! Replace all occurances of from with string
  void ReplaceAllInPlace(const MString& From, const MString& To); 
  //! Replace all occurances of from with string and return the new string
  MString& ReplaceAll(const MString& From, const MString& To); 
  
  //! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string
  void ReplaceAtEndInPlace(const MString& From, const MString& To); 
  //! Replace the string "From" at the end of the string with "To", if "From" is at the end of the string and return the new string
  MString& ReplaceAtEnd(const MString& From, const MString& To); 
  
  //! Remove all occurances of From 
  void RemoveAllInPlace(const MString& From);
  //! Remove all occurances of From and return the new string
  MString& RemoveAll(const MString& From);
  
  void StripFrontInPlace(const char S = ' ') { while (Length() > 0 && m_String[0] == S) m_String.erase(0, 1); }
  MString& StripFront(const char S = ' ') { StripFrontInPlace(S); return *this; }
  void StripBackInPlace(const char S = ' ') { while (Length() > 0 && m_String[Length()-1] == S) m_String.erase(Length()-1, 1); }
  MString& StripBack(const char S = ' ') { StripBackInPlace(S); return *this; }
  void StripInPlace(const char S = ' ') { StripFront(S); StripBack(S); }
  MString& Strip(const char S = ' ') { StripInPlace(S); return *this; }
  
  void ToLowerInPlace() { for (size_t p = 0; p < m_String.size(); ++p) m_String[p] = std::tolower(m_String[p]); } 
  MString& ToLower() { ToLowerInPlace(); return *this; } 
  void ToUpperInPlace() { for (size_t p = 0; p < m_String.size(); ++p) m_String[p] = std::toupper(m_String[p]); } 
  MString& ToUpper() { ToUpperInPlace(); return *this; } 
  
  
  
  //! Read one line into this string replace all previous content
  istream& ReadLine(istream& in);
  //! Read one line into this string replace all previous content
  void Read(istream& in);
  //! Return a hash value of the string
  long GetHash() const;
  
  // Analyze
  
  //! Return true if the string contains the given string
  bool Contains(const MString& S) const { return (m_String.find(S.m_String) != string::npos); }
  //! Return true if the string begins with the given string
  bool BeginsWith(const MString& S) const; 
  //! Return true if the string end with the given string
  bool EndsWith(const MString& S) const; 
  
  //! Split the string in substring using the delimeter
  vector<MString> Tokenize(const MString& Delimeter, bool IgnoreEmpty = true) const;

  //! Test if the string is a number -- ignores whitespace at beginning and end
  bool IsNumber() const;
  
  
  static const size_t npos;
  
  // protected methods:
 protected:

  // private methods:
 private:
   //! This function ONLY exists to prevent the compiler to make a built-in one --- DO NOT USE
  //const char* operator[](int) { return 0; } 



  // protected members:
 protected:


  // private members:
 private:
  //! The string
  string m_String;


#ifdef ___CINT___
 public:
  ClassDef(MString, 0) // no description
#endif

};

istream& operator>>(istream& in, MString& S);
ostream& operator<<(ostream& out, const MString& S);

inline bool operator==(const MString& S1, const MString& S2) { return ((S1.GetString() == S2.GetString()) ? true : false); }
inline bool operator==(const MString& S1, const char* S2) { return ((S1.GetString() == S2) ? true : false); }
inline bool operator==(const char* S1, const MString& S2) { return ((S1 == S2.GetString()) ? true : false); }

inline bool operator!=(const MString& S1, const MString& S2) { return ((S1.GetString() != S2.GetString()) ? true : false); }
inline bool operator!=(const MString& S1, const char* S2) { return ((S1.GetString() != S2) ? true : false); }
inline bool operator!=(const char* S1, const MString& S2) { return ((S1 != S2.GetString()) ? true : false); }

inline MString operator+(const MString& S1, const MString& S2) { return MString(S1.GetString() + S2.GetString()); }
inline MString operator+(const MString& S1,  const char* S2) { return MString(S1.GetString() + S2); }
inline MString operator+(const char* S1, const MString& S2) { return MString(S1) + S2; }
inline MString operator+(const MString& S1, char S2) { return MString(S1.GetString() + S2); }
inline MString operator+(char S1, const MString& S2) { return MString(S1) + S2; }
inline MString operator+(const MString& S1, int N) { MString S(S1); S += N; return S; }
inline MString operator+(int N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, unsigned int N) { MString S(S1); S += N; return S; }
inline MString operator+(unsigned int N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, short N) { MString S(S1); S += N; return S; }
inline MString operator+(short N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, unsigned short N) { MString S(S1); S += N; return S; }
inline MString operator+(unsigned short N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, long N) { MString S(S1); S += N; return S; }
inline MString operator+(long N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, unsigned long N) { MString S(S1); S += N; return S; }
inline MString operator+(unsigned long N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, float N) { MString S(S1); S += N; return S; }
inline MString operator+(float N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }
inline MString operator+(const MString& S1, double N) { MString S(S1); S += N; return S; }
inline MString operator+(double N, const MString& S2) { ostringstream out; out<<N; MString S = out.str(); return S + S2; }


#endif


////////////////////////////////////////////////////////////////////////////////
