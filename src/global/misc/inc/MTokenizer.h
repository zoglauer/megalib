/*
 * MTokenizer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTokenizer__
#define __MTokenizer__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TArrayI.h>
#include <TArrayD.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MTime.h"
// Standard libs:
#include <cstdlib>
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! A tokenizer for parsing MEGAlib-style text lines into typed tokens
class MTokenizer
{
  // public interface:
 public:
  //! Default constructor
  MTokenizer();
  //! Standard constructor
  //! Separator is the separator between keywords
  //! When AllowComposed == true, then elements with "." such as "Sphere.Source"
  //! are split into the two tokens "Sphere" and "Source", otherwise they remain one token
  MTokenizer(const char Separator, const bool AllowComposed = true);
  //! Standard constructor which automatically analyzes the text
  //! Separator is the separator between keywords
  //! When AllowComposed == true, then elements with "." such as "Sphere.Source"
  //! are split into the two tokens "Sphere" and "Source", otherwise they remain one token
  MTokenizer(const MString& Text, const char Separator = ' ', const bool AllowComposed = true);
  //! Default destructor
  virtual ~MTokenizer();

  //! Set the separator between the tokens (default: space)
  void SetSeparator(const char Separator);
  //! When true, then elements with "." such as "Sphere.Source"
  //! are split into the two tokens "Sphere" and "Source", otherwise they remain one token
  void AllowComposed(const bool Composed);

  //! Split the text into tokens assuming only whitespace-separated tokens
  bool AnalyzeFast(MString Text);

  //! Split the text into tokens including math mode and combined first tokens with "."
  bool Analyze(MString Text, const bool AllowMaths = true);
  //! Same as Analyze
  bool Analyse(MString Text, const bool AllowMaths = true) { return Analyze(Text, AllowMaths); }

  //! Check all tokens if the maths is OK
  bool CheckAllMaths();
  
  //! Return a copy of the original text
  MString GetText() const;

  //! Return the number of tokens
  unsigned int GetNTokens() const;
  //! Return true if Token is at position i
  bool IsTokenAt(const unsigned int i, const MString& Token, const bool IgnoreCase = false) const;
  //! Return true if the first token is composed, i.e. something like "Sphere.Source"
  bool IsComposited() const;

  //! Return the token at i as string
  MString GetTokenAt(const unsigned int i) const;

  //! Return the token at i as string, or "" in case of error
  MString GetTokenAtAsString(const unsigned int i) const;
  //! Return all tokens at and after i as one string, or "" in case of error
  MString GetTokenAfterAsString(const unsigned int i) const;
  //! Return the token at i as double, or 0 in case of error
  double GetTokenAtAsDouble(const unsigned int i) const;
  //! Return the token at i as float, or 0 in case of error
  float GetTokenAtAsFloat(const unsigned int i) const;
  //! Return the token at i as int, or 0 in case of error
  int GetTokenAtAsInt(const unsigned int i) const;
  //! Return the token at i as unsigned int, or 0 in case of error
  unsigned int GetTokenAtAsUnsignedInt(const unsigned int i) const;
  //! Return the token at i as unsigned int using a fast digit-only parser
  //! The token must consist of ASCII digits only, otherwise the result is undefined
  unsigned int GetTokenAtAsUnsignedIntFast(const unsigned int i) const;
  //! Return the token at i as long, or 0 in case of error
  long GetTokenAtAsLong(const unsigned int i) const;
  //! Return the token at i as unsigned long, or 0 in case of error
  unsigned long GetTokenAtAsUnsignedLong(const unsigned int i) const;
  //! Return the token at i as boolean, or false in case of error
  bool GetTokenAtAsBoolean(const unsigned int i) const;
  //! Return the token at i as MTime, or MTime(0) in case of error
  MTime GetTokenAtAsTime(const unsigned int i) const;
  //! Return the tokens at and after i as ROOT TArrayI, or an empty array in case of error
  TArrayI GetTokenAtAsIntArray(const unsigned int i) const;
  //! Return the tokens at and after i as ROOT TArrayD, or an empty array in case of error
  TArrayD GetTokenAtAsDoubleArray(const unsigned int i) const;
  //! Return the tokens at and after i as vector of doubles, or an empty vector in case of error
  vector<double> GetTokenAtAsDoubleVector(const unsigned int i) const;
  //! Return the tokens at and after i as vector of floats, or an empty vector in case of error
  vector<float> GetTokenAtAsFloatVector(const unsigned int i) const;
  //! Return the tokens at and after i as vector of ints, or an empty vector in case of error
  vector<int> GetTokenAtAsIntVector(const unsigned int i) const;
  //! Return the tokens at and after i as vector of strings, or an empty vector in case of error
  //! The boolean indicates that the strings are enclosed in quotation marks
  vector<MString> GetTokenAtAsStringVector(const unsigned int i, bool StringsAreInQuotationMarks = false) const;
  //! Return the tokens at and after i as vector of unsigned ints, or an empty vector in case of error
  vector<unsigned int> GetTokenAtAsUnsignedIntVector(const unsigned int i) const;

  //! Evaluate the math functions in the given string
  static bool EvaluateMaths(MString& Token);
  //! Check if the string contains a math environment "{ ... }"
  static bool IsMaths(const MString& Token);
  //! Check if the maths is correct
  static bool CheckMaths(const MString& Token);

  //! Return a diagnostics string of the data content
  MString ToString();
  //! Return a compact version of the tokenized content
  MString ToCompactString();
  

  // protected methods:
 protected:
  //! Initializes all member variables to default values
  void Init();
  //! Resets the data of the tokenizer, but leaves m_Separator and m_AllowComposed unchanged
  void Reset();

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! The original text
  MString m_Text;
  //! The original text split into tokens
  vector<MString> m_Tokens;

  //! The separator e.g. a space
  char m_Separator;

  //! When true, then elements with "." such as "Sphere.Source"
  //! are split into the two tokens "Sphere" and "Source", otherwise they remain one token
  bool m_AllowComposed;
  //! True if the first token was composed, e.g. "Sphere.Source"
  bool m_Composited;

#ifdef ___CLING___
 public:
  ClassDef(MTokenizer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
