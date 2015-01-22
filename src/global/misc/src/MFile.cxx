/*
 * MFile.cxx
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


// MEGAlib:
#include "MGlobal.h"
#include "MAssert.h"
#include "MFile.h"
#include "MTimer.h"
#include "MStreams.h"

// ROOT libs:
#include <TSystem.h>
#include <TROOT.h>
#include <TRandom.h>
#include <TThread.h>

// Standard libs:
#include <iostream>
#include <streambuf>
#include <limits>
#include <cstdlib>
#include <cstdio>
using namespace std;


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFile)
#endif


const MString MFile::c_TypeUnknown = "___TypeUnknown___";
const int MFile::c_VersionUnknown = -1;


////////////////////////////////////////////////////////////////////////////////


unsigned int MFile::c_Write   = 0;
unsigned int MFile::c_Create  = 0;
unsigned int MFile::c_Read    = 1;


////////////////////////////////////////////////////////////////////////////////


MFile::MFile()
{
  // Construct an instance of MFile

  m_Progress = 0;  
  m_ZipFile = 0;

  Reset();
}


////////////////////////////////////////////////////////////////////////////////


MFile::~MFile()
{
  // Delete this instance of MFile

  Close();
  
  if (m_OwnProgress == true) {
    delete m_Progress;
  } else if (m_Progress != 0) {
    m_Progress->SetValue(0, m_ProgressLevel);
  }
}


////////////////////////////////////////////////////////////////////////////////



void MFile::Reset()
{
  // Close the file and set all values to default values
  
  Close();

  m_FileName = "";
  m_Way = c_Read;

  m_FileType = c_TypeUnknown;
  m_Version = c_VersionUnknown;

  m_IsOpen = false;

  m_FileLength = 0;

  // m_Progress = 0; // Already managed by initial close!
  m_ProgressLevel = 0;
  m_OwnProgress = true;
  m_Canceled = false;
  m_SkippedProgressUpdates = 0;

  // m_ZipFile = 0; // Already managed by the initial close
  m_WasZipped = false;
  m_ZippedFileName = g_StringNotDefined;

  m_FileLength = 0;
  m_HasFileLength = false;
  
  // The maximum allowed file length
  m_MaxFileLength = numeric_limits<streamsize>::max()/100*95;
  //m_MaxFileLength = 100000;
}


////////////////////////////////////////////////////////////////////////////////


void MFile::SetVersion(const int Version)
{
  // Set the version

  m_Version = Version;
}

 
////////////////////////////////////////////////////////////////////////////////


int MFile::GetVersion() const
{
  // Return the version of this file

  return m_Version;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::FileExists(const char* FileName)
{
  // Check if the file exists:

  return Exists(MString(FileName));  
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::Exists(MString FileName)
{
  // Check if the file exists:

  MFile::ExpandFileName(FileName);
  if (FileName.Length() <= 1) return false; 
  if (FileName.EndsWith("/") == true) return false;
  
  if (FileName == GetDirectoryName(FileName)) { // does not work in all cases..!
    return false;
  }

  // Check if we can open it:
  ifstream in;
  in.open(FileName, ios::in);
  if (in.is_open() == false) {
    return false;
  }
  // Get the file size:
  in.seekg(0, ios_base::end);
  streampos Length = in.tellg();
  in.seekg(0, ios_base::beg);
  if (in.good() == false) {
    in.close();
    return false;
  }
  
  // Check if we can read something
  if (Length > 0) {
    char c;
    in.get(c);
    if (in.good() == false) {
      in.close();
      return false;
    }
  }
  
  in.close();

  return true;  
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::ApplyPath(MString& FileName, const MString& Path)
{
  // Sets the path of "Path" as new path to "FileName"
  
  if (gSystem->IsAbsoluteFileName(FileName) == true) return true;

  MString NewPath = Path;
  ExpandFileName(NewPath);
  
  MString DirName = GetDirectoryName(NewPath); 
  
  FileName = DirName + MString("/") + FileName;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::ProgramExists(MString Program)
{
  // Return true if the specified file exists:

  char* Path;
  Path = gSystem->Which(gSystem->Getenv("PATH"), Program);

  if (Path != 0) {
    delete Path;
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////////////////


MString MFile::GetDirectoryName(const MString& Name)
{
  // Check if FileName is a directory, if not make it one

  MString WorkingDir = "";
  MString FileName = Name;

  ExpandFileName(FileName);  

  MString WindowsDir = "";
  if (FileName.Length() >= 2 && FileName[1] == ':') {
    WindowsDir = FileName.GetSubString(0, 2); 
  }
  WorkingDir += gSystem->DirName(FileName);
  
  // Some ROOT versions have a windows working dir bug:
  if (WindowsDir != "" && WorkingDir.BeginsWith(WindowsDir) == false) {
    WorkingDir = WindowsDir + WorkingDir;
  }
  
  return WorkingDir;
}


////////////////////////////////////////////////////////////////////////////////


void MFile::ExpandFileName(MString& FileName, const MString& WorkingDir)
{
  // Just in case: Expand file name:
  
  // We have to switch to TString...
  TString Name(FileName.Data());
  gSystem->ExpandPathName(Name);
  gSystem->ExpandPathName(Name);
  FileName = Name;
  
  // On Unix system we start always with a "/", on windows we have a : before the first "/" or "\"
  // Problem: What is with file:// something??
  if (FileName.BeginsWith("/") == false && FileName.BeginsWith("\\") == false && !(FileName.Length() >= 2 && FileName[1] == ':')) {
    if (WorkingDir == "") {
      FileName = MString(gSystem->WorkingDirectory()) + "/" + FileName;
    } else {
      FileName = GetDirectoryName(WorkingDir) + "/" + FileName;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::Open(MString FileName, unsigned int Way)
{
  // Open the file in different modes

  // Close the file just in case we are open
  Close();

  m_FileLength = 0;
  m_HasFileLength = false;
  
  m_FileName = FileName;
  if (m_FileName == "") {
    mgui<<"You need to give a file name, before I can open a file."<<error;
    return false;
  }

  // Just in case: Expand file name:
  ExpandFileName(m_FileName);

  // If the file is zipped we have to unzip it
  
  if (CheckFileExtension("gz") == true) {
    m_WasZipped = true;
  } else {
    m_WasZipped = false;
  }
  
  if (m_WasZipped == true) {
    if (Way == c_Read) {
      m_ZipFile = gzopen(m_FileName, "rb");
    } else {
      m_ZipFile = gzopen(m_FileName, "wb");
    }
    if (m_ZipFile == NULL) {
      mgui<<"Unable to open file \""<<m_FileName<<"\""<<endl;
      return false;
    }
  } else {
    m_File.clear();
    if (Way == c_Read) {
      m_File.open(m_FileName, ios_base::in);
    } else {
      m_File.open(m_FileName, ios_base::out);
    }
    if (m_File.is_open() == false) {
      mgui<<"Unable to open file \""<<m_FileName<<"\""<<endl;
      return false;
    }
  }

  // We are open now
  m_IsOpen = true;
  m_Way = Way;
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::Rewind()
{
  // Rewind to the beginning of the file

  if (m_IsOpen == false) {
    return false;
  }

  if (m_WasZipped == true) {
    gzrewind(m_ZipFile);
  } else {
    m_File.clear();
    m_File.seekg(0);
  }
  
  if (m_Progress != 0) {
    UpdateProgress();
    m_Progress->ResetTimer();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::Close()
{
  if (IsOpen() == false) return true;

  // Close the file first
  if (m_WasZipped == true) {
    gzclose(m_ZipFile);
  } else {
    m_File.close();
  }
  m_IsOpen = false;

  ShowProgress(false);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


  //! Return true is the file is good
bool MFile::IsGood() 
{ 
  if (m_WasZipped == true) {
    return (gzeof(m_ZipFile) == 0 ? true : false);
  } else {
    return m_File.good();
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFile::Clear() 
{ 
  if (m_WasZipped == true) {
    // Don't do anything...
  } else {
    return m_File.clear(); 
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFile::Seek(streampos Pos) 
{ 
  //! Seek the given position
  
  if (m_WasZipped == true) {
    gzseek(m_ZipFile, Pos, SEEK_SET);
  } else {
    m_File.seekg(Pos);
  }
}


////////////////////////////////////////////////////////////////////////////////


  //! Seek the given position
void MFile::Seek(streamoff Offset, ios_base::seekdir Way) 
{ 
  if (m_WasZipped == true) {
    if (Way == ios_base::beg) {
      gzseek(m_ZipFile, (z_off_t) Offset, SEEK_SET);
    } else if (Way == ios_base::cur) {
      gzseek(m_ZipFile, (z_off_t) Offset, SEEK_CUR);
    } else if (Way == ios_base::end) {
      gzseek(m_ZipFile, (z_off_t) Offset, SEEK_END);
    }
  } else {
    m_File.seekg(Offset, Way); 
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write some text
void MFile::Write(const ostringstream& S) 
{ 
  if (m_WasZipped == true) {
    gzputs(m_ZipFile, S.str().c_str());
  } else {
    m_File<<S.str(); 
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Write some text
void MFile::Write(const MString& S) 
{   
  if (m_WasZipped == true) {
    gzputs(m_ZipFile, S);
  } else {
    m_File<<S; 
  }
}



////////////////////////////////////////////////////////////////////////////////


//! Write some text
void MFile::Write(const char c) 
{ 
  if (m_WasZipped == true) {
    gzputc(m_ZipFile, c);
  } else {
    m_File<<c; 
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Flush all written text
void MFile::Flush() 
{ 
  if (m_WasZipped == true) {
    // We do not want to do this, since it degrades perfromance...
  } else {
    m_File<<flush;
  }
}
 
  


////////////////////////////////////////////////////////////////////////////////


//! Get one character
void MFile::Get(char& c) 
{ 
  if (m_WasZipped == true) {  
    c = '\0';
    int i = gzgetc(m_ZipFile);
    if (i == -1) {
      if (gzeof(m_ZipFile) == 0) {
        int ErrorCode;
        cout<<"Error in MFile::Get(char& c): "<<gzerror(m_ZipFile, &ErrorCode)<<endl;
      }
      return;
    }
    c = (char) i;
  } else {
    m_File.get(c);
  }
}


////////////////////////////////////////////////////////////////////////////////


//! Get one float
void MFile::Get(float& f) 
{ 
  if (m_WasZipped == true) {  
    f = 0;
    string temp;
    int i;
    while (gzeof(m_ZipFile) == 0) {
      i = gzgetc(m_ZipFile);
      if (i == -1) {
        if (gzeof(m_ZipFile) == 0) {
          int ErrorCode;
          cout<<"Error: "<<gzerror(m_ZipFile, &ErrorCode)<<endl;
        }
        return;
      }
      if (i == ' ' || i == '\n' || i == '\0' || i == '\t') break; 
      temp += (char) i;
    }
    f = atof(temp.c_str());
  } else {
    m_File>>f;
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::ReadLine(MString& String)
{
  //! Read one line

  String.Clear();
  
  if (m_WasZipped == true) {
    unsigned int Length = 1000;
    char* Buffer = new char[Length];
    char* Return = 0;
    
    do {
      Buffer[0] = '\0';
      Return = gzgets(m_ZipFile, Buffer, Length-1);
      if (Return == Z_NULL) {
        if (gzeof(m_ZipFile) == 0) {
          int ErrorCode;
          cout<<"Error: "<<gzerror(m_ZipFile, &ErrorCode)<<endl;
        }
        delete [] Buffer;
        return false;
      }
      String.Append(Return);
    } while (strlen(Return) == Length-2);
    String.RemoveAll('\n');
    delete [] Buffer;
  } else {
    String.ReadLine(m_File);
  }
  
  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::ReadLine(char* String, streamsize Size, char Delimeter)
{
  //! Read one line
  if (m_WasZipped == true) {
    for (unsigned int i = 0; i < Size; ++i) {
      int c = gzgetc(m_ZipFile);
      if (c == -1 || (char) c == Delimeter) {
        String[i] = '\0';
        return true; 
      }
      String[i] = (char) c;
    }
  } else {
    m_File.getline(String, Size, Delimeter);
  }
  
  return true;
}
  
  
////////////////////////////////////////////////////////////////////////////////


void MFile::ShowProgress(bool Show)
{
  //

  if (Show == true) {
    if (gClient == 0 || gClient->GetRoot() == 0 || gROOT->IsBatch() == true) {
      cout<<"Can't show progress bar, because you do not have a GUI"<<endl;
    } else {
      if (m_OwnProgress == true) {
        delete m_Progress;
      }
      m_Progress = new MGUIProgressBar(0, "Progress", "Progress of analysis");
      m_Progress->SetMinMax(0, 1);
      m_OwnProgress = true;
      m_Canceled = false;
      m_SkippedProgressUpdates = 0;
    }
  } else {
    if (m_OwnProgress == true) {
      delete m_Progress;
    }
    m_Progress = 0;
  }
}


////////////////////////////////////////////////////////////////////////////////


void MFile::SetProgressTitle(MString Main, MString Sub)
{
  // Set the title of the current progress bar

  if (m_Progress != 0) {
    m_Progress->SetTitles(Main, Sub);
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::IsShowProgress() 
{
  // Return show progress   

  if (m_Progress != 0) return true;
  return false;
}


////////////////////////////////////////////////////////////////////////////////


void MFile::SetProgress(MGUIProgressBar* Progress, int Level)
{
  // Take over a progressbar

  delete m_Progress;
  m_Progress = Progress;
  m_ProgressLevel = Level;
  m_OwnProgress = false;
  m_SkippedProgressUpdates = 0;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::UpdateProgress(unsigned int UpdatesToSkip)
{
  // Update the Progress Dialog, if it is visible
  // Return false, when "Cancel" has been pressed
  
  if (m_Canceled == true) return false;
  if (m_Progress == 0 || GetFileLength() == (streampos) 0) return true;

  if (++m_SkippedProgressUpdates < UpdatesToSkip) return true;
  m_SkippedProgressUpdates = 0;
  
  TThread::Lock(); // GUI is not allowed to be accessed from multiple threads!
  
  double Value = (double) GetFilePosition() / (double) GetFileLength();
  
  m_Progress->SetValue(Value, m_ProgressLevel);
  gSystem->ProcessEvents();

  TThread::UnLock();
  
  if (m_Progress->TestCancel() == true) {
    ShowProgress(false);
    m_Canceled = true;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::IsOpen()
{
  // Return true if file is open
  
  return m_IsOpen;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::CheckFileExtension(MString Extension)
{
  // Check if the extension of this file is "Extension"

  MString Upper(Extension);
  Upper.ToUpper();
  Upper.Prepend(".");

  MString Lower(Extension);
  Lower.ToLower();
  Lower.Prepend(".");

  if (m_FileName.EndsWith(Upper) == true || 
      m_FileName.EndsWith(Lower) == true) {
    return true;
  } else {
    return false;
  }
}


////////////////////////////////////////////////////////////////////////////////


MString MFile::RelativeFileName(MString RelFileName, MString AbsFileName) 
{
  // Make RelFileName relative to AbsFileName
  // Rel: /home/andreas/Test/MyFile.tra
  // Abs: /home/andreas/MasterTest.tra
  // Ret: ./Test/MyFile.tra

  MFile::ExpandFileName(RelFileName);
  MFile::ExpandFileName(AbsFileName);

  int MinLength = min(RelFileName.Last('/'), AbsFileName.Last('/'));
  
  int PreviousSlash = -1;
  for (int i = 0; i <= MinLength; ++i) {
    if (RelFileName[i] != AbsFileName[i]) {
      break;
    }
    if (RelFileName[i] == '/') PreviousSlash = i;
  }
  int RemainingSlashesAbs = 0;
  for (int i = PreviousSlash+1; i < int(AbsFileName.Length()); ++i) {
    if (AbsFileName[i] == '/') {
      RemainingSlashesAbs++;
    }
  }
  
  if (PreviousSlash > 0) { 
    RelFileName.Remove(0, PreviousSlash+1);

    if (RemainingSlashesAbs > 0) {
      for (int i = 0; i < RemainingSlashesAbs; ++i) {
        RelFileName.Prepend("../");
      }
    } else {
      RelFileName.Prepend("./");
    }
  }

  return RelFileName;
}


////////////////////////////////////////////////////////////////////////////////


MString MFile::GetBaseName(const MString& Name)
{
  //! Return the base name of a file

  if (Name.Length() == 0) return Name;
  if (Name.Length() == 1 && Name[0] == '/') return Name;
  size_t Pos = Name.Last('/');
  if (Pos == MString::npos) return Name;
  
  return Name.GetSubString(Pos+1);
}
  

////////////////////////////////////////////////////////////////////////////////


streampos MFile::GetUncompressedFileLength(bool Redetermine)
{
  // Return the file length
  // Since this is a random access operation it should be very fast...

  if (Redetermine == false && m_HasUncompressedFileLength == true) return m_UncompressedFileLength;
  
  if (IsOpen() == false) {
    merr<<"File "<<m_FileName<<" not open!"<<show;
    return 0;
  }

  streampos Length;
  if (m_WasZipped == true) {
    if (m_Way == c_Read) {
      
      // First get the compressed file size
      ifstream in;
      in.open(m_FileName);
      in.seekg(0, ios_base::end);
      double CompressedTotal = in.tellg();
      in.close();
      
      Length = 0;
      if (CompressedTotal > 1000000) {
        cout<<"Handling a larger compressed file... this might take a while to initialize..."<<endl;
        // Get the uncompressed file size after stepping ahead the compressed total ...
        double Uncompressed = gzseek(m_ZipFile, CompressedTotal, SEEK_SET);
        // ... and the compressed position
        gzgetc(m_ZipFile); // Need one to get the correct offset 
        double Compressed = gzoffset(m_ZipFile);
      
        //cout<<CompressedTotal<<":"<<Uncompressed<<":"<<Compressed<<endl;
        
        // Now calculate the 97% position minus 100000:
        streampos Ahead = (streampos) (Uncompressed / Compressed * CompressedTotal - 100000);
        Ahead = 0.97*Ahead;
        //cout<<"Ahead: "<<Ahead<<endl;
        
        Length = gzseek(m_ZipFile, Ahead, SEEK_SET);
      }
     
      while (gzgetc(m_ZipFile) != -1) Length += 1;
      gzrewind(m_ZipFile);
      
      //cout<<"Uncompressed file length: "<<Length<<endl;
    } else {
      Length = (streampos) gzoffset(m_ZipFile); // We are already at the end
    }
  } else {
    if (m_Way == c_Read) {
      streampos Current = m_File.tellg();
      m_File.seekg(0, ios_base::end);
      Length = m_File.tellg();
      m_File.seekg(Current, ios_base::beg);
    } else {
      Length = m_File.tellp(); // We are already at the end
    }
  }    

  m_UncompressedFileLength = Length;
  m_HasUncompressedFileLength = true;
  
  return Length;
}
  

////////////////////////////////////////////////////////////////////////////////


streampos MFile::GetFileLength(bool Redetermine)
{
  // Return the file length on disk

  if (Redetermine == false && m_HasFileLength == true) return m_FileLength;
  
  if (IsOpen() == false) {
    merr<<"File "<<m_FileName<<" not open!"<<show;
    return 0;
  }

  streampos Length;
  if (m_WasZipped == true) {
    if (m_Way == c_Read) {
      // First get the compressed file size
      ifstream in;
      in.open(m_FileName);
      in.seekg(0, ios_base::end);
      Length = in.tellg();
      in.close();
    } else {
      Length = (streampos) gzoffset(m_ZipFile); // We are already at the end
    }
  } else {
    if (m_Way == c_Read) {
      streampos Current = m_File.tellg();
      m_File.seekg(0, ios_base::end);
      Length = m_File.tellg();
      m_File.seekg(Current, ios_base::beg);
    } else {
      Length = m_File.tellp(); // We are already at the end
    }
  }    

  m_FileLength = Length;
  m_HasFileLength = true;
  
  return Length;
}


////////////////////////////////////////////////////////////////////////////////


streampos MFile::GetFilePosition()
{
  // Return the file length
  // Since this is a random access operation it should be very fast...

  if (IsOpen() == false) {
    merr<<"File "<<m_FileName<<" not open!"<<show;
    return 0;
  }

  streampos Pos;
  if (m_WasZipped == true) {
    Pos = (streampos) gzoffset(m_ZipFile);
  } else {
    Pos = m_File.tellg();
  }
  
  return Pos;
}


////////////////////////////////////////////////////////////////////////////////


streampos MFile::GetUncompressedFilePosition()
{
  // Return the file length
  // Since this is a random access operation it should be very fast...

  if (IsOpen() == false) {
    merr<<"File "<<m_FileName<<" not open!"<<show;
    return 0;
  }

  streampos Pos;
  if (m_WasZipped == true) {
    Pos = (streampos) gztell(m_ZipFile);
  } else {
    Pos = m_File.tellg();
  }
  
  return Pos;
}


// MFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
