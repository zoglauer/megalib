/*
 * MTransceiverTcpIp.cxx
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
// MTransceiverTcpIp
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTransceiverTcpIp.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include <TServerSocket.h>
#include <TThread.h>
#include <TMessage.h>
#include <TClass.h>
#include <TRandom.h>

// MEGAlib libs:
#include "MStreams.h"
#include "MTimer.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MTransceiverTcpIp)
#endif


////////////////////////////////////////////////////////////////////////////////


void* StartTransceiverThread(void* Transceiver)
{
  ((MTransceiverTcpIp *) Transceiver)->TransceiverLoop();
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


int MTransceiverTcpIp::m_ThreadId = 0;

const unsigned int MTransceiverTcpIp::c_ModeASCIIText = 0;
const unsigned int MTransceiverTcpIp::c_ModeRawEventList = 1;
const unsigned int MTransceiverTcpIp::c_MaxRawMessageLength = 1024*1024;


////////////////////////////////////////////////////////////////////////////////


MTransceiverTcpIp::MTransceiverTcpIp(MString Name, MString Host, unsigned int Port, unsigned int Mode)
{
  // Construct an instance of MTransceiverTcpIp

  m_Name = Name;
  m_Host = Host;
  m_Port = Port;
  m_Mode = Mode;
  if (m_Mode != c_ModeRawEventList && m_Mode != c_ModeASCIIText) m_Mode = c_ModeASCIIText;

  m_NStringsToSend = 0;
  m_NStringsToReceive = 0;

  m_NLostStrings = 0;
  
  m_NReceivedStrings = 0; 
  m_NSentStrings = 0;
 
  m_NResets = 0;

  m_MaxBufferSize = numeric_limits<unsigned int>::max();

  m_IsConnected = false;
  m_WishConnection = false;
  m_IsServer = false;
  // By default we try client first and fall back to server if needed.
  m_WishServer = true;
  m_WishClient = true;

  m_IsThreadRunning = false;

  m_TransceiverThread = nullptr;
   
  m_Verbosity = 2;
}


////////////////////////////////////////////////////////////////////////////////


MTransceiverTcpIp::~MTransceiverTcpIp()
{
  // Delete this instance of MTransceiverTcpIp

  if (m_IsConnected == true) {
    Disconnect();
  }
  if (m_TransceiverThread != nullptr) {
    StopTransceiving();  
  }
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIp::ClearBuffers() 
{
  //! Clear the send and receive buffers

  m_SendMutex.Lock();
  
  m_StringsToSend.clear();
  m_NStringsToSend = 0;
  
  m_SendMutex.UnLock();
  
  m_ReceiveMutex.Lock();

  m_StringsToReceive.clear();
  m_NStringsToReceive = 0;

  m_ReceiveMutex.UnLock();
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIp::Connect(bool WaitForConnection, double TimeOut)
{
  // Connect to the given host.
  
  if (m_IsConnected == true) return true;

  if (m_WishClient == false && m_WishServer == false) {
    if (m_Verbosity >= 3) {
      cout<<"Transceiver "<<m_Name<<": Connection request ignored because both client and server roles are disabled"<<endl;
    }
    return false;
  }

  m_WishConnection = true;
  
  if (m_IsThreadRunning == false) {
    StartTransceiving();
  }
    
  if (WaitForConnection == true) {
    if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Waiting for connection to "<<m_Host<<":"<<m_Port<<endl; 
    MTimer Passed;
    while (Passed.GetElapsed() <= TimeOut && m_IsConnected == false) {
      gSystem->Sleep(10);
    }
    if (m_IsConnected == false) {
      m_WishConnection = false;
      StopTransceiving();
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Connection to "<<m_Host<<":"<<m_Port<<" failed!"<<endl;
      return false;
    }
    if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Connected to "<<m_Host<<":"<<m_Port<<endl; 
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIp::Disconnect(bool WaitForDisconnection, double TimeOut)
{
  // Disconnect from host

  m_WishConnection = false;
  StopTransceiving();

  if (m_IsConnected == true) {
    if (WaitForDisconnection == true) {
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Waiting disconnection!"<<endl;
      MTimer Passed;
      while (Passed.GetElapsed() <= TimeOut && m_IsConnected == true) {
        gSystem->Sleep(10);
      }
      if (m_IsConnected == true) {
        if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Disconnection from "<<m_Host<<":"<<m_Port<<" failed!"<<endl;
        return false;
      }
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Disconnected from "<<m_Host<<":"<<m_Port<<endl;
    }
  }

  // Clean up:
  ClearBuffers();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIp::StartTransceiving()
{
  // Starts the multithreaded TransceiverLoop

  if (m_TransceiverThread != nullptr) return;

  m_StopThread = false;
  
  m_ThreadId++;
  const int Length = 100;
  char Name[Length];
  snprintf(Name, Length, "Transceiver #%i", m_ThreadId);

  m_IsThreadRunning = false;
  m_TransceiverThread = 
    new TThread(Name, 
                (void(*) (void *)) &StartTransceiverThread, 
                (void*) this);
  m_TransceiverThread->SetPriority(TThread::kHighPriority);
  m_TransceiverThread->Run();
  
  while (m_IsThreadRunning == false) {
    // Never do this in a thread! gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }
  if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Transceiver thread running!"<<endl;    
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIp::StopTransceiving()
{
  m_StopThread = true;
  
  while (m_IsThreadRunning == true) {
    // Never do this in a thread! gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }

  if (m_TransceiverThread != nullptr) {
    delete m_TransceiverThread;
    m_TransceiverThread = nullptr;
  }
  m_IsThreadRunning = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIp::Send(const MString& String)
{
  // Now put the events into a list

  m_SendMutex.Lock();
  
  // Add it to the end of the queue
  m_StringsToSend.push_back(MString(String.Data())); // make sure we don't use the string copy mechanism    
  m_NStringsToSend++;

  // If we have more than N events we remove the oldest from the buffer...
  if (m_NStringsToSend > m_MaxBufferSize) {
    m_StringsToSend.pop_front();
    m_NLostStrings++;
    m_NStringsToSend--;
    if (m_NLostStrings == 1 || m_NLostStrings == 10 || m_NLostStrings == 100 || m_NLostStrings == 1000 ||  m_NLostStrings == 10000 || m_NLostStrings % 100000 == 0) { 
      if (m_Verbosity >= 2) cout<<"Transceiver "<<m_Name<<": Error: Buffer overflow: Packets/Events have been lost (total loss: "<<m_NLostStrings<<")"<<endl;
    }
  }
  
  m_SendMutex.UnLock();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverTcpIp::Receive(MString& String)
{
  // Check if an object is in the received strings list:

  m_ReceiveMutex.Lock();

  if (m_NStringsToReceive > 0) {
    m_NStringsToReceive--;
    String = m_StringsToReceive.front();
    m_StringsToReceive.pop_front();
    m_ReceiveMutex.UnLock();
    return true;
  }
  m_ReceiveMutex.UnLock();
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverTcpIp::TransceiverLoop()
{
  // Main thread loop for connecting, receiving and sending data 
  // Since thread safety is a problem, this code is not object oriented but
  // Spaghetti style...

  if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": thread running!"<<endl;    

  m_IsThreadRunning = true;

  int Status = 0;
  TServerSocket* ServerSocket = nullptr;    // A server
  TSocket* Socket = nullptr;                // A full-duplex connection to another host
  bool SleepAllowed = true;

  unsigned int TextMessageLength;
  if (m_Mode == c_ModeRawEventList) {
    TextMessageLength = 256; // Don't change!
  } else {
    TextMessageLength = 1024*1024; // it's 2011, no need to be humble...    
  }
  char* TextMessage= new char[TextMessageLength+1];
  MString RawMessage;
  MString Message;
  bool MessageComplete = false;
  bool MessageDropped = false;
 
  int SleepAmount = 2;
  
  while (true) {
    // This is the main loop of the thread. It consists of five parts:
    // (1) Handle stopping the thread if necessary
    // (2) Establish contact to remote host
    // (3) Receive messages
    // (4) Send messages
    // (5) Sleep if no other tasks have to be done

    SleepAllowed = true;
    
    // Some initial sanity checks
    if (m_IsConnected == true) {
      m_TimeSinceLastConnection.Reset();
      
      if (Socket == nullptr || Socket->IsValid() == false || Socket->TestBit(TSocket::kBrokenConn)) {
        if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Found a broken connection... Resetting!"<<endl;
        
        if (Socket != nullptr) {
          Socket->Close("force");
          delete Socket;
          Socket = nullptr;
          ++m_NResets;
        }
        
        m_IsConnected = false;
        m_IsServer = false;
      }
    }
    
    
    // Step 0:
    // Check if the thread should be stopped (and this the connection)
    
    if (m_StopThread == true) {
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Stopping thread...!"<<endl;

      if (Socket != nullptr) {
        Socket->Close("force");
        delete Socket;
        Socket = nullptr;
      }

      m_IsConnected = false;
      m_IsServer = false;
      
      break;
    }
    
    
    // Step 1: 
    // Connect if not connected and a connection is wished
    
    
    if (m_IsConnected == false) {
      if (m_WishConnection == true) {
        if (m_WishClient == false && m_WishServer == false) {
          if (m_Verbosity >= 3) {
            cout<<"Transceiver "<<m_Name<<": Connection request ignored because both client and server roles are disabled"<<endl;
          }
          gSystem->Sleep(SleepAmount);
          continue;
        }
        
        // Try to (re-) connect as client:
        if (m_WishClient == true) {
          int Level = gErrorIgnoreLevel;
          gErrorIgnoreLevel = kFatal;
          m_SocketMutex.Lock(); // socket initilization is not reentrant as of 5.34.22 (bu bug report is submitted)!
          Socket = new TSocket(m_Host, m_Port); //, 10000000);
          m_SocketMutex.UnLock();
          gErrorIgnoreLevel = Level;
          
          if (Socket->IsValid() == true) {
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Connection established as client!"<<endl;
            Socket->SetOption(kNoBlock, 1);
            m_IsServer = false;
            m_IsConnected = true;
          } else {
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Unable to connect as client..."<<endl;
            Socket->Close("force");
            delete Socket;
            Socket = nullptr;
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


          // ROOT returns a negative pointer value on accept failure.
          if (reinterpret_cast<intptr_t>(Socket) > 0) {
            Socket->SetOption(kNoBlock, 1);
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Connection established as server!"<<endl;
            m_IsServer = true;
            m_IsConnected = true;  
          } else {
            Socket = nullptr; // Since it can be negative... yes...
            if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": Unable to connect as server, trying again later..."<<endl;
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
      if (m_Verbosity >= 3) cout<<"Transceiver "<<m_Name<<": No longer wishing connection..."<<endl;

      Socket->Close("force");
      delete Socket;
      Socket = nullptr;

      m_IsConnected = false;
      m_IsServer = false;
      
      continue; // Back to start
    }


    // Step 3: 
    // Receive data
    
    // Add the object to the list:
    if (m_Mode == c_ModeASCIIText) {
      Socket->SetOption(kNoBlock, 1); // don't block!
      Status = Socket->Recv(TextMessage, TextMessageLength);
      Message = TextMessage;
      //cout<<"Receive text"<<endl;
      //cout<<Message<<endl;
    } else if (m_Mode == c_ModeRawEventList) {
      MessageComplete = false;
      MessageDropped = false;
      do {
        Socket->SetOption(kNoBlock, 1); // don't block!
        for (unsigned int c = 0; c < TextMessageLength+1; ++c) TextMessage[c] = '\0'; // TextMessage is one larger then TextMessageLength!
        Status = Socket->RecvRaw(TextMessage, TextMessageLength);
        for (unsigned int c = 0; c < TextMessageLength; ++c) {
          if (TextMessage[c] == '\0') { 
            break; 
          }
          RawMessage += TextMessage[c];
        }
        size_t EN = RawMessage.Index("EN", 0);
        if (EN != MString::npos) {
          Message = RawMessage.GetSubString(0, EN);
          RawMessage = RawMessage.Remove(0, EN+2);
          if (RawMessage.Length() > 0 && RawMessage[0] == '\n') {
            RawMessage = RawMessage.Remove(0, 1);
          }
          MessageComplete = true;
          break;
        }
        if (RawMessage.Length() > c_MaxRawMessageLength) {
          cout<<"Transceiver "<<m_Name<<": Error: Raw message exceeded maximum allowed size"<<endl;
          RawMessage = "";
          MessageDropped = true;
          break;
        }
        if (Status <= 0) { 
          break;
        }
      } while (true);
    } else {
      cout<<"Transceiver "<<m_Name<<": Error: Unknown transceiving mode:"<<m_Mode<<endl;
    }
      
    // Handle error codes
    if (Status <= 0 && Status != -4) {
      if (Status == 0) {
        cout<<"Transceiver "<<m_Name<<": Error: Connection lost!"<<endl;
      } else if (Status == -1) {
        cout<<"Transceiver "<<m_Name<<": Error: A connection error occurred!"<<endl;
       } else {
        cout<<"Transceiver "<<m_Name<<": Error: Unknown connection problem! Status: "<<Status<<", error code:"<<Socket->GetErrorCode()<<endl;        
      }
      
      Socket->Close("force");
      delete Socket;
      Socket = nullptr;
      
      m_IsConnected = false;
      m_IsServer = false;
      ++m_NResets;
      
      continue; // back to the beginning....
    } 
    // If status == -4 and no complete raw message was assembled, we have nothing to do
    else if (Status == -4 && MessageComplete == false) {
      // Empty...
    } 
    // If the raw message exceeded the allowed size, skip it
    else if (m_Mode == c_ModeRawEventList && MessageDropped == true) {
      // Empty...
    }
    // If status > 0, or a complete raw message was assembled, we got a message
    else {
      m_ReceiveMutex.Lock();
      
      //cout<<"Received:"<<endl;
      //cout<<Message<<endl;
      
      if (m_NStringsToReceive < m_MaxBufferSize) {
        m_StringsToReceive.push_back(Message);
        m_NReceivedStrings++;
        m_NStringsToReceive++;
      } else {
        m_NReceivedStrings++;
        m_NLostStrings++;
      } 
        
      m_ReceiveMutex.UnLock();
        
      SleepAllowed = false; // because we might have to receive more events...
    }



    // Step 4:
    // Send data:

    // Create a message out of the first entry of the list and send the event...
    if (m_NStringsToSend > 0) {

      m_SendMutex.Lock();
      Message = m_StringsToSend.front().Data(); // Make sure we don't copy the string...
      m_SendMutex.UnLock();
      
      Socket->SetOption(kNoBlock, 0); // Not sure about this...
      if (m_Mode == c_ModeASCIIText) {
        Status = Socket->Send(Message.Data());
      } else if (m_Mode == c_ModeRawEventList) {
        while (Message.Length() % TextMessageLength != 0) Message += '\0';
        Status = Socket->SendRaw(Message.Data(), Message.Length());
      } else {
        cout<<"Transceiver "<<m_Name<<": Error: Unknown transceiving mode:"<<m_Mode<<endl;
      }
      
      if (Status < 0) {
        cout<<"Transceiver "<<m_Name<<": Sending failed with status "<<Status<<" and error code "<<Socket->GetErrorCode()<<endl;

        Socket->Close("force");
        delete Socket;
        Socket = nullptr;
        
        m_IsConnected = false;
        m_IsServer = false;
        ++m_NResets;
      } else {
        m_SendMutex.Lock();
        if (m_StringsToSend.empty() == false) {
          m_StringsToSend.pop_front();
          m_NStringsToSend--;
          m_NSentStrings++;
        }
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
  
  delete [] TextMessage;
  m_IsThreadRunning = false;
}


// MTransceiverTcpIp.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
