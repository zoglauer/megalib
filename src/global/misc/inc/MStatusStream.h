/*
 * MStatusStream.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


//---------------------------------------------------------------------------------------------

#if !defined(__Def_StatusStreams__)
#define __Def_StatusStreams__

//---------------------------------------------------------------------------------------------

//#undef ___Def_StringStream 
#define ___Def_StringStream 

//---------------------------------------------------------------------------------------------

// Standard libs:
#include <iostream>
using std::ostream;
#include <fstream>
#ifdef ___Def_StringStream
#include <sstream>
using std::ostringstream;
#else
#include <strstream.h>
#endif
#include <vector>
using std::vector;
using std::cout;
using std::endl;
using std::fstream;
#include <string>
using std::string;

// ROOT libs:
#include <TROOT.h>
#include <MString.h>
#include <TGLabel.h>

// MEGAlib libs:
#include "MStreams.h"

class MStatusStream;


//---------------------------------------------------------------------------------------------

MStatusStream& endl(MStatusStream& StatusStream);
ostream& operator<<(ostream& os, MStatusStream& StatusStream);

//---------------------------------------------------------------------------------------------


class MStatusStream : public MLog
{
 public:
  MStatusStream();
  virtual ~MStatusStream() {};

  bool Connect(TGLabel* Label);
  bool Disconnect(TGLabel* Label);

  void ConnectStdOut();
  void DisconnectStdOut();

  virtual void EndLine();

  MStatusStream& printf(const char* Format, ...);
  MStatusStream& operator<<(char c)                 { m_Stream<<c; return (*this); } 
  MStatusStream& operator<<(unsigned char c)        { m_Stream<<c; return (*this); }
  MStatusStream& operator<<(signed char c)          { m_Stream<<c; return (*this); } 
  MStatusStream& operator<<(const char *s)          { m_Stream<<s; return (*this); }
  MStatusStream& operator<<(const unsigned char *s) { m_Stream<<s; return (*this); } 
  MStatusStream& operator<<(const signed char *s)   { m_Stream<<s; return (*this); } 
  MStatusStream& operator<<(const void *p)          { m_Stream<<p; return (*this); } 
  MStatusStream& operator<<(int n)                  { m_Stream<<n; return (*this); }  
  MStatusStream& operator<<(unsigned int n)         { m_Stream<<n; return (*this); }  
  MStatusStream& operator<<(long n)                 { m_Stream<<n; return (*this); }  
  MStatusStream& operator<<(unsigned long n)        { m_Stream<<n; return (*this); } 
  MStatusStream& operator<<(double n)               { m_Stream<<n; return (*this); } 
  MStatusStream& operator<<(float n)                { m_Stream<<n; return (*this); } 
  MStatusStream& operator<<(MString S)              { (*this)<<((char*) S.Data()); return (*this); } 
  MStatusStream& operator<<(MStatusStream& (*func)(MStatusStream&))   { return (*func)(*this); }


 protected:
  vector<TGLabel*> m_Labels;

  bool m_ConnectStdOut;
};

#endif

// MLog: the end...
//---------------------------------------------------------------------------------------------
