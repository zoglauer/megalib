/*
 * MPrelude.cxx
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


////////////////////////////////////////////////////////////////////////////////
//
// MPrelude
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MPrelude.h"

// Standard libs:
#include <fstream>

// ROOT libs:

// MEGAlib libs:
#include "MFile.h"
#include "MGUIPrelude.h"
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MPrelude)
#endif


////////////////////////////////////////////////////////////////////////////////


MPrelude::MPrelude()
{
  // Construct an instance of MPrelude
  
}


////////////////////////////////////////////////////////////////////////////////


MPrelude::~MPrelude()
{
  // Delete this instance of MPrelude
}


////////////////////////////////////////////////////////////////////////////////


bool MPrelude::Play()
{
  // First load the .megalib.cfg file
  if (m_Settings.Read() == false) {
    return false;
  }

  // Show the change log once, and again whenever it changes
  MString ChangeLogFile = "$(MEGALIB)/doc/ChangeLog.txt";
  MFile::ExpandFileName(ChangeLogFile);
  if (MFile::Exists(ChangeLogFile) == true) {
    ifstream in;
    in.open(ChangeLogFile);
    if (in.is_open() == true) {
      MString ChangeLog;
      ChangeLog.Read(in);
      if (ChangeLog.GetHash() != m_Settings.GetChangeLogHash()) {
        MGUIPrelude* P = new MGUIPrelude("Change Log", 
                                         "Please read the following change log file very carefully\n"
                                         "It contains information about enhancements, bugs, or changes required to run MEGAlib.", 
                                         ChangeLog, "OK");
        P->Create();
        delete P;     

        m_Settings.SetChangeLogHash(ChangeLog.GetHash());
      }
    } else {
      merr<<"Unable to open change log file \""<<ChangeLogFile<<"\""<<endl;
    }
  }

  // Finally save the global configuration file again
  if (m_Settings.Write() == false) {
    return false;
  }
  
  return true;
}



// MPrelude.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
