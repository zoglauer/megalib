/*
 * MRESEIterator.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MRESEIterator__
#define __MRESEIterator__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <TROOT.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////



//! Iterate through list of (linked) RESEs
//! Usage:
//! MRESEIterator I(Start); // Start is the start point of the sequence
//! MRESE* R = nullptr;
//! while ((R = I->GetNextRESE()) != nullptr) {  /* do something with R */ }
class MRESEIterator
{
  // public interface:
 public:
  MRESEIterator();
  MRESEIterator(MRESE* RESE);
  virtual ~MRESEIterator();

  bool Start(MRESE* RESE);
  MRESE* GetNextRESE();
  int GetNRESEs();
  MRESE* GetRESEAt(int i);

  MRESE* GetPrevious() { return m_Previous; }
  MRESE* GetCurrent() { return m_Current; }
  MRESE* GetNext() { return m_Next; }

  // protected methods:
 protected:
  //MRESEIterator() {};
  //MRESEIterator(const MRESEIterator& RESEIterator) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  bool m_IsStart;
  MRESE* m_Start;        

  MRESE* m_Previous;        
  MRESE* m_Current;        
  MRESE* m_Next;        


#ifdef ___CINT___
 public:
  ClassDef(MRESEIterator, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
