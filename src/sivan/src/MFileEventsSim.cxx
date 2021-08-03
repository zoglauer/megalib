/*
 * MFileEventsSim.cxx
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
// MFileEventsSim
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEventsSim.h"

// Standard libs:
#include <string>
using namespace std;

// ROOT libs:
#include "Rtypes.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileEventsSim)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileEventsSim::MFileEventsSim(MDGeometryQuest* Geometry) : MFileEvents()
{
  // Construct an instance of MFileEventsSim

  m_Geometry = Geometry;
  Init();
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsSim::MFileEventsSim() : MFileEvents()
{
  // Construct an instance of MFileEventsSim

  m_Geometry = nullptr;

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MFileEventsSim::~MFileEventsSim()
{
  // Delete this instance of MFileEventsSim
}


////////////////////////////////////////////////////////////////////////////////


void MFileEventsSim::Init()
{
  // Construct an instance of MFileEventsSim

  m_FileType = "sim";
  m_SimulationStartAreaFarField = 0.0;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::Open(MString FileName, unsigned int Way, bool IsBinary)
{
  // Open the file

  if (m_Geometry == nullptr) {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"No geometry present"<<endl;    
    return false;
  }

  m_IncludeFileUsed = false;
  if (m_IncludeFile != nullptr) {
    delete m_IncludeFile; 
  }
  m_IncludeFile = new MFileEventsSim();
  dynamic_cast<MFileEventsSim*>(m_IncludeFile)->SetGeometry(m_Geometry);
  m_IncludeFile->SetIsIncludeFile(true);

  if (MFileEvents::Open(FileName, Way, IsBinary) == false) {
    return false;
  }
  
  m_SimulatedEvents = 0;
  m_SimulationStartAreaFarField = 0;
  m_HasSimulatedEvents = false;

  if (Way == c_Read) {
    if (m_Geometry->IsScanned() == false) {
      mout<<"We do not have a properly initialized geometry!"<<endl;
      return false;
    }

    // Find some initial keywords - but read at least until first SE or IN:
    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) break;

      if (Line.BeginsWith("SimulationStartAreaFarField") == true || Line.BeginsWith("StartAreaFarField") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read SimulationStartAreaFarField"<<endl;              
        } else {
          m_SimulationStartAreaFarField = Tokens.GetTokenAtAsDouble(1);
        }
      } else if (Line.BeginsWith("IN") == true) {
        break; 
      } else if (Line.BeginsWith("SE") == true) {
        break; 
      } else if (Line.BeginsWith("STARTBINARYSTREAM") == true) {
        m_IsBinary = true;
        break; 
      }
    }
  }
  
  
  // Now rewind - we need a clean slate for the GetNextEvent parser
  MFile::Rewind();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::ParseFooter(const MString& Line)
{
  // Parse the footer
  
  // Handle common data in the base class
  MFileEvents::ParseFooter(Line);
  
  // In case the job crashed badly we might have no TS, thus use the last ID
  if (Line.BeginsWith("ID") == true) {
    MTokenizer Tokens;
    Tokens.Analyze(Line);
    if (Tokens.GetNTokens() == 3) {
      m_SimulatedEvents = Tokens.GetTokenAtAsInt(2);
      m_HasSimulatedEvents = true;
    }
  }
  if (Line.BeginsWith("TS") == true) {
    MTokenizer Tokens;
    Tokens.Analyze(Line);
    if (Tokens.GetNTokens() != 2) {
      mout<<"Error while opening file "<<m_FileName<<": "<<endl;
      mout<<"Unable to read TS keyword"<<endl;
      return false;
    } else {
      m_SimulatedEvents = Tokens.GetTokenAtAsInt(1);
      m_HasSimulatedEvents = true;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


long MFileEventsSim::GetSimulatedEvents()
{
  //! Return the number of simulated events

  if (m_HasSimulatedEvents == false) {
    ReadFooter(); 
  }
  
  return m_SimulatedEvents;
}
  

////////////////////////////////////////////////////////////////////////////////


void MFileEventsSim::UpdateObservationTimes(MSimEvent* Event)
{
  //! Update the observation times using the given event
  
  // If the overall observation time has alreday been set, don't change anything
  if (m_HasObservationTime == true) return;
  
  // Otherwise set everything
  if (m_HasStartObservationTime == false) {
    m_StartObservationTime = Event->GetTime();
    m_HasStartObservationTime = true;
  }
  m_EndObservationTime = Event->GetTime();
  m_HasEndObservationTime = true;
  
  // Do not set m_HasObservationTime
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent* MFileEventsSim::GetNextEvent(bool Analyze)
{
  if (m_IsBinary == true) {
    return GetNextEventBinary(Analyze);
  } else {
    return GetNextEventASCII(Analyze);
  }
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent* MFileEventsSim::GetNextEventBinary(bool Analyze)
{
  // Return the next event... or nullptr if it is the last one
  // So remember to test for no more events!
  
  // Start with updating the progress, return 0 if cancel has pressed
  if (UpdateProgress() == false) return nullptr;  
  
  
  MSimEvent* SimEvent = nullptr;
  
  if (m_IncludeFileUsed == true) {
    SimEvent = dynamic_cast<MFileEventsSim*>(m_IncludeFile)->GetNextEvent(Analyze);
    if (SimEvent == nullptr) {
      if (m_IncludeFile->IsCanceled() == true) m_Canceled = true;
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;
      if (m_Canceled == true) return nullptr;
    } else {
      UpdateObservationTimes(SimEvent);
      return SimEvent;
    }
  }

  if (GetFilePosition() == 0) {
  
    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) break;
      if (Line == "STARTBINARYSTREAM") {
        break;
      }
      // Include another file:
      else if (Line[0] == 'I' && Line[1] == 'N') {

        if (OpenIncludeFile(Line) == true) {
        
          if (SimEvent != nullptr) {
            merr<<"A sim file should either contain includes IN or events and not both!"<<endl;
            delete SimEvent;
            SimEvent = nullptr;
          }
        
          SimEvent = dynamic_cast<MFileEventsSim*>(m_IncludeFile)->GetNextEvent(Analyze);
          if (SimEvent == nullptr) {
            m_IncludeFile->Close();
            m_IncludeFileUsed = false;;
          } else {
            UpdateObservationTimes(SimEvent);
            return SimEvent;
          }
        } else {
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        }
      } 
    }
  }

 
  // Max size of the binary store
  const unsigned long MaxFill = 500000;
  
  // We are in the binary file stream, let's make sure we have at least 1 MB in the data store
  if (IsGood()) {
    if (m_BinaryStore.GetArraySizeUnread() < 0.8*MaxFill) {
      m_BinaryStore.Truncate();
      Read(m_BinaryStore, MaxFill - m_BinaryStore.GetArraySize());
    }
  }

  if (m_BinaryStore.GetArraySizeUnread() >= 2) {
    MString Flag = m_BinaryStore.GetString(2);
    m_BinaryStore.ProgressPosition(-2);
    if (Flag != "SE" && Flag != "EN") {
      cout<<"ERROR: File parsing error. Expected SE or EN, but got: "<<Flag<<". Progressing to next sync flag"<<endl;
      while (Flag != "SE" && Flag != "EN") {
        m_BinaryStore.ProgressPosition(+1);
        Flag = m_BinaryStore.GetString(2);
        m_BinaryStore.ProgressPosition(-2);
        
        if (m_BinaryStore.GetArraySizeUnread() < 0.8*MaxFill) {
          m_BinaryStore.Truncate();
          Read(m_BinaryStore, MaxFill - m_BinaryStore.GetArraySize());
        }
      }
      cout<<"INFO: Recovered. Found flag: "<<Flag<<endl;
    }
    
    
    if (Flag == "SE") {
      if (SimEvent == nullptr) {
        SimEvent = new MSimEvent();
        SimEvent->SetGeometry(m_Geometry);
      }
      
      try {
        if (SimEvent->ParseBinary(m_BinaryStore) == false) {
          delete SimEvent;
          SimEvent = nullptr;
        }
      } catch (...) {
        cout<<"ERROR: Unable to read a binary event!"<<endl;
        if (SimEvent != nullptr) {
          delete SimEvent;
          SimEvent = nullptr;
        }
      }
      
      // That's a dirty little trick in case we forgot the EN in the file
      // Works since there is currently no case where a sim file has neither no IAs or no HTs...
      if (SimEvent != nullptr && (SimEvent->GetNIAs() != 0 || SimEvent->GetNHTs() != 0)) {
        UpdateObservationTimes(SimEvent);
        return SimEvent;
      }
    } else if (Flag == "EN") {
      ReadFooter(true);
    }
  }
  
  

  // The end of the (master) file has been reached...
  ShowProgress(false);

  // Final clean up
  if (SimEvent != nullptr) {
    delete SimEvent;
  }

  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent* MFileEventsSim::GetNextEventASCII(bool Analyze)
{
  // Return the next event... or nullptr if it is the last one
  // So remember to test for no more events!
  
  // Start with updating the progress, return 0 if cancel has pressed
  if (UpdateProgress() == false) return nullptr;  
  
  
  MSimEvent* SimEvent = nullptr;
  
  if (m_IncludeFileUsed == true) {
    SimEvent = dynamic_cast<MFileEventsSim*>(m_IncludeFile)->GetNextEvent(Analyze);
    if (SimEvent == nullptr) {
      if (m_IncludeFile->IsCanceled() == true) m_Canceled = true;
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;
      if (m_Canceled == true) return nullptr;
    } else {
      UpdateObservationTimes(SimEvent);
      return SimEvent;
    }
  }
  
  bool BeyondFirstSE = true;
  if (GetFilePosition() == 0) BeyondFirstSE = false;
  
  MString Line;
  while (IsGood() == true) {
    if (ReadLine(Line) == false) break;
    if (Line.Length() < 2) continue;
    
    if (Line[0] == 'S' && Line[1] == 'E') {
      if (BeyondFirstSE == true) { // If this is not the first event...
        if (SimEvent != nullptr) {
          if (Analyze == true) {
            SimEvent->Analyze();
          }
          UpdateObservationTimes(SimEvent);
        }
        return SimEvent;
      } else {
        BeyondFirstSE = true;
        SimEvent = new MSimEvent();
        SimEvent->SetGeometry(m_Geometry);
      }
    } else if (Line[0] == 'N' && Line[1] == 'F') {
      if (OpenNextFile(Line) == 0) {
        mout<<"Did not find a valid continuation file..."<<endl;
        return nullptr;
      }
      
      if (SimEvent != nullptr) {
        if (Analyze == true) {
          SimEvent->Analyze();
        }
        UpdateObservationTimes(SimEvent);
        return SimEvent;
      }
    } 
    // Include another file:
    else if (Line[0] == 'I' && Line[1] == 'N') {
      
      if (OpenIncludeFile(Line) == true) {
        
        if (SimEvent != nullptr) {
          merr<<"A sim file should either contain includes IN or events and not both!"<<endl;
          delete SimEvent;
          SimEvent = nullptr;
        }
        
        SimEvent = dynamic_cast<MFileEventsSim*>(m_IncludeFile)->GetNextEvent(Analyze);
        if (SimEvent == nullptr) {
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;;
        } else {
          UpdateObservationTimes(SimEvent);
          return SimEvent;
        }
      } else {
        m_IncludeFile->Close();
        m_IncludeFileUsed = false;
      }
    } 
    // The end of file keyword
    else if (Line[0] == 'E' && Line[1] == 'N') {
      // What ever we decide to do next, first read the footer in continue mode
      ReadFooter(true);
      
      if (SimEvent != nullptr) {
        if (Analyze == true) {
          SimEvent->Analyze();
        }
        UpdateObservationTimes(SimEvent);
        return SimEvent;
      }
    } 
    // All other keyword directly related to an event
    else {
      // Let the raw event scan the line
      if (BeyondFirstSE == true) {
        if (SimEvent == nullptr) {
          SimEvent = new MSimEvent();
          SimEvent->SetGeometry(m_Geometry);
        }
        SimEvent->AddRawInput(Line, m_Version);
      }
    }
  }
  
  // That's a dirty little trick in case we forgot the EN in the file
  // Works since there is currently no case where a sim file has neither no IAs or no HTs...
  if (SimEvent != nullptr && (SimEvent->GetNIAs() != 0 || SimEvent->GetNHTs() != 0)) {
    UpdateObservationTimes(SimEvent);
    return SimEvent;
  }
  
  // The end of the (master) file has been reached...
  ShowProgress(false);
  
  // Final clean up
  if (SimEvent != nullptr) {
    delete SimEvent;
  }
  
  return nullptr;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::AddText(const MString& Text)
{
  // Write some text:

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  if (m_IsOpen == false) return false;

  if (m_IncludeFileUsed == true) {
    dynamic_cast<MFileEventsSim*>(m_IncludeFile)->AddText(Text);
    if (m_IncludeFile->GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  } else {
    Write(Text);
    Flush();
    if (m_IsIncludeFile == false && GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::AddEvent(MSimEvent* Event)
{
  // Write this event:

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }
  
  if (m_IsOpen == false) return false;

  if (m_IncludeFileUsed == true) {
    dynamic_cast<MFileEventsSim*>(m_IncludeFile)->AddEvent(Event);
    if (m_IncludeFile->GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  } else {
    if (m_IsBinary == true) {
      MBinaryStore Store;
      Event->ToBinary(Store, MSimEvent::c_StoreSimulationInfoAll, true, 25);
      Write(Store);
    } else {
      Write(Event->ToSimString(MSimEvent::c_StoreSimulationInfoAll, 0));
      Flush();
    }
    if (m_IsIncludeFile == false && GetFileLength() > GetMaxFileLength()) {
      return CreateIncludeFile();
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::OpenIncludeFile(const MString& Line)
{
  bool Return = MFileEvents::OpenIncludeFile(Line);

  if (Return == true) {
    m_SimulatedEvents += dynamic_cast<MFileEventsSim*>(m_IncludeFile)->GetSimulatedEvents();
  }

  return Return;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEventsSim::CloseEventList()
{
  // Close the event list after last event (per default add "EN"(D))

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  ostringstream out;
  if (m_IsBinary == true) {
    MBinaryStore S;
    S.AddString("EN", 2);
    Write(S);
    out<<endl<<"ENDBINARYSTREAM"<<endl;
  } else {
    out<<"EN"<<endl;
  }
  out<<endl;
  out<<"TE "<<m_ObservationTime<<endl;
  out<<"TS "<<m_SimulatedEvents<<endl;
  out<<endl;
  Write(out);
  
  return true;
}


// MFileEventsSim.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
