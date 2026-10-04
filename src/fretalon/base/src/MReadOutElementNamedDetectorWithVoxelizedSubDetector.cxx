/*
 * MReadOutElementNamedDetectorWithVoxelizedSubDetector.cxx
 *
 * Copyright (C) by Andreas Zoglauer, Valentina Fioretti
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
// MReadOutElementNamedDetectorWithVoxelizedSubDetector
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MReadOutElementNamedDetectorWithVoxelizedSubDetector.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MReadOutElementNamedDetectorWithVoxelizedSubDetector)
#endif


////////////////////////////////////////////////////////////////////////////////


//! Default constructor
MReadOutElementNamedDetectorWithVoxelizedSubDetector::MReadOutElementNamedDetectorWithVoxelizedSubDetector()
    : MReadOutElement(),
      m_DetectorID(""),
      m_CrystalID(g_UnsignedIntNotDefined),
      m_VoxelXID(g_UnsignedIntNotDefined),
      m_VoxelYID(g_UnsignedIntNotDefined),
      m_VoxelZID(g_UnsignedIntNotDefined)
{
}

////////////////////////////////////////////////////////////////////////////////

//! Parameterized constructor
MReadOutElementNamedDetectorWithVoxelizedSubDetector::MReadOutElementNamedDetectorWithVoxelizedSubDetector(
  const MString& DetectorID,
  unsigned int CrystalID,
  unsigned int VoxelXID,
  unsigned int VoxelYID,
  unsigned int VoxelZID)
    : MReadOutElement(),
      m_DetectorID(DetectorID),
      m_CrystalID(CrystalID),
      m_VoxelXID(VoxelXID),
      m_VoxelYID(VoxelYID),
      m_VoxelZID(VoxelZID)
{
}

////////////////////////////////////////////////////////////////////////////////


//! Default destructor
MReadOutElementNamedDetectorWithVoxelizedSubDetector::~MReadOutElementNamedDetectorWithVoxelizedSubDetector()
{
}


////////////////////////////////////////////////////////////////////////////////


//! Clone this read-out element - the returned element must be deleted!
MReadOutElementNamedDetectorWithVoxelizedSubDetector* MReadOutElementNamedDetectorWithVoxelizedSubDetector::Clone() const
{
  MReadOutElementNamedDetectorWithVoxelizedSubDetector* R = new MReadOutElementNamedDetectorWithVoxelizedSubDetector(
    m_DetectorID,
    m_CrystalID,
    m_VoxelXID,
    m_VoxelYID,
    m_VoxelZID);
  return R;
}
////////////////////////////////////////////////////////////////////////////////


//! Clear the content of this read-out element
void MReadOutElementNamedDetectorWithVoxelizedSubDetector::Clear()
{
  MReadOutElement::Clear();
  m_DetectorID = "";
  m_CrystalID = g_UnsignedIntNotDefined;
  m_VoxelXID = g_UnsignedIntNotDefined;
  m_VoxelYID = g_UnsignedIntNotDefined;
  m_VoxelZID = g_UnsignedIntNotDefined;
}

////////////////////////////////////////////////////////////////////////////////


//! Return true if this read-out element is of the given type
bool MReadOutElementNamedDetectorWithVoxelizedSubDetector::IsOfType(const MString& String) const
{
  if (String == "voxel3d")
    return true;

  return false;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the type of this read-out element
MString MReadOutElementNamedDetectorWithVoxelizedSubDetector::GetType() const
{
  return "voxel3d";
}


//! Test for equality
bool MReadOutElementNamedDetectorWithVoxelizedSubDetector::operator==(const MReadOutElement& R) const
{
  const MReadOutElementNamedDetectorWithVoxelizedSubDetector* Other = dynamic_cast<const MReadOutElementNamedDetectorWithVoxelizedSubDetector*>(&R);
  if (Other == nullptr) {
    if (g_Verbosity >= c_Error) {
      cout<<"ERROR: Comparison with different read-out element type"<<endl;
    }
    return false;
  }

  if (m_DetectorID != Other->m_DetectorID)
    return false;
  if (m_CrystalID != Other->m_CrystalID)
    return false;

  if (m_VoxelXID != Other->m_VoxelXID)
    return false;
  if (m_VoxelYID != Other->m_VoxelYID)
    return false;
  if (m_VoxelZID != Other->m_VoxelZID)
    return false;

  return true;
}


////////////////////////////////////////////////////////////////////////////////


//! Return the number of parsable elements
unsigned int MReadOutElementNamedDetectorWithVoxelizedSubDetector::GetNumberOfParsableElements() const
{
  return 5;
}


////////////////////////////////////////////////////////////////////////////////


//! Parse the data from the tokenizer
bool MReadOutElementNamedDetectorWithVoxelizedSubDetector::Parse(const MTokenizer& T, unsigned int StartElement)
{
  if (T.GetNTokens() < StartElement + GetNumberOfParsableElements()) {
    if (g_Verbosity >= c_Error) {
      cout<<"ERROR: Not enough elements to parse. Number of tokens is "<<T.GetNTokens()<<" and less than 5"<<endl;
    }
    return false;
  }

  m_DetectorID = T.GetTokenAtAsString(StartElement);
  m_CrystalID = T.GetTokenAtAsUnsignedIntFast(StartElement + 1);
  m_VoxelXID = T.GetTokenAtAsUnsignedIntFast(StartElement + 2);
  m_VoxelYID = T.GetTokenAtAsUnsignedIntFast(StartElement + 3);
  m_VoxelZID = T.GetTokenAtAsUnsignedIntFast(StartElement + 4);

  if (m_DetectorID == "") {
    if (g_Verbosity >= c_Warning) {
      cout<<"WARNING: Parsed empty DetectorID (token index "<< StartElement<<")."<<endl;
    }
  }

  if (m_CrystalID == g_UnsignedIntNotDefined ||
      m_VoxelXID == g_UnsignedIntNotDefined ||
      m_VoxelYID == g_UnsignedIntNotDefined ||
      m_VoxelZID == g_UnsignedIntNotDefined) {

    if (g_Verbosity >= c_Warning) {
      cout<<"WARNING: Parsed undefined ID(s): "
          <<"Crystal = "<<m_CrystalID
          <<" Vx = "<<m_VoxelXID
          <<" Vy = "<<m_VoxelYID
          <<" Vz = "<<m_VoxelZID<<endl;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


//! Return the data as parsable string
MString MReadOutElementNamedDetectorWithVoxelizedSubDetector::ToParsableString(bool WithDescriptor) const
{
  MString Return;
  if (WithDescriptor == true) {
    Return += "voxel3d ";
  }
  Return += m_DetectorID;
  Return += " ";
  Return += m_CrystalID;
  Return += " ";
  Return += m_VoxelXID;
  Return += " ";
  Return += m_VoxelYID;
  Return += " ";
  Return += m_VoxelZID;

  return Return;
}

////////////////////////////////////////////////////////////////////////////////


//! Convert content to a string
MString MReadOutElementNamedDetectorWithVoxelizedSubDetector::ToString() const
{

  if (m_DetectorID == "") {
    if (g_Verbosity >= c_Warning) {
      cout<<"WARNING: called an element with empty DetectorID"<<endl;
    }
  }

  if (m_CrystalID == g_UnsignedIntNotDefined ||
      m_VoxelXID == g_UnsignedIntNotDefined ||
      m_VoxelYID == g_UnsignedIntNotDefined ||
      m_VoxelZID == g_UnsignedIntNotDefined) {

      if (g_Verbosity >= c_Warning) {
          cout<<"WARNING: called undefined ID(s): "
         <<"Crystal = "<<m_CrystalID
         <<" Vx = "<<m_VoxelXID
         <<" Vy = "<<m_VoxelYID
         <<" Vz = "<<m_VoxelZID<<endl;
      }
  }

  ostringstream OS;
  OS<<"DetectorID: "<<m_DetectorID<<", CrystalID: "<<m_CrystalID<<", VoxelID: ("<<m_VoxelXID<<", "<<m_VoxelYID<<", "<<m_VoxelZID<<")";

  return OS.str();
}

////////////////////////////////////////////////////////////////////////////////


//! Stream the data
ostream& operator<<(ostream& os, const MReadOutElementNamedDetectorWithVoxelizedSubDetector& R)
{
  os<<R.ToString();
  return os;
}


// MReadOutElementNamedDetectorWithVoxelizedSubDetector.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
