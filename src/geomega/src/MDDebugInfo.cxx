/*
 * MDDebugInfo.cxx
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


#ifdef ___CINT___
ClassImp(MDDebugInfo)
#endif


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::MDDebugInfo() : m_Text(""), m_FileName(""), m_Line(0)
{
  // Default constructor
}


////////////////////////////////////////////////////////////////////////////////


MDDebugInfo::MDDebugInfo(const MString& Text, const MString& FileName, const int Line) :
  m_Text(Text), m_FileName(FileName), m_Line(Line)
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
