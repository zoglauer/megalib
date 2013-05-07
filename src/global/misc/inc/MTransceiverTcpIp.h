/*
 * MTransceiverTcpIp.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTransceiverTcpIp__
#define __MTransceiverTcpIp__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <sstream>
#include <string>
#include <list>
using namespace std;

// ROOT libs:
#include "TThread.h"
#include "TMutex.h"
#include "TSocket.h"


// MEGAlib libs:
#include "MGlobal.h"
#include "MTransceiver.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


void* StartTransceiverThread(void* ObjectReceiver);


////////////////////////////////////////////////////////////////////////////////


class MTransceiverTcpIp
{
  // public interface:
 public:
  //! Standard constructor given a name, host name, port and the transceiving mode:
  //! The transceiving mode is one of:
  //! c_ModeASCIIText: Send as text which is embedded in a ROOT TMessage object
  //! c_ModeRawEventList: Send as RAW text event list. The event or the event list 
  //! must be terminated with a "EN" and the package size must be a multiple of 256
  //! and filled with '/0' characters
  MTransceiverTcpIp(MString Name = "A transceiver", MString HostName = "localhost", unsigned int Port = 9090, unsigned int Mode = c_ModeASCIIText);
  //! Default desctructor
  ~MTransceiverTcpIp();

  //! Set the name of the transceiver
  void SetName(MString Name) { m_Name = Name; }
  //! Get the name of the transceiver
  MString GetName() const { return m_Name; }

  //! Set the host name
  void SetHost(MString Host) { m_Host = Host; }
  //! Get the host name
  MString GetHost() const { return m_Host; }
  
  //! Set the port on the host
  void SetPort(unsigned int Port) { m_Port = Port; }
  //! Get the port on the host
  unsigned int GetPort() { return m_Port; }
  
  //! Connect. If wait for connection is true, and we ran into time-out, return false, otherwise always true 
  bool Connect(bool WaitForConnection = false, double TimeOut = 60);
  //! Disconnect. If wait for connection is true, and we ran into time-out, return false, otherwise always true 
  bool Disconnect(bool WaitForDisconnection = false, double TimeOut = 60);
 
  //! Return true if we are connected
  bool IsConnected() { return m_IsConnected; }
  //! Return true if we are connected as server
  bool IsServer() { return m_IsServer; }

  //! Send a string
  bool Send(const MString& String);
  //! Receive a string 
  bool Receive(MString& String);

  //! Set the maximum buffer size (in strings) after which to loose events
  void SetMaximumBufferSize(unsigned int MaxBufferSize) { m_MaxBufferSize = MaxBufferSize; }

  //! Get the number of events still to be sent
  unsigned int GetNStringsToSend() const { return m_NStringsToSend; }
  //! Get the number of events still to be receive (i.e. which are in the receive buffer)
  unsigned int GetNStringsToReceive() const { return m_NStringsToReceive; }

  //! The (multithreaded) transceiver loop
  void TransceiverLoop();

  //! The data is transmitted as text (internally as TMessage object)
  static const unsigned int c_ModeASCIIText;
  //! The data is transmitted as raw text as an event list. This requires the event or the event list to be terminated with an "EN"
  static const unsigned int c_ModeRawEventList;
  
  // protected methods:
 protected:
  //! Start the transceiving
  void StartTransceiving();
  //! Stop the transceiving
  void StopTransceiving();

  
  
  // private methods:
 private:
  //! Degenerate copy constructor --- when cannot copy construct this class
  //MTransceiverTcpIp(const MTransceiverTcpIp&);
  //! Degenerate assignment operator --- we cannot copy this class
  //MTransceiverTcpIp& operator=(const MTransceiverTcpIp&);
  

  // protected members:
 protected:


  // private members:
 private:
  //! Name of the transceiver
  MString m_Name;
  //! Name of the host (e.g. "localhost")
  MString m_Host;
  //! The port (e.g. 9090)
  unsigned int m_Port;

  //! The transceiving mode: 
  unsigned int m_Mode;

  //! True if we should read everything multi-threaded
  bool m_IsMultiThreaded;
  //! The thread where the receiving and transmitting happens
  TThread *m_TransceiverThread;     
  //! Unique Id for the thread...
  static int m_ThreadId;
  //! Flag indicating to stop the thread
  bool m_StopThread;
  //! Flag indicating that the thread is running
  bool m_IsThreadRunning;

  
  //! List of objects still waiting for sending
  list<MString> m_StringsToSend;
  //! number of objects still waiting for sending
  unsigned int m_NStringsToSend;
  //! A mutex for the send queue
  TMutex m_SendMutex;

  //! List of objects which have been received and which are still in the buffer
  list<MString> m_StringsToReceive;
  //! list of objects which have been received and which are still in the buffer
  unsigned int m_NStringsToReceive;
  //! A mutex for the receive queue
  TMutex m_ReceiveMutex;

  //! The maximum buffer size:
  unsigned int m_MaxBufferSize;

  //! Counter for the number of received strings, etc.
  unsigned int m_NReceivedStrings;
  //! Counter for the number of sent strings, etc.
  unsigned int m_NSentStrings;

  //! Counter for the number of lost strings due to buffer overflow
  unsigned int m_NLostStrings;

  //! True if a connection is established
  bool m_IsConnected;
  //! True if this tranceiver tries to connect
  bool m_WishConnection;

  //! True if this is a server
  bool m_IsServer;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MTransceiverTcpIp, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
