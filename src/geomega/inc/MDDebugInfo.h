/*
 * MDDebugInfo.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MDDebugInfo__
#define __MDDebugInfo__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MStreams.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MDDebugInfo
{
  // public interface:
 public:
  //! Default constructor
  MDDebugInfo();
  //! Standard constructor
  MDDebugInfo(const MString& Text, const MString& FileName, const int LineNumber);
  //! Copy constructor
  MDDebugInfo(const MDDebugInfo& DebugInfo);
  //! Default destructor
  virtual ~MDDebugInfo();

  MDDebugInfo& operator=(const MDDebugInfo& DebugInfo);

  //! Set the text
  void SetText(const MString& Text) { m_Text = Text; }
  //! Set the file name
  void SetFileName(const MString& FileName) { m_FileName = FileName; }
  //! Set the line number
  void SetLine(int LineNumber) { m_Line = LineNumber; }

  //! Return true if the text contains the string
  bool Contains(MString Text) { return m_Text.Contains(Text); }

  //! Replace some strings...
  void Replace(MString Old, MString New, bool WholeWordsOnly = false);
  //! Replace some text with a number
  void ReplaceFirst(MString Old, double New);
  //! Return the text
  MString GetText() const { return m_Text; };
  //! Return the file name
  MString GetFileName() const { return m_FileName; }
  //! Return the line number
  int GetLine() const { return m_Line; }
  
  //! Prepend some text
  void Prepend(MString Pre) { SetText(Pre + m_Text); }

  //! Print an error message
  void Error(MString Message) const;

  // protected methods:
 protected:


  // private methods:
 private:


  // protected members:
 protected:


  // private members:
 private:
  //! The text
  MString m_Text;
  //! The original file name
  MString m_FileName;
  //! The original line number
  int m_Line;

#ifdef ___CINT___
 public:
  ClassDef(MDDebugInfo, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
