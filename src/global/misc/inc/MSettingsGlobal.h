/*
 * MSettingsGlobal.h
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


#ifndef __MSettingsGlobal__
#define __MSettingsGlobal__


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


class MSettingsGlobal
{
  // public Session:
 public:
  //! The default constructor
  MSettingsGlobal();
  //! The default destructor
  virtual ~MSettingsGlobal();

  //! Read all data from the default/stored file
  virtual bool Read();
  //! Write all data to the default/stored file
  virtual bool Write();

  //! Set the changelog hash
  void SetChangeLogHash(long ChangeLogHash) { m_ChangeLogHash = ChangeLogHash; }
  //! Get the changelog hash
  long GetChangeLogHash() const { return m_ChangeLogHash; }

  //! Set the font scaler
  void SetFontScaler(MString FontScaler) { m_FontScaler = FontScaler; }
  //! Get the font scaler
  MString GetFontScaler() const { return m_FontScaler; }



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

  //! The hash of the change-log file
  long m_ChangeLogHash;
  //! The font scaling type: one of normal, large, huge, gigantic
  MString m_FontScaler;



#ifdef ___CLING___
 public:
  ClassDef(MSettingsGlobal, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
