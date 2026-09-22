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
}


////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MFileReadOuts::~MFileReadOuts()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Open the read-out file
bool MFileReadOuts::Open(MString FileName, unsigned int Way)
{
  m_IncludeFileUsed = false;
  m_IncludeFile = new MFileReadOuts();
  m_IncludeFile->SetIsIncludeFile(true);

  if (MFileEvents::Open(FileName, c_Read) == false) {
    return false;
  }
    
  bool Error = false;
  bool FoundCB = false;
  m_HasEndClock = true;

  m_ReadOutFileFormat.Clear();
  m_ReadOutPrototypes.clear();
  
  int Lines = 0;
  int MaxLines = 100;
  
  
  // Stage one: Find out what kind of file we have
  MFile::Rewind();
  
  MString Line;
  while (IsGood() == true) {
    
    if (++Lines >= MaxLines) break;
    if (ReadLine(Line) == false) break;
    
    if (Line.BeginsWith("UF") == true) {
      if (AddReadOutUnitPrototype(Line) == false) {
        mout<<"Error while opening file "<<m_FileName<<": "<<endl;
        mout<<"Unable to parse UF line, or unable to add the new read-out file format"<<endl;
        Error = true;
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
  
  if (m_ReadOutFileFormat.GetNumberOfReadOutUnits() == 0) {
    mout<<"Error in file: "<<m_FileName<<":"<<endl;
    mout<<"No read-out element type / data format found in the file!"<<endl;
    Close();
    return false;
  }

  // Now do the sanity checks:
  if (m_FileType != "dat" && m_FileType != "roa") {
    mout<<"Error while opening file "<<m_FileName<<": "<<endl;
    mout<<"The file type must be \"dat\" or \"roa\" (case is ignored) - you have \""<<m_FileType<<"\""<<endl;
    Close();
    return false;
  }
  
  return !Error;
}


////////////////////////////////////////////////////////////////////////////////


//! Build the read-out data described by a UF read-out data format such as "adc" or "adcwithtiming".
//! Returns nullptr on error
MReadOutData* MFileReadOuts::CreateReadOutData(const MString& ReadOutDataFormat)
{
  // Split the format in read-out data names
  vector<MString> RODNames;
  int Underscore = ReadOutDataFormat.Tokenize("_").size();
  int Minus = ReadOutDataFormat.Tokenize("-").size();
  int With = ReadOutDataFormat.Tokenize("with").size();
  if (Minus > 1 && Underscore == 1 && With == 1) {
    RODNames = ReadOutDataFormat.Tokenize("-");
  } else if (Minus == 1 && Underscore > 1 && With == 1) {
    RODNames = ReadOutDataFormat.Tokenize("_");
  } else if (Minus == 1 && Underscore == 1 && With > 1) {
    RODNames = ReadOutDataFormat.Tokenize("with");
  } else {
    RODNames.push_back(ReadOutDataFormat);
  }

  // Get the read-out data prototypes from the registry and wrap them if necessary
  MReadOutData* ROD = nullptr;
  for (auto Name: RODNames) {
    MReadOutData* New = MFretalonRegistry::Instance().GetReadOutData(Name);
    if (New == nullptr) {
      mout<<"Error in file: "<<m_FileName<<":"<<endl;
      mout<<"No read-out data of type "<<Name<<" is registered!"<<endl;
      delete ROD;
      return nullptr;
    }
    New->SetWrapped(ROD);
    ROD = New;
  }

  return ROD;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse a UF line containing the read-out unit data and add it to the prototypes
//! Return false on error
bool MFileReadOuts::AddReadOutUnitPrototype(const MString& Line)
{
  // Parse into a copy: the format is only updated once the prototype exists, so both stay index-aligned
  MReadOutFileFormat Format = m_ReadOutFileFormat;
  if (Format.ParseUF(Line) == false) return false;
  // Already known, e.g. the header is read again after the rewind in Open()
  if (Format.GetNumberOfReadOutUnits() == m_ReadOutFileFormat.GetNumberOfReadOutUnits()) return true;

  const unsigned int Unit = Format.GetNumberOfReadOutUnits() - 1;

  MReadOutElement* ROE = MFretalonRegistry::Instance().GetReadOutElement(Format.GetElementType(Unit));
  if (ROE == nullptr) {
    mout<<"Error in file: "<<m_FileName<<":"<<endl;
    mout<<"No read-out element of type \""<<Format.GetElementType(Unit)<<"\" is registered!"<<endl;
    return false;
  }

  MReadOutData* ROD = CreateReadOutData(Format.GetDataType(Unit));
  if (ROD == nullptr) {
    delete ROE;
    // Error message written in CreateReadOutData
    return false;
  }

  // MReadOut clones both, so the originals are released again right away
  m_ReadOutPrototypes.push_back(MReadOut(*ROE, *ROD));
  delete ROE;
  delete ROD;

  m_ReadOutFileFormat = Format;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the special information at the end of file
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


//! Return the next event
//! If SelectedDetectorID is non-negative then restrict yourself to SelectedDetectorID
//! SelectedDetectorSide:
//!   < 0: all
//!     0: negative side
//!     1: positive side
//!   >=2: all
bool MFileReadOuts::ReadNext(MReadOutSequence& ROS, int SelectedDetectorID, int SelectedDetectorSide)
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
    bool Return = dynamic_cast<MFileReadOuts*>(m_IncludeFile)->ReadNext(ROS, SelectedDetectorID, SelectedDetectorSide);
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
        bool Return = dynamic_cast<MFileReadOuts*>(m_IncludeFile)->ReadNext(ROS, SelectedDetectorID, SelectedDetectorSide);
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
      

    // Part 3: Handle a UF line appearing after the header, e.g., from a different file
    if (Line.BeginsWith("UF") == true) {
      if (AddReadOutUnitPrototype(Line) == false) {
        mout<<"Error in file: "<<m_FileName<<":"<<endl;
        mout<<"Unable to use the read-out unit declared by \""<<Line<<"\""<<endl;
      }
      continue;
    }


    // Part 4: Parse the read-out and add it to the store
    const unsigned int Unit = m_ReadOutFileFormat.FindByLine(Line);
    if (Unit != g_UnsignedIntNotDefined) {
      T.AnalyzeFast(Line);

      MReadOut& Prototype = m_ReadOutPrototypes[Unit];

      MReadOutElement& ROE = Prototype.GetReadOutElement();
      ROE.Parse(T, 1);

      MReadOutData& ROD = Prototype.GetReadOutData();
      ROD.Parse(T, 1 + ROE.GetNumberOfParsableElements());

      if (SelectedDetectorID < 0 || (SelectedDetectorID >= 0 && (int) ROE.GetDetectorID() == SelectedDetectorID)) {
        if (SelectedDetectorSide < 0 || SelectedDetectorSide >= 2 || (dynamic_cast<MReadOutElementDoubleStrip*>(&ROE) != nullptr && (int) dynamic_cast<MReadOutElementDoubleStrip*>(&ROE)->IsLowVoltageStrip() == SelectedDetectorSide)) {
          ROS.AddReadOut(Prototype);
        }
      }
      continue;
    }
    

    // Part 5: All the rest of the parsing is handled in the MReadOutSequence and its derived class MReadOutAssembly
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
