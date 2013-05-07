/*
 * MBPStorage.h
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBPStorage__
#define __MBPStorage__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:
#include <vector>
using namespace std;

// ROOT libs:
#include <TObjArray.h>
#include <TMatrix.h>
#include <TThread.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MVector.h"
#include "MBPDataImage.h"
#include "MBackprojection.h"
#include "MEventSelector.h"
#include "MFileEventsTra.h"

// Forward declarations:
class MBPData;


////////////////////////////////////////////////////////////////////////////////


class MBPStorage
{
  // Public Interface:
 public:
  //! Standard constructor
  MBPStorage(int Algorithm, int CoordinateSystem, unsigned int NThreads = 1);
  //! Default destructor
  ~MBPStorage();

  //! Set the image dimensions
  void SetViewport(double xMin, double xMax, int xNBins, 
                   double yMin, double yMax, int yNBins,
                   double zMin = 0, double zMax = 0, int zNBins = 1,
                   MVector xAxis = MVector(1.0, 0.0, 0.0), MVector zAxis = MVector(0.0, 0.0, 1.0));

  //! Set the Gaussian response 
  void SetResponseGaussian(const double Transversal, const double Longitudinal, const double Pair);
  //! Set the energy-leakage response 
  void SetResponseEnergyLeakage(const double Longitudinal, const double Transversal);
  //! Set response matrices
  bool SetResponsePRM(const TString& ComptonTrans, const TString& ComptonLong, const TString& Pair);

  //! Set the sensitivity matrix
  void SetSensitivity(MSensitivity* Sensitivity);
  //! Set the pointsources which are cut out of the picture
  void SetDeselectedPointSources(TObjArray* DeselectedPS);

  //! Set the memory managment features
  void SetMemoryManagment(int MaxRAM, int MaxSwap, int Exhausted, int Accuracy);

  //! Set the event selector 
  void SetEventSelector(const MEventSelector& Selector);
  //! Set the geometry
  void SetGeometry(MDGeometryQuest* Geometry);
  //! Open the event file
  bool SetFileName(TString FileName); 

  //! Compute all response slices
  bool ComputeResponseSlices();

  //! Entry function for the threads
  void* ResponseSliceComputationThread(unsigned int ThreadID);

  //! Return the number of events the response is made of
  unsigned int GetNEvents();
  //! Return the number of bins of the image-space
  int GetNImageBins();

  //! Return the response slice corresponding to the i-th event
  MBPData* GetResponseSlice(unsigned int i);
  //! Add a response slice
  void AddResponseSlice(MBPData* Slice);

  //! Return the amount of RAM currently used for the response slices
  unsigned long GetUsedBytes() { return m_UsedBytes; }

  // protected methods:
 protected:


  // private members:
 private:

  // Response calculation

  //! Number of bins (x*y*z) of the image
  int m_NBins;  
  //! type of EM-Algorithm (1:EM, 2:OSEM, 3:SAGE)
  int m_EMAlgorithm;

  //! The response slices in list-mode
  vector<MBPData*> m_BPEvents;
  //! The backprojection algorithm classs --- one per thread
  vector<MBackprojection*> m_BPs;
 
  //! The event selector
  MEventSelector m_Selector;
  //! The event file
  MFileEventsTra m_EventFile;

  // Multi-threading

  //! Number of threads
  unsigned int m_NThreads;
  //! Storing the threads:
  vector<TThread*> m_Threads;
  //! Storing a flag that the thread is running
  vector<bool> m_ThreadIsInitialized;
  //! Storing a flag that the thread is finished
  vector<bool> m_ThreadIsFinished;

  // Memory management

  //! Maximum amount of RAM in bytes to be used on this computer
  unsigned long m_MaxBytes;
  //! Flag indicating the storage accuracy of the response slices
  int m_ComputationAccuracy;
  
  //! Flag indicating we went out of memory
  bool m_OutOfMemory;
  
  //! Currently used bytes for the response
  unsigned long m_UsedBytes;


#ifdef ___CINT___
 public:
  ClassDef(MBPStorage, 0) // Computes and stores system matrix
#endif

};

////////////////////////////////////////////////////////////////////////////////

class MBPStorage_ThreadCaller
{
 public:
  //! Standard constructor
  MBPStorage_ThreadCaller(MBPStorage* S, unsigned int ThreadID) {
    m_Storage = S;
    m_ThreadID = ThreadID;
  }

  //! Return the calling class
  MBPStorage* GetThreadCaller() { return m_Storage; }
  //! Return the thread ID
  unsigned int GetThreadID() { return m_ThreadID; }

 private:
  //! Store the calling class for retrieval
  MBPStorage* m_Storage;
  //! ID of the worker thread
  unsigned int m_ThreadID;
};

////////////////////////////////////////////////////////////////////////////////

void MBPStorage_CallThread(void* address);

#endif


////////////////////////////////////////////////////////////////////////////////
