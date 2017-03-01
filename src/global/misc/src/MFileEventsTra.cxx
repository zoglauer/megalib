/*
 * MFileEventsTra.cxx
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
// MFileEventsTra
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsTra.h"

// Standard libs:

// ROOT libs:
#include "TThread.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MComptonEvent.h"
#include "MPairEvent.h"
#include "MPhotoEvent.h"
#include "MMuonEvent.h"
#include "MUnidentifiableEvent.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileEventsTra)
#endif


////////////////////////////////////////////////////////////////////////////////


void MFileEventsTra_CallThread(void* Address)
{
  MFileEventsTra* File = ((MFileEventsTra_ThreadCaller*) Address)->GetThreadCaller();
  File->ThreadedReading();
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsTra::MFileEventsTra() : MFileEvents()
{
  // Construct an instance of MFileEventsTra

  m_EventType = MPhysicalEvent::c_Unknown;

  m_FileType = "tra";

  m_Fast = false;
  m_ParseDelayed = false;

  m_Threaded = false;
  m_StopThread = false;
  m_Thread = 0;

  m_AutomaticProgressUpdates = true;

  m_Version = 1;

  m_NDataSets = 0;
  m_DataSets.clear();

  // This seems to be a good number for the avaerage analysis as of 2009 on a E5420 quad core
  m_MinimumNDataSets = 100;
  // This seems to be a good maximum number for the avaerage analysis as of 2009 on a E5420 quad core
  // If the only analysis task is to reject the event
  m_MinimumNDataSetsMax = 10000;
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsTra::~MFileEventsTra()
{
  // Delete this instance of MFileEventsTra
}

////////////////////////////////////////////////////////////////////////////////


bool MFileEventsTra::Close()
{
  // Delete this instance of MFileEventsTra

  // Kill the thread if it still exists
  if (m_Threaded == true && m_Thread != 0) {
    m_StopThread = true;
    while (m_Thread->GetState() != TThread::kCanceledState) {
      TThread::Sleep(0, 1000000);
    }
  }

  // In case there are still data sets available
  while (m_DataSets.size() > 0) {
    MPhysicalEvent* P = m_DataSets.front();
    m_DataSets.pop_front();
    delete P;
  }

  return MFileEvents::Close();
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsTra::Open(MString FileName, unsigned int Way)
{
  // Derived open which initializes the include file

  m_IncludeFileUsed = false;
  MFileEventsTra* I = new MFileEventsTra();
  I->SetIsIncludeFile(true);
  I->SetFastFileParsing(m_Fast);
  m_IncludeFile = dynamic_cast<MFileEvents*>(I);

  m_MoreEvents = true;

  return MFileEvents::Open(FileName, Way);
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsTra::StartThread()
{
  m_Threaded = true;
  m_StopThread = false;

  m_Thread = new TThread("A thread", (void(*) (void *)) &MFileEventsTra_CallThread, (void*) new MFileEventsTra_ThreadCaller(this));
  m_Thread->Run();

  TThread::Sleep(0, 100000000);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void* MFileEventsTra::ThreadedReading()
{
  // This function is part of the thread
  // Lock all pieces of this class which might be accessed by the other thread!

  TThread::Lock();
  m_NDataSets = 0;
  m_DataSets.clear();
  m_MoreEvents = true;
  TThread::UnLock();

  int ReaderThreadSleepTime = 10000000;
  double TimeReaderThreadSlept = 0.0;

  unsigned int CurrentSize;
  unsigned int StepSize = 10;
  unsigned int MinSize = m_MinimumNDataSets;
  unsigned int MaxSize = 3*m_MinimumNDataSets/2;

  MPhysicalEvent* P = 0;
  vector<MPhysicalEvent*> Ps;
  Ps.reserve(StepSize);

  while (true) {
    TThread::Lock();
    CurrentSize = m_NDataSets;
    MinSize = m_MinimumNDataSets;
    MaxSize = 3*m_MinimumNDataSets/2;
    TThread::UnLock();
    //cout<<"Size tra store: "<<CurrentSize<<" vs. "<<MinSize<<":"<<MaxSize<<endl;
    if (CurrentSize <= MinSize) {
      while (m_NDataSets < MaxSize) {
      //for (unsigned int i = CurrentSize; i < MaxSize/StepSize; ++i) {
        // cout<<"Reading..."<<endl;
        for (unsigned int p = 0; p < StepSize; ++p) {
          P = ReadNextEvent();
          if (P == 0) {
            //cout<<"No More events..."<<endl;
            TThread::Lock();
            m_MoreEvents = false;
            for (unsigned int p = 0; p < Ps.size(); ++p) {
              m_DataSets.push_back(Ps[p]);
              m_NDataSets++;
            }
            Ps.clear();
            TThread::UnLock();
            // Missing: end thread here
            cout<<"Time reader thread slept: "<<TimeReaderThreadSlept<<" sec (the thread will most likely sleep during worker thread initialization)"<<endl;
            return 0;
          }
          Ps.push_back(P);
        }
        TThread::Lock();
        //cout<<"Adding..."<<Ps.size()<<endl;
        for (unsigned int p = 0; p < Ps.size(); ++p) {
          m_DataSets.push_back(Ps[p]);
          m_NDataSets++;
        }
        Ps.clear();
        TThread::UnLock();
      }
    } else {
      TThread::Sleep(0, ReaderThreadSleepTime);
      //cout<<"Reader thread sleeping with "<<CurrentSize<<" (min: "<<MinSize<<") events in stack..."<<endl;
      TimeReaderThreadSlept += ReaderThreadSleepTime/1000000000.0;
    }
    if (m_StopThread == true) {
      return 0;
    }
    //gSystem->ProcessEvents();
  }

  //cout<<"Finishing thread..."<<endl;
  TThread::Lock();
  m_MoreEvents = false;
  TThread::UnLock();

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MFileEventsTra::GetNextEvent()
{
  // Return the next event from the list, wait if non is present...

  if (m_AutomaticProgressUpdates == true) {
    if (UpdateProgress() == false) {
      return 0;
    }
  }

  if (m_Threaded == false) {
    return ReadNextEvent();
  } else {
    // As long as we have still events on file or in the storage, try to get it

    // Due to a complex locking mechanism - we need an endless while loop...
    // (If you think, you don't need this loop, you forgot, how threads work.)
    while (true) {

      // Make sure we read only as long as there are still enough events in the store:
      // No strict requirement for locking, since intermediate values in the variables leed only
      // to a wrong sleep, which is irrelevant!
      while (m_MoreEvents == true && m_NDataSets <= 5) {
        //cout<<"Not enough data to analyze (available: "<<m_NDataSets<<", minimum: "<<m_MinimumNDataSets<<")... sleeping..."<<endl;
        TThread::Lock();
        if (m_MinimumNDataSets < m_MinimumNDataSetsMax) {
          m_MinimumNDataSets = (5*m_MinimumNDataSets)/4;
          if (m_MinimumNDataSets > m_MinimumNDataSetsMax) {
            m_MinimumNDataSets = m_MinimumNDataSetsMax;
          }
        }
        // Hard coded storage limit...
        TThread::UnLock();
        // Sleep for 0.025 sec and hope we have more events now
        // Don't make the time too small, or the above "m_MinimumNDataSets" will grow insanely...
        TThread::Sleep(0, 50000000);
      }

      // Retrieve the data
      // We have to lock everything, since the store might go empty while we are within the loop!
      TThread::Lock();
      if (m_MoreEvents == true || m_NDataSets > 0) {
        if (m_NDataSets > 0) {
          MPhysicalEvent* P = m_DataSets.front();
          m_DataSets.pop_front();
          m_NDataSets--;
          TThread::UnLock();

          // update the end of the observation time
          if (m_HasObservationTime == false) {
            if (m_HasStartObservationTime == false) {
              m_StartObservationTime = P->GetTime();
              m_HasStartObservationTime = true;
            }
            m_EndObservationTime = P->GetTime();
            m_HasEndObservationTime = true;
          }

          return P;
        }
      }
      // We exit when there are no more events left
      if (m_MoreEvents == false && m_NDataSets == 0) {
        TThread::UnLock();
        break;
      }
      TThread::UnLock();

    } // while (true)
  }

  // No more events left or user abort...
  return 0;
}


////////////////////////////////////////////////////////////////////////////////


MPhysicalEvent* MFileEventsTra::ReadNextEvent()
{
  // Return the next event... or 0 if it is the last one
  // So remember to test for more events!

  MPhysicalEvent* Phys = nullptr;

  if (m_IncludeFileUsed == true) {
    Phys = ((MFileEventsTra*) m_IncludeFile)->GetNextEvent();
    if (Phys != nullptr) {
      return Phys;
    } else {
      if (m_IncludeFile->IsCanceled() == true) {
        m_Canceled = true;
      }
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;

      if (m_Canceled == true) return nullptr;
    }
  }

  m_EventType = MPhysicalEvent::c_Unknown;

  // Read until we reach a CO or PA or <to be continued>
  MString Line;
  while (IsGood() == true) {
    if (ReadLine(Line) == false) break;
    if (Line.Length() < 2) continue;

    if (Line[0] == 'E' && Line[1] == 'T') {
      if (Line.Length() < 5) {
        cout<<"Error reading event type"<<endl;
        m_EventType = MPhysicalEvent::c_Unknown;
        break;
      }
      if (Line[3] == 'C' && Line[4] == 'O') {
        MComptonEvent* P = new MComptonEvent();
        P->Stream(*this, m_Version, true, m_Fast, m_ParseDelayed);
        m_EventType = MPhysicalEvent::c_Compton;
        Phys = (MPhysicalEvent*) P;
        break;
      } else  if (Line[3] == 'P' && Line[4] == 'A') {
        MPairEvent* P = new MPairEvent();
        P->Stream(*this, m_Version, true, m_Fast, m_ParseDelayed);
        m_EventType = MPhysicalEvent::c_Pair;
        Phys = (MPhysicalEvent*) P;
        break;
      } else  if (Line[3] == 'P' && Line[4] == 'H') {
        MPhotoEvent* P = new MPhotoEvent();
        P->Stream(*this, m_Version, true, m_Fast, m_ParseDelayed);
        m_EventType = MPhysicalEvent::c_Photo;
        Phys = (MPhysicalEvent*) P;
        break;
      } else  if (Line[3] == 'M' && Line[4] == 'U') {
        MMuonEvent* P = new MMuonEvent();
        P->Stream(*this, m_Version, true, m_Fast, m_ParseDelayed);
        m_EventType = MPhysicalEvent::c_Muon;
        Phys = (MPhysicalEvent*) P;
        break;
      } else  if (Line[3] == 'U' && Line[4] == 'N') {
        MUnidentifiableEvent* P = new MUnidentifiableEvent();
        P->Stream(*this, m_Version, true, m_Fast, m_ParseDelayed);
        m_EventType = MPhysicalEvent::c_Unidentifiable;
        Phys = (MPhysicalEvent*) P;
        break;
      } else {
        cout<<"Unknown event"<<endl;
        m_EventType = MPhysicalEvent::c_Unknown;
        break;
      }
    } else if (Line[0] == 'N' && Line[1] == 'F') {

      if (OpenNextFile(Line) == 0) {
        mout<<"Did not find a valid continuation file..."<<endl;
        return 0;
      }
    } else if (Line[0] == 'I' && Line[1] == 'N') {

      if (OpenIncludeFile(Line) == true) {
        Phys = ((MFileEventsTra*) m_IncludeFile)->GetNextEvent();
        if (Phys == 0) {
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        } else {
          return Phys;
        }
      } else {
        m_IncludeFile->Close();
        m_IncludeFileUsed = false;
      }
    }
  }

  // If this is the case, we have reached the end of the file
  if (Phys == nullptr) {
    ReadFooter(true);
  }

  return Phys;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsTra::AddText(const MString& Text)
{

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  if (m_IsOpen == false) return false;

  if (m_IncludeFileUsed == true) {
    ((MFileEventsTra*) m_IncludeFile)->AddText(Text);
    if (m_IncludeFile->GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  } else {
    Write(Text);
    if (m_IsIncludeFile == false && GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsTra::AddEvent(MPhysicalEvent* Tra)
{
  // Add an event to the stream

  if (Tra == nullptr) return false;

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  if (m_IsOpen == false) return false;

  if (m_IncludeFileUsed == true) {
    ((MFileEventsTra*) m_IncludeFile)->AddEvent(Tra);
    if (m_IncludeFile->GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  } else {
    ostringstream out;
    out<<"SE"<<endl;
    Write(out);
    Tra->Stream(*this, m_Version, false);
    if (m_IsIncludeFile == false && GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  }

  return true;
}


// MFileEventsTra.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
