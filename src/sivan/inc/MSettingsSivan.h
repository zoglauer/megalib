/*
 * MSettingsSivan.h
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


#ifndef __MSettingsSivan__
#define __MSettingsSivan__


////////////////////////////////////////////////////////////////////////////////


// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MSettings.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MSettingsSivan : public MSettings
{
  // public Session:
 public:
  MSettingsSivan(bool AutoLoad = true);
  ~MSettingsSivan();

  void SetRealism(const int Realism);
  int GetRealism() const;

  void SetNInitializationEvents(const int NEvents) { m_NInitializationEvents = NEvents; }
  unsigned int GetNInitializationEvents() const{ return m_NInitializationEvents; }


  //! Set the special GUI mode (this is not saved to file!)
  void SetSpecialMode(const bool SpecialMode) { m_SpecialMode = SpecialMode; }
  //! Get the special GUI mode (this is not saved to file!)
  bool GetSpecialMode() const { return m_SpecialMode; }

  // protected members:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);


  // private members:
 private:
  //! Set the realism, i.e. is noising on or off
  int m_Realism;
  //! Number of events which are are used to initialize the initial size of dimensional hitsograms
  int m_NInitializationEvents;

  //! The special GUI mode flag
  bool m_SpecialMode;


#ifdef ___CLING___
 public:
  ClassDef(MSettingsSivan, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
