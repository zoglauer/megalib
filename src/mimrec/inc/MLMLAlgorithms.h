/*
 * MLMLAlgorithms.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MLMLAlgorithms__
#define __MLMLAlgorithms__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBPData.h"
#include "MExposure.h"
#include "MBackground.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MLMLAlgorithms
{
  // public interface:
 public:
  //! Default constructor
  MLMLAlgorithms();
  //! Default destructor
  virtual ~MLMLAlgorithms();

  //! Use the stop criterion
  void UseStopCriterionByIterations(unsigned int NIterations);
  //! Return true if the stop criterion is fullfilled
  bool IsStopCriterionFullfilled();
  //! Reset data stored for the stop criterion
  void ResetStopCriterion();

  //! Return an initial image -- has to be overwritten
  virtual double* GetInitialImage() = 0;
  //! Perform one iteration -- has to be overwritten
  virtual bool DoOneIteration() = 0;

  //! Return the current reconstructed image
  virtual double* GetImage();

  //! Return the maximum number of iterations
  unsigned int GetMaxNIterations() const { return m_MaxNIterations; }

  //! Set if GUI interaction are possible (i.e. if ProcessEvents is called)
  //! This should be always set to false in a multi-threaded environment!
  void EnableGUIInteractions(bool EnableGUIInteractions = true) { m_EnableGUIInteractions = EnableGUIInteractions; }

  //! Set the list-mode response matrix
  virtual bool SetResponseSlices(vector<MBPData*>& Data, int NImageBins);
  //! Set the exposure map
  virtual void SetExposure(MExposure* Exposure);
  //! Set the backgroynd
  virtual void SetBackground(MBackground* Background);
  //! Set the number of threads
  virtual void SetNumberOfThreads(unsigned int NThreads);


  //! ID for the classic MLEM algorithm
  static const unsigned int c_ClassicEM;
  //! ID for the ordered subset EM algorithm
  static const unsigned int c_OSEM;

  //! ID for the stop criterion By Iterations
  static const unsigned int c_StopAfterIterations;
  //! ID for the stop criterion By Iterations
  static const unsigned int c_StopAfterLikelihoodIncrease;

  // protected methods:
 protected:
  //! Determine the apportionment of the events for the threads
  virtual void CalculateEventApportionment();

  //! Shuffle the events around - does notthing here, but in ordered subsets algorithm
  virtual void Shuffle() {};

  // private methods:
 private:



  // protected members:
 protected:
  //! Stores the ID of the used stoip criterion
  unsigned int m_UsedStopCriterion;
  //! Stores the maximum number of iterations for the "ByIterations" stop criterion
  unsigned int m_MaxNIterations;

  //! Number of iterations to perform
  //unsigned int m_NIterations;
  //! Number of already performed iterations
  unsigned int m_NPerformedIterations;

  //! The number of threads intended to be used
  unsigned int m_NThreads;
  //! The number of threads really used
  unsigned int m_NUsedThreads;
  
  //! Number of image bins
  unsigned int m_NBins;
  //! Number of events
  unsigned int m_NEvents;

  //! estimated image after several iterations
  double* m_Lj;
  //! sensitivity
  double* m_Sj;
  //! result of the convolution
  double* m_Yi;
  //! Inverted result of the convolution
  double* m_InvYi;
  //! background rates
  double* m_Ri;
  //! estimated background scaling factor after several iterations
  double m_BgdS;
  //! total background
  double m_BgdT;

  //! PSF storage
  vector<MBPData*> m_Storage;

  //! Event apportionment for the threads
  vector<pair<unsigned int, unsigned int>> m_EventApportionment;
  //! Flags indicating the threads are running:
  vector<bool> m_ThreadRunning;


  //! The exposure map
  MExposure* m_Exposure;
  //! Background model
  MBackground* m_Background;

  //! True if GUI interaction should be enabled, i.e. if ProcessEvents() is called (default: true)
  bool m_EnableGUIInteractions;


  // private members:
 private:


#ifdef ___CINT___
 public:
  ClassDef(MLMLAlgorithms, 0) // base class for other LML algorithms e.g. classic EM or OS-EM
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
