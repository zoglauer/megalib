/*
 * MERTrackWithinCrossStripDetectorTMVADataSet.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MERTrackWithinCrossStripDetectorTMVADataSet__
#define __MERTrackWithinCrossStripDetectorTMVADataSet__


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


//! A single data set storing the pattern of a Compton recoil electron in a cross-strip detector
class MERTrackWithinCrossStripDetectorTMVADataSet
{
  // public interface:
 public:
  //! Default constructor
  MERTrackWithinCrossStripDetectorTMVADataSet();
  //! Standard constructor - initialize for the given number of x and y strips
  MERTrackWithinCrossStripDetectorTMVADataSet(unsigned int NXStrips, unsigned int NYStrips);
  //! Default destuctor
  virtual ~MERTrackWithinCrossStripDetectorTMVADataSet();

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
  bool FillResultData(const MVector& InteractionPosition, const MVector& EkectronDirections);

  
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
  
  //! The interaction position X
  Float_t m_ResultPositionX;
  //! The interaction position Y
  Float_t m_ResultPositionY;
  //! The interaction position Z
  Float_t m_ResultPositionZ;
  //! The recoil electron direction X
  Float_t m_ResultDirectionX;
  //! The recoil electron direction Y
  Float_t m_ResultDirectionY;
  //! The recoil electron direction Z
  Float_t m_ResultDirectionZ;


  
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
   
  
  
#ifdef ___CLING___
 public:
  ClassDef(MERTrackWithinCrossStripDetectorTMVADataSet, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
