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
  bool JoinRSPFiles(MString Prefix, vector<MString> Type);
  bool JoinROOTFiles(MString Prefix, vector<MString> Type);

private:
  /// True, if the analysis needs to be interrupted
  bool m_Interrupt;

  ///

  /// 
  bool m_Statistics;
  MString m_FileName;

  /// 
  bool m_Append;
  vector<MString> m_AppendFileNames;

  /// 
  bool m_Divide;
  MString m_DividendFileName;
  MString m_DivisorFileName;

  /// 
  bool m_Ratio;

  /// 
  bool m_Probability;

  /// 
  bool m_Join;
  MString m_Prefix;

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
