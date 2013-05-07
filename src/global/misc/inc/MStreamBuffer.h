/*
 * MStreamBuffer.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MStreamBuffer__
#define __MStreamBuffer__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <ostream>
#include <sstream>
#include <string>
#include <streambuf>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
using namespace std;

// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MStreamBuffer : public streambuf
{
  // public interface:
 public:
  MStreamBuffer();
  virtual ~MStreamBuffer();

  void SetHeader(MString Header) { m_Header = Header; }
  void SetPrefix(MString Prefix) { m_Prefix = Prefix; }
  void SetRejection(MString Rejection); 

  bool Connect(MString FileName, bool Append = true, bool TimePrefix = false);
  bool Disconnect(MString FileName);

  void DumpToStdOut(bool Enable) { m_StdOut = Enable; }
  void DumpToStdErr(bool Enable) { m_StdErr = Enable; }
  void DumpToGui(bool Enable) { m_Gui = Enable; }

  void ShowOnce() { m_ShowOnce = true; }

  void Enable(bool Enable) { m_Enabled = Enable; }

  void endl();
  void fatal();
  void logonly();
  void show(int Mode = c_Info);

  static const int c_Info = 0;
  static const int c_Warn = 1;
  static const int c_Error = 2;


  // protected methods:
 protected:
  //MStreamBuffer() {};
  //MStreamBuffer(const MStreamBuffer& StreamBuffer) {};

  void put_buffer(void);
  void put_char(int);

  // private methods:
 private:
  int	overflow(int);
  int	sync();



  // protected members:
 protected:


  // private members:
 private:
  bool m_Enabled;

  bool m_StdOut;
  bool m_StdErr;
  bool m_Gui;

  MString m_Header;
  bool m_IsHeaderShown;

  MString m_Prefix;
  bool m_IsFirst;

	vector<ofstream*> m_FileStream;
	vector<MString> m_FileNames;
	vector<bool> m_FileTimePrefix;

  list<MString> m_Rejections;

  bool m_ShowNotShown;
  MString m_NotShownString;
  int m_GuiType;
  bool m_ShowOnce;

#ifdef ___CINT___
 public:
  ClassDef(MStreamBuffer, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
