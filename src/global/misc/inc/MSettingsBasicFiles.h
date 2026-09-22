/*
 * MSettingsBasicFiles.h
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
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



#ifdef ___CLING___
 public:
  ClassDef(MSettingsBasicFiles, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
