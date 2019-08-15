/*
 * MFileEvents.cxx
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
// MFileEvents
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileEvents.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MTime.h"
#include "MTokenizer.h"
#include "MGUIProgressBar.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileEvents)
#endif


////////////////////////////////////////////////////////////////////////////////


const int MFileEvents::c_NoId = -1;


////////////////////////////////////////////////////////////////////////////////


MFileEvents::MFileEvents() : MFile()
{
  // Construct an instance of MFileEvents

  m_MEGAlibVersion = "";
  m_GeometryFileName = "";

  m_NEvents = 0;
  m_ExtensionNumber = 0;

  m_IncludeFile = nullptr;
  m_IncludeFileUsed = false;
  m_IsIncludeFile = false;

  m_OriginalFileName = "";

  m_HasStartObservationTime = false;
  m_HasEndObservationTime = false;

  m_ObservationTime = 0.0;
  m_HasObservationTime = false;

  m_NIncludeFiles = 0;
  m_NOpenedIncludeFiles = 0;
}


////////////////////////////////////////////////////////////////////////////////


MFileEvents::~MFileEvents()
{
  // Delete this instance of MFileEvents

  delete m_IncludeFile;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::Open(MString FileName, unsigned int Way)
{
  // Open the file and read some basic data common to all MEGAlib event files

  if (MFile::Open(FileName, Way) == false) {
    return false;
  }

  // If we are in read mode, we search for version, type, etc. information....
  // Since this function is time critical it is written partly in C
  if (Way == c_Read) {
    bool FoundVersion = false;
    bool FoundType = false;
    bool FoundGeometry = false;
    bool FoundMEGAlibVersion = false;
    bool FoundTB = false;
    bool FoundIN = false;

    m_HasStartObservationTime = false;
    m_HasEndObservationTime = false;

    m_ObservationTime = 0.0;
    m_HasObservationTime = false;

    m_NIncludeFiles = 0;
    m_NOpenedIncludeFiles = 0;

    m_NIncludeFiles = 0;
    m_NOpenedIncludeFiles = 0;

    int Lines = 0;
    int MaxLines = 100;

    MString Line;
    while (IsGood() == true) {
      if (++Lines >= MaxLines) break;
      if (ReadLine(Line) == false) {
        break;
      }
      if (FoundType == false) {
        if (Line.BeginsWith("Ty") == true || Line.BeginsWith("TY") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() != 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read file type (should be "<<m_FileType<<")"<<endl;
          } else {
            m_FileType = Tokens.GetTokenAtAsString(1);
            m_FileType.ToLower();
            FoundType = true;
          }
        }
      }
      if (FoundVersion == false) {
        if (Line.BeginsWith("Version") == true || Line.BeginsWith("VE") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() != 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read file version."<<endl;
          } else {
            m_Version = Tokens.GetTokenAtAsInt(1);
            FoundVersion = true;
          }
        }
      }
      if (FoundGeometry == false) {
        if (Line.BeginsWith("Geometry") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() < 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read geometry name."<<endl;
          } else {
            m_GeometryFileName = Tokens.GetTokenAfterAsString(1);
            FoundGeometry = true;
          }
        }
      }
      if (FoundMEGAlibVersion == false) {
        if (Line.BeginsWith("MEGAlib") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() < 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read geometry name."<<endl;
          } else {
            m_MEGAlibVersion = Tokens.GetTokenAfterAsString(1);
            FoundMEGAlibVersion = true;
          }
        }
      }
      if (FoundTB == false) {
        if (Line.BeginsWith("TB") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() != 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read TB keyword"<<endl;
          } else {
            m_StartObservationTime = Tokens.GetTokenAtAsDouble(1);
            m_HasStartObservationTime = true;
            FoundTB = true;
          }
        }
      }
      if (FoundIN == false) {
        if (Line.BeginsWith("IN") == true) {
          ++m_NIncludeFiles;
          ++MaxLines;
        }
      }
    }
  }


  // Store the file name, since it might change when we go through a file tree via NF keyword
  m_OriginalFileName = m_FileName;

  // Now rewind - don't use the local one, since it reopens the file...
  MFile::Rewind();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::ReadFooter(bool Continue)
{
  // Read the footer of the file

  streampos Position = GetUncompressedFilePosition();

  if (Continue == false) {
    MFile::Rewind(!m_IsIncludeFile);
    if (GetUncompressedFileLength() > (streampos) 100000) {
      Seek(GetUncompressedFileLength(false) - streamoff(100000));
    }
  }

  MString Line;
  int NLinesRead = 0;
  while (IsGood() == true) {
    if (ReadLine(Line) == false) {
      break;
    }
    NLinesRead++;
    if (Line.Length() < 2) continue;

    ParseFooter(Line);
  }


  if (Continue == false) {
    Clear(); // We very likely have reached the end of the file
    Seek(Position);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::ParseFooter(const MString& Line)
{
  // Parse the footer

  // In case the job crashed badly we might have no TE, thus use the last TI
  if (Line[0] == 'T' && Line[1] == 'I') {
    MTokenizer Tokens;
    Tokens.Analyze(Line);
    if (Tokens.GetNTokens() < 2) {
      mout<<"Error while opening file "<<m_FileName<<": "<<endl;
      mout<<"Unable to read TI keyword"<<endl;
      return false;
    } else {
      m_EndObservationTime = Tokens.GetTokenAtAsDouble(1);
      m_ObservationTime = m_EndObservationTime - m_StartObservationTime;
      m_HasObservationTime = true;
    }
  }
  if (Line[0] == 'T' && Line[1] == 'E') {
    MTokenizer Tokens;
    Tokens.Analyze(Line);
    if (Tokens.GetNTokens() != 2) {
      mout<<"Error while opening file "<<m_FileName<<": "<<endl;
      mout<<"Unable to read TE keyword"<<endl;
      return false;
    } else {
      m_EndObservationTime = Tokens.GetTokenAtAsDouble(1);
      m_HasEndObservationTime = true;
      m_ObservationTime = m_EndObservationTime - m_StartObservationTime;
      m_HasObservationTime = true;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::Rewind(bool)
{
  // Rewind to the beginning of the file
  // Since we might be somewhere within a file tree, we simply start over

  if (m_IsOpen == false) {
    return false;
  }

  bool ProgressShown = (m_Progress != nullptr) ? true : false;
  Close();
  Open(m_OriginalFileName, m_Way);
  ShowProgress(ProgressShown);

  return true;
}

////////////////////////////////////////////////////////////////////////////////


MTime MFileEvents::GetObservationTime()
{
  if (m_HasObservationTime == false) {
    if (m_HasStartObservationTime == true && m_HasEndObservationTime == true) {
      m_ObservationTime = m_EndObservationTime - m_StartObservationTime;
    } else {
      ReadFooter();
    }
  }

  return m_ObservationTime;
}

////////////////////////////////////////////////////////////////////////////////


void MFileEvents::SetGeometryFileName(const MString Geometry)
{
  // Set the geoemtry

  m_GeometryFileName = Geometry;
  MFile::ExpandFileName(m_GeometryFileName);
}


////////////////////////////////////////////////////////////////////////////////


MString MFileEvents::GetGeometryFileName() const
{
  // Return the geometry

  return m_GeometryFileName;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::WriteHeader()
{
  // Write basic header information

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  MTime Now;

  ostringstream Header;
  Header<<"Type      "<<m_FileType<<endl;
  Header<<"Version   "<<m_Version<<endl;
  Header<<"Geometry  "<<m_GeometryFileName<<endl;
  Header<<endl;
  Header<<"Date      "<<Now.GetSQLString()<<endl;
  Header<<"MEGAlib   "<<g_VersionString<<endl;
  Header<<endl;
  Write(Header);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::AddFooter(const MString& Text)
{
  // Add text to the file as footer - it always goes into the master file!
  // You should NOT add events afterwards!

  // There is no test is the file size is exceeded!

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  if (m_IsOpen == false) return false;

  // Text always goes to the main file!
  if (Text.IsEmpty() == false) {
    ostringstream ToWrite;
    ToWrite<<endl<<"FT START"<<endl;
    ToWrite<<Text<<endl;
    ToWrite<<"FT STOP"<<endl<<endl;
    Write(ToWrite);

    if (GetFileLength() >= numeric_limits<streamsize>::max()) {
      mout<<"Warning: Writing footer resulted in exceeding of max file size..."<<endl;
      mout<<"         Some closing remarks might be lost..."<<endl;
    }
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::CloseEventList()
{
  // Close the event list after last event (per default add "EN"(D))

  if (m_Way == c_Read) {
    merr<<"Only valid if file is in write-mode!"<<endl;
    massert(m_Way != c_Read);
    return false;
  }

  ostringstream ToWrite;
  ToWrite<<"EN"<<endl;
  ToWrite<<endl;
  ToWrite<<"TE "<<m_ObservationTime<<endl;
  ToWrite<<endl;
  Write(ToWrite);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::OpenNextFile(const MString& Line)
{
  // Switch to the next file...

  MTokenizer Tokens;
  Tokens.Analyze(Line);
  if (Tokens.GetNTokens() < 2 || Tokens.GetTokenAtAsString(0) != "NF") {
    mout<<"Cannot parse \"NF\" (next) file!"<<endl;
    return false;
  }
  MString FileName = Tokens.GetTokenAfterAsString(1);


  // If the original file was zipped, we are now in a temporary directory
  // and most likely cannot find the file
  FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + FileName;
  if (m_WasZipped == true && FileName.EndsWith(".gz") == false) {
    FileName += ".gz";
  }
  ExpandFileName(FileName);

  // Temporarily store the progress info:
  bool Progress = IsShowProgress();

  Close();

  // We need to wait after Close() to assign the new file name!
  m_FileName = FileName;

  MTime ObservationTime = GetObservationTime(); // preserve the original observation time
  MString OriginalFileName = m_OriginalFileName; // preserve the original file name
  if (Open(m_FileName) == false) {
    return false;
  }
  m_OriginalFileName = OriginalFileName;
  m_ObservationTime += ObservationTime;

  // Reset the progress info:
  ShowProgress(Progress);

  mout<<"Changing to new file "<<m_FileName<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::CreateNextFile()
{
  // Write the NF information to the current file and open the next one...

  // Create new file name

  // (a) Remove Suffix
  MString NewFileName = m_FileName;
  NewFileName.Remove(NewFileName.Length() - m_FileType.Length()-1, m_FileType.Length()+1);
  //cout<<NewFileName<<endl;
  // (b) Check for Extension
  if (m_ExtensionNumber > 0) {
    ostringstream out;
    out<<".id"<<m_ExtensionNumber;
    MString Id(out.str().c_str());
    NewFileName.Remove(NewFileName.Length() - Id.Length(), Id.Length());
    //cout<<NewFileName<<endl;
  }
  // (c) Build the new name:
  m_ExtensionNumber++;
  ostringstream New;
  New<<NewFileName<<".id"<<m_ExtensionNumber<<"."<<m_FileType;
  NewFileName = New.str().c_str();
  //cout<<NewFileName<<endl;


  // Write NF information - per default all Event files are is c_Cpp / Ascii mode
  ostringstream out;
  out<<"SE"<<endl;
  out<<"NF "<<NewFileName<<endl;
  Write(out);

  // Open new file
  Close();
  m_FileName = NewFileName;
  if (MFile::Open(m_FileName, c_Write) == false) {
    return false;
  }

  // Write header information - this ensures that the file can be used by itself
  WriteHeader();

  mout<<"Changing to new file "<<m_FileName<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileEvents::CreateIncludeFileName(const MString& FileName)
{

  // (a) Remove Suffix
  MString NewFileName = FileName;
  NewFileName.Remove(NewFileName.Length() - m_FileType.Length()-1, m_FileType.Length()+1);
  //cout<<"Remove type: "<<NewFileName<<endl;
  // (b) Check for Extension
  if (m_ExtensionNumber > 0) {
    ostringstream out;
    out<<".id"<<m_ExtensionNumber;
    MString Id(out.str().c_str());
    NewFileName.Remove(NewFileName.Length() - Id.Length(), Id.Length());
    //cout<<"Remove extension: "<<NewFileName<<endl;
  }
  // (c) Build the new name:
  m_ExtensionNumber++;
  ostringstream New;
  New<<NewFileName<<".id"<<m_ExtensionNumber<<"."<<m_FileType;
  NewFileName = New.str().c_str();
  //cout<<"New file: "<<NewFileName<<endl;

  return NewFileName;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::CreateIncludeFile()
{
  // Write the NF information to the current file and open the next one...

  // Create new file name

  MString IncludeFileName;

  if (m_IncludeFileUsed == false) {
    // Close this file:
    CloseEventList();
    Close();

    // And rename it:
    IncludeFileName = CreateIncludeFileName(m_FileName);
    gSystem->Rename(m_FileName, IncludeFileName);

    // Reopen it as new and write the header:
    if (MFile::Open(m_FileName, c_Write) == false) {
      return false;
    }

    // Write header information - this ensures that the file can be used by itself
    WriteHeader();
    ostringstream out;
    out<<"IN "<<MFile::RelativeFileName(IncludeFileName, m_FileName)<<endl;
    Write(out);
  } else {
    IncludeFileName = m_IncludeFile->GetFileName();
    m_IncludeFile->CloseEventList();
    m_IncludeFile->Close();
  }

  // Open the next file:
  MString NewFileName = CreateIncludeFileName(IncludeFileName);

  // Write IN information - per default all Event files are is c_Cpp / Ascii mode
  ostringstream out;
  out<<"IN "<<MFile::RelativeFileName(NewFileName, m_FileName)<<endl<<flush;
  Write(out);

  cout<<"Changing to new include file "<<MFile::RelativeFileName(NewFileName, m_FileName)<<endl;

  // Open new file
  if (m_IncludeFile->Open(NewFileName, c_Write) == false) {
    return false;
  }
  m_IncludeFileUsed = true;
  m_IncludeFile->SetVersion(GetVersion());
  m_IncludeFile->SetGeometryFileName(GetGeometryFileName());

  // Write header information - this ensures that the file can be used by itself
  m_IncludeFile->WriteHeader();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::CloseIncludeFile()
{
  if (m_IncludeFile != nullptr && m_IncludeFile->IsOpen()) {
    m_ObservationTime += m_IncludeFile->GetObservationTime();
    m_HasObservationTime = true;
    m_IncludeFile->Close();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::OpenIncludeFile(const MString& Line)
{
  // Open an include file

  if (IsCanceled() == true) return false;
  UpdateProgress();

  MTokenizer Tokens;
  Tokens.Analyze(Line);
  if (Tokens.GetNTokens() < 2 || Tokens.GetTokenAtAsString(0) != "IN") {
    mout<<"Cannot parse \"IN\"clude file!"<<endl;
    return false;
  }
  MString FileName = Tokens.GetTokenAfterAsString(1);


  // If the original file was zipped, we are now in a temporary directory
  // and most likely cannot find the file
  if (FileName.BeginsWith("/") == false) {
    FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + FileName;
  }
  if (m_WasZipped == true && FileName.EndsWith(".gz") == false) {
    FileName += ".gz";
  }

  ExpandFileName(FileName);

  CloseIncludeFile(); // Updates also observation time
  m_IncludeFileUsed = true;

  if (m_IncludeFile->Open(FileName) == false) {
    m_IncludeFileUsed = false;
    return false;
  }

  if (m_Progress != nullptr) {
    m_IncludeFile->SetProgress(m_Progress, m_ProgressLevel+1);
    UpdateProgress();
  }

  mout<<"Switched to file "<<FileName<<endl;

  ++m_NOpenedIncludeFiles;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MFileEvents::GetNEvents(bool Count)
{
  // Search the highest Id and return it

  // Return the number of events in this file:
  // Warning this resets the current file position!!

  // Save the current position in order to go back to it
  streampos CurrentPos = GetUncompressedFilePosition();
  MString CurrentFileName = m_FileName;

  Seek(0, ios_base::end);
  streampos Max = GetUncompressedFilePosition();
  streamoff Start = Max - streampos(10000);
  if (int(Start) < 0) Start = streampos(0);
  Seek(Start, ios_base::beg);

  int Id = c_NoId;

  if (Count == false) {
    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) {
        break;
      }
      if (Line.Length() < 2) continue;

      if (Line[0] == 'S' && Line[1] == 'E') {

        if (sscanf(Line.Data(), "SE %i", &Id) != 1) {
          if (sscanf(Line.Data(), "SE %i;%*i;%*i;%*i", &Id) != 1) {
            Id = c_NoId;
          }
          Id = c_NoId;
        }
      } else if  (Line[0] == 'I' && Line[1] == 'D') {
        if (sscanf(Line.Data(), "ID %i %*i", &Id) != 1) {
          Id = c_NoId;
        }
      } else if  (Line[0] == 'I' && Line[1] == 'N') {
        Count = true;
        break;
      } else if  (Line[0] == 'N' && Line[1] == 'F') {
        Count = true;
        break;
      }
    }
  }

  if (Count == true || Id == c_NoId) {
    Id = 0;
    Rewind();

    MString Line;
    while (IsGood() == true) {
      if (ReadLine(Line) == false) {
        break;
      }
      if (Line[0] == 'S' && Line[1] == 'E') {
        Id++;
      } else if (Line[0] == 'I' && Line[1] == 'N') {
        if (OpenIncludeFile(Line) == true) {
          Id += m_IncludeFile->GetNEvents(true);
          m_IncludeFile->Close();
          m_IncludeFileUsed = false;
        } else {
          Id = -1;
          break;
        }
      } else if (Line[0] == 'N' && Line[1] == 'F') {
        if (OpenNextFile(Line) == 0) {
          Id = -1;
          break;
        }
      }
    }
  }

  if (m_FileName != CurrentFileName) {
    Close();
    if (MFile::Open(CurrentFileName) == false) {
      return 0;
    }
  }

  Clear();
  Seek(CurrentPos);

  return Id;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::UpdateProgress(unsigned int UpdatesToSkip)
{
  // Update the Progress Dialog, if it is visible
  // Return false, when "Cancel" has been pressed

  // We cannot update the progress bar from anything but the main thread
  if (TThread::SelfId() != g_MainThreadID) {
    //cout<<"Update wrong thread: "<<TThread::SelfId()<<":"<<g_MainThreadID<<endl;
    return true;
  }

  // GUI is not allowed to be accessed from multiple threads!
  m_ProgressMutex.Lock();

  if (m_Canceled == true) {
    m_ProgressMutex.UnLock();
    return false;
  }
  if (m_Progress == nullptr || GetFileLength() == (streampos) 0) {
    m_ProgressMutex.UnLock();
    return true;
  }

  if (++m_SkippedProgressUpdates < UpdatesToSkip) {
    m_ProgressMutex.UnLock();
    return true;
  }
  m_SkippedProgressUpdates = 0;

  double Value = 0;
  if (m_NIncludeFiles > 0) {
    if (m_NOpenedIncludeFiles > 0) {
      Value = (double) (m_NOpenedIncludeFiles - 1) / (double) m_NIncludeFiles; // -1 since we do not count the currently opened as finished
      if (Value < 0) Value = 0;
    }
  } else {
    Value = (double) GetFilePosition() / (double) GetFileLength();
  }

  m_Progress->SetValue(Value, m_ProgressLevel);
  if (TThread::SelfId() == g_MainThreadID) {
    gSystem->ProcessEvents();
  }

  if (m_Progress->TestCancel() == true) {
    ShowProgressNoLock(false);
    m_Canceled = true;
    m_ProgressMutex.UnLock();

    return false;
  }

  m_ProgressMutex.UnLock();

  if (m_IncludeFile != nullptr) {
    m_IncludeFile->UpdateProgress(UpdatesToSkip);
  }

  return true;
}


// MFileEvents.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
