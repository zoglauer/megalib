/*
 * MResponseManipulator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MResponseManipulator__
#define __MResponseManipulator__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Manipulate response file (add, join, show statsitics, view, etc.)
class MResponseManipulator
{
public:
  //! Default constructor
  MResponseManipulator();
  //! Default destructor
  virtual ~MResponseManipulator();
  
  //! Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  //! Analyze what ever needs to be analyzed...
  bool Analyze();
  //! Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

protected:
  //! Dump the statistics
  bool Statistics();
  //! Append files
  bool Append();
  //! Show the content of a file
  bool Show();
  //! Collapse a few dimensions
  bool Collapse();
  //! Divide two file
  bool Divide();
  //! Calculate the ration of two files
  bool Ratio();
  //! Calculate a probabillity
  bool Probability();
  //! Main function for joining
  bool Join();
  //! Joind rsp files
  bool JoinRSPFiles(MString Prefix, vector<MString> Type);
  //! Joind ROOT files
  bool JoinROOTFiles(MString Prefix, vector<MString> Type);

private:
  //! True, if the analysis needs to be interrupted
  bool m_Interrupt;

  // Single file operations

  //! The main file name
  MString m_FileName;

  //! Show the statistics
  bool m_Statistics;

  //! Viewing options:
  bool m_Show;
  float m_x1;
  float m_x2;
  float m_x3;
  float m_x4;
  float m_x5;
  float m_x6;
  float m_x7;
  float m_x8;
  float m_x9;
  float m_x10;
  float m_x11;
  float m_x12;
  float m_x13;
  float m_x14;
  float m_x15;
  float m_x16;
  float m_x17;

  bool m_Collapse = true;
  vector<bool> m_CollapseAxis;
 

  // Operations on two files

  //! The first file, usually the dividend
  MString m_DividendFileName;
  //! The second file, usually the divisor
  MString m_DivisorFileName;

  //! Divide two files
  bool m_Divide;

  //! Create the ratio
  bool m_Ratio;

  //! Calculate a probability 
  bool m_Probability;

  // Operations in multiple files

  //! Join files based on a profix
  bool m_Join;
  //! The prefix
  MString m_Prefix;

  //! Apend two or more files
  bool m_Append;
  //! The file names
  vector<MString> m_AppendFileNames;

  // Viewing options

  //! Normalize the view 
  bool m_Normalized;

  //! Small numbers are zero
  bool m_Zero;

  //! Smooth the image N times.
  int m_NSmooths;

  // Reading

  //! Multi-threaded reading
  bool m_MultiThreaded;

#ifdef ___CLING___
 public:
  ClassDef(MResponseManipulator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
