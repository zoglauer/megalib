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
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MTokenizer.h"

// Standard libs:
#include <vector>
using namespace std;

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MParser : public MFile
{
  // public interface:
 public:
  //! Construct a object of type MTokenizer and analyse the tokens
  //! Separator is the separator between keywords
  //! When AllowComposed == true, then elements with "." such as "Sphere.Source" 
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  explicit MParser(char Separator = ' ', bool AllowComposed = false);
  // Default destructor
  virtual ~MParser();

  //! Open the file
  //! If in READ-MODE: read the data into tokens and calls Parse()
  //! If in WRITE-MODE: nothing else
  virtual bool Open(MString FileName, unsigned int Way = 1);

  //! Return the number of lines in the file
  unsigned int GetNLines();

  //! Removes the specified line 
  bool RemoveLine(unsigned int Line);
  //! Inserts and tokenizes a line before the given line number
  bool InsertLineBefore(MString Line, unsigned int i);

  //! READ-MODE ONLY: Return the tokenized lines
  MTokenizer* GetTokenizerAt(unsigned int Line);
  //! READ-MODE ONLY: Return the line as text
  MString GetLine(unsigned int Line);

  //! READ-MODE ONLY: Special!!
  //! Some derived classes do not read the whole file during Open()
  //! An example is the response class, which can have large data files requiring optimization
  //! This reads and tokenizes one line of the file
  bool TokenizeLine(MTokenizer& T);
  //! READ-MODE ONLY: Special!!
  //! Some derived classes do not read the whole file during Open()
  //! An example is the response class, which can have large data files requiring optimization
  //! This reads one single float from the file
  bool GetFloat(float& f);


  //! WRITE-MODE ONLY: Adds and tokenizes a line at the end
  virtual bool AddLine(MString Line);

  //! Dump a type message related to the given line
  void Typo(int Line, MString Error); 

  // protected methods:
 protected:
  //! Parse the tokenized data
  //! Derived classes can perform all the parsing here
  //! This function is called during Open()
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
  //! are split in the two tokens "Sphere" and "Source", otherwise it is one token 
  bool m_AllowComposed;


#ifdef ___CINT___
 public:
  ClassDef(MParser, 0) // A file parser
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
