/*
 * MTransceiver.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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


#ifdef ___CINT___
 public:
  ClassDef(MTransceiver, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
