/*
 * MERStripPairingDataSet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERStripPairingDataSet__
#define __MERStripPairingDataSet__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include "TMVA/Reader.h"

// MEGAlib libs:
#include "MGlobal.h"
#include "MRESE.h"
#include "MDGeometryQuest.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


//! A dummy definition of a class
class MERStripPairingDataSet
{
  // public interface:
 public:
   //! Default constructor
   MERStripPairingDataSet();
   //! Standard constructor - initialize for the given number of x and y strips
   MERStripPairingDataSet(unsigned int NXStrips, unsigned int NYStrips);
   //! Default destuctor 
  virtual ~MERStripPairingDataSet();

  //! Create data set for the given number of a and y strips
  void Initialize(unsigned int NXStrips, unsigned int NYStrips);
  
  //! Create trees
  //! Tree must be deleted afterwards
  TTree* CreateTree(MString Name);
  
  //! Create readers
  //! Reader must be deleted afterwards
  TMVA::Reader* CreateReader();
  
  //! Fill the data sets from RESEs
  bool FillEventData(Long64_t ID, vector<unsigned int>& XStripIDs, vector<unsigned int>& YStripIDs, vector<double>& XStripEnergies, vector<double>& YStripEnergies);
  
  //! Fill the evaluation section, the real interaction positions
  bool FillResultData(unsigned int NInteractions, bool UndetectedInteractions, vector<double>& Interactions);

  
  
  //! Simulation ID -- might overflow but this is just for diagnostics
  Float_t m_SimulationID;
  //! The IDs of the triggered x strips
  vector<Float_t> m_XStripIDs;
  //! The IDs of the triggered y strips
  vector<Float_t> m_YStripIDs;
  //! The deposited energies in the x strips
  vector<Float_t> m_XStripEnergies;
  //! The deposited energies in the y strips
  vector<Float_t> m_YStripEnergies;
  
  //! The number of actual intersections
  Float_t m_ResultNumberOfInteractions;
  //! Bool (0 = false, 1 = true) indicating if there are undetected intersections
  Float_t m_ResultUndetectedInteractions;
  //! The hit intersections (filling order: x + y * m_XStripIDs.size())
  vector<Float_t> m_ResultInteractions;


  
  // protected methods:
 protected:

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:  
  //! The allowed number of triggered x strips
  unsigned int m_NXStrips;
  //! The allowed number of triggered y strips
  unsigned int m_NYStrips;
   
  
  
#ifdef ___CINT___
 public:
  ClassDef(MERStripPairingDataSet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
