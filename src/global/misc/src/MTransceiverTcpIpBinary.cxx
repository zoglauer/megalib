/*
 * MTransceiverTcpIpBinary.cxx
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
// MTransceiverTcpIpBinary
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTransceiverTcpIpBinary.h"

// Standard libs:
#include <limits>
#include <vector>
#include <algorithm>
using namespace std;

// ROOT libs:
#include <TServerSocket.h>
#include <TSocket.h>
#include <TThread.h>
#include <TMessage.h>
#include <TClass.h>
#include <TRandom.h>

// MIWorks libs:
#include "MStreams.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MTransceiverTcpIpBinary)
#endif


////////////////////////////////////////////////////////////////////////////////


void* StartTcpIpBinaryTransceiverThread(void* Transceiver)
{
  ((MTransceiverTcpIpBinary *) Transceiver)->TransceiverLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MTransceiverTcpIpBinary::m_ThreadId = 0;


////////////////////////////////////////////////////////////////////////////////


MTransceiverTcpIpBinary::MTransceiverTcpIpBinary(MString Name, MString Host, unsigned int Port)
{
  // Construct an instance of MTransceiverTcpIpBinary

  m_Name = Name;
  m_Host = Host;
  m_Port = Port;
  
  m_NPacketsToSend = 0;
  m_NBytesToSend = 0;
  m_NPacketsToReceive = 0;

  m_NLostPackets = 0;
  
  m_NReceivedPackets = 0; 
  m_NReceivedBytes = 0; 
  m_NSentPackets = 0;
  m_NSentBytes = 0;
  
  m_NResets = 0;
  
  m_MaxBufferSize = 10000000;

  m_IsConnected = false;
  m_WishConnection = false;
  m_AutomaticReconnection = true;
  
  m_IsServer = false;
  m_WishServer = true;
  m_WishClient = true;
  
  m_IsThreadRunning = false;

  m_TransceiverThread = 0;
  
  m_Verbosity = 2;
}


////////////////////////////////////////////////////////////////////////////////


MTransceiverTcpIpBinary::~MTransceiverTcpIpBinary()
{
  // Delete this instance of MTransceiverTcpIpBinary

  if (m_IsConnected == true) {
    Disconnect();
  }
  if (m_TransceiverThread != 0) {
    StopTransceiving();  
  }
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIpBinary::ClearBuffers() 
{
  //! Clear the send and receive buffers

  m_SendMutex.Lock();
  
  m_PacketsToSend.clear();
  m_NPacketsToSend = 0;
  m_NBytesToSend = 0;
  
  m_SendMutex.UnLock();
  


  m_ReceiveMutex.Lock();

  m_NPacketsToReceive = 0;
  m_PacketsToReceive.clear();

  m_ReceiveMutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIpBinary::Connect(bool WaitForConnection, double TimeOut)
{
  // Connect to the given host.
  
  m_AutomaticReconnection = true; // must come before any return
  if (m_IsConnected == true) return true;
  m_WishConnection = true;
  
  if (m_IsThreadRunning == false) {
    StartTransceiving();
  }
    
  if (WaitForConnection == true) {
    if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Waiting for connection to "<<m_Host<<":"<<m_Port<<endl; 
    MTimer Passed;
    while (Passed.GetElapsed() <= TimeOut && m_IsConnected == false) {
      gSystem->Sleep(10);
    }
    if (m_IsConnected == false) {
      StopTransceiving();
      if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Connection to "<<m_Host<<":"<<m_Port<<" failed!"<<endl;
      return false;
    }
    if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Connected to "<<m_Host<<":"<<m_Port<<endl; 
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIpBinary::Disconnect(bool WaitForDisconnection, double TimeOut)
{
  // Disconnect from host

  m_WishConnection = false;
  StopTransceiving();

  if (m_IsConnected == true) {
    if (WaitForDisconnection == true) {
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Waiting for disconnection!"<<endl;
      MTimer Passed;
      while (Passed.GetElapsed() <= TimeOut && m_IsConnected == true) {
        gSystem->Sleep(10);
      }
      if (m_IsConnected == true) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Disconnection from "<<m_Host<<":"<<m_Port<<" failed!"<<endl;
        return false;
      }
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Disconnected from "<<m_Host<<":"<<m_Port<<endl;
    }
  }

  // Clean up:
  // TODO: Why not clear buffers
  m_PacketsToSend.clear();
  m_NPacketsToSend = 0;
  m_NBytesToSend = 0;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIpBinary::StartTransceiving()
{
  // Starts the multithreaded TransceiverLoop

  if (m_TransceiverThread != 0) return;

  m_StopThread = false;
  
  m_ThreadId++;
  const int Length = 100;
  char Name[Length];
  snprintf(Name, Length, "Transceiver #%i", m_ThreadId);

  m_IsThreadRunning = false;
  m_TransceiverThread = 
    new TThread(Name, 
                (void(*) (void *)) &StartTcpIpBinaryTransceiverThread, 
                (void*) this);
  m_TransceiverThread->SetPriority(TThread::kHighPriority);
  m_TransceiverThread->Run();
  
  while (m_IsThreadRunning == false) {
    // Never do this in a thread! gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }
  
  if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Transceiver thread running"<<endl;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIpBinary::StopTransceiving()
{
  m_StopThread = true;
  
  while (m_IsThreadRunning == true) {
    // Never do this in a thread! gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }
    
  //m_TransceiverThread->Kill();
  m_TransceiverThread = 0;
  m_IsThreadRunning = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIpBinary::Send(const vector<unsigned char>& Packet)
{
  // Now put the events into a list

  m_SendMutex.Lock();
    
  // Add it to the end of the queue
  m_PacketsToSend.push_back(Packet);
  m_NPacketsToSend++;
  m_NBytesToSend += Packet.size();

  // If we have more than N bytes we remove the oldest from the buffer... we are just removing one at a time!
  while (m_NBytesToSend > m_MaxBufferSize && m_NPacketsToSend > 0 && m_PacketsToSend.empty() == false) {
    m_NBytesToSend -= m_PacketsToSend.front().size();
    m_NPacketsToSend--;
    m_NLostPackets++;
    m_PacketsToSend.pop_front();
    if (m_NLostPackets == 1 || m_NLostPackets == 10 || m_NLostPackets == 100 || m_NLostPackets == 1000 || m_NLostPackets == 10000 || m_NLostPackets % 100000 == 0) {
      if (m_Verbosity >= 2) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Buffer overflow: Packets lost (total loss: "<<m_NLostPackets<<")"<<endl;
    }
  }

  m_SendMutex.UnLock();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIpBinary::SyncedReceive(vector<unsigned char>& Packet, vector<unsigned char>& Sync, unsigned int MaxPackets)
{
  // Check if something is in the received list, 
  // If it starts with Sync, and there is more starting with Sync store the first one with sync

  if (Sync.size() == 0) return false;
  
  m_ReceiveMutex.Lock();
  
  deque<unsigned char> S;
  for (unsigned char c: Sync) S.push_back(c);
  
  deque<unsigned char>::iterator Start;
  Start = std::search(m_PacketsToReceive.begin(), m_PacketsToReceive.end(), S.begin(), S.end());
  if (Start != m_PacketsToReceive.end()) {
    deque<unsigned char>::iterator Stop = Start;
    deque<unsigned char>::iterator E;
    deque<unsigned char>::iterator SearchStart = Stop;
    advance(SearchStart, Sync.size());
    //cout<<"Transceiver "<<m_Name<<": Total size: "<<m_NPacketsToReceive<<endl;
    while ((E = search(SearchStart, m_PacketsToReceive.end(), Sync.begin(), Sync.end())) != m_PacketsToReceive.end() && MaxPackets-- > 0) {
      Stop = E;
      SearchStart = E;
      advance(SearchStart, Sync.size());
    }
    if (Stop != Start) {
      for (auto I = Start; I != Stop; ++I) {
        Packet.push_back((*I));
      }
      m_PacketsToReceive.erase(Start, Stop);
      //m_NPacketsToReceive = m_PacketsToReceive.size();
      if (m_NPacketsToReceive < Packet.size()) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): BAD Error: the Size of the current packet ("<<Packet.size()<<") is larger than what I expect to have as total packets ("<<m_NPacketsToReceive<<")! Resyncing..."<<endl;
        m_NPacketsToReceive = m_PacketsToReceive.size();
      } else {
        m_NPacketsToReceive -= Packet.size();
      }
      m_ReceiveMutex.UnLock();
      return true;
    }
  }

  m_ReceiveMutex.UnLock();
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIpBinary::Receive(vector<unsigned char>& Packet)
{
  // Check if an object is in the received strings list:

  m_ReceiveMutex.Lock();
  
  if (m_NPacketsToReceive > 0) {
    Packet.clear();
    for (unsigned char c: m_PacketsToReceive) {
      Packet.push_back(c);
    }
    m_NPacketsToReceive = 0;
    m_PacketsToReceive.clear();
    m_ReceiveMutex.UnLock();
    return true;
  }
  m_ReceiveMutex.UnLock();
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIpBinary::TransceiverLoop()
{
  // Main thread loop for connecting, receiving and sending data 
  // Since thread safety is a problem, this code is not object oriented but
  // Spaghetti style...

  if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): thread running!"<<endl;    

  m_IsThreadRunning = true;

  int Status = 0;
  TServerSocket* ServerSocket = 0;    // A server
  TSocket* Socket = 0;                // A full-duplex connection to another host
  bool SleepAllowed = true;

  int ReadPacketSize = 1024*1024;
  vector<unsigned char> ReadPacket(ReadPacketSize+1, '\0'); // it's 2011, no need to be humble...
  
  int SleepAmount = 20;
  
 
  while (true) {
    //cout<<"Transceiver loop"<<endl;
    
    // This is the main loop of the thread. It consists of five parts:
    // (1) Handle stopping the thread if necessary
    // (2) Establish contact to remote host
    // (3) Receive messages
    // (4) Send messages
    // (5) Sleep if not other tasks have to be done

    SleepAllowed = true;
    
    // Some initial sanity checks
    if (m_IsConnected == true) {
      m_TimeSinceLastConnection.Reset();
      
      if (Socket->IsValid() == false || Socket->TestBit(TSocket::kBrokenConn)) {
        if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Found a broken connection... Resetting!"<<endl;
        
        if (Socket != 0) {
          Socket->Close("force");
          delete Socket;
          Socket = 0;
          ++m_NResets;
        }
        
        m_IsConnected = false;
        m_IsServer = false;
      }
    }
    
    
    // Step 0:
    // Check if the thread should be stopped (and this the connection)
    
    if (m_StopThread == true) {
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Stopping thread...!"<<endl;

      if (Socket != 0) {
        Socket->Close("force");
        delete Socket;
        Socket = 0;
        ++m_NResets;
      }

      m_IsConnected = false;
      m_IsServer = false;
      
      break;
    }
    
    
    // Step 1: 
    // Connect if not connected and a connection is wished
    
    if (m_IsConnected == false) {
      if (m_WishConnection == true && m_AutomaticReconnection == true) {
        
        // Try to (re-) connect as client:
        if (m_WishClient == true) {
          int Level = gErrorIgnoreLevel;
          gErrorIgnoreLevel = kFatal;
          m_SocketMutex.Lock(); // socket initilization is not reentrant as of 5.34.22 (bu bug report is submitted)!
          Socket = new TSocket(m_Host, m_Port); //, 10000000);
          m_SocketMutex.UnLock();
          gErrorIgnoreLevel = Level;
          
          if (Socket->IsValid() == true) {
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Connection established as client!"<<endl;
            Socket->SetOption(kNoBlock, 1);
            m_IsServer = false;
            m_IsConnected = true;
          } else {
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Unable to connect as client..."<<endl;
            Socket->Close("force");
            delete Socket;
            Socket = 0;
            ++m_NResets;
            if (m_WishServer == false) {
              gSystem->Sleep(SleepAmount);
              continue;
            }
          }
        }
          
        // If we where unable to connect as client try as server:
        if (m_WishServer == true && m_IsConnected == false) {

          m_SocketMutex.Lock(); // socket initilization is not reentrant as of 5.34.22 (bu bug report is submitted)!
          ServerSocket = new TServerSocket(m_Port, true, 10000000);
          ServerSocket->SetOption(kNoBlock,1);
          m_SocketMutex.UnLock();

          // Wait for a client to connect - we add a random amount to make sure that two instances of this class can connect at same point in time
          gSystem->Sleep(10*SleepAmount + gRandom->Integer(10*SleepAmount));
          Socket = ServerSocket->Accept();
        
          ServerSocket->Close("force");
          delete ServerSocket;


          if (long(Socket) > 0) {
            Socket->SetOption(kNoBlock, 1);
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Connection established as server!"<<endl;
            m_IsServer = true;
            m_IsConnected = true;  
          } else {
            Socket = 0; // Since it can be negative... yes...
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Unable to connect as server, trying again later..."<<endl;
            gSystem->Sleep(SleepAmount);
            continue;
          }
        } 
      } 
      // Case: We are not connected and do not want to be connected... SLEEP!
      else {
        gSystem->Sleep(SleepAmount);
        continue;
      }
    }
    // If we are connected but wish disconnection
    else if (m_IsConnected == true && m_WishConnection == false) {
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): No longer wishing connection..."<<endl;

      Socket->Close("force");
      delete Socket;
      Socket = 0;
      ++m_NResets;

      m_IsConnected = false;
      m_IsServer = false;
      
      continue; // Back to start
    }

    
    // Step 3: 
    // Receive data
      
    // Add the object to the list:
    deque<unsigned char> NewPacket;
    unsigned int MaxLoops = 100; 
    do {
      Socket->SetOption(kNoBlock, 1); // don't block!
      Status = Socket->RecvRaw((void*) &ReadPacket[0], ReadPacketSize, kDontBlock);
      // cout<<"Read Status: "<<Status<<" and string starting with "<<ReadPacket[0]<<endl;
      for (int c = 0; c < Status; ++c) {
        NewPacket.push_back(ReadPacket[c]);
      }
      if (Status < ReadPacketSize || --MaxLoops == 0) break;
    } while (true);

     
    // Handle error codes
    if (Status <= 0 && Status != -4) {
      if (Status == 0) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Error: Connection lost!"<<endl;
      } else if (Status == -1) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Error: A connection error occurred!"<<endl;
      } else if (Status == -5) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Error: Connection reset by peer!"<<endl;
       } else {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Error: Unknown connection problem! Status: "<<Status<<", error code:"<<Socket->GetErrorCode()<<endl;        
      }
      
      Socket->Close("force");
      delete Socket;
      Socket = 0;
      ++m_NResets;
      
      m_IsConnected = false;
      
      continue; // back to the beginning....
    } 
    // If status == 0 we have either no message received or lost connection
    else if (Status == -4) {
      // Empty...
    } 
    // If status > 0, we got a message
    else {
      m_TimeSinceLastIO.Reset();
      unsigned long NewPacketSize = NewPacket.size();
      if (NewPacketSize > 0) {
        m_ReceiveMutex.Lock();
      
        //cout<<"Transceiver "<<m_Name<<": Received something from "<<m_Host<<":"<<m_Port<<" of size "<<NewPacket.size()<<endl;
      
        if (m_NPacketsToReceive + NewPacketSize > m_MaxBufferSize) {
          if (NewPacketSize > m_NPacketsToReceive) {
            m_NLostPackets += m_NPacketsToReceive;
            m_NPacketsToReceive = 0;
            m_PacketsToReceive.clear();
          } else {
            deque<unsigned char>::iterator Stop = m_PacketsToReceive.begin();
            advance(Stop, NewPacketSize);
            m_PacketsToReceive.erase(m_PacketsToReceive.begin(), Stop);
            m_NLostPackets += NewPacketSize;
            m_NPacketsToReceive -= NewPacketSize;
          }
          if (m_Verbosity >= 2) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Buffer overflow: Deleted oldest "<<NewPacketSize<<" bytes! Now "<<m_NPacketsToReceive<<" bytes are in the buffer"<<endl;
        }
        
        m_PacketsToReceive.insert(m_PacketsToReceive.end(), NewPacket.begin(), NewPacket.end());
        m_NReceivedPackets += NewPacketSize;
        m_NReceivedBytes += NewPacketSize;
        m_NPacketsToReceive += NewPacketSize;

        //cout<<m_Name<<": Received packet size: "<<m_NPacketsToReceive<<endl;
        
        m_ReceiveMutex.UnLock();
        
        SleepAllowed = false; // because we might have to receive more events...
      }
    }



    // Step 4:
    // Send data:
    m_SendMutex.Lock();
    bool SomethingToSend = (m_NPacketsToSend > 0) ? true : false;
    m_SendMutex.UnLock();
    
    
    // Create a message out of the first entry of the list and send the event...
    if (SomethingToSend == true) {

      m_SendMutex.Lock();
      if (m_PacketsToSend.empty() == true) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): BAD Error: the deque seems to be empty ("<<m_PacketsToSend.size()<<"), but the counter says otherwise ("<<m_NPacketsToSend<<")! Resyncing..."<<endl;
        m_NPacketsToSend = 0;
        m_SendMutex.UnLock();
        continue;
      }
      vector<unsigned char>& Packet = m_PacketsToSend.front(); // Make sure we don't copy the string...
      m_SendMutex.UnLock();
      
      if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Trying to send something to "<<m_Host<<":"<<m_Port<<" ..."<<endl;
      Socket->SetOption(kNoBlock, 0); // Not sure about this...
      Status = Socket->SendRaw((void*) &Packet[0], Packet.size());

      
      if (Status < 0) {
        if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Error (ERR="<<Status<<"): Sending failed!"<<endl;

        Socket->Close("force");
        delete Socket;
        Socket = 0;
        ++m_NResets;
        m_IsConnected = false;
        
        continue; // Back --- we have to open a new socket
      } else {
        m_TimeSinceLastIO.Reset();
        m_SendMutex.Lock();
        //cout<<"Sent "<<Packet.size()<<" bytes --- "<<m_NPacketsToSend<<":"<<m_PacketsToSend.size()<<endl;
        
        if (m_PacketsToSend.empty() == true) {
          if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): BAD Error: the deque seems to be empty ("<<m_PacketsToSend.size()<<"), but the counter says otherwise ("<<m_NPacketsToSend<<")! Resyncing..."<<endl;
          m_NPacketsToSend = 0;
          m_SendMutex.UnLock();
          continue;
        }
        if (m_NPacketsToSend == 0) {
          if (m_Verbosity >= 1) cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): BAD Error: the packet counter says there are no packets left ("<<m_NPacketsToSend<<"), but we reached this point... resyncing and skipping"<<endl;
          m_NPacketsToSend = m_PacketsToSend.size();
          m_SendMutex.UnLock();
          continue;
        }

        cout<<"Transceiver "<<m_Name<<" ("<<m_Host<<":"<<m_Port<<"): Successfully sent something to "<<m_Host<<":"<<m_Port<<" ..."<<endl;
        m_NPacketsToSend--;
        m_NBytesToSend -= Packet.size();
        m_NSentBytes += Packet.size();
        m_PacketsToSend.pop_front();
        m_NSentPackets++;
       
        m_SendMutex.UnLock();

        SleepAllowed = false; // No sleep because we might have more work to do (i.e. send more events)

        continue;
      }
    }


    // Step 5:
    // We reach this part of the code, if we are connected, have nothing to send and didn't receive anything
    // Thus sleep...
    
    if (m_StopThread == false && SleepAllowed == true) {
      gSystem->Sleep(SleepAmount);
    }

    continue;
  }
  
  m_IsThreadRunning = false;
}


// MTransceiverTcpIpBinary.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
