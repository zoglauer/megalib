/*
 * MTransceiver.h
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


#ifndef __MTransceiver__
#define __MTransceiver__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MTransceiver
{
  // public interface:
 public:
  //! Standard constructor of a transceiver
  MTransceiver(TString Name = "No Name");
  //! Standard destuctor
  virtual ~MTransceiver();

  //! Set the name of the transceiver
  void SetName(TString Name) { m_Name = Name; }
  //! Get the name of the transceiver
  TString GetName() const { return m_Name; }

  virtual bool Connect(bool WaitForConnection = false, double TimeOut = 60) = 0;
  virtual bool Disconnect(bool WaitForDisconnection = false, double TimeOut = 60) = 0;

  virtual bool Send(TObject* Object) = 0;
  virtual TObject* Receive() = 0;

  virtual void StartTransceiving() = 0;
  virtual void StopTransceiving() = 0;

  // protected methods:
 protected:
  virtual void TransceiverLoop() = 0;

  // private methods:
 private:



  // protected members:
 protected:
  bool m_IsServer;
  bool m_IsConnected;

  TString m_Name;

  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MTransceiver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
