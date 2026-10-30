/*
 * MParser.h
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
  //! The fast mode assumes all token are seperated by a space only (no maths, no . mode)
  bool TokenizeLine(MTokenizer& T, bool Fast = false);
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


#ifdef ___CLING___
 public:
  ClassDef(MParser, 0) // A file parser
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
