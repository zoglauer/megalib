/*
 * MFileHistograms.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileHistograms__
#define __MFileHistograms__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include "TH1.h"
#include "TH2.h"
#include "TH3.h"

// MEGAlib libs:
#include "MGlobal.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MFileHistograms
{
  // public interface:
 public:
  //! Default constructor
  MFileHistograms();
  //! Default destructor - deletes all histograms
  virtual ~MFileHistograms();

  //! Read all histograms from the file
  bool Read(MString FileName);

  //! Get the number of TH1's
  unsigned int GetNH1s() const { return m_H1s.size(); }
  //! Get the number of TH2's
  unsigned int GetNH2s() const { return m_H2s.size(); }
  //! Get the number of TH3's
  unsigned int GetNH3s() const { return m_H3s.size(); }
  
  //! Return a specific H1 histogram, 0 if out od bounds
  TH1* GetH1(unsigned int i) { if (i >= m_H1s.size()) return 0; return m_H1s[i]; } 
  //! Return a specific H2 histogram, 0 if out od bounds
  TH2* GetH2(unsigned int i) { if (i >= m_H2s.size()) return 0; return m_H2s[i]; } 
  //! Return a specific H3 histogram, 0 if out od bounds
  TH3* GetH3(unsigned int i) { if (i >= m_H3s.size()) return 0; return m_H3s[i]; } 

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:
  //! List of all TH1* histograms in the file
  vector<TH1*> m_H1s;
  //! List of all TH2* histograms in the file
  vector<TH2*> m_H2s;
  //! List of all TH3* histograms in the file
  vector<TH3*> m_H3s;

  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MFileHistograms, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
