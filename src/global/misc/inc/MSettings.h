/*
 * MSettings.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettings__
#define __MSettings__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"
#include "MSettingsInterface.h"
#include "MSettingsBasicFiles.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettings : public MSettingsBasicFiles
{
  // public Session:
 public:
  //! The default constructor
  MSettings(MString NameMasterNode);
  //! The default destructor
  virtual ~MSettings();

  //! Read all data from the default/stored file
  virtual bool Read();
  //! Read all data from a file
  virtual bool Read(MString FileName);
  //! Write all data to the default/stored file
  virtual bool Write();
  //! Write all data to a file
  virtual bool Write(MString FileName);

  //! Set the settings file name
  void SetSettingsFileName(MString SettingsFileName) { m_SettingsFileName = SettingsFileName; }
  //! Return the settings file name
  MString GetSettingsFileName() { return m_SettingsFileName; }

  //! Set the file version as unsigned int
  void SetVersion(unsigned int Version) { m_Version = Version; }
  //! Retrieve the file version as unsigned int  
  unsigned int GetVersion() const { return m_Version; }

  //! Set the MEGAlib version as unsigned int
  void SetMEGAlibVersion(unsigned int MEGAlibVersion) { m_MEGAlibVersion = MEGAlibVersion; }
  //! Retrieve the MEGAlib version as unsigned int  
  unsigned int GetMEGAlibVersion() const { return m_MEGAlibVersion; }

  //! Change one settings field
  //! Syntax: TestPositions.CoordinateSystemSpherical.Theta=100
  //! The names correspond to the XML nodes <TestPositions> <CoordinateSystemSpherical> <Theta>
  //! The value after the = is the new entry
  //! Return true on success
  bool Change(MString NewField);

  // protected methods:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Writes all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private methods:
 private:



  // protected members:
 protected:
  //! Name of the master node of the document
  MString m_NameMasterNode;

  //! The settings file name
  MString m_SettingsFileName;
  
  //! The default settings file name, e.g ".mimrec.cfg"
  MString m_DefaultSettingsFileName;

  //! Version of this file
  unsigned int m_Version;

  //! MEGAlib version used to create this file
  unsigned int m_MEGAlibVersion;


#ifdef ___CINT___
 public:
  ClassDef(MSettings, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
