/*
 * MDDebugInfo.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MDDebugInfo
//
// This class contains the text, file name and line number in the file of the
// geometry setup file in order to allow the debugging of the setup file.
//
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDDebugInfo.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MDDebugInfo)
#endif


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::MDDebugInfo() : m_Text(""), m_FileName(""), m_Line(0), m_TokenizerWithMathsUpToDate(false), m_TokenizerWithoutMathsUpToDate(false), m_IsTokenizerValid(true)
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::MDDebugInfo(const MString& Text, const MString& FileName, const int Line) :
  m_Text(Text), m_FileName(FileName), m_Line(Line), m_TokenizerWithMathsUpToDate(false), m_TokenizerWithoutMathsUpToDate(false), m_IsTokenizerValid(true)
{
  // Standard constructor
}


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::MDDebugInfo(const MDDebugInfo& DebugInfo)
{
  // Copy constructor

  *this = DebugInfo;
}


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::~MDDebugInfo()
{
  // default destructor
}

////////////////////////////////////////////////////////////////////////////////


MDDebugInfo& MDDebugInfo::operator=(const MDDebugInfo& DebugInfo)
{
  m_Text = DebugInfo.m_Text;
  m_FileName = DebugInfo.m_FileName;
  m_Line = DebugInfo.m_Line;
  m_TokenizerWithMathsUpToDate = false; 
  m_TokenizerWithoutMathsUpToDate = false;
  m_IsTokenizerValid = DebugInfo.m_IsTokenizerValid;

  return *this;
}


////////////////////////////////////////////////////////////////////////////////


void MDDebugInfo::Replace(MString Old, MString New, bool WholeWordsOnly)
{
  // Replace some text

  if (WholeWordsOnly == true) {
    // This algorithm also appears in MDGeometry::ReplaceWholeWords
    if (m_Text.Length() > 0) {
      size_t Pos = 0;
      while ((Pos = m_Text.Index(Old, Pos)) != MString::npos) {
        if (m_Text.Length() > Pos + Old.Length()) {
          if (isalnum(m_Text[Pos + Old.Length()]) != 0 || 
              m_Text[Pos + Old.Length()] == '_') {
            Pos += Old.Length();
            continue;
          }
        }
        if (Pos > 0) {
          if (isalnum(m_Text[Pos - 1]) != 0 || 
              m_Text[Pos - 1] == '_') {
            Pos += Old.Length();
            continue;
          }        
        }
        m_Text.Replace(Pos, Old.Length(), New);
        Pos += New.Length();
      }
    } 
  } else {
    m_Text.ReplaceAll(Old, New);
  }
  
  m_TokenizerWithMathsUpToDate = false; 
  m_TokenizerWithoutMathsUpToDate = false;  
}


////////////////////////////////////////////////////////////////////////////////


void MDDebugInfo::ReplaceFirst(MString Old, double Number)
{
  // Replace some text

  MString New;
  New += Number;

  // This algorithm also appears in MDGeometry::ReplaceWholeWords
  if (m_Text.Length() > 0) {
    size_t Pos = 0;
    if ((Pos = m_Text.Index(Old, Pos)) != MString::npos) {
      m_Text.Replace(Pos, Old.Length(), New);
    }
  }
  
  m_TokenizerWithMathsUpToDate = false; 
  m_TokenizerWithoutMathsUpToDate = false;  
}

  
////////////////////////////////////////////////////////////////////////////////


MTokenizer& MDDebugInfo::GetTokenizer(bool AllowMaths)
{
  // Return the tokenizer - the flag indicates if the maths, or no-maths version is requested
 
  if (AllowMaths == false) {
    if (m_TokenizerWithoutMathsUpToDate == false) {
      if (m_TokenizerWithoutMaths.Analyse(m_Text, false) == false) {
        m_IsTokenizerValid = false;
      }
      m_TokenizerWithoutMathsUpToDate = true;
    }
    return m_TokenizerWithoutMaths;
  } else {
    if (m_TokenizerWithMathsUpToDate == false) {
      if (m_TokenizerWithMaths.Analyse(m_Text, true) == false) {
        m_IsTokenizerValid = false;
      }
      m_TokenizerWithMathsUpToDate = true;
    }
    return m_TokenizerWithMaths;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MDDebugInfo::Error(MString Message) const
{
  // Print an error message

  mout<<endl;
  mout<<"   ***  Error  ***  in setup file "<<GetFileName()<<" at line "<<GetLine()<<":"<<endl;
  mout<<"\""<<GetText()<<"\""<<endl;
  mout<<Message<<endl;
  mout<<"Stopping to scan geometry file!"<<endl;
  mout<<endl;
}
  

// MDDebugInfo.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
