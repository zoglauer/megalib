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
#include "zlib.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MGUIProgressBar.h"

// Forward declarations:
#include <fstream>
#include <sstream>
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
  //! Return true is the file is good
  virtual bool IsGood();
  //! Clear all flags
  virtual void Clear();

  //! Return the file length on disk
  virtual streampos GetFileLength(bool Redetermine = false);
  //! Return the file length as if the file were uncompressed
  virtual streampos GetUncompressedFileLength(bool Redetermine = false);

  //! Return the current file position on disk
  virtual streampos GetFilePosition();
  //! Return the current file position as if the file were uncompressed
  virtual streampos GetUncompressedFilePosition();

  //! Return the maximum allowed file length
  virtual streampos GetMaxFileLength() { return m_MaxFileLength; }

  //! Seek the given position
  virtual void Seek(streampos Pos);
  //! Seek the given position
  virtual void Seek(streamoff Offset, ios_base::seekdir Way);

  //! Write some text (and clear the stream)
  virtual void Write(const ostringstream& S);
  //! Write some text
  virtual void Write(const MString& S);
  //! Write some text
  virtual void Write(const double d);
  //! Write some text
  virtual void Write(const char c);
  //! Flush all written text
  virtual void Flush();
 
  
  //! Get one character - returns false if before the read IsGood() would return false
  virtual bool Get(char& c);
  //! Get one float - returns false if before the read IsGood() would return false
  virtual bool Get(float& f);

  //! Read one line the MEGAlib way - returns false if before the read IsGood() would return false
  virtual bool ReadLine(MString& String);
  //! Read one line the C way - returns false if before the read IsGood() would return false
  virtual bool ReadLine(char* String, streamsize Size, char Delimeter);

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

  //! The file type string
  MString m_FileType;
  //! The version of the file
  int m_Version;

  //! True if the file is open
  bool m_IsOpen;      

  //! The Mode: read or write
  unsigned int m_Way;

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
  //! The basic file stream for normal C++ 
  fstream m_File;

  //! The basic file stream for zlib 
  gzFile m_ZipFile;

  //! The known file length on disk -- if it has not yet been determined m_HasFileLength is false
  streampos m_FileLength;
  //! True if the file length has already been determined
  bool m_HasFileLength;

  //! The known uncompressed file length -- if it has not yet been determined m_HasFileLength is false
  streampos m_UncompressedFileLength;
  //! True if the file length has already been determined
  bool m_HasUncompressedFileLength;
  
  //! The maximum allowed file length
  streamsize m_MaxFileLength;
  
  

#ifdef ___CINT___
 public:
  ClassDef(MFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
