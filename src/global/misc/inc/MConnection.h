/*
 * MConnection.h
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


#ifndef __MConnection__
#define __MConnection__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MConnection 
{
  // public members:
 public:
  static const unsigned int c_None;
  static const unsigned int c_AllEvents;
  static const unsigned int c_UevtaEvents;
  static const unsigned int c_EvtaEvents;
  static const unsigned int c_PevEvents;
  static const unsigned int c_Housekeeping;
  static const unsigned int c_Commands;
  static const unsigned int c_All;
  static const unsigned int c_AllExceptCommands;


  // public interface:
 public:
  explicit MConnection(MString Name = "None", unsigned int Type = c_All, 
                       MString StartTransceiver = "None", 
                       MString StopTransceiver = "None");
  virtual ~MConnection();

  virtual const char* GetName() const;

  void SetType(unsigned int Type);
  unsigned int GetType();

  void SetStartTransceiverName(MString Name);
  MString GetStartTransceiverName();

  void SetStopTransceiverName(MString Name);
  MString GetStopTransceiverName();

  // protected methods:
 protected:
  //MConnection() {};
  //MConnection(const MConnection& Connection) {};

  // private methods:
 private:




  // private members:
 private:
  MString m_Name;
  unsigned int m_Type;
  MString m_StartTransceiverName;
  MString m_StopTransceiverName;
  

#ifdef ___CLING___
 public:
  ClassDef(MConnection, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
