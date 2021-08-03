/*
 * MSimBinaryOptions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimBinaryOptions__
#define __MSimBinaryOptions__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <bitset>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A SimBinaryOptions definition of a class
class MSimBinaryOptions
{
  // public interface:
 public:
   //! Default constructor
   MSimBinaryOptions();
   //! Default constructor
   MSimBinaryOptions(uint32_t Data);
   //! Default destuctor 
  virtual ~MSimBinaryOptions();
  
  void SetUse32BitOptionsPrecision() { m_Data[0] = 0; }
  void SetUse64BitOptionsPrecision() { m_Data[0] = 1; }
  unsigned int GetOptionsPrecision() { return m_Data[0] == 0 ? 32 : 64; }
  
  void SetUse32BitBinaryPrecision() { m_Data[1] = 0; }
  void SetUse64BitBinaryPrecision() { m_Data[1] = 1; }
  unsigned int GetBinaryPrecision() { return m_Data[1] == 0 ? 32 : 64; }
  
  void SetUse32BitEventIDPrecision() { m_Data[2] = 0; }
  void SetUse64BitEventIDPrecision() { m_Data[2] = 1; }
  unsigned int GetEventIDPrecision() { return m_Data[2] == 0 ? 32 : 64; }
  
  void SetUse8BitOriginIDPrecision() { m_Data[3] = 0; m_Data[4] = 0; }
  void SetUse16BitOriginIDPrecision() { m_Data[3] = 0; m_Data[4] = 1; }
  void SetUse32BitOriginIDPrecision() { m_Data[3] = 1; m_Data[4] = 1; }
  unsigned int GetOriginIDPrecision() { return 8 + 8*m_Data[4] + 16*m_Data[3]; }
  
  void SetUse64BitTimePrecision() { m_Data[5] = 0; }
  void SetUse96BitTimePrecision() { m_Data[5] = 1; }
  unsigned int GetTimePrecision() { return m_Data[5] == 0 ? 64 : 96; }
  
  
  void SetHasVeto() { m_Data[8] = 1; }
  bool HasVeto() { return m_Data[8] == 0 ? false : true; }
  void SetHasBadFlags()  { m_Data[9] = 1; }
  bool HasBadFlags() { return m_Data[9] == 0 ? false : true; }
  
  void SetHasOrigins() { m_Data[10] = 1; }
  bool HasOrigins() { return m_Data[10] == 0 ? false : true; }
  void SetHasTime() { m_Data[11] = 1; }
  bool HasTime() { return m_Data[11] == 0 ? false : true; }
  
  
  void SetHasIAs() { m_Data[16] = 1; }
  bool HasIAs() { return m_Data[16] == 0 ? false : true; }
  void SetHasHTs() { m_Data[17] = 1; }
  bool HasHTs() { return m_Data[17] == 0 ? false : true; }
  void SetHasGRs() { m_Data[18] = 1; }
  bool HasGRs() { return m_Data[18] == 0 ? false : true; }
  void SetHasXEs() { m_Data[19] = 1; }
  bool HasXEs() { return m_Data[19] == 0 ? false : true; }
  void SetHasDRs() { m_Data[20] = 1; }
  bool HasDRs() { return m_Data[20] == 0 ? false : true; }
  
  void SetHasOnlySingleINIT() { m_Data[23] = 1; }
  bool HasOnlySingleINIT() { return m_Data[23] == 0 ? false : true; }
  
  void SetHasGalacticPointing() { m_Data[24] = 1; }
  bool HasGalacticPointing() { return m_Data[24] == 0 ? false : true; }
  void SetHasDetectorRotation() { m_Data[25] = 1; }
  bool HasDetectorRotation() { return m_Data[25] == 0 ? false : true; }
  void SetHasHorizonPointing() { m_Data[26] = 1; }
  bool HasHorizonPointing() { return m_Data[26] == 0 ? false : true; }
  
  uint32_t Get32Bit() { return static_cast<uint32_t>(m_Data.to_ulong()); };
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  bitset<32> m_Data;


#ifdef ___CLING___
 public:
  ClassDef(MSimBinaryOptions, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
