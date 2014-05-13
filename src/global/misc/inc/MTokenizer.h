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
#include <MString.h>
#include <TArrayI.h>
#include <TArrayD.h>

// MEGAlib libs:
#include "MGlobal.h"

// Standard libs:
#include <cstdlib>
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MTokenizer
{
  // public interface:
 public:
  //! Default constructor
  MTokenizer();
  //! Standard constructor
  //! Separator is the separator between keywords
  //! When AllowComposed == true, then elements with "." such as "Sphere.Source" 
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  MTokenizer(const char Separator, const bool AllowComposed = true);
  //! Standard constructor which automatically analyses the text
  //! Separator is the separator between keywords
  //! When AllowComposed == true, then elements with "." such as "Sphere.Source" 
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  MTokenizer(const MString& Text, const char Separator = ' ', const bool AllowComposed = true);
  virtual ~MTokenizer();

  //! Set the separator between the tokens (default: space)
  void SetSeparator(const char Separator);
  //! When true, then elements with "." such as "Sphere.Source" 
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  void AllowComposed(const bool Composed);

  //! Split the text into tokens
  bool Analyze(MString Text, const bool AllowMaths = true);
  //! Same as Analyse
  bool Analyse(MString Text, const bool AllowMaths = true) { return Analyze(Text, AllowMaths); }

  //! Return a copy of the original text
  MString GetText() const;

  //! Return the number of tokens
  unsigned int GetNTokens() const;
  //! Check if the given Token is at position i
  bool IsTokenAt(const unsigned int i, const MString& Token, const bool IgnoreCase = false) const;
  //! Check if the irst token is composed i.e. something like "Sphere.Source"
  bool IsComposited() const;

  //! Return the token at i as string
  MString GetTokenAt(const unsigned int i) const;

  //! Return the token at i as string --- return "" in case of error
  MString GetTokenAtAsString(const unsigned int i) const;
  //! Return all token AT AND AFTER i as string --- return "" in case of error
  MString GetTokenAfterAsString(const unsigned int i) const;
  //! Return the token at i as double --- return 0 in case of error
  double GetTokenAtAsDouble(const unsigned int i) const;
  //! Return the token at i as float --- return 0 in case of error
  float GetTokenAtAsFloat(const unsigned int i) const;
  //! Return the token at i as int --- return 0 in case of error
  int GetTokenAtAsInt(const unsigned int i) const;
  //! Return the token at i as unsigned int --- return 0 in case of error
  unsigned int GetTokenAtAsUnsignedInt(const unsigned int i) const;
  //! Return the token at i as long --- return 0 in case of error
  long GetTokenAtAsLong(const unsigned int i) const;
  //! Return the token at i as unsigned long --- return 0 in case of error
  unsigned long GetTokenAtAsUnsignedLong(const unsigned int i) const;
  //! Return the token at i as double --- return FALSE in case of error
  bool GetTokenAtAsBoolean(const unsigned int i) const;
  //! Return the token AT AND AFTER i as ROOT TArrayI --- return 0 in case of error
  TArrayI GetTokenAtAsIntArray(const unsigned int i) const;
  //! Return the token AT AND AFTER i as ROOT TArrayD --- return 0 in case of error
  TArrayD GetTokenAtAsDoubleArray(const unsigned int i) const;
  //! Return the token AT AND AFTER i as vector of doubles --- return empty array in case of error
  vector<double> GetTokenAtAsDoubleVector(const unsigned int i) const;
  //! Return the token AT AND AFTER i as vector of floats --- return empty array in case of error
  vector<float> GetTokenAtAsFloatVector(const unsigned int i) const;

  //! Evaluate the math functions in the given string
  static bool EvaluateMaths(MString& Token);
  //! Check if the string contains a math environment "{ ... }"
  static bool IsMaths(const MString& Token);

  //! Return a diagnostics string of the data content
  MString ToString();


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
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  bool m_AllowComposed;
  //! Return whether the first token was composed e.g. "Sphere.Source"
  bool m_Composited;

#ifdef ___CINT___
 public:
  ClassDef(MTokenizer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
