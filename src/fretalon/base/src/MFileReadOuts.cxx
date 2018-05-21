/*
 * MFileReadOuts.cxx
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
// MFileReadOuts
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileReadOuts.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MStreams.h"
#include "MTokenizer.h"
#include "MFretalonRegistry.h"
#include "MReadOut.h"
#include "MReadOutElement.h"
#include "MReadOutElementDoubleStrip.h"
#include "MReadOutData.h"
#include "MReadOutDataADCValue.h"
#include "MReadOutDataTiming.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileReadOuts)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MFileReadOuts::MFileReadOuts() : MFileEvents()
{
  // Construct an instance of MFileReadOuts
  
  m_FileType = "roa";

  m_StartClock = numeric_limits<long>::max();
  m_EndClock = numeric_limits<long>::max();
  m_HasEndClock = false;
  
  m_NEventsInFile = 0;
  m_NGoodEventsInFile = 0;
  
  m_ROE = 0;
  m_ROD = 0;
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MFileReadOuts::~MFileReadOuts()
{
  delete m_ROE;
  delete m_ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Open the file
bool MFileReadOuts::Open(MString FileName, unsigned int Way)
{
  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileReadOuts();
  m_IncludeFile->SetIsIncludeFile(true);

  if (MFileEvents::Open(FileName, c_Read) == false) {
    return false;
  }
    
  bool Error = false;
  bool FoundUF = false;
  bool FoundCB = false;
  m_HasEndClock = true;
  
  MString ReadOutElementFormat = "";
  MString ReadOutDataFormat = "";
  
  delete m_ROE;
  m_ROE = 0;
  delete m_ROD;
  m_ROD = 0;
  
  int Lines = 0;
  int MaxLines = 100;
  
  
  // Stage one: Find out what kind of file we have
  MFile::Rewind();
  
  MString Line;
  while (IsGood() == true) {
    
    if (++Lines >= MaxLines) break;
    if (ReadLine(Line) == false) break;
    
    if (FoundUF == false) {
      if (Line.BeginsWith("UF") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 3) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read UF keyword"<<endl;              
          Error = true;
        } else {
          ReadOutElementFormat = Tokens.GetTokenAt(1);
          ReadOutDataFormat = Tokens.GetTokenAt(2);
          FoundUF = true;
          //cout<<"Found: read-out element format: "<<ReadOutElementFormat<<", read-out data format: "<<ReadOutDataFormat<<endl;
        }
      }
    }
    if (FoundCB == false) {
      if (Line.BeginsWith("CB") == true) {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() != 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read file version."<<endl;              
          Error = true;
        } else {
          m_StartClock = Tokens.GetTokenAtAsDouble(1);
          FoundCB = true;
        }
      }
    }
  }
  MFile::Rewind();
  
  if (ReadOutElementFormat == "" || ReadOutDataFormat == "") {
    cout<<"No read-out element type / data format found in the file!"<<endl;
    Close();
    return false;
  }
  
  // Create the read-out elements and data to fill
  if ((m_ROE = MFretalonRegistry::Instance().GetReadOutElement(ReadOutElementFormat)) == 0) {
    cout<<"No read-out element of type \""<<ReadOutElementFormat<<"\" is registered!"<<endl;
    Close();
    return false;
  }
  
  // Assemble the ROD
  vector<MString> RODNames;
  int Minus = ReadOutDataFormat.Tokenize("-").size();
  int With = ReadOutDataFormat.Tokenize("with").size();
  if (Minus > With) {
    RODNames = ReadOutDataFormat.Tokenize("-");
  } else if (Minus < With) {
    RODNames = ReadOutDataFormat.Tokenize("with");
  } else {
    RODNames.push_back(ReadOutDataFormat); 
  }
  
  vector<MReadOutData*> RODs;
  for (auto Name: RODNames) {
    MReadOutData* ROD = MFretalonRegistry::Instance().GetReadOutData(Name);
    if (ROD == 0) {
      cout<<"No read-out data of type "<<Name<<" is registered!"<<endl;
      return false;
    }
    RODs.push_back(ROD);
  }
  
  m_ROD = 0; // should already been 0 before
  for (auto ROD: RODs) {
    MReadOutData* NewROD = ROD->Clone();
    NewROD->SetWrapped(m_ROD);
    m_ROD = NewROD;
  }
  
  // Now do the sanity checks:
  if (m_FileType != "dat" && m_FileType != "roa") {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"The file type must be \"dat\" or \"roa\" (case is ignored) - you have \""<<m_FileType<<"\""<<endl; 
    Close();
    Error = true;
    return false;
  }
  
  return !Error;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileReadOuts::ParseFooter(const MString& Line)
{
  // Parse the footer
  
  // Handle common data in the base class
  MFileEvents::ParseFooter(Line);
  
  if (Line[0] == 'C' && Line[1] == 'E') {
    MTokenizer Tokens;
    Tokens.Analyze(Line);
    if (Tokens.GetNTokens() != 2) {
      mout<<"Error while opening file "<<m_FileName<<": "<<endl;
      mout<<"Unable to read CE keyword"<<endl;
      return false;
    } else {
      m_EndClock = Tokens.GetTokenAtAsDouble(1);
      m_HasEndClock = true;
    }
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileReadOuts::ReadNext(MReadOutSequence& ROS, int SelectedDetectorID)
{
  // Return next single event from file... or 0 if there are no more.
  
  if (IsOpen() == false) {
    return false; 
  }
  
  ROS.Clear();
  
  bool Error = false;
  MString Line;
  
  if (UpdateProgress(50) == false) {
    return false;
  }

  // If we have an include file, we get the event from it!
  if (m_IncludeFileUsed == true) {
    bool Return = dynamic_cast<MFileReadOuts*>(m_IncludeFile)->ReadNext(ROS, SelectedDetectorID);
    if (ROS.GetNumberOfReadOuts() == 0 || Return == false) {
      m_IncludeFile->Close();
      m_IncludeFileUsed = false;
    } else {
      m_NGoodEventsInFile++;
      return true;
    }
  }

  
  
  MTokenizer T(' ', false);  
  
  // Read file line-by-line, returning 'Event' when it's read a complete, non-empty event.
  while (IsGood() == true) {
    if (ReadLine(Line) == false) break;
    if (Line.Length() < 2) continue;
          
    // Part 1: The event is completed.  Check to see if we're at the following "SE".
    if ((Line[0] == 'S' && Line[1] == 'E') ||
        (Line[0] == 'I' && Line[1] == 'N')) {
      // If the event is empty, then we ignore it and prepare for the next event:
      //mout << "MNCTFileEventsDat::ReadNextEvent: Done reading event" << endl;
      m_NEventsInFile++;
      if (ROS.GetNumberOfReadOuts() == 0) {
        ROS.Clear();
      } else {
        // Done reading a non-empty event.  Return it:
        //mout<<"MNCTFileEventsDat::ReadNextEvent: Returning good event: "<<long(Event)<<endl;
        m_NGoodEventsInFile++;
        if (Error == true) {
          mout<<"An error occured during reading the event with ID "<<ROS.GetID()<<endl;
          mout<<"(If the error is really bad, then there might event not be an ID)"<<endl;
          mout<<"I pass the event on anyway."<<endl;
        }
        return !Error;
      }
    } // SE
    
    // Part 2: Handle IN
    if (Line[0] == 'I' && Line[1] == 'N') {

      if (OpenIncludeFile(Line) == true) {
        //mout<<"Switched to new include file: "<<m_IncludeFile->GetFileName()<<endl;
        // Now we have to read the first event:
        bool Return = dynamic_cast<MFileReadOuts*>(m_IncludeFile)->ReadNext(ROS, SelectedDetectorID);
        if (ROS.GetNumberOfReadOuts() == 0 || Return == false) {
          //mout<<"Closing: "<<m_IncludeFile->GetFileName()<<endl;
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        } else {
          m_NGoodEventsInFile++;
          return true;
        }        
      } else {
        mgui<<"Your current file contains a \"IN\" -- include file -- directive."<<endl
            <<"However, the file could not be found or read: "<<m_IncludeFile->GetFileName()<<show;
      }
      
      continue;
    }
      
    // Part 3: Handle UH - this can only be done here.
    if (Line[0] == 'U' && Line[1] == 'H') {
      T.AnalyzeFast(Line);
      
      m_ROE->Parse(T, 1);
      m_ROD->Parse(T, 1 + m_ROE->GetNumberOfParsableElements());
      
      // cout<<"Combined: "<<m_ROD->ToString()<<" vs. "<<m_ROD->GetCombinedType()<<endl;
      
      if (SelectedDetectorID < 0 || (SelectedDetectorID >= 0 && (int) m_ROE->GetDetectorID() == SelectedDetectorID)) {
        MReadOut RO(*m_ROE, *m_ROD);
        ROS.AddReadOut(RO);
        //cout<<"Added: "<<RO.ToString()<<endl;
      }
      continue;
    }
    
    // Part 4: All the rest is handled in the MReadOutSequence and its derived class MReadOutAssembly
    ROS.Parse(Line);
     
  } // End of while(m_File.good() == true)
  
  // Done reading.  No more new events.
  if (ROS.GetNumberOfReadOuts() == 0) {
    ROS.Clear();
  } else {
    // Done reading a non-empty event.  Return it:
    //mout << "MNCTFileEventsDat::GetNextEvent: Returning good event (at end of function)" << endl;
    m_NGoodEventsInFile++;
    if (Error == true) {
      mout<<"An error occured during reading the event with ID "<<ROS.GetID()<<endl;
      mout<<"(If the error is really bad, then there might event not be an ID)"<<endl;
      mout<<"I pass the event on anyway."<<endl;
    }
    return !Error;
  }
  
  //cout<<"Returning 0"<<endl;
  
  return false;
}


// MFileReadOuts.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
