/* 
 * MStatusSteam
 *
 * Copyright (C) by Andreas Zoglauer
 * All rights reserved.
 *
 * This code implementation is the intellectual property of Andreas Zoglauer.
 * By copying or distributing this file or any program based on this file,
 * you accept this statement and all its terms.
 *
 * For further information see the file License.doc. 
 *
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
