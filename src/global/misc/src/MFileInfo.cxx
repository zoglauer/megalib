/*
 * MFileInfo.cxx                                   v0.1  01/01/2001
 *
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 *
 * This code implementation is the intellectual property of
 * Andreas Zoglauer.
 *
 * By copying, distributing or modifying the Program (or any work
 * based on the Program) you indicate your acceptance of this statement,
 * and all its terms.
 *
 */


////////////////////////////////////////////////////////////////////////////////
//
// MFileInfo
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileInfo.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MFileInfo)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileInfo::MFileInfo()
{
  // Construct an instance of MFileInfo

  m_FileName = "/tmp/noname";
  m_Creator = "Rainer Zufall";
  m_Isotopes = "266-Mt";
  m_Position = MVector(0.0, 0.0, 0.0);
  m_Comment = "No comment!";
}


////////////////////////////////////////////////////////////////////////////////


MFileInfo::~MFileInfo()
{
  // Delete this instance of MFileInfo
}


////////////////////////////////////////////////////////////////////////////////


void MFileInfo::SetFileName(MString FileName)
{
  // Set the file name

  m_FileName = FileName;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileInfo::GetFileName()
{
  // Return the filename

  return m_FileName;
}


////////////////////////////////////////////////////////////////////////////////


void MFileInfo::SetCreator(MString Creator)
{
  // Set the cretor of the file

  m_Creator = Creator;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileInfo::GetCreator()
{
  // return the creator of the file

  return m_Creator;
}


////////////////////////////////////////////////////////////////////////////////


void MFileInfo::SetIsotopes(MString Isotopes)
{
  // Set the isotopes

  m_Isotopes = Isotopes;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileInfo::GetIsotopes()
{
  // Return the isotopes

  return m_Isotopes;
}


////////////////////////////////////////////////////////////////////////////////


void MFileInfo::SetComment(MString Comment)
{
  // Set the comment

  m_Comment = Comment;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileInfo::GetComment()
{
  // Return the comment

  return m_Comment;
}


////////////////////////////////////////////////////////////////////////////////


void MFileInfo::SetPosition(MVector Position)
{
  // Set the position of the source

  m_Position = Position;
}


////////////////////////////////////////////////////////////////////////////////


MVector MFileInfo::GetPosition()
{
  // Return the position of the source

  return m_Position;
}


////////////////////////////////////////////////////////////////////////////////


MString MFileInfo::ToString()
{
  // Display the content of this class

  ostringstream S;

  S<<"File name: "<<m_FileName<<endl;
  S<<"Creator:   "<<m_Creator<<endl;
  S<<"Isotopes:  "<<m_Isotopes<<endl;
  S<<"Position:  "<<m_Position<<endl;
  S<<"Comment:   "<<m_Comment<<endl;

  return S.str().c_str();
}


// MFileInfo.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
