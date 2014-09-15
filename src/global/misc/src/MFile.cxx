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
  // Set all values to default values

  m_FileName = "";
  m_Way = c_Read;

  m_FileType = c_TypeUnknown;
  m_Version = c_VersionUnknown;

  m_IsOpen = false;

  m_FileLength = 0;

  m_Progress = 0;
  m_ProgressLevel = 0;
  m_OwnProgress = true;
  m_Canceled = false;
  m_SkippedProgressUpdates = 0;

  m_WasZipped = false;
  m_ZippedFileName = g_StringNotDefined;

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

  ifstream in;
  in.open(FileName, ios::in);
  if (in.is_open() == false) {
    return false;
  }
  char c;
  in.get(c);
  if (in.good() == false) {
    in.close();
    return false;
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

  m_FileName = FileName;
  if (m_FileName == "") {
    mgui<<"You need to give a file name, before I can open a file."<<error;
    return false;
  }

  // Just in case: Expand file name:
  ExpandFileName(m_FileName);

  // If the file is zipped we have to unzip it
  m_WasZipped = false;
  if (Way == c_Read && CheckFileExtension("gz") == true) {
    MString Temp = gSystem->TempDirectory();

    // Create a new temporary file name
    MString NewFileName = "";
    do {
      NewFileName = Temp + MString("/");
      for (int i = 0; i < 32; ++i) {
        NewFileName += (char) (int('a') + gRandom->Integer(26));
      }
      NewFileName += ".";
      MString Type = m_FileType;
      Type.ToLower();
      NewFileName += Type;
    } while (Exists(NewFileName) == true);

    // Unzip
    mout<<"Unzipping file... please stand by..."<<endl;
    MString Unzip = MString("gunzip -c ") + m_FileName + MString(" > ") + NewFileName;
    int Success = gSystem->Exec(Unzip);

    if (Success != 0) {
      remove(NewFileName);
      mgui<<"Unable to unzip: "<<endl
          <<"\""<<m_FileName<<"\""<<error;
      return false;
    }

    // Store information about the old and new file
    m_WasZipped = true;
    m_ZippedFileName = m_FileName;
    m_FileName = NewFileName;
  }


  // Do a sanity check on the file type
  if (m_FileType != c_TypeUnknown) {
    if (CheckFileExtension(m_FileType) == false) {
      mgui<<"The file: "<<endl
          <<"\""<<m_FileName<<"\""
          <<endl<<"has not the correct extension \""<<m_FileType<<"\"!"<<error;
      return false;
    }
  }

  // Make sure the file exists when we try to read it
  if (Way == c_Read && FileExists(m_FileName) == false) {
    mgui<<"The file: "<<endl
        <<"\""<<m_FileName<<"\""<<endl
        <<"does not exist!"<<error;
    return false;
  }

  // Finally open it

  m_File.clear();
  if (Way == c_Write) {
    m_File.open(m_FileName, ios_base::out);
  } else {
    m_File.open(m_FileName, ios_base::in);
  }

  if (m_File.is_open() == false) {
    mgui<<"Unable to open file \""<<m_FileName<<"\""<<endl;
    return false;
  }

  // Determine the file length
  if (Way == c_Read) {
    m_File.seekg(0, ios_base::end);
    m_FileLength = m_File.tellg();
    m_File.seekg(0, ios_base::beg);
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

  m_File.clear();
  m_File.seekg(0);

  if (m_Progress != 0) {
    UpdateProgress();
    m_Progress->ResetTimer();
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MFile::Close()
{
  // If we created an unzipped copy we have to delete the unzipped version
  if (m_WasZipped == true) {
    // Sanity check: the file name must be in the temporary directory
    if (m_FileName.BeginsWith(gSystem->TempDirectory()) == true) {
      remove(m_FileName);
    }
  }

  if (IsOpen() == true) {
    m_File.close();
    m_IsOpen = false;
	}

  ShowProgress(false);

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
  if (m_Progress == 0 || m_FileLength == (streampos) 0) return true;

  if (++m_SkippedProgressUpdates < UpdatesToSkip) return true;
  m_SkippedProgressUpdates = 0;
  
  TThread::Lock(); // GUI is not allowed to be accessed from multiple threads!
  
  double Value = (double) m_File.tellg() / (double) m_FileLength;
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


streampos MFile::GetFileLength()
{
  // Return the file length
  // Since this is a random access operation it should be very fast...

  if (IsOpen() == false) {
    merr<<"File "<<m_FileName<<" not open!"<<show;
    return 0;
  }

  streampos Length;
  if (m_Way == c_Read) {
    streampos Current = m_File.tellg();
    m_File.seekg(0, ios_base::end);
    Length = m_File.tellg();
    m_File.seekg(Current, ios_base::beg);
  } else {
    Length = m_File.tellp();
  }

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

  return m_File.tellg();
}




// MFile.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
