/*
 * MFileResponse.cxx
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
// MFileResponse
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileResponse.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MResponseMatrix.h"
#include "MResponseMatrixO1.h"
#include "MResponseMatrixO2.h"
#include "MResponseMatrixO3.h"
#include "MResponseMatrixO4.h"
#include "MResponseMatrixO5.h"
#include "MResponseMatrixO6.h"
#include "MResponseMatrixO7.h"
#include "MResponseMatrixO8.h"
#include "MResponseMatrixO9.h"
#include "MResponseMatrixO10.h"
#include "MResponseMatrixO11.h"
#include "MResponseMatrixO12.h"
#include "MResponseMatrixO13.h"
#include "MResponseMatrixO14.h"
#include "MResponseMatrixO15.h"
#include "MResponseMatrixO16.h"
#include "MResponseMatrixO17.h"
#include "MResponseMatrixON.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileResponse)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileResponse::MFileResponse() : MParser(' ', false)
{
  // Construct an instance of MFileResponse

  m_ValuesCentered = true;
  m_Name = g_StringNotDefined;
  m_Hash = 0;
  m_NumberOfSimulatedEvents = 0;
  m_FarFieldStartArea = 0;
}


////////////////////////////////////////////////////////////////////////////////


MFileResponse::~MFileResponse()
{
  // Delete this instance of MFileResponse
}


////////////////////////////////////////////////////////////////////////////////


bool MFileResponse::Open(MString FileName, unsigned int Way)
{
  // Open the file and do the parsing


  if (MFile::Open(FileName, Way) == false) {
    mlog<<"MFileResponse::Open: Unable to open file "<<FileName<<"."<<endl;
    return false;
  }

  if (Way == c_Read) {
    // Read header information:
    MTokenizer T;
    bool ValuesCenteredFound = false;
    
    // Check the first 100 lines for Version and type: 
    int Lines = 100;
    while (TokenizeLine(T) == true) {
      Lines--;
      if (T.GetNTokens() < 2) continue;
      if (T.GetTokenAt(0) == "Version") {
        m_Version = T.GetTokenAtAsInt(1);
      } else if (T.GetTokenAt(0) == "Type") {
        m_FileType = T.GetTokenAtAsString(1);
      } else if (T.GetTokenAt(0) == "NM") {
        m_Name = T.GetTokenAfterAsString(1);
      } else if (T.GetTokenAt(0) == "TS") {
        m_NumberOfSimulatedEvents = T.GetTokenAtAsLong(1);
      } else if (T.GetTokenAt(0) == "SA") {
        m_FarFieldStartArea = T.GetTokenAtAsDouble(1);
      } else if (T.GetTokenAt(0) == "HA") {
        m_Hash = T.GetTokenAtAsUnsignedLong(1);
      } else if (T.GetTokenAt(0) == "CE") {
        m_ValuesCentered = T.GetTokenAtAsBoolean(1);
        ValuesCenteredFound = true;
      } else if (T.GetTokenAt(0) == "StartStream") {
        // Avoid parsing the stream...
        break;
      }
      if (Lines == 0) break;
      if (m_Version != c_VersionUnknown && 
          m_FileType != c_TypeUnknown &&
          m_Name != g_StringNotDefined &&
          m_Hash != 0 &&
          ValuesCenteredFound == true) break;
    }
    if (ValuesCenteredFound == false) {
      mimp<<"Error: No CE info found in response file: "<<FileName<<endl
          <<"       Thus I do not know if the values belong to the center of the bins or the edges!"<<endl
          <<"       If the file is an absorption file, definitely regenerate it (details see documentation)!"<<endl
          <<"       For now I assume the values are centered - but I might be wrong!"<<show;
      m_ValuesCentered = true;
    }
    if (m_Version == c_VersionUnknown) {
      mout<<"Warning: No version info found in the response file \""<<m_Name<<"\"!!"<<endl;
    }
    if (m_FileType == c_TypeUnknown) {
      mout<<"Error: No file type info found in the response file \""<<m_Name<<"\"!!"<<endl;
      Close();
      return false;
    }
    
    Rewind();
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////


MResponseMatrix* MFileResponse::Read(MString FileName)
{
  // Open the file and read it;
  
  if (Open(FileName) == false) return nullptr;

  // Open automatically reads the header information!
  // We only need the file type info here!

  Close();

  if (m_FileType == c_TypeUnknown) {
    mout<<"MFileResponse::Read: Unable to find type of file "<<FileName<<"."<<endl;
    return 0;
  }

  MResponseMatrix* R = nullptr;

  if (m_FileType == "ResponseMatrixON" || m_FileType == "ResponseMatrixONStream") {
    MResponseMatrixON* RON = new MResponseMatrixON();
    if (RON->Read(FileName) == true) {
      R = RON;
    }
  } else if (m_FileType == "ResponseMatrixO1" || m_FileType == "ResponseMatrixO1Stream") {
    MResponseMatrixO1* RO1 = new MResponseMatrixO1();
    if (RO1->Read(FileName) == true) {
      R = RO1;
    }
  } else if (m_FileType == "ResponseMatrixO2" || m_FileType == "ResponseMatrixO2Stream") {
    MResponseMatrixO2* RO2 = new MResponseMatrixO2();
    if (RO2->Read(FileName) == true) {
      R = RO2;
    }
  } else if (m_FileType == "ResponseMatrixO3" || m_FileType == "ResponseMatrixO3Stream") {
    MResponseMatrixO3* RO3 = new MResponseMatrixO3();
    if (RO3->Read(FileName) == true) {
      R = RO3;
    }
  } else if (m_FileType == "ResponseMatrixO4" || m_FileType == "ResponseMatrixO4Stream") {
    MResponseMatrixO4* RO4 = new MResponseMatrixO4();
    if (RO4->Read(FileName) == true) {
      R = RO4;
    }
  } else if (m_FileType == "ResponseMatrixO5" || m_FileType == "ResponseMatrixO5Stream") {
    MResponseMatrixO5* RO5 = new MResponseMatrixO5();
    if (RO5->Read(FileName) == true) {
      R = RO5;
    }
  } else if (m_FileType == "ResponseMatrixO6" || m_FileType == "ResponseMatrixO6Stream") {
    MResponseMatrixO6* RO6 = new MResponseMatrixO6();
    if (RO6->Read(FileName) == true) {
      R = RO6;
    }
  } else if (m_FileType == "ResponseMatrixO7" || m_FileType == "ResponseMatrixO7Stream") {
    MResponseMatrixO7* RO7 = new MResponseMatrixO7();
    if (RO7->Read(FileName) == true) {
      R = RO7;
    }
  } else if (m_FileType == "ResponseMatrixO8" || m_FileType == "ResponseMatrixO8Stream") {
    MResponseMatrixO8* RO8 = new MResponseMatrixO8();
    if (RO8->Read(FileName) == true) {
      R = RO8;
    }
  } else if (m_FileType == "ResponseMatrixO9" || m_FileType == "ResponseMatrixO9Stream") {
    MResponseMatrixO9* RO9 = new MResponseMatrixO9();
    if (RO9->Read(FileName) == true) {
      R = RO9;
    }
  } else if (m_FileType == "ResponseMatrixO10" || m_FileType == "ResponseMatrixO10Stream") {
    MResponseMatrixO10* RO10 = new MResponseMatrixO10();
    if (RO10->Read(FileName) == true) {
      R = RO10;
    }
  } else if (m_FileType == "ResponseMatrixO11" || m_FileType == "ResponseMatrixO11Stream") {
    MResponseMatrixO11* RO11 = new MResponseMatrixO11();
    if (RO11->Read(FileName) == true) {
      R = RO11;
    }
  } else if (m_FileType == "ResponseMatrixO12" || m_FileType == "ResponseMatrixO12Stream") {
    MResponseMatrixO12* RO12 = new MResponseMatrixO12();
    if (RO12->Read(FileName) == true) {
      R = RO12;
    }
  } else if (m_FileType == "ResponseMatrixO13" || m_FileType == "ResponseMatrixO13Stream") {
    MResponseMatrixO13* RO13 = new MResponseMatrixO13();
    if (RO13->Read(FileName) == true) {
      R = RO13;
    }
  } else if (m_FileType == "ResponseMatrixO14" || m_FileType == "ResponseMatrixO14Stream") {
    MResponseMatrixO14* RO14 = new MResponseMatrixO14();
    if (RO14->Read(FileName) == true) {
      R = RO14;
    }
  } else if (m_FileType == "ResponseMatrixO15" || m_FileType == "ResponseMatrixO15Stream") {
    MResponseMatrixO15* RO15 = new MResponseMatrixO15();
    if (RO15->Read(FileName) == true) {
      R = RO15;
    }
  } else if (m_FileType == "ResponseMatrixO16" || m_FileType == "ResponseMatrixO16Stream") {
    MResponseMatrixO16* RO16 = new MResponseMatrixO16();
    if (RO16->Read(FileName) == true) {
      R = RO16;
    }
  } else if (m_FileType == "ResponseMatrixO17" || m_FileType == "ResponseMatrixO17Stream") {
    MResponseMatrixO17* RO17 = new MResponseMatrixO17();
    if (RO17->Read(FileName) == true) {
      R = RO17;
    }
  } else {
    mout<<"MFileResponse::Read: Unknown file type ("<<m_FileType<<") in file "<<FileName<<"."<<endl;
    return 0;
  }

  return R;
}


////////////////////////////////////////////////////////////////////////////////


//! Write some text and clear the stream
void MFileResponse::Write(ostringstream& S) 
{ 
  MFile::Write(S);
  S.str("");
}


////////////////////////////////////////////////////////////////////////////////


//! Write some text
void MFileResponse::Write(const double d) 
{   
  MFile::Write(d);
  MFile::Write(' ');
}


// MFileResponse.cxx: the end...
////////////////////////////////////////////////////////////////////////////////

