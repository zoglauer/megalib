/*
 * MParser.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MParser__
#define __MParser__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MTokenizer.h"
#include "MString.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////

//! A tokenizing file parser built on top of MFile
class MParser : public MFile
{
  // public interface:
 public:
  //! Standard constructor defining the token separator and composed-token handling
  explicit MParser(char Separator = ' ', bool AllowComposed = false);
  //! Default destructor
  virtual ~MParser();

  //! Open the file and, in read mode, tokenize all lines and call Parse()
  virtual bool Open(MString FileName, unsigned int Way = 1);

  //! Return the number of lines in the file
  unsigned int GetNLines();

  //! Remove the specified stored line
  bool RemoveLine(unsigned int LineNumber);
  //! Inserts and tokenizes a line before the given line number
  bool InsertLineBefore(MString Line, unsigned int LineNumber);

  //! READ-MODE ONLY: Return the tokenized lines
  MTokenizer* GetTokenizerAt(unsigned int LineNumber);
  //! READ-MODE ONLY: Return the line as text
  MString GetLine(unsigned int LineNumber);

  //! READ-MODE ONLY: Special handling for streamed parsers
  //! Some derived classes do not read the whole file during Open()
  //! An example is the response class, which can have large data files requiring optimization
  //! This reads and tokenizes one line of the file
  //! The fast mode assumes all tokens are separated by spaces only (no maths, no composed-token mode)
  bool TokenizeLine(MTokenizer& Tokenizer, bool Fast = false);
  //! READ-MODE ONLY: Special handling for streamed parsers
  //! Some derived classes do not read the whole file during Open()
  //! An example is the response class, which can have large data files requiring optimization
  //! This reads one single float from the file
  bool GetFloat(float& Float);


  //! READ-MODE ONLY: Adds and tokenizes a line at the end
  virtual bool AddLine(MString Line);

  //! Dump an error message related to the given line
  void Typo(int LineNumber, MString Error); 

  // protected methods:
 protected:
  //! Parse the tokenized data after Open() has loaded it in read mode
  virtual bool Parse();


  // private methods:
 private:

  // protected members:
 protected:
  //! All tokenized lines
  vector<MTokenizer*> m_Lines;
  //! The separator of the tokens e.g. a space
  char m_Separator;
  //! When true, then elements with "." such as "Sphere.Source"
  //! are split into the two tokens "Sphere" and "Source", otherwise it remains one token
  bool m_AllowComposed;


#ifdef ___CLING___
 public:
  ClassDef(MParser, 0) // A file parser
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
