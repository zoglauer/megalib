/*
 * MEREventClusterizerDataSet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MEREventClusterizerDataSet__
#define __MEREventClusterizerDataSet__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MEREventClusterizerDataSet
{
  // public interface:
 public:
   //! Default constructor
   MEREventClusterizerDataSet();
   //! Standard constructor - initialize for the given number of x and y strips
   MEREventClusterizerDataSet(unsigned int NHits, unsigned int MaxGroups);
   //! Default destuctor 
  virtual ~MEREventClusterizerDataSet();

  //! Create data set for the given number of a and y strips
  void Initialize(unsigned int NHits, unsigned int NMaxGroups);
  
  //! Create trees
  //! Tree must be deleted afterwards
  TTree* CreateTree(MString Name);
  
  //! Create readers
  //! Reader must be deleted afterwards
  TMVA::Reader* CreateReader();
  
  //! Fill the data sets from RESEs
  bool FillEventData(Long64_t ID, vector<MRESE*>& RESEs);
  
  //! Fill the evaluation section, the real interaction positions
  bool FillResultData(vector<vector<int>>& HitOriginIDs);

  //! Write the currently loaded data set to a string
  MString ToString();
  
  
  //! Simulation ID -- might overflow / truncate but this is just for diagnostics
  Float_t m_SimulationID;
  //! Measured energies
  vector<Float_t> m_Energies;
  //! Measured positions x
  vector<Float_t> m_PositionsX;
  //! Measured positions y
  vector<Float_t> m_PositionsY;
  //! Measured positions z
  vector<Float_t> m_PositionsZ;
  
  //! The IDs of the hit x strips
  vector<vector<Float_t>> m_ResultHitGroups;
  
  
  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:  
  //! The allowed number of triggered x strips
  unsigned int m_NHits;
  //! The allowed number of triggered y strips
  unsigned int m_NMaxGroups;
   
  
  
#ifdef ___CLING___
 public:
  ClassDef(MEREventClusterizerDataSet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
