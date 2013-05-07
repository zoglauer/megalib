/*
 * MSettingsBasicFiles.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSettingsBasicFiles__
#define __MSettingsBasicFiles__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettingsInterface.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsBasicFiles : public MSettingsInterface
{
  // public Session:
 public:
  MSettingsBasicFiles();
  virtual ~MSettingsBasicFiles();

  virtual MString GetCurrentFileName();
  virtual bool SetCurrentFileName(MString File);

  MString GetGeometryFileName();
  bool SetGeometryFileName(MString FileName);

  unsigned int GetNFileHistories();
  MString GetFileHistoryAt(unsigned int i);
  void AddFileHistory(MString FileName);

  unsigned int GetNGeometryHistories();
  MString GetGeometryHistoryAt(unsigned int i);
  void AddGeometryHistory(MString FileName);

  //! Maximum length of the file history
  static const unsigned int m_MaxHistory;

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

  //! Name of the default program file 
  MString m_CurrentFileName;
  //! File history as shown in the first menu
  vector<MString> m_FileHistory;

  //! Name of the geometry file
  MString m_GeometryFileName;
  //! Geomery history as shown in the first menu
  vector<MString> m_GeometryHistory;



#ifdef ___CINT___
 public:
  ClassDef(MSettingsBasicFiles, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
