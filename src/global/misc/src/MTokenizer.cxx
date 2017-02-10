/*
 * MTokenizer.cxx
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
// MTokenizer
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTokenizer.h"

// Standard libs:
#include <iomanip>

// ROOT libs:
#include "TFormula.h"
#include "MString.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MTokenizer)
#endif


////////////////////////////////////////////////////////////////////////////////


MTokenizer::MTokenizer()
{
  // Construct a object of type MTokenizer

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MTokenizer::MTokenizer(const char Separator, const bool AllowComposed)
{
  // Construct a object of type MTokenizer and analyse the token
  // Separator is the separator between keywords
  // When AllowComposed == true, then elements with "." such as "Sphere.Source" 
  // are split in the two tokens "Sphere" and "Source", otherwise it is one token 

  Init();
  m_Separator = Separator;
  m_AllowComposed = AllowComposed;
}


////////////////////////////////////////////////////////////////////////////////


MTokenizer::MTokenizer(const MString& Text, const char Separator, const bool AllowComposed)
{
  // Construct a object of type MTokenizer and analyse the token
  // Separator is the separator between keywords
  // When AllowComposed == true, then elements with "." such as "Sphere.Source" 
  // are split in the two tokens "Sphere" and "Source", otherwise it is one token 

  Init();
  m_Separator = Separator;
  m_AllowComposed = AllowComposed;

  Analyse(Text);
}


////////////////////////////////////////////////////////////////////////////////


MTokenizer::~MTokenizer()
{
  // default destructor
}


////////////////////////////////////////////////////////////////////////////////


void MTokenizer::Init()
{
  // Action common to all constructors:

  m_Composited = false;
  m_AllowComposed = true;
  m_Separator = ' ';
}


////////////////////////////////////////////////////////////////////////////////


void MTokenizer::SetSeparator(const char Separator)
{
  // Set the type of separator (space, semicolon...)
  m_Separator = Separator;
}


////////////////////////////////////////////////////////////////////////////////


void MTokenizer::AllowComposed(const bool Composed)
{
  // Allow composed tokens like Bla.blabla
  m_AllowComposed = Composed;
}


////////////////////////////////////////////////////////////////////////////////


void MTokenizer::Reset()
{
  // Action common to all constructors:

  m_Tokens.clear();
  m_Composited = false;
}


////////////////////////////////////////////////////////////////////////////////


MString MTokenizer::GetText() const
{
  // Return the base text which has been analysed

  return m_Text;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTokenizer::GetNTokens() const
{
  // Return the number of tokens

  return m_Tokens.size();
}


////////////////////////////////////////////////////////////////////////////////


MString MTokenizer::GetTokenAt(const unsigned int i) const
{
  // Return the token at position i

  if (i < m_Tokens.size()) {
    return m_Tokens[i];
  } else {
    mlog<<"MString MTokenizer::GetTokenAt(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return MString("");
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MTokenizer::GetTokenAtAsString(const unsigned int i) const
{
  // Return the token at position i as MString

  // Error protection is done in GetTokenAt(int i)
  return GetTokenAt(i);
}


////////////////////////////////////////////////////////////////////////////////


MString MTokenizer::GetTokenAfterAsString(const unsigned int i) const
{
  // Return the token at position i and higher as MString

  MString T;
  for (unsigned int j = i; j < m_Tokens.size(); j++) {
    if (j < m_Tokens.size()-1) {
      T += GetTokenAt(j) + " ";
    } else {
      T += GetTokenAt(j);
    }
  }

  return T;
}


////////////////////////////////////////////////////////////////////////////////


double MTokenizer::GetTokenAtAsDouble(const unsigned int i) const
{
  // Return the token at position i as double 


  if (i < m_Tokens.size()) {
    return atof(m_Tokens[i]);
  } else {
    mlog<<"MString MTokenizer::GetTokenAtAsDouble(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    abort();
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


float MTokenizer::GetTokenAtAsFloat(const unsigned int i) const
{
  // Return the token at position i as double 


  if (i < m_Tokens.size()) {
    return atof(m_Tokens[i]);
  } else {
    mlog<<"MString MTokenizer::GetTokenAtAsFloat(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


int MTokenizer::GetTokenAtAsInt(const unsigned int i) const
{
  // Return the token at position i as integer


  if (i < m_Tokens.size()) {
    return int(atof(m_Tokens[i])); // atoi(m_Tokens[i]); is not working in cases such as 2.35e+02
  } else {
    mlog<<"int MTokenizer::GetTokenAtAsInt(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTokenizer::GetTokenAtAsUnsignedIntFast(const unsigned int i) const
{
  // Return the token at position i as unsigned integer
  
  if (i < m_Tokens.size()) {
    // From: http://stackoverflow.com/questions/16826422/c-most-efficient-way-to-convert-string-to-int-faster-than-atoi
    uint32_t val = 0;
    const char* str = m_Tokens[i].Data();
    while (*str) {
      val = (val << 1) + (val << 3) + *(str++) - 48;
    }
    return val;
  } else {
    mlog<<"int MTokenizer::GetTokenAtAsUnsignedInt(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MTokenizer::GetTokenAtAsUnsignedInt(const unsigned int i) const
{
  // Return the token at position i as unsigned integer
  
  if (i < m_Tokens.size()) {
    int Value = int(atof(m_Tokens[i]));
    if (Value < 0) {
      mlog<<"int MTokenizer::GetTokenAtAsUnsignedInt(int i)"<<endl;
      mlog<<"Value is negative!!!"<<endl;
      return 0;
    }
    return (unsigned int) Value;
  } else {
    mlog<<"int MTokenizer::GetTokenAtAsUnsignedInt(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


long MTokenizer::GetTokenAtAsLong(const unsigned int i) const
{
  // Return the token at position i as long integer

  if (i < m_Tokens.size()) {
    return atol(m_Tokens[i]);
  } else {
    mlog<<"long MTokenizer::GetTokenAtAsLong(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


unsigned long MTokenizer::GetTokenAtAsUnsignedLong(const unsigned int i) const
{
  // Return the token at position i as long integer

  if (i < m_Tokens.size()) {
    return strtoul(m_Tokens[i], NULL, 0);
  } else {
    mlog<<"long MTokenizer::GetTokenAtAsLong(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


TArrayI MTokenizer::GetTokenAtAsIntArray(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  if (i < m_Tokens.size()) {
    TArrayI A(m_Tokens.size() - i);
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A[j-i] = GetTokenAtAsInt(j);
    }
    return A;
  } else {
    mlog<<"TArrayI MTokenizer::GetTokenAtAsIntArray(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return TArrayI(0);
  }
}


////////////////////////////////////////////////////////////////////////////////


TArrayD MTokenizer::GetTokenAtAsDoubleArray(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  if (i < m_Tokens.size()) {
    TArrayD A(m_Tokens.size() - i);
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A[j-i] = GetTokenAtAsDouble(j);
    }
    return A;
  } else {
    mlog<<"TArrayD MTokenizer::GetTokenAtAsDoubleArray(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return TArrayD(0);
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<double> MTokenizer::GetTokenAtAsDoubleVector(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  vector<double> A;
  if (i < m_Tokens.size()) {
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A.push_back(GetTokenAtAsDouble(j));
    }
    return A;
  } else {
    mlog<<"vector<double> MTokenizer::GetTokenAtAsDoubleVector(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return A;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<float> MTokenizer::GetTokenAtAsFloatVector(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  vector<float> A;
  if (i < m_Tokens.size()) {
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A.push_back(GetTokenAtAsFloat(j));
    }
    return A;
  } else {
    mlog<<"vector<float> MTokenizer::GetTokenAtAsFloatVector(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return A;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<int> MTokenizer::GetTokenAtAsIntVector(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  vector<int> A;
  if (i < m_Tokens.size()) {
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A.push_back(GetTokenAtAsInt(j));
    }
    return A;
  } else {
    mlog<<"vector<int> MTokenizer::GetTokenAtAsIntVector(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return A;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<unsigned int> MTokenizer::GetTokenAtAsUnsignedIntVector(const unsigned int i) const
{
  // Return all tokens from position i to end as integer array

  vector<unsigned int> A;
  if (i < m_Tokens.size()) {
    for (unsigned int j = i; j < m_Tokens.size(); ++j) {
      A.push_back(GetTokenAtAsUnsignedInt(j));
    }
    return A;
  } else {
    mlog<<"vector<unsigned int> MTokenizer::GetTokenAtAsUnsignedIntVector(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return A;
  }
}


////////////////////////////////////////////////////////////////////////////////


vector<MString> MTokenizer::GetTokenAtAsStringVector(const unsigned int i, bool StringsAreInQuotationMarks) const
{
  //! Return the token AT AND AFTER i as vector of strings --- return empty array in case of error
  
  vector<MString> A;
  if (i < m_Tokens.size()) {
    if (StringsAreInQuotationMarks == false) {
      for (unsigned int j = i; j < m_Tokens.size(); ++j) {
        A.push_back(GetTokenAtAsString(j));
      }
    } else {
      bool Started = false;
      for (unsigned int j = i; j < m_Tokens.size(); ++j) {
        MString Current = GetTokenAtAsString(j);
        if (Started == false) {
          if (Current.BeginsWith("\"") == false) {
            mlog<<"The string tokens are not correctly enclosed in quotation marks! You will be missing data!"<<endl;
            A.clear();
            return A;
          } else {
            if (Current.EndsWith("\"") == false) {
              Started = true;
            }
            Current.RemoveAllInPlace("\"");
            A.push_back(Current);
          }
        } else {
          if (Current.EndsWith("\"") == true) {
            Started = false;
            Current.RemoveAllInPlace("\"");
          }
          A.back() += " ";
          A.back() += Current;
        }
      }  
      if (Started == true) {
        mlog<<"The string tokens are not correctly enclosed in quotation marks! You will be missing data!"<<endl;
        A.clear();
        return A;
      }
    }
    return A;
  } else {
    mlog<<"vector<unsigned int> MTokenizer::GetTokenAtAsUnsignedIntVector(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return A;
  }  
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::GetTokenAtAsBoolean(const unsigned int i) const
{
  // Return the token at position i as boolean

  if (i < m_Tokens.size()) {
    MString Token = m_Tokens[i];
    Token.ToLower();
    if (Token == "false") {
      return false;
    } else if (Token == "true") {
      return true;
    }
    return (Token.ToInt() == 0) ? false : true;
  } else {
    mlog<<"bool MTokenizer::GetTokenAtAsBoolean(int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MTime MTokenizer::GetTokenAtAsTime(const unsigned int i) const
{
  //! Return the token at i as MTime --- return MTime(0) in case of error

  if (i < m_Tokens.size()) {
    MTime Time;
    Time.Set(m_Tokens[i]);
    return Time;
  } else {
    mlog<<"bool MTokenizer::GetTokenAtAsTime(const unsigned int i): "<<endl;
    if (m_Tokens.size() > 0) {
      mlog<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")\n";
      mlog<<"The text line was: \""<<m_Text<<"\""<<endl;
    } else {
      mlog<<"The Tokenizer is empty!"<<endl;
    }
    return MTime(0);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::IsTokenAt(const unsigned int i, const MString& Token, bool const IgnoreCase) const
{
  // Return true, if Token is identical with the token at position i

  if (m_Tokens.size() == 0) return false;

  if (i < m_Tokens.size()) {
    if (IgnoreCase == true) {
      MString LowerCase = Token;
      LowerCase.ToLower();
      MString Original = m_Tokens[i];
      Original.ToLower();
      return (LowerCase == Original) ? true : false;
    } else {
      return (Token == m_Tokens[i]) ? true : false;
    }
  } else {
    merr<<"Index ("<<i<<") out of bounds (min=0, max="<<m_Tokens.size()-1<<")"<<endl;
    mout<<"The text line was: \""<<m_Text<<"\""<<endl;
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::IsComposited() const
{
  // Return true if the first token was composited, i.e. like "ACS.Material"

  return m_Composited;
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::AnalyzeFast(MString Text)
{
  // Split the Text into its tokens

  // Reset();
  unsigned int ExistingTokens = m_Tokens.size();
  unsigned int NewTokens = 0;
  
  m_Text = Text;

  int Length = Text.Length();
  const char* pText = Text.Data();

  int TokenStart = 0;
  int TokenLength = 0;
  for (int i = 0; i < Length; ++i) {
        
    if (pText[i] != m_Separator && pText[i] != '\t' && pText[i] != '\n' && pText[i] != '\0') { 
      TokenLength++;
      if (i < Length-1) continue;
    }

    // Now we have a separator and data

    if (TokenLength == 0) {
      TokenStart = i+1;
      continue;
    } else { // Token length > 0
      if (TokenLength > 1 && pText[TokenStart] == '/' && pText[TokenStart+1] == '/') {
        // Standard comment
        break;
      } else if (pText[TokenStart] == '#') {
        // Standard comment
        break;
      } else {
        // Now we have a good token.
        if (NewTokens < ExistingTokens) {
          //m_Tokens[NewTokens] = Text.GetSubString(TokenStart, TokenLength);
          m_Tokens[NewTokens] = MString(&pText[TokenStart], TokenLength);
        } else {
          //m_Tokens.push_back(Text.GetSubString(TokenStart, TokenLength));
          m_Tokens.push_back(MString(&pText[TokenStart], TokenLength));
        }
        ++NewTokens;
        //m_Tokens.push_back(MString(&pText[TokenStart], TokenLength));
      }
    }

    if (pText[i] == '\n' || pText[i] == '\0') {
      break;
    }
    TokenStart = i+1;
    TokenLength = 0;
  }

  if (m_Tokens.size() != NewTokens) m_Tokens.resize(NewTokens);
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::Analyze(MString Text, const bool AllowMaths)
{
  // Split the Text into its tokens
  // White space is the prime delimiter, then the first token is splitted
  // by a point as delimiter
  // Text surrounded by [...] is one token - splitted tokens are not recognized
  // Text surrounded by {...} is considered as equation - splitted tokens are not recognized

  //cout<<Text<<endl;

  Reset();
  m_Text = Text;

  // Remove white spaces at the beginning of the line:
  Text.StripFrontInPlace(' ');
  Text.StripBackInPlace(' ');
  Text.RemoveAll("\r");

  MString Token;
  bool IsFirstToken = true;
  int MathDepth = 0;
  int MathMatch = 0;

  int Length = Text.Length();
  const char* pText = Text.Data();

  const char* pToken = 0;
  int TokenStart = 0;
  int TokenLength = 0;
  for (int i = 0; i < Length; ++i) {

    // # or ! at the beginning is always a comment
    if (pText[0] == '#' || pText[0] == '!') {
      break;
    }

    if (pText[i] == '{') {
      MathDepth++;
      MathMatch++;
    }
    if (pText[i] == '}') {
      MathMatch--;
    }

    if (TokenLength > 0 && pText[i-TokenLength] == '[') {
      TokenLength++;
      if (i < Length-1 && pText[i] != ']') continue;
    } else if (TokenLength > 0 && pText[i-TokenLength] == '{') {
      TokenLength++;
      if (i < Length-1 && pText[i] != '}') {
        continue;
      } else if (i < Length-1 && pText[i] == '}') {
        MathDepth--;
        if (MathDepth != 0) continue;
      }
    } else if (pText[i] != m_Separator && pText[i] != '\t' && pText[i] != '\n' && pText[i] != '\0') { 
      TokenLength++;
      if (i < Length-1) continue;
    }

    // Now we have to consider several cases:
    // (a) The token is empty --> skip this token
    // (b) The token starts with '//', then this AND all following tokens are comment
    // (c) The token starts with [, then it ends with ]
    // (d) the good ones

    Token = Text.GetSubString(TokenStart, TokenLength);
    pToken = Token.Data();

    //cout<<"|"<<Token<<"|"<<endl;

    if (TokenLength == 0) {
      TokenStart = i+1;
      continue;
    } else if (TokenLength > 1 && (pToken[0] == '/' && pToken[1] == '/')) {
      // Standard comment
      break;
    } else if (TokenLength > 1 && (pToken[0] == '#' && pToken[1] == '#')) {
      // Standard comment
      break;
    } else if (TokenLength > 0 && pToken[0] == '[' && pToken[TokenLength-1] == ']') {
      m_Tokens.push_back(Token);
      IsFirstToken = false;
    } else if (TokenLength > 0 && pToken[0] == '{' && pToken[TokenLength-1] == '}') {
      if (AllowMaths == true) {
        if (EvaluateMaths(Token) == false) {
          merr<<"Unable to scan math token \""<<Token<<"\"correctly!"<<endl;
          return false;
        }
      }
      m_Tokens.push_back(Token);
      IsFirstToken = false;
    } else {
      // Now we have a good token.
      // If it is the first token, then split it again by a '.' (dot)
      //mlog<<"Good: "<<Token<<endl;
      //cout<<"Composed"<<(int) IsFirstToken<<"!"<<(int) m_AllowComposed<<endl;
      if (IsFirstToken == true && m_AllowComposed == true) {
        IsFirstToken = false;
        if (Token.Contains('.') == true) {
          // Add the token before the dot ...
          m_Tokens.push_back(Token.GetSubString(0, Token.First('.')));
          // after the dot ...
          m_Tokens.push_back(Token.GetSubString(Token.First('.')+1, Token.Length()));
          // This was a composited first token
          m_Composited = false;
        } else {
          m_Tokens.push_back(Token);
          //mlog<<"Added: "<<Token<<endl;
        }
      } else {
        m_Tokens.push_back(Token);
        IsFirstToken = false;
      }
    }

    if (pText[i] == '\n' || pText[i] == '\0') {
      break;
    }
    TokenStart = i+1;
    TokenLength = 0;
  }

  //cout<<ToString()<<endl;

  if (MathMatch != 0) {
    merr<<"There was an error with the math environment. The number of { and } do not match!"<<endl;
    merr<<pText<<show;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::IsMaths(const MString& Token)
{
  // Test if Token is a math expression

  if (Token.BeginsWith("{") && Token.EndsWith("}")) {
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTokenizer::EvaluateMaths(MString& Token)
{
  // Evaluate the maths expressions in Token

  if (IsMaths(Token) == false) return false;

  Token.ReplaceAll("{", " ");
  Token.ReplaceAll("}", " ");

  // First check, if the token is evaluable:
  TFormula Formula;
  if (Formula.Compile(Token) != 0) {
    return false;
  } else {
    ostringstream out;
    out<<scientific<<setprecision(16)<<Formula.Eval(0.0);
    Token = out.str().c_str();
    return true;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MTokenizer::ToString()
{
  ostringstream out;

  if (m_Tokens.size() > 0) {
    out<<"Tokenizer content ("<<m_Tokens.size()<<" Tokens):"<<endl;
    for (unsigned int i = 0; i < m_Tokens.size(); i++) {
      out<<"Token "<<i<<": \""<<GetTokenAt(i)<<"\""<<endl;
    }
  } else {
    out<<"Tokenizer empty!"<<endl;
  }

  return out.str().c_str();
}


// MTokenizer.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
