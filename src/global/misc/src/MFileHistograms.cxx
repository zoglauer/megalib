/*
 * MFileHistograms.cxx
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
// MFileHistograms
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MFileHistograms.h"

// Standard libs:
#include <limits>
using namespace std;

// ROOT libs:
#include "TFile.h"
#include "TCanvas.h"
#include "TKey.h"
#include "TIterator.h"
#include "TList.h"
#include "TClass.h"

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MFile.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MFileHistograms)
#endif


////////////////////////////////////////////////////////////////////////////////


MFileHistograms::MFileHistograms()
{
  // Construct an instance of MFileHistograms
}


////////////////////////////////////////////////////////////////////////////////


MFileHistograms::~MFileHistograms()
{
  // Delete this instance of MFileHistograms

}


////////////////////////////////////////////////////////////////////////////////


bool MFileHistograms::Read(MString FileName) 
{
  if (MFile::Exists(FileName) == false) return false;
  
  TFile* F = new TFile(FileName);
  
  TIter I(F->GetListOfKeys());
  TKey* Key = 0;

  while ((Key = (TKey*) I()) != 0) {
    TClass *Class = gROOT->GetClass(Key->GetClassName());
    if (Class->InheritsFrom("TH1")) {
      TH1* H = dynamic_cast<TH1*>(Key->ReadObj());
      H->SetDirectory(0);
      if (Class->InheritsFrom("TH3")) {
        m_H3s.push_back(dynamic_cast<TH3*>(H));       
        m_H3s.push_back(dynamic_cast<TH3*>(H));       
      } else if (Class->InheritsFrom("TH2")) {
        m_H2s.push_back(dynamic_cast<TH2*>(H));       
      } else {
        m_H1s.push_back(H);
      }
    } else if (Class->InheritsFrom("TCanvas")) {
      TCanvas* C = (TCanvas*) Key->ReadObj();
      TIter CanvasContent(C->GetListOfPrimitives());
      TObject* Object;
      while ((Object = CanvasContent()) != 0) {
        dynamic_cast<TH1*>(Object)->SetDirectory(0);
        if (Object->InheritsFrom("TH1")) {
          m_H1s.push_back(dynamic_cast<TH1*>(Object));
        } else if (Object->InheritsFrom("TH2")) {
          m_H2s.push_back(dynamic_cast<TH2*>(Object));
        } else if (Object->InheritsFrom("TH3")) {
          m_H3s.push_back(dynamic_cast<TH3*>(Object));
        }
      }
    }
  }
  
  F->Close();
  
  return true;
}


// MFileHistograms.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
