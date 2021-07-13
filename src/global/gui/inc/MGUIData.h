/*
 * MGUIData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MGUIData__
#define __MGUIData__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MXmlDocument.h"
#include "MXmlNode.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MGUIData
{
  // public Session:
 public:
  MGUIData();
  virtual ~MGUIData();

  bool ReadData(MString FileName = g_StringNotDefined);
  bool ReadDataOld(MString FileName = g_StringNotDefined);
  bool SaveData(MString FileName = g_StringNotDefined);

  int GetModificationLevel(bool Reset = true);

  virtual MString GetCurrentFileName();
  virtual bool SetCurrentFileName(MString File);

  MString GetDataFileName();
  bool SetDataFileName(MString DataFileName);

  MString GetGeometryFileName();
  bool SetGeometryFileName(MString FileName);

  void SetMEGAlibVersion(unsigned int MEGAlibVersion) { m_MEGAlibVersion = MEGAlibVersion; }
  unsigned int GetMEGAlibVersion() const { return m_MEGAlibVersion; }

  unsigned int GetNFileHistories();
  MString GetFileHistoryAt(unsigned int i);
  void AddFileHistory(MString FileName);

  unsigned int GetNGeometryHistories();
  MString GetGeometryHistoryAt(unsigned int i);
  void AddGeometryHistory(MString FileName);

  void SetSpecialMode(const bool SpecialMode) { m_SpecialMode = SpecialMode; }
  bool GetSpecialMode() const { return m_SpecialMode; }

  //! Maximum length of the file history
  static const unsigned int m_MaxHistory;

  // protected methods:
 protected:
  void Modify(int m);

  virtual void ReadDataLocal(FILE* File) {};
  virtual bool ReadDataLocal(MXmlDocument* Node) { return false; }
  virtual bool SaveDataLocal(MXmlDocument* Node) { return false; }


  // private methods:
 private:



  // protected members:
 protected:
  /// Name of the master node of the document
  MString m_MasterNodeName;

  /// Default name of this file, e.g ".mimrec.cfg"
  MString m_DefaultFileName;
  /// Name of this file
  MString m_DataFileName;
  /// Name of the default program file 
  MString m_CurrentFile;
  /// Name of the geometry file
  MString m_GeometryFileName;
  /// MEGAlib version used to create this file
  unsigned int m_MEGAlibVersion;
  /// File history as shown in the first menu
  vector<MString> m_FileHistory;
  /// Geomery history as shown in the first menu
  vector<MString> m_GeometryHistory;
  /// Special mode with more possibilities
  bool m_SpecialMode;

  /// Do not know for what this is good HERE
  int m_ModificationLevel;


#ifdef ___CLING___
 public:
  ClassDef(MGUIData, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
