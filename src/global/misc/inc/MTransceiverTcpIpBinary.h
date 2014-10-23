/*
 * MTransceiverTcpIpBinary.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MTransceiverTcpIpBinary__
#define __MTransceiverTcpIpBinary__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <sstream>
#include <string>
#include <list>
#include <deque>
using namespace std;

// ROOT libs:
#include "TThread.h"
#include "TMutex.h"
#include "TSocket.h"


// MEGAlib libs:
#include "MGlobal.h"
#include "MTime.h"
#include "MTimer.h"
#include "MTransceiver.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


void* StartTcpIpBinaryTransceiverThread(void* ObjectReceiver);


////////////////////////////////////////////////////////////////////////////////


class MTransceiverTcpIpBinary
{
  // public interface:
 public:
  //! Standard constructor given a name, host name, and port:
  MTransceiverTcpIpBinary(MString Name = "A TCP/IP transceiver in binary mode", MString HostName = "localhost", unsigned int Port = 9090);
  //! Default desctructor
  ~MTransceiverTcpIpBinary();

  //! Set the name of the transceiver
  void SetName(MString Name) { m_Name = Name; }
  //! Get the name of the transceiver
  MString GetName() const { return m_Name; }

  //! Set the verbosity of the debug output
  //! (0: quiet, 1: errors, 2: errors & warnings (default), 3: errors, warnings & info)
  void SetVerbosity(unsigned int Verbosity) { m_Verbosity = Verbosity; }
  
  //! Set the host name
  void SetHost(MString Host) { m_Host = Host; }
  //! Get the host name
  MString GetHost() const { return m_Host; }
  
  //! Set the port on the host
  void SetPort(unsigned int Port) { m_Port = Port; }
  //! Get the port on the host
  unsigned int GetPort() { return m_Port; }
  
  //! Request this connection to be a client
  void RequestClient(bool Client = true) { m_WishClient = Client; if (m_WishClient) m_WishServer = false; } 
  //! Request this connection to be a server
  void RequestServer(bool Server = true) { m_WishServer = Server; if (m_WishServer) m_WishClient = false; } 
  
  //! Connect. If wait for connection is true, and we ran into time-out, return false, otherwise always true 
  bool Connect(bool WaitForConnection = false, double TimeOut = 60);
  //! Disconnect. If wait for connection is true, and we ran into time-out, return false, otherwise always true 
  bool Disconnect(bool WaitForDisconnection = false, double TimeOut = 60);
 
  //! Return true if we are connected
  bool IsConnected() { return m_IsConnected; }

  //! Return the time of the last connection of a successful receive or send
  MTime GetTimeLastConnection() const { return m_TimeLastConnection; }
  
  //! Send something binary
  bool Send(const vector<unsigned char>& Bytes);
  //! Receive something binary
  bool Receive(vector<unsigned char>& Bytes);
  //! Receive something with sync word Sync but not more than MaxPackets 
  //! The last package will always be missed since we will not find a new syncword for the last package
  bool SyncedReceive(vector<unsigned char>& Packet, vector<unsigned char>& Sync, unsigned int MaxPackets = 1000);

  //! Set the maximum buffer size (in strings) after which to loose events
  //! Since we use C++ lists on char's we have an overhead of 16 : 1 on 64-bit systems!
  void SetMaximumBufferSize(unsigned long MaxBufferSize) { m_MaxBufferSize = MaxBufferSize; }

  //! Get the number of objects still to be sent
  unsigned long GetNPacketsToSend() const { return m_NPacketsToSend; }
  //! Get the number of packets still to be receive (i.e. which are in the receive buffer)
  unsigned long GetNPacketsToReceive() const { return m_NPacketsToReceive; }

  //! Get the number of sent bytes
  unsigned long GetNSentBytes() const { return m_NSentBytes; }
  //! Get the number of receives bytes
  unsigned long GetNReceivedBytes() const { return m_NReceivedBytes; }

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

  //! The verbosity (0: quiet, 1: errors, 2: errors & warnings (default), 3: errors, warnings & info)
  unsigned int m_Verbosity;

  //! The time of the last connection
  MTime m_TimeLastConnection;
  
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
  //! Number of bytes still waiting for sending
  unsigned int m_NBytesToSend;
  //! A mutex for the send queue
  TMutex m_SendMutex;

  //! List of packets which have been received and which are still in the buffer
  deque<unsigned char> m_PacketsToReceive;
  //! Number of packets which have been received and which are still in the buffer
  unsigned int m_NPacketsToReceive;
  //! A mutex for the receive queue
  TMutex m_ReceiveMutex;

  //! The maximum buffer size:
  unsigned long m_MaxBufferSize;

  //! Counter for the number of received strings, etc.
  unsigned long m_NReceivedPackets;
  //! Counter for the number of received bytes
  unsigned long m_NReceivedBytes;
  //! Counter for the number of sent strings, etc.
  unsigned long m_NSentPackets;
  //! Counter for the number of sent bytes
  unsigned long m_NSentBytes;

  //! Counter for the number of lost strings due to buffer overflow
  unsigned long m_NLostPackets;

  //! True if a connection is established
  bool m_IsConnected;
  //! True if this tranceiver tries to connect
  bool m_WishConnection;

  //! True if this connection is intended as server
  bool m_WishServer;
  //! True if this connection is intended as client
  bool m_WishClient;
  
  //! True if this is a server
  bool m_IsServer;
  
  
#ifdef ___CINT___
 public:
  ClassDef(MTransceiverTcpIpBinary, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
