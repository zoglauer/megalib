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

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
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

  m_IncludeFile = 0;
  m_IncludeFileUsed = false;
  m_IsIncludeFile = false;

  m_OriginalFileName = "";

  m_ObservationTime = 0.0;
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
    bool FoundTE = false;

    int Lines = 0;
    int MaxLines = 100;

    double BeginObservationTime = 0.0;
    double EndObservationTime = 0.0;

    MString Line;
    while (m_File.good() == true) {
      if (++Lines >= MaxLines) break;
      Line.ReadLine(m_File);
      
      if (FoundType == false) {
        if (Line.BeginsWith("Type") == true) {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() != 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read file type (should be "<<m_FileType<<")"<<endl;              
          } else {
            m_FileType = Tokens.GetTokenAtAsString(1);
            FoundType = true;
          }
        }
      }
      if (FoundVersion == false) {
        if (Line.BeginsWith("Version") == true) {
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
            BeginObservationTime = Tokens.GetTokenAtAsDouble(1);
            FoundTB = true;
          }
        }
      }
    }
    // Now go to the end of the file to find the TE keyword
    m_File.clear();
    if (m_FileLength > (streampos) 100000) {
      m_File.seekg(m_FileLength - streamoff(100000));
    } else {
      // start from the beginning...
      MFile::Rewind();
    }
    Line.ReadLine(m_File); // Ignore the first line
    while (m_File.good() == true) {
      Line.ReadLine(m_File);
      if (Line.Length() < 2) continue;

      // In case the job crashed badly we might have no TE, thus use the last TI
      if (Line[0] == 'T' && Line[1] == 'I') {
        MTokenizer Tokens;
        Tokens.Analyze(Line);
        if (Tokens.GetNTokens() < 2) {
          mout<<"Error while opening file "<<m_FileName<<": "<<endl;
          mout<<"Unable to read TI keyword"<<endl;              
        } else {
          EndObservationTime = Tokens.GetTokenAtAsDouble(1);
        }
      }
      if (FoundTE == false) {
        if (Line[0] == 'T' && Line[1] == 'E') {
          MTokenizer Tokens;
          Tokens.Analyze(Line);
          if (Tokens.GetNTokens() != 2) {
            mout<<"Error while opening file "<<m_FileName<<": "<<endl;
            mout<<"Unable to read TE keyword"<<endl;              
          } else {
            EndObservationTime = Tokens.GetTokenAtAsDouble(1);
            FoundTE = true;
          }
        }
      }
    }
    MFile::Rewind();

//     // Uncomment the MEGAlib stuff some time in the future (now = 2008-09-15)
//     if (FoundVersion == false || FoundType == false || FoundGeometry == false /*|| FoundMEGAlibVersion == false*/) { 
//       mout<<"A problem occurred while opening file "<<m_FileName<<": "<<endl;
//       mout<<"The following information is not found within the first "<<MaxLines<<" lines:"<<endl;
//       if (FoundVersion == false) {
//         mout<<"Version information not found!"<<endl;
//       }
//       if (FoundType == false) {
//         mout<<"Type information not found!"<<endl;
//       }
//       if (FoundGeometry == false) {
//         mout<<"Geometry information not found!"<<endl;
//       }
//       /*
//       if (FoundMEGAlibVersion == false) {
//         mout<<"MEGAlib version information not found!"<<endl;
//       }
//       */
//     }

//     if (FoundTB == true) { // FoundTE is not required since if TE is not there, the last TI is used
//       m_ObservationTime = EndObservationTime - BeginObservationTime;
//     } else {
//       m_ObservationTime = 0.0;
//     }
    m_ObservationTime = EndObservationTime - BeginObservationTime;
  }


  // Store the file name, since it might change when we go through a file tree via NF keyword
  m_OriginalFileName = m_FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFileEvents::Rewind()
{
  // Rewind to the beginning of the file
  // Since we might be somewhere within a file tree, we simply start over

  if (m_IsOpen == false) {
    return false;
  }

  bool ProgressShown = (m_Progress != 0) ? true : false;
  Close();
  Open(m_OriginalFileName, m_Way);
  ShowProgress(ProgressShown);

  return true;
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

  m_File<<"Type      "<<m_FileType<<endl;
  m_File<<"Version   "<<m_Version<<endl;
  m_File<<"Geometry  "<<m_GeometryFileName<<endl;
  m_File<<endl;
  m_File<<"Date      "<<Now.GetSQLString()<<endl;
  m_File<<"MEGAlib   "<<g_VersionString<<endl;
  m_File<<endl;

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
    m_File<<endl<<"FT START"<<flush;
    m_File<<Text<<flush;
    m_File<<"FT STOP"<<endl<<flush;

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

  m_File<<"EN"<<endl;
  m_File<<endl;
  m_File<<"TE "<<m_ObservationTime<<endl;
  m_File<<endl;
  
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
  if (m_WasZipped == true) {
    FileName = MFile::GetDirectoryName(m_ZippedFileName) + MString("/") + gSystem->BaseName(FileName) + MString(".gz");
  } else {
    FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + gSystem->BaseName(FileName);
  }

  ExpandFileName(FileName);

  // Temporarily store the progress info:
  bool Progress = IsShowProgress();

  Close();

  // We need to wait after Close() to assign the new file name! 
  m_FileName = FileName;

  double ObservationTime = m_ObservationTime; // preserve the original observation time
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
  cout<<NewFileName<<endl;
  // (b) Check for Extension
  if (m_ExtensionNumber > 0) {
    ostringstream out;
    out<<".id"<<m_ExtensionNumber;
    MString Id(out.str().c_str());
    NewFileName.Remove(NewFileName.Length() - Id.Length(), Id.Length());    
    cout<<NewFileName<<endl;
  }
  // (c) Build the new name:
  m_ExtensionNumber++;
  ostringstream New;
  New<<NewFileName<<".id"<<m_ExtensionNumber<<"."<<m_FileType;
  NewFileName = New.str().c_str();
  cout<<NewFileName<<endl;


  // Write NF information - per default all Event files are is c_Cpp / Ascii mode
  m_File<<"SE"<<endl;
  m_File<<"NF "<<NewFileName<<endl<<flush;
  

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
    m_File<<"IN "<<MFile::RelativeFileName(IncludeFileName, m_FileName)<<endl<<flush;
  } else {
    IncludeFileName = m_IncludeFile->GetFileName();
    m_IncludeFile->CloseEventList();
    m_IncludeFile->Close();
  }

  // Open the next file:
  MString NewFileName = CreateIncludeFileName(IncludeFileName);

  // Write IN information - per default all Event files are is c_Cpp / Ascii mode
  m_File<<"IN "<<MFile::RelativeFileName(NewFileName, m_FileName)<<endl<<flush;

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


bool MFileEvents::OpenIncludeFile(const MString& Line)
{
  // Open an include file

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
  if (m_WasZipped == true) {
    FileName = MFile::GetDirectoryName(m_ZippedFileName) + MString("/") + FileName + MString(".gz");
  } else {
    FileName = MFile::GetDirectoryName(m_FileName) + MString("/") + FileName;
  }

  ExpandFileName(FileName);

  m_IncludeFile->Close();
  m_IncludeFileUsed = true;

  if (m_IncludeFile->Open(FileName) == false) {
    m_IncludeFileUsed = false;
    return false;
  }

  if (m_Progress != 0) {
    m_IncludeFile->SetProgress(m_Progress, m_ProgressLevel+1);
    m_Progress->SetValue(0, m_ProgressLevel+1);
  }

  m_ObservationTime += m_IncludeFile->GetObservationTime();

  mout<<"Swiched to "<<FileName<<" and added "<<m_IncludeFile->GetObservationTime()<<" sec observation time."<<endl;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


int MFileEvents::GetNEvents(bool Count) 
{
  // Search the highest Id and return it

  // Return the number of events in this file:
  // Warning this resets the current file position!!

  streampos Current = m_File.tellg();
  MString CurrentFileName = m_FileName;

  m_File.seekg(0, ios_base::end);
  streampos Max = m_File.tellg();
  streamoff Start = Max - streampos(10000);
  if (int(Start) < 0) Start = streampos(0); 
  m_File.seekg(Start, ios_base::beg);

  int Id = c_NoId;

  if (Count == false) {
    MString Line;
    while (m_File.good() == true) {
      Line.ReadLine(m_File);
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
    while (m_File.good() == true) {
      Line.ReadLine(m_File);
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
  
  m_File.clear();
  m_File.seekg(Current, ios_base::beg);
  
  return Id;
}


// MFileEvents.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
