/*
 * MGUIDataRealta.cxx
 *
 *
 * Copyright (C) 1998-2010 by Andreas Zoglauer.
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
// MGUIDataRealta.cxx
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MGUIDataRealta.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"

////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MGUIDataRealta)
#endif


////////////////////////////////////////////////////////////////////////////////


MGUIDataRealta::MGUIDataRealta() : MGUIData()
{
  // default constructor
  m_DefaultFileName = TString(gSystem->ConcatFileName(gSystem->HomeDirectory(),
                                                      ".realta.cfg"));
  m_DataFileName = m_DefaultFileName;

  m_MasterNodeName = "RealtaConfigurationFile";

  ReadData();
}


////////////////////////////////////////////////////////////////////////////////


MGUIDataRealta::~MGUIDataRealta()
{
  // default destructor

  SaveData();
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataRealta::SaveDataLocal(MXmlDocument* Document)
{
  // Step two: save the keywords and the belonging data:

  //new MXmlNode(Document, "Realism", m_Realism);
  //new MXmlNode(Document, "NInitializationEvents", m_NInitializationEvents);

  return true;
}


////////////////////////////////////////////////////////////////////////////////


bool MGUIDataRealta::ReadDataLocal(MXmlDocument* Document)
{  
  /*
  MXmlNode* aNode = 0;
  
  if ((aNode = Document->GetNode("Realism")) != 0) {
    m_Realism = aNode->GetValueAsInt();
  }
  if ((aNode = Document->GetNode("NInitializationEvents")) != 0) {
    m_NInitializationEvents = aNode->GetValueAsInt();
  }
  */
    
  return true;
}


// MGUIDataRealta.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

