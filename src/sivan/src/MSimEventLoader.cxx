/*
 * MSimEventLoader.cxx
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
// MSimEventLoader
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MSimEventLoader.h"

// Standard libs:
using namespace std;
#include <iostream>
#include <fstream>

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MSimEventLoader)
#endif


////////////////////////////////////////////////////////////////////////////////


MSimEventLoader::MSimEventLoader()
{
  // default constructor

  cout<<"Remark: This event loader is deprecated, since it is less capable than the new one!"<<endl;
  
  Init();
}


////////////////////////////////////////////////////////////////////////////////


MSimEventLoader::~MSimEventLoader()
{
  // default destructor

  Init();
}


////////////////////////////////////////////////////////////////////////////////


MSimEventLoader::MSimEventLoader(MString FileName)
{
  //

  SetFileName(FileName);
  
  Init();
}  


////////////////////////////////////////////////////////////////////////////////


void MSimEventLoader::Init()
{
  // Common initializations:

  m_LineLength = 250;
  m_LineBuffer = new char[m_LineLength];

  m_Event = new MSimEvent();
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


void MSimEventLoader::SetFileName(MString FileName)
{
  m_FileName = FileName;

  OpenFile();
}


////////////////////////////////////////////////////////////////////////////////


void MSimEventLoader::SetGeometry(MDGeometryQuest *Geo)
{
  m_Geometry = Geo;
  m_Event->SetGeometry(m_Geometry);
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEventLoader::OpenFile()
{
  m_FileStream = new fstream();
  // gcc 2.95.3: m_FileStream->open(m_FileName, ios::in, 0664);
  m_FileStream->open(m_FileName, ios_base::in);

  //cout<<"Try to open: "<<m_FileName<<endl;

  if (IsFileOpen() == false) {
    Error("bool MSimEventLoader::OpenFile()",
          "Can't open file %s!", m_FileName.Data());
    return false;
  }

  m_FileStream->seekg(0, ios::end);
  m_FileLength = m_FileStream->tellg();
  m_FileStream->seekg(0, ios::beg);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MSimEventLoader::IsFileOpen()
{
  // Has the file been opened successfully?

  return m_FileStream->is_open();
}


////////////////////////////////////////////////////////////////////////////////


MSimEvent* MSimEventLoader::GetNextEvent()
{
  // Returns the next simulated event or 0 if there are no more events

  while (m_FileStream->eof() == false) {
    if (m_FileStream->getline(m_LineBuffer, m_LineLength, '\n')) {
      if (m_Event->AddRawInput(m_LineBuffer) == true) {
        return m_Event;
      }
    }
  }

  return 0;
}


////////////////////////////////////////////////////////////////////////////////


double MSimEventLoader::GetProgress()
{
  // Return the read-process as fraction of read bytes:

  return (double) m_FileStream->tellg() / (double) m_FileLength;
}


////////////////////////////////////////////////////////////////////////////////






// MSimEventLoader.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
