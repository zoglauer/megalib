/*
 * MSimCluster.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MSimCluster__
#define __MSimCluster__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// Root libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MDVolumeSequence.h"

// Forward declarations:
class MSimHT;


////////////////////////////////////////////////////////////////////////////////


class MSimCluster
{
  // public interface:
 public:
  MSimCluster();
  virtual ~MSimCluster();

  MVector GetPosition();
  double GetEnergy();
  int GetDetector();
  double GetTime();
  //! Return all origins
  vector<int> GetOrigins() const;
  //! Return the volume sequence of this hit
  MDVolumeSequence* GetVolumeSequence();

  unsigned int GetNHTs();
  MSimHT* GetHTAt(unsigned int i);
  bool AddHT(MSimHT* HT);

  // Convert the content to a hit
  MSimHT* CreateHT();

  MString ToString();

  bool HasOrigin(int Origin);

  // protected methods:
 protected:


  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:
  //! List of events
  vector<MSimHT*> m_HTList;

  //! Type of the detector where the hit took place
  int m_Detector;
  //! Position, where the hit took place
  MVector m_Position;  
  //! Energy deposit
  double m_Energy;
  //! Time of the deposit
  double m_Time;           
  //! Particles from these vertices (= origins) contributed to this hit
  vector<int> m_Origins; 
  //! Volume history of theis hit
  MDVolumeSequence* m_VolumeSequence;   
  
#ifdef ___CINT___
 public:
  ClassDef(MSimCluster, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
