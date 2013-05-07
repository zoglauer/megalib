/*
 * MCSVFunctionLoader.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MCSVFUNCTIONLOADER__
#define __MCSVFUNCTIONLOADER__

#include "MGlobal.h"

using namespace std;
#include <iostream>
#include <string>
#include <cstdio>

#include <MString.h>
#include <TGMsgBox.h>
#include <TMath.h>

//
//////////////////////////////////////////////////////////////////////////////////
//

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
