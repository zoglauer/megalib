/*
 * MTransceiverUDP.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTransceiverUDP__
#define __MTransceiverUDP__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <sstream>
#include <string>
#include <list>
using namespace std;

// ROOT libs:
#include "TThread.h"
#include "TMutex.h"
#include "TUDPSocket.h"


// MEGAlib libs:
#include "MGlobal.h"
#include "MTransceiver.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


void* StartUDPTransceiverThread(void* ObjectReceiver);


////////////////////////////////////////////////////////////////////////////////


class MTransceiverUDP
{
  // public interface:
 public:
  //! Standard constructor given a name, host name, and port:
  MTransceiverUDP(MString Name = "A UDP transceiver", MString HostName = "localhost", unsigned int Port = 9090);
  //! Default desctructor
  ~MTransceiverUDP();

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
  
  //! Connect - starts the threads
  bool Connect();
  //! Disconnect - ends the thread
  bool Disconnect();
 
  //! Return true if we are connected
  bool IsConnected() { return m_IsConnected; }

  //! Send something binary
  bool Send(const vector<unsigned char>& Bytes);
  //! Receive something binary
  bool Receive(vector<unsigned char>& Bytes);

  //! Set the maximum buffer size (in strings) after which to loose events
  void SetMaximumBufferSize(unsigned int MaxBufferSize) { m_MaxBufferSize = MaxBufferSize; }

  //! Get the number of objects still to be sent
  unsigned int GetNPacketsToSend() const { return m_NPacketsToSend; }
  //! Get the number of packets still to be receive (i.e. which are in the receive buffer)
  unsigned int GetNPacketsToReceive() const { return m_NPacketsToReceive; }

  //! The (multithreaded) transceiver loop
  void TransceiverLoop();

  
  // protected methods:
 protected:
  //! Start the transceiving
  void StartTransceiving();
  //! Stop the transceiving
  void StopTransceiving();

  
  
  // private methods:
 private:
  

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

  //! The thread where the receiving and transmitting happens
  TThread* m_TransceiverThread;     
  //! Unique Id for the thread...
  static int m_ThreadId;
  //! Flag indicating to stop the thread
  bool m_StopThread;
  //! Flag indicating that the thread is running
  bool m_IsThreadRunning;

  //! List of byte packets still waiting for sending
  list<vector<unsigned char>> m_PacketsToSend;
  //! Number of objects still waiting for sending
  unsigned int m_NPacketsToSend;
  //! A mutex for the send queue
  TMutex m_SendMutex;

  //! List of packets which have been received and which are still in the buffer
  list<vector<unsigned char>> m_PacketsToReceive;
  //! Number of packets which have been received and which are still in the buffer
  unsigned int m_NPacketsToReceive;
  //! A mutex for the receive queue
  TMutex m_ReceiveMutex;

  //! The maximum buffer size:
  unsigned int m_MaxBufferSize;

  //! Counter for the number of received strings, etc.
  unsigned int m_NReceivedPackets;
  //! Counter for the number of sent strings, etc.
  unsigned int m_NSentPackets;

  //! Counter for the number of lost strings due to buffer overflow
  unsigned int m_NLostPackets;

  //! True if a connection is established
  bool m_IsConnected;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MTransceiverUDP, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
