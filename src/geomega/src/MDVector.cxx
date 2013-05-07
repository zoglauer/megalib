/*
 * MDVector.cxx
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
// MDVector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MDVector.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MDVector)
#endif


////////////////////////////////////////////////////////////////////////////////


MDVector::MDVector(const MString Name): m_Name(Name)
{
  // Construct an instance of MDVector
}


////////////////////////////////////////////////////////////////////////////////


MDVector::~MDVector()
{
  // Delete this instance of MDVector
}


////////////////////////////////////////////////////////////////////////////////


MVector MDVector::GetPosition(unsigned int i) 
{
  //! Return the position at pos i - g_VectorNotDefined if out of bounds

  if (i < m_Positions.size()) {
    return m_Positions[i];
  }

  return g_VectorNotDefined;
}


////////////////////////////////////////////////////////////////////////////////


double MDVector::GetValue(unsigned int i)
{
  //! Return the value at pos i - g_DoubleNotDefined id out of bounds

  if (i < m_Values.size()) {
    return m_Values[i];
  }
  
  return g_DoubleNotDefined;
}


// MDVector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
