/*
 * MStatusStream.cxx
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


//---------------------------------------------------------------------------------------------

// Standard libs:

// ROOT libs:
#include <TGMsgBox.h>

// Include the header:
#include "MStatusStream.h"

// MEGAlib libs:
//#include "MTime.h"


//---------------------------------------------------------------------------------------------
//
//
MStatusStream& endl(MStatusStream& StatusStream)
{
  StatusStream.EndLine();
  return StatusStream;
}


//---------------------------------------------------------------------------------------------
//
//
ostream& operator<<(ostream& os, MStatusStream& StatusStream)
{
  os<<StatusStream.str();
  return os;
}



//---------------------------------------------------------------------------------------------
//
//
MStatusStream::MStatusStream() 
{
  m_ConnectStdOut = false;
}

//---------------------------------------------------------------------------------------------
//
//
bool MStatusStream::Connect(TGLabel* Label)
{
  // Check if we are already connected:
  for (unsigned int i = 0; i < m_Labels.size(); i++) {
    if (m_Labels[i] == Label) {
      return true;
    }
  }

  m_Labels.push_back(Label);
  return true;
}


//---------------------------------------------------------------------------------------------
//
//
bool MStatusStream::Disconnect(TGLabel* Label)
{
  // Check if we are already connected:
  for (unsigned int i = 0; i < m_Labels.size(); ++i) {
    if (m_Labels[i] == Label) {
      m_Labels[i] = 0;
      for (unsigned int j = i+1; j < m_Labels.size(); ++j) {
        m_Labels[j-1] = m_Labels[j];
      }
      m_Labels.resize(m_Labels.size()-1);
    }
  }

  return true;
}



//---------------------------------------------------------------------------------------------
//
//
void MStatusStream::ConnectStdOut()
{
  m_ConnectStdOut = true;
}


//---------------------------------------------------------------------------------------------
//
//
void MStatusStream::DisconnectStdOut()
{
  m_ConnectStdOut = false;
}


//---------------------------------------------------------------------------------------------
//
//
MStatusStream& MStatusStream::printf(const char* Format, ...)
{
  if (Format == 0) return (*this);

  va_list args;
  va_start(args, Format);
  Printf(Format, args);
  va_end(args);

  return (*this);
}


//---------------------------------------------------------------------------------------------
//
//
void MStatusStream::EndLine()
{ 
  MStr::EndLine();
  if (m_ConnectStdOut == true) {
    cout<<str();
  }
  for (unsigned int i = 0; i < m_Labels.size(); ++i) {
    MString S = GetString();
    S.ReplaceAll("\0", "");
    S.ReplaceAll("\n", "");
    m_Labels[i]->SetText(S);
  }

  WriteToFile();
  Reset();
}


// MStreams: the end...
//---------------------------------------------------------------------------------------------
