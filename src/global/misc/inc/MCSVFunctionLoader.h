/*
 * MCSVFunctionLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCSVFunctionLoader__
#define __MCSVFunctionLoader__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <iostream>
#include <string>
#include <cstdio>
using namespace std;

// ROOT libs:
#include <TGMsgBox.h>
#include <TMath.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MCSVFunctionLoader
{
  // Public Interface:
public:
  MCSVFunctionLoader();
  MCSVFunctionLoader(MString Filename, MString Keyword);
  ~MCSVFunctionLoader();
  
  void SetFilename(MString Filename);
  void SetKeyword(MString Keyword);
  
  bool Load();
  
  double GetY(double x);
  
  // protected methods:
protected:
  void Sort();
  
  // private methods:
private:
  
  
  // proteced members:
protected:
  
  // private members:
private:
  MString m_Keyword;
  MString m_Filename;
  
  int m_NEntries;
  double* m_X;
  double* m_Y;
};

#endif
