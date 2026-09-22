/*
 * MConnection.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MConnection
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MConnection.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MConnection)
#endif


////////////////////////////////////////////////////////////////////////////////


const unsigned int MConnection::c_None = 0;
const unsigned int MConnection::c_AllEvents = 1;
const unsigned int MConnection::c_UevtaEvents = 2;
const unsigned int MConnection::c_EvtaEvents = 3;
const unsigned int MConnection::c_PevEvents = 4;
const unsigned int MConnection::c_Housekeeping = 5;
const unsigned int MConnection::c_Commands = 6;
const unsigned int MConnection::c_All = 7;
const unsigned int MConnection::c_AllExceptCommands = 8;


////////////////////////////////////////////////////////////////////////////////


MConnection::MConnection(MString Name, unsigned int Type, MString StartTransceiver, 
                         MString StopTransceiver)
  : m_Name(Name), m_Type(Type),
    m_StartTransceiverName(StartTransceiver), 
    m_StopTransceiverName(StopTransceiver)
{
  // Construct an instance of MConnection
}


////////////////////////////////////////////////////////////////////////////////


MConnection::~MConnection()
{
  // Delete this instance of MConnection
}


////////////////////////////////////////////////////////////////////////////////


const char* MConnection::GetName() const
{
  // Return the name

  return m_Name;
}


////////////////////////////////////////////////////////////////////////////////


void MConnection::SetType(unsigned int Type)
{
  m_Type = Type;
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MConnection::GetType()
{
  return m_Type;
}


////////////////////////////////////////////////////////////////////////////////


void MConnection::SetStartTransceiverName(MString Name)
{
  m_StartTransceiverName = Name;
}


////////////////////////////////////////////////////////////////////////////////


MString MConnection::GetStartTransceiverName()
{
  return m_StartTransceiverName;
}


////////////////////////////////////////////////////////////////////////////////


void MConnection::SetStopTransceiverName(MString Name)
{
  m_StopTransceiverName = Name;
}
  


////////////////////////////////////////////////////////////////////////////////


MString MConnection::GetStopTransceiverName()
{
  return m_StopTransceiverName;
}


////////////////////////////////////////////////////////////////////////////////



// MConnection.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
