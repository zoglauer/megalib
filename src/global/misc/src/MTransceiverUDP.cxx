/*
 * MTransceiverUDP.cxx
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
// MTransceiverUDP
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MTransceiverUDP.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
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
ClassImp(MTransceiverUDP)
#endif


////////////////////////////////////////////////////////////////////////////////


void* StartUDPTransceiverThread(void* Transceiver)
{
  ((MTransceiverUDP *) Transceiver)->TransceiverLoop();
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


int MTransceiverUDP::m_ThreadId = 0;


////////////////////////////////////////////////////////////////////////////////


MTransceiverUDP::MTransceiverUDP(MString Name, MString Host, unsigned int Port)
{
  // Construct an instance of MTransceiverUDP

  m_Name = Name;
  m_Host = Host;
  m_Port = Port;

  m_NPacketsToSend = 0;
  m_NPacketsToReceive = 0;

  m_NLostPackets = 0;
  
  m_NReceivedPackets = 0; 
  m_NSentPackets = 0;

  m_MaxBufferSize = numeric_limits<unsigned int>::max();

  m_IsConnected = false;
  
  m_IsThreadRunning = false;

  m_TransceiverThread = 0;
}


////////////////////////////////////////////////////////////////////////////////


MTransceiverUDP::~MTransceiverUDP()
{
  // Delete this instance of MTransceiverUDP

  if (m_IsConnected == true) {
    Disconnect();
  }
  if (m_TransceiverThread != 0) {
    StopTransceiving();  
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverUDP::Connect()
{
  // Connect to the given host.
  
  if (m_IsConnected == true) return true;
  
  if (m_IsThreadRunning == false) {
    StartTransceiving();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverUDP::Disconnect()
{
  // Disconnect from host

  StopTransceiving();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverUDP::StartTransceiving()
{
  // Starts the multithreaded TransceiverLoop

  if (m_TransceiverThread != 0) return;

  m_StopThread = false;
  
  m_ThreadId++;
  char Name[100];
  sprintf(Name, "Transceiver #%i", m_ThreadId);

  m_IsThreadRunning = false;
  m_TransceiverThread = 
    new TThread(Name, 
                (void(*) (void *)) &StartUDPTransceiverThread, 
                (void*) this);
  m_TransceiverThread->SetPriority(TThread::kHighPriority);
  m_TransceiverThread->Run();
  
  while (m_IsThreadRunning == false) {
    gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }
  //cout<<"Receiver thread running!"<<endl;    
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverUDP::StopTransceiving()
{
  m_StopThread = true;
  
  while (m_IsThreadRunning == true) {
    gSystem->ProcessEvents();
    gSystem->Sleep(10);
  }
    
  //m_TransceiverThread->Kill();
  m_TransceiverThread = 0;
  m_IsThreadRunning = false;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverUDP::Send(const vector<unsigned char>& Packet)
{
  // Now put the events into a list

  m_SendMutex.Lock();
  // Add it to the end of the queue
  m_PacketsToSend.push_back(Packet);
  m_NPacketsToSend++;

  // If we have more than N events we remove the oldest from the buffer...
  if (m_NPacketsToSend > m_MaxBufferSize) {
    m_PacketsToSend.pop_front();
    m_NLostPackets++;
    m_NPacketsToSend--;
    cout<<"Buffer overflow: One packet lost (total loss: "<<m_NLostPackets<<")"<<endl;
  }
  m_SendMutex.UnLock();
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MTransceiverUDP::Receive(vector<unsigned char>& Packet)
{
  // Check if an object is in the received strings list:

  m_ReceiveMutex.Lock();

  if (m_NPacketsToReceive > 0) {
    m_NPacketsToReceive--;
    Packet = m_PacketsToReceive.front();
    m_PacketsToReceive.pop_front();
    m_ReceiveMutex.UnLock();
    return true;
  }
  m_ReceiveMutex.UnLock();
  
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MTransceiverUDP::TransceiverLoop()
{
  // Main thread loop for connecting, receiving and sending data 
  // Since thread safety is a problem, this code is not object oriented but
  // Spaghetti style...

  m_IsThreadRunning = true;

  int Status = 0;
  TUDPSocket* Socket = 0;
  bool SleepAllowed = true;

  int ReadPacketSize = 1024*1024;
  vector<unsigned char> ReadPacket(ReadPacketSize+1, '\0'); // it's 2011, no need to be humble...
  
  int SleepAmount = 100;
  
  while (true) {
    // This is the main loop of the thread. It consists of five parts:
    // (1) Handle stopping the thread if necessary
    // (2) Establish contact to remote host
    // (3) Receive messages
    // (4) Send messages
    // (5) Sleep if not other tasks have to be done

    //cout<<"loop..."<<endl;
    
    SleepAllowed = true;
    
    // Step 0:
    // Check if the thread should be stopped (and this the connection)
    
    if (m_StopThread == true) {
      if (Socket != 0) {
        Socket->Close("force");
        delete Socket;
        Socket = 0;
      }

      m_IsConnected = false;
      
      break;
    }
    
    
    // Step 1: 
    // Connect if not connected and a connection is wished
    
    if (m_IsConnected == false) {
      //cout<<"(Re-) Creating socket..."<<endl;
      
      int Level = gErrorIgnoreLevel;
      gErrorIgnoreLevel = kFatal;
      Socket = new TUDPSocket(m_Host, m_Port); //, 10000000);
      gErrorIgnoreLevel = Level;
        
      // If we where unable to connect as client try as server:
      if (Socket->IsValid() == false) {
        cout<<"Error in UDP transceiver "<<m_Name<<" (ERR="<<Socket->GetErrorCode()<<"): Unable to create socket."<<endl;
          
        Socket->Close("force");
        delete Socket;
        Socket = 0;
        m_IsConnected = false;
      
        continue; // back to the beginning....
      } 
      m_IsConnected = true;
    }

    // Step 3:
    // Send data:

    // Create a message out of the first entry of the list and send the event...
    if (m_NPacketsToSend > 0) {

      Status = Socket->Select(TUDPSocket::kWrite, 10);
      if (Status != 1) {
        cout<<"Error in UDP transceiver "<<m_Name<<" (ERR="<<Status<<"): : Could not switch to writing"<<endl;
        continue; // back to the beginning
      }

      m_SendMutex.Lock();
      vector<unsigned char>& Packet = m_PacketsToSend.front(); // Make sure we don't copy the string...
      m_SendMutex.UnLock();
      cout<<"Sent something..."<<endl;
      Status = Socket->SendRaw((void*) &Packet[0], Packet.size());

      
      if (Status < 0) {
        cout<<"Error in UDP transceiver "<<m_Name<<" (ERR="<<Status<<"): Sending failed!"<<endl;

        Socket->Close("force");
        delete Socket;
        Socket = 0;
        m_IsConnected = false;
        
        continue; // Back --- we have to open a new socket
      } else {
        m_SendMutex.Lock();
        m_NPacketsToSend--;
        m_PacketsToSend.pop_front();
        m_NSentPackets++;
        m_SendMutex.UnLock();

        SleepAllowed = false; // No sleep because we might have more work to do (i.e. send more events)

        continue;
      }
    }

    
    // Step 4: 
    // Receive data
    Status = Socket->Select(TUDPSocket::kRead, 10);
    //cout<<"Read switch status: "<<Status<<endl;
    if (Status < 0) {
      cout<<"Error in UDP transmitter "<<m_Name<<" (ERR="<<Status<<"): Could not switch to reading"<<endl;
      continue;
    }

    if (Status == 1) {
      //cout<<"Read something!"<<endl;
      vector<unsigned char> NewPacket;
      do {
        //for (unsigned int c = 0; c < ReadPacketSize+1; ++c) ReadPacket[c] = '\0'; // ReadPacket is one larger 
        Status = Socket->RecvRaw((void*) &ReadPacket[0], ReadPacketSize, kDontBlock);
        //cout<<"Read Status: "<<Status<<" and string starting with "<<ReadPacket[0]<<endl;
        for (int c = 0; c < Status; ++c) {
          NewPacket.push_back(ReadPacket[c]);
        }
        if (Status < ReadPacketSize) break;
      } while (true);

      //cout<<"Read packed size: "<<NewPacket.size()<<endl;
      if (NewPacket.size() > 0) {
        m_ReceiveMutex.Lock();
      
        //cout<<"Received:"<<endl;
        //cout<<Message<<endl;
      
        if (m_NPacketsToReceive < m_MaxBufferSize) {
          m_PacketsToReceive.push_back(NewPacket);
          m_NReceivedPackets++;
          m_NPacketsToReceive++;
        } else {
          m_NReceivedPackets++;
          m_NLostPackets++;
        } 
        
        m_ReceiveMutex.UnLock();
        
        SleepAllowed = false; // because we might have to receive more events...
      }
    } else {
      //cout<<"Nothing to read!"<<endl;
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


// MTransceiverUDP.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
