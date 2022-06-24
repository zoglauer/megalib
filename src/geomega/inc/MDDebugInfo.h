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
#include "MTokenizer.h"

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

  //! Comparison operator
  MDDebugInfo& operator=(const MDDebugInfo& DebugInfo);

  //! Set the text
  void SetText(const MString& Text) { m_Text = Text; m_TokenizerWithMathsUpToDate = false; m_TokenizerWithoutMathsUpToDate = false; }
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
  
  //! Return the tokenizer - the flag indicates if the maths, or no-maths version is requested
  MTokenizer& GetTokenizer(bool AllowMaths = true);
  
  //! Return true is the tokenizer is valid
  bool IsTokenizerValid() { return m_IsTokenizerValid; }
  
  //! Prepend some text
  void Prepend(MString Pre) { SetText(Pre + m_Text); m_TokenizerWithMathsUpToDate = false; m_TokenizerWithoutMathsUpToDate = false; }

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

  //! Something has been modified since the tokenizer have been calculated
  bool m_TokenizerWithMathsUpToDate; 
  //! The standard tokenizer
  MTokenizer m_TokenizerWithMaths;

  //! Something has been modified since the tokenizer have been calculated
  bool m_TokenizerWithoutMathsUpToDate; 
  //! The tokenizer without maths
  MTokenizer m_TokenizerWithoutMaths;
  
  //! True if all tokenizers are valid
  bool m_IsTokenizerValid;
  
#ifdef ___CLING___
 public:
  ClassDef(MDDebugInfo, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
