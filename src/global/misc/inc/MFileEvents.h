/*
 * MFileEvents.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileEvents__
#define __MFileEvents__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MFile.h"
#include "MTime.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileEvents : public MFile
{
  // public interface:
 public:
  //! Default constructor - the read mode as well as file name are passed on via Open!
  MFileEvents();
  //! Default destructor
  virtual ~MFileEvents();

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way, bool IsBinary);

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName, unsigned int Way) { return Open(FileName, Way, false); }

  //! The Open method has to be derived to initialize the include file:
  virtual bool Open(MString FileName) { return Open(FileName, MFile::c_Read, false); }

  //! Update the progress dialog GUI -- allow to skip a certain amount of updates
  virtual bool UpdateProgress(unsigned int UpdatesToSkip = 0);

  //! Rewind to the beginning of the file
  virtual bool Rewind(bool NotUsed = true);

  //! Set the geometry name
  void SetGeometryFileName(const MString Geometry);
  //! Retrieve the geometry name
  MString GetGeometryFileName() const;

  //! Retrieve MEGAlib version ID with which this file has be safed
  MString GetMEGAlibVersion() const { return m_MEGAlibVersion; }

  //! Write a header to file
  virtual bool WriteHeader();

  //! Add text to the file as footer - it always goes into the master file!
  //! You should NOT add events afterwards!
  virtual bool AddFooter(const MString& Text);

  //! Close the event list after last event (per default add "END")
  virtual bool CloseEventList();

  //! Set if this is an include file
  void SetIsIncludeFile(bool IsIncludeFile) { m_IsIncludeFile = IsIncludeFile; }

  //! Return the number of events in this file (it counts the "SE")
  int GetNEvents(bool Count);

  //! Return the observation time in this event
  //! If it has not yet been read, read it (can be very slow if the file is gzip'ed)
  MTime GetObservationTime();
  
  //! Set the observation time 
  void SetObservationTime(MTime ObservationTime) { m_ObservationTime = ObservationTime; m_HasObservationTime = true; }
  //! Return true if we have a start observation time
  bool HasStartObservationTime() const { return m_HasStartObservationTime; }
  //! Return the start observation time
  MTime GetStartObservationTime() const { return m_StartObservationTime; }
  //! Set the start observation time
  void SetStartObservationTime(MTime ObservationTime) { m_StartObservationTime = ObservationTime; m_HasStartObservationTime = true; }
  //! Return true if we have an end observation time
  bool HasEndObservationTime() const { return m_HasEndObservationTime; }
  //! Return the end observation time
  MTime GetEndObservationTime() const { return m_EndObservationTime; }
  //! Set the end observation time
  void SetEndObservationTime(MTime ObservationTime) { m_EndObservationTime = ObservationTime; m_HasEndObservationTime = true; }

  // protected methods:
 protected:
  //! Open a file given by the "NF" keyword
  virtual bool OpenNextFile(const MString& Line);
  //! Create a file indicated by a "NF" keyword
  virtual bool CreateNextFile();

  //! Open a file given by the "IN" keyword
  virtual bool OpenIncludeFile(const MString& Line);
  //! Close the current include file
  virtual bool CloseIncludeFile();
  
  //! Create a file indicated by a "IN" keyword
  virtual bool CreateIncludeFile();
  //! Create a file name indicated suiting for the "IN" keyword
  MString CreateIncludeFileName(const MString& FileName);

  //! Read the special information at the end of file
  //! If Continue is true, we do not seek to the end of the file, but just read the current file to the end
  bool ReadFooter(bool Continue = false);
  //! Parse the special information at the end of file -- add your special parsing in there
  virtual bool ParseFooter(const MString& Line);
  
  //! ID indicating there is no ID
  static const int c_NoId;

  // private methods:
 private:



  // protected members:
 protected:
  //! The underlying geometry file name
  MString m_GeometryFileName;
  //! The MEGAlib version string used to create the file
  MString m_MEGAlibVersion; 

  //! The number of events in this file
  int m_NEvents;

  //! The number of include files in this file
  int m_NIncludeFiles;
  //! The number of opened include files 
  int m_NOpenedIncludeFiles;
  
  //! The used extension of the file when creating new ones via the NF extension
  int m_ExtensionNumber;

  //! Sim and tra files can have included files:
  MFileEvents* m_IncludeFile;
  //! Is the included file in use
  bool m_IncludeFileUsed;
  //! True is this is an included file:
  bool m_IsIncludeFile;

  //! The original file name (different from m_FileName in case ofjumping from file to file via NF keyword)
  MString m_OriginalFileName;

  //! True if a TE keyword should only be interpreted as footer TE after an EN line
  bool m_FooterTENeedsEN;
  //! True if we have already seen the EN line of the footer
  bool m_FooterENSeen;
 
  //! Has a start observation time
  bool m_HasStartObservationTime;
  //! The start of the observation time
  MTime m_StartObservationTime;
  
  //! Has end observation time
  bool m_HasEndObservationTime;
  //! The end of the observation time
  MTime m_EndObservationTime;
  
  //! True if we have an observation time
  bool m_HasObservationTime;
  //! The total observation time
  MTime m_ObservationTime;
  
  // private members:
 private:


#ifdef ___CLING___
 public:
  ClassDef(MFileEvents, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
