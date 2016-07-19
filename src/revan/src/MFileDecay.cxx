/*
 * MFileDecay.cxx
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
// MFileDecay
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileDecay.h"

// Standard libs:
#include <iomanip>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"



////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileDecay)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileDecay::MFileDecay() : MParser(' ', false)
{
  // Construct an instance of MFileDecay
}


////////////////////////////////////////////////////////////////////////////////


MFileDecay::~MFileDecay()
{
  // Delete this instance of MFileDecay
}


////////////////////////////////////////////////////////////////////////////////


bool MFileDecay::Open(MString FileName, unsigned int Way)
{
  // Open the file and do the parsing

  if (MParser::Open(FileName, Way) == false) {
    mlog<<"MFileDecay::Open: Unable to open file "<<FileName<<"."<<endl;
    return false;
  }

  int Version = 1;
  MString Type = "Decay"; 

  MTokenizer* T = 0;

  // Check the first 100 lines for Version and type: 
  bool VersionFound = false;
  bool TypeFound = false;
  for (unsigned int i = 0; i < GetNLines(); ++i) {
    T = GetTokenizerAt(i);
    if (T->GetNTokens() == 0) continue;
    if (T->GetTokenAt(0) == "Version") {
      Version = T->GetTokenAtAsInt(1);
      VersionFound = true;
    } else if (T->GetTokenAt(0) == "Type") {
      Type = T->GetTokenAtAsString(1);
      TypeFound = true;
    }
    if (VersionFound == true && TypeFound == true) break;
  }
  
  if (Type != "Decay") {
    mout<<"A file of type \""<<Type<<"\" is not decay file!"<<endl;
    return false;
  }

  if (Version < 1 || Version > 1) {
    mout<<"Decay file version "<<Version<<" not handled!"<<endl;
    return false;
  }

  m_Energies.clear();
  m_EnergyErrors.clear();

  for (unsigned int i = 0; i < GetNLines(); ++i) {
    T = GetTokenizerAt(i);
    if (T->GetNTokens() == 0) continue;
    if (T->GetTokenAt(0) == "DE" && T->GetNTokens() == 3) {
      m_Energies.push_back(T->GetTokenAtAsDouble(1));
      m_EnergyErrors.push_back(T->GetTokenAtAsDouble(2));
    }
  }

  if (m_Energies.size() == 0) {
    mgui<<"File \""<<FileName<<"\" did not contain any decays!"<<endl;
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////






// MFileDecay.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

