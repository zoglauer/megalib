/*
 * MParser.cxx
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
// MParser
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MParser.h"

// Standard libs:

// ROOT libs:
#include <TObjString.h>

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFileManager.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MParser)
#endif


////////////////////////////////////////////////////////////////////////////////


MParser::MParser(char Separator, bool AllowComposed) : MFile()
{
  // Construct an instance of MParser

  m_Separator = Separator;
  m_AllowComposed = AllowComposed;
}


////////////////////////////////////////////////////////////////////////////////


MParser::~MParser()
{
  // Delete this instance of MParser
  
  for (unsigned int l = 0; l < m_Lines.size(); ++l) { 
    delete m_Lines[l];
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MParser::Open(MString FileName, unsigned int Way)
{
  // Open the file and do the parsing

  if (MFile::Open(FileName, Way) == false) {
    mlog<<"MParser: Unable to open file \""<<FileName<<"\"."<<endl;
    return false;
  }

  Rewind();
  if (Way == c_Read) {

    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) break;
      AddLine(Line);
    }
 
    Rewind();

    if (Parse() == false) {
      mlog<<"MParser: Unable to parse file \""<<FileName<<"\""<<endl;
      return false;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MParser::Parse()
{
  // Do the actual parsing - has to be overwritten!

  return true;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MParser::GetNLines()
{
  // Return the number of stored lines

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return 0;
  }

  return m_Lines.size();
}
 

////////////////////////////////////////////////////////////////////////////////


bool MParser::AddLine(MString Line)
{
  // Add a line to the line storage

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return false;
  }

  m_Lines.push_back(new MTokenizer(Line, m_Separator, m_AllowComposed));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MTokenizer* MParser::GetTokenizerAt(unsigned int LineNumber)
{
  // Return the line with nume LineNumber

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return 0;
  }

  if (LineNumber >= GetNLines()) {
    merr<<"Index out of bounds"<<endl;
    return 0;
  } 

  return m_Lines[LineNumber];
}

////////////////////////////////////////////////////////////////////////////////


MString MParser::GetLine(unsigned int LineNumber)
{
  //! READ-MODE ONLY: Return the line as text

  MString Line;
  
  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return 0;
  }

  if (LineNumber >= GetNLines()) {
    merr<<"Index out of bounds"<<endl;
    return 0;
  }   
  
  return m_Lines[LineNumber]->GetText();
}

  
////////////////////////////////////////////////////////////////////////////////


bool MParser::InsertLineBefore(MString Line, unsigned int LineNumber)
{
  // Insert Before position i

  if (LineNumber > GetNLines()) {
    massert(LineNumber <= GetNLines());
    return false;
  } 

  m_Lines.insert(m_Lines.begin()+LineNumber, new MTokenizer(Line, m_Separator, m_AllowComposed));

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MParser::RemoveLine(unsigned int LineNumber)
{
  // Remove line Line

  if (LineNumber >= GetNLines()) {
    massert(LineNumber < GetNLines());
    return false;
  } 

  m_Lines.erase(m_Lines.begin()+LineNumber);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MParser::Typo(int Line, MString Error)
{
  // Print some error message

  mlog<<endl;
  mlog<<"   ***  Error  ***  in parsed file "<<m_FileName<<" at line "<<Line+1<<":"<<endl;
  mlog<<"   ***  Error  ***  "<<Error<<endl;
  mlog<<"   Line ("<<GetTokenizerAt(Line)->GetNTokens()<<" token"<<((GetTokenizerAt(Line)->GetNTokens() == 1) ? "" : "s")<<"): \""<<GetTokenizerAt(Line)->GetText()<<"\""<<endl;
  mlog<<endl;
}


////////////////////////////////////////////////////////////////////////////////


bool MParser::TokenizeLine(MTokenizer& T, bool Fast)
{
  // Tokenize one line of the file
  // Return false if an error occurs

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return false;
  }

  MString Line;
  if (ReadLine(Line) == false) return false;
  /*
  char c;
  MString Line;
  while (true) {
    Get(c);
    if (IsGood() == false) {
      return false;
    }
    if (c != '\n' && c != '\0') {
      Line += c;
    } else {
      break;
    }
  }
  */
  
  if (Fast == true) {
    T.AnalyzeFast(Line);    
  } else {
    T.Analyze(Line);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MParser::GetFloat(float& f)
{
  // Get a float from the file

  if (m_Way != c_Read) {
    merr<<"Only valid if file is in read-mode!"<<endl;
    massert(m_Way == c_Read);
    return false;
  }

  Get(f);

  return true;
}




// MParser.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
