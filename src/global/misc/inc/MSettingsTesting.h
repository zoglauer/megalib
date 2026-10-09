/*
 * MSettings.h
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


#ifndef __MSettingsTesting__
#define __MSettingsTesting__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MString.h"
#include "MSettings.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! Class representing the settings of the unit & and end-to-end tests and their test driver
class MSettingsTesting : public MSettings
{
  // public interface:
 public:
  //! Default constructor with the default settings
  MSettingsTesting();
  //! Default destructor
  virtual ~MSettingsTesting();

  //! Set the log directory: the working files of failed tests and the logs of the test driver go here
  void SetLogDirectory(const MString& LogDirectory) { m_LogDirectory = LogDirectory; }
  //! Get the log directory (default: /tmp/$USER/megalib_testing_logs)
  MString GetLogDirectory() const { return m_LogDirectory; }

  //! Set the time out for the tests in seconds
  //! The driver kills a test after it, the end-to-end tests derive their time limits from it 
  //! 0 means no time out
  void SetTimeout(double Timeout) { m_Timeout = Timeout; }
  //! Get the time out of one test in seconds (default: 120)
  double GetTimeout() const { return m_Timeout; }

  //! Set the identifier of the machine on which the time out was calibrated, empty if there was no calibration
  void SetMachineId(const MString& MachineId) { m_MachineId = MachineId; }
  //! Get the identifier of the machine on which the time out was calibrated, empty if there was no calibration
  MString GetMachineId() const { return m_MachineId; }

  //! Set how much slower the calibrated machine is than the reference machine (below 1: faster)
  void SetMachineSlowdown(double MachineSlowdown) { m_MachineSlowdown = MachineSlowdown; }
  //! Get how much slower the calibrated machine is than the reference machine (default: 1)
  double GetMachineSlowdown() const { return m_MachineSlowdown; }

  // protected methods:
 protected:
  //! Read all data from an XML tree
  virtual bool ReadXml(MXmlNode* Node);
  //! Write all data to an XML tree
  virtual bool WriteXml(MXmlNode* Node);

  // private methods:
 private:
  //! Return the default log directory /tmp/$USER/megalib_testing_logs
  MString GetDefaultLogDirectory() const;

  // protected members:
 protected:

  // private members:
 private:
  //! The log directory
  MString m_LogDirectory;
  //! The time out of one test in seconds
  double m_Timeout;
  //! The identifier of the machine on which the calibration was made
  MString m_MachineId;
  //! How much slower the calibrated machine is than the reference machine
  double m_MachineSlowdown;


#ifdef ___CLING___
 public:
  ClassDef(MSettingsTesting, 0) // The settings of the tests
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
