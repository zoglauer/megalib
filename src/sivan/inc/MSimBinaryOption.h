/*
 * MMSimBinaryOptions.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MMSimBinaryOptions__
#define __MMSimBinaryOptions__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A MSimBinaryOptions definition of a class
class MMSimBinaryOptions
{
  // public interface:
 public:
   //! Default constructor
   MMSimBinaryOptions(uint16_t Data);
   //! Default constructor
   MMSimBinaryOptions(uint32_t Data);
   //! Default destuctor 
  virtual ~MMSimBinaryOptions();
  
  void SetIs32Bit() { m_Data[0] = 1; }
  bool Is32Bit() { return m_Data[0] == 0 ? false : true; }
  
  void SetHasVeto() { m_Data[1] = 1; }
  bool HasVeto() { return m_Data[0] == 0 ? false : true; }
  void SetHasBadFlags()  { m_Data[2] = 1; }
  bool HasBadFlags() { return m_Data[0] == 0 ? false : true; }
  
  void SetHasIA() { m_Data[3] = 1; }
  bool HasIA() { return m_Data[0] == 0 ? false : true; }
  void SetHasHT() { m_Data[4] = 1; }
  bool HasHT() { return m_Data[0] == 0 ? false : true; }
  void SetHasGR() { m_Data[5] = 1; }
  bool HasGR() { return m_Data[0] == 0 ? false : true; }
  void SetHasXE() { m_Data[6] = 1; }
  bool HasXE() { return m_Data[0] == 0 ? false : true; }
  void SetHasDR() { m_Data[7] = 1; }
  bool HasDR() { return m_Data[0] == 0 ? false : true; }
  
  void SetHasGalacticPointing() { m_Data[8] = 1; }
  bool HasGalacticPointing() { return m_Data[0] == 0 ? false : true; }
  void SetHasDetectorPoining() { m_Data[9] = 1; }
  bool HasDetectorPoining() { return m_Data[0] == 0 ? false : true; }
  void SetHasHorizonPointing() { m_Data[10] = 1; }
  bool HasHorizonPointing() { return m_Data[0] == 0 ? false : true; }
  
  
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
  ClassDef(MMSimBinaryOptions, 1)
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
