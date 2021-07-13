/*
 * MConnection.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
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
