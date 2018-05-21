/*
 * MTransceiver.cxx
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MTransceiver
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTransceiver.h"

// Standard libs:

// ROOT libs:

// MIWorks libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MTransceiver)
#endif


////////////////////////////////////////////////////////////////////////////////


MTransceiver::MTransceiver(TString Name)
{
  // Construct an instance of MTransceiver  

  m_IsServer = false;
  m_IsConnected = false;
  m_Name = Name;
}


////////////////////////////////////////////////////////////////////////////////


MTransceiver::~MTransceiver()
{
  // Delete this instance of MTransceiver
}


// MTransceiver.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
