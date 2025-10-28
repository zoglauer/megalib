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
#include "TFile.h"
#include "TMutex.h"
#include "zlib.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MBinaryStore.h"

// Standard libs:
#include <fstream>
#include <sstream>
#include <streambuf>
using namespace std;

// Forward declarations:
class MGUIProgressBar;


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

  //! Open the file for reading or writing in binary or ASCII mode
  virtual bool Open(MString FileName, unsigned int Way, bool IsBinary);
  //! Open the file for reading or writing - this assumes we are always in ASCII mode
  virtual bool Open(MString FileName, unsigned int Way) { return Open(FileName, Way, false); }
  //! Open the file for reading - this assumes we are always read in ASCII mode
  virtual bool Open(MString FileName) { return Open(FileName, c_Read, false); }
  //! Close the file
  virtual bool Close();
  //! Rewind the file
  virtual bool Rewind(bool ResetProgressStatistics = true);

  //! Return true if the file is open
  virtual bool IsOpen();
  //! Return true is the file is good
  virtual bool IsGood();
  //! Clear all flags
  virtual void Clear();

  //! Return true if the file is binary
  virtual bool IsBinary() { return m_IsBinary; }
  
  //! Set the compression level
  virtual void SetCompressionLevel(unsigned int CompressionLevel = 6);
  
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

  //! Write a new line
  virtual void WriteLine();
  //! Write some text and clear the stream
  virtual void Write(const ostringstream& S);
  //! Write some text, a new line, and clear the stream
  virtual void WriteLine(const ostringstream& S);
  //! Write some text
  virtual void Write(const MString& S);
  //! Write some text + new line
  virtual void WriteLine(const MString& S);
  //! Write some text
  virtual void Write(const double d);
  //! Write some text
  virtual void Write(const char c);
  //! Write binary
  virtual void Write(MBinaryStore& Store);
  
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

  //! Read CharactersToRead (or until end of file)  - returns false if before the read IsGood() would return false
  virtual bool Read(MBinaryStore& Store, unsigned int CharactersToRead);
  
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
  //! Return true if the cancel button has been pressed
  bool IsCanceled() const { return m_Canceled; }

  //! Return true if the file extensions are correct
  virtual bool CheckFileExtension(MString Extension);

  //! Return true if the file exists and if it is readable
  static bool Exists(MString FileName);
  //! Return true if the file exists
  static bool FileExists(const char* FileName); // deprecated
  //! Return true if the file was removed successfully
  static bool Remove(MString FileName);
  //! Check if a program exists
  static bool ProgramExists(MString Program);
  //! Sets the path of "Path" as new path to "FileName" IF FileName has a relative path
  static bool ApplyPath(MString& FileName, const MString& Path);
  //! Expand a file name e.g. $MEGALIB/src to /home/andreas/Software/MEGAlib/src
  static bool ExpandFileName(MString& FileName, const MString& WorkingDir = "");
  //! Make RelFileName relative to AbsFileName
  //! Relative: /home/andreas/Test/MyFile.tra
  //! Absolute: /home/andreas/MasterTest.tra
  //! Return:   ./Test/MyFile.tra
  static MString RelativeFileName(MString RelFileName, MString AbsFileName);
  //! Return the directory name (replacement for inconsistent gSystem->DirName)
  static MString GetDirectoryName(const MString& Name);
  //! Return the base file name
  static MString GetBaseName(const MString& Name);
  //! Return the current working directory
  static MString GetWorkingDirectory();
  

  //! The file modes: Write to a new file
  static unsigned int c_Write;
  //! The file modes: Write to a new file
  static unsigned int c_Create;
  //! The file modes: Read from an existing file
  static unsigned int c_Read;

  // protected methods:
 protected:
  //! The show progress functions without mutex locking
  void ShowProgressNoLock(bool Show = true);
  //! Return true is the file is good without mutex locking
  virtual bool IsGoodNoLock();


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

  //! Is this a binary file
  bool m_IsBinary;
  
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
  //! The mutex guarding multithreaded access to the progress bar
  TMutex m_ProgressMutex;

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

  //! The file mutex
  TMutex m_FileMutex;

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

  //! A frequently used buffer
  char* m_ReadLineBuffer;
  //! The length of the frquently used read-line buffer
  unsigned long m_ReadLineBufferLength;

  //! Compression level (gzip: 1..9)
  unsigned int m_CompressionLevel;
  
  
#ifdef ___CLING___
 public:
  ClassDef(MFile, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
