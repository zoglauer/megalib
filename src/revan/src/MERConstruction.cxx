/*
 * MERConstruction.cxx
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
// MERConstruction
//
// Base class for event reconstruction tasks, e.g. find clusters, tracks,
// Compton sequence
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MERConstruction.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MERConstruction)
#endif


////////////////////////////////////////////////////////////////////////////////


MERConstruction::MERConstruction()
{
  // Construct an instance of MERConstruction

  m_List = 0;
  m_Geometry = 0;
}


////////////////////////////////////////////////////////////////////////////////


MERConstruction::~MERConstruction()
{
  // Delete this instance of MERConstruction

  m_List = 0; // do not delete
  m_Geometry = 0; // do not delete
}


////////////////////////////////////////////////////////////////////////////////


bool MERConstruction::Analyze(MRawEventIncarnations* List)
{
  // Analyze the raw event...
  m_List = List;

  ModifyEventList();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MERConstruction::ModifyEventList()
{
  // Modification routine at the beginning of the analysis to add/modify 
  // the events in the list before the analysis

  return;
}


// MERConstruction.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
