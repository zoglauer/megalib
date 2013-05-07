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

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseManipulator
{
public:
  /// Default constructor
  MResponseManipulator();
  /// Default destructor
  ~MResponseManipulator();
  
  /// Parse the command line
  bool ParseCommandLine(int argc, char** argv);
  /// Analyze what eveer needs to be analyzed...
  bool Analyze();
  /// Interrupt the analysis
  void Interrupt() { m_Interrupt = true; }

protected:
  bool Append();
  bool Show();
  bool Divide();
  bool Statistics();
  bool Ratio();
  bool Probability();
  bool Join();
  bool FindFiles(MString Prefix, MString Type);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  ///

  /// 
  bool m_Statistics;
  string m_FileName;

  /// 
  bool m_Append;
  vector<string> m_AppendFileNames;

  /// 
  bool m_Divide;
  string m_DividendFileName;
  string m_DivisorFileName;

  /// 
  bool m_Ratio;

  /// 
  bool m_Probability;

  /// 
  bool m_Join;
  string m_Prefix;

  /// Viewing options:
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
  bool m_Normalized;

  ///
  bool m_Zero;

  /// 
  int m_NSmooths;


#ifdef ___CINT___
 public:
  ClassDef(MResponseManipulator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
