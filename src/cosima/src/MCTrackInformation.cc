/*
 * MCTrackInformation.cxx
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


// Cosima:
#include "MCTrackInformation.hh"


/******************************************************************************
 * Default constructor
 */
MCTrackInformation::MCTrackInformation() 
  : m_IsDigitized(false), m_Id(-1), m_OriginId(-1) 
{
  // Intentionally left blank
}


/******************************************************************************
 * Extended constructor
 */
MCTrackInformation::MCTrackInformation(int Id, int OriginId) 
  : m_IsDigitized(false), m_Id(Id), m_OriginId(OriginId) 
{
  // Intentionally left blank
}


/******************************************************************************
 * Default destructor
 */
MCTrackInformation::~MCTrackInformation()
{
  // Intentionally left blank
}


/******************************************************************************
 * Set, if the digitization of the last hit was successful
 */
void MCTrackInformation::SetDigitized(const bool IsDigitized)
{
  m_IsDigitized = IsDigitized;
}


/******************************************************************************
 * Return true, if the digitization of the last hit was successful
 */
bool MCTrackInformation::IsDigitized() const
{
  return m_IsDigitized;
}


/******************************************************************************
 * Set the id if this track
 */
void MCTrackInformation::SetId(const int Id)
{
  m_Id = Id;
}


/******************************************************************************
 * Return the id of this track
 */
int MCTrackInformation::GetId() const
{
  return m_Id;
}



/******************************************************************************
 * Set the originating track Id 
 */
void MCTrackInformation::SetOriginId(const int OriginId)
{
  m_OriginId = OriginId;
}


/******************************************************************************
 * Return the originating track Id 
 */
int MCTrackInformation::GetOriginId() const
{
  return m_OriginId;
}


/*
 * MCTrackInformation.cc: the end...
 ******************************************************************************/
