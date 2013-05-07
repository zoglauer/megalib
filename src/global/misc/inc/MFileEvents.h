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
  virtual bool Open(MString FileName, unsigned int Way = MFile::c_Read);

  //! Rewind to the beginning of the file
  virtual bool Rewind();

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
  //! Attention: This information is only complete after all files have been scanned,
  //! i.e. after GetEvents has been called for all events!
  //! In addition, this is the observation time for the total file, NOT up to the just read events!!
  double GetObservationTime() const { return m_ObservationTime; }
  
  //! Set the observation time 
  void SetObservationTime(double ObservationTime) { m_ObservationTime = ObservationTime; }

  // protected methods:
 protected:
  //! Open a file given by the "NF" keyword
  virtual bool OpenNextFile(const MString& Line);
  //! Create a file indicated by a "NF" keyword
  virtual bool CreateNextFile();

  //! Open a file given by the "IN" keyword
  virtual bool OpenIncludeFile(const MString& Line);
  //! Create a file indicated by a "IN" keyword
  virtual bool CreateIncludeFile();
  //! Create a file name indicated suiting for the "IN" keyword
  MString CreateIncludeFileName(const MString& FileName);

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
 
  //! The observation time
  double m_ObservationTime;
  
  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MFileEvents, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
