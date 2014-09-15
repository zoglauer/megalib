/*
 * MFile.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFile__
#define __MFile__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TFile.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIProgressBar.h"

// Forward declarations:
#include <fstream>
#include <streambuf>
using namespace std;

////////////////////////////////////////////////////////////////////////////////


class MFile
{
  // public interface:
 public:
  //! Default constructor
  MFile();
  //! Default destructor
  virtual ~MFile();

  //! Close the file and reset all data to default values
  virtual void Reset();

  //! Open the file for reading or writing 
  virtual bool Open(MString FileName, unsigned int Way = c_Read);
  //! Close the file
  virtual bool Close();
  //! Rewind the file
  virtual bool Rewind();

  //! Return true if the file is open
  virtual bool IsOpen();
  //! Return the file length
  virtual streampos GetFileLength();
  //! Return the current file position
  virtual streampos GetFilePosition();


  //! Set the file name - this does not open any file and you have to give the file name when you call Open()
  void SetFileName(MString FileName) { m_FileName = FileName; }
  //! Get the file name
  MString GetFileName() const { return m_FileName; }

  //! Set the file version
  void SetVersion(const int Version);
  //! Get the current file version
  int GetVersion() const;

  //! Set the file type e.g. "sim" or "tra"
  void SetFileType(const MString& Type) { m_FileType = Type; }
  //! Get the file type e.g. "sim" or "tra"
  MString GetFileType() const { return m_FileType; }

  //! Returns true if the progress dialog is shown
  bool IsShowProgress();
  //! Show the progress dialog GUI
  virtual void ShowProgress(bool Show = true);
  //! Update the progress dialog GUI -- allow to skip a certain amount of updates  
  virtual bool UpdateProgress(unsigned int UpdatesToSkip = 0);
  //! Set the titles of the progress dialog GUI -- only works after a call to ShowProgress
  void SetProgressTitle(MString Main, MString Sub);
  //! Use another progress dialog GUI instead of this one
  void SetProgress(MGUIProgressBar* ProgressBar, int Level);

  //! Return true if the file extensions are correct
  virtual bool CheckFileExtension(MString Extension);

  //! Return true is the file exists and if it is readable
  static bool Exists(MString FileName);
  //! Return true is the file exists
  static bool FileExists(const char* FileName); // depreciated
  //! Check if a program exists
  static bool ProgramExists(MString Program);
  //! Sets the path of "Path" as new path to "FileName" IF FileName has a relative path 
  static bool ApplyPath(MString& FileName, const MString& Path);
  //! Expand a file name e.g. $MEGALIB/src to /home/andreas/Software/MEGAlib/src
  static void ExpandFileName(MString& FileName, const MString& WorkingDir = "");
  //! Make RelFileName relative to AbsFileName
  //! Relative: /home/andreas/Test/MyFile.tra
  //! Absolute: /home/andreas/MasterTest.tra
  //! Return:   ./Test/MyFile.tra
  static MString RelativeFileName(MString RelFileName, MString AbsFileName);
  //! Return the directory name (replacement for inconsistent gSystem->DirName)
  static MString GetDirectoryName(const MString& Name);
  //! Return the directory name (replacement for inconsistent gSystem->DirName)
  static MString GetBaseName(const MString& Name);


  //! The file modes: Write to a new file  
  static unsigned int c_Write;
  //! The file modes: Write to a new file  
  static unsigned int c_Create;
  //! The file modes: Read from an existing file  
  static unsigned int c_Read;

  // protected methods:
 protected:

  // private methods:
 private:

  //public members
 public:

  // protected members:
 protected:
  //! Name of the file (empty string if not yet open)
  MString m_FileName;

  MString m_FileType;
  int m_Version;

  //! True if the file is open
  bool m_IsOpen;      

  //! The Mode: read or write
  unsigned int m_Way;

  //! The basic file stream
  fstream m_File;

  //! The known file length in READ mode
  streampos m_FileLength;
  //! The maximum allowed file length
  streamsize m_MaxFileLength;

  //! The Progress bar
  MGUIProgressBar* m_Progress;
  //! Files might be deeply nested - show a progressbar for every nesting
  int m_ProgressLevel;
  //! True, if the progress bar has been created here
  bool m_OwnProgress;
  //! Count the skipped progress updates
  unsigned int m_SkippedProgressUpdates;
  //! The progress has been cancelled via GUI
  bool m_Canceled;

  //! True if the original file was compress
  bool m_WasZipped;
  //! FileName of the original zipped file
  MString m_ZippedFileName;

  //! String indicating an unknwon file type
  static const MString c_TypeUnknown;
  //! ID indicating an unknown version ID
  static const int c_VersionUnknown;

  // private members:
 private:



#ifdef ___CINT___
 public:
  ClassDef(MFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
