/*
 * MBPStorage.cxx
 *
 *
 * Copyright (C) 1998-2009 by Andreas Zoglauer.
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
// MBPStorage.cxx
//
//
// Computes and stores the data of the system-matrix.
// Make sure to set all data properly in advance.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPStorage.h"

// Standard libs:
#include <iostream>
#include <limits>
using namespace std;

// ROOT libs:

// MEGAlib libs:
#include "MAssert.h"
#include "MStreams.h"
#include "MBackprojection.h"
#include "MBackprojectionSpherePoints.h"
#include "MBackprojectionSphereArea.h"
#include "MBackprojectionCart2DArea.h"
#include "MBackprojectionCart3DArea.h"
#include "MBPDataSparseImage.h"
#include "MBPDataSparseImageOneByte.h"
#include "MPhysicalEvent.h"
#include "MBPDataImage.h"
#include "MBPDataImageOneByte.h"
#include "MGUIProgressBar.h"
#include "MSystem.h"
#include "MResponse.h"
#include "MResponseGaussian.h"
#include "MResponsePRM.h"
#include "MResponseEnergyLeakage.h"
#include "MSensitivity.h" 


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBPStorage)
#endif


////////////////////////////////////////////////////////////////////////////////


void MBPStorage_CallThread(void* Address)
{
  MBPStorage* Storage = ((MBPStorage_ThreadCaller*) Address)->GetThreadCaller();
  Storage->ResponseSliceComputationThread(((MBPStorage_ThreadCaller*) Address)->GetThreadID());
}


////////////////////////////////////////////////////////////////////////////////


MBPStorage::MBPStorage(int Algorithm, int CoordinateSystem, unsigned int NThreads)
{
  // Initialize the system-matriy:
  //
  // Algorithm:         1: EM, 2: OS-EM, 3: SAGE
  // CoordinateSystem:  1: Spherical, 2: 2D-Cartesian 3: 3D-Cartesian

  m_NThreads = NThreads;
  if (m_NThreads < 1) m_NThreads = 1;

  m_EMAlgorithm = Algorithm;
  for (unsigned int t = 0; t < m_NThreads; ++t) {
    // Initialize the backprojection-class
    if (Algorithm == 2 && (CoordinateSystem == MProjection::c_Spheric || CoordinateSystem == MProjection::c_Galactic)) {
      m_BPs.push_back((MBackprojection *) new MBackprojectionSphereArea());
    }
    //else if (Algorithm == 1 && (CoordinateSystem == MProjection::c_Spheric || CoordinateSystem == MProjection::c_Galactic)) { 
    //  m_BPs.push_back((MBackprojection *) new MBackprojectionSphereLine());
    //}
    else if (Algorithm == 0 && (CoordinateSystem == MProjection::c_Spheric || CoordinateSystem == MProjection::c_Galactic)) {
      m_BPs.push_back((MBackprojection *) new MBackprojectionSpherePoints());
    }
    else if (Algorithm == 2 && CoordinateSystem == MProjection::c_Cartesian2D) {
      m_BPs.push_back((MBackprojection *) new MBackprojectionCart2DArea());
    }
    else if (Algorithm == 2 && CoordinateSystem == MProjection::c_Cartesian3D) {
      m_BPs.push_back((MBackprojection *) new MBackprojectionCart3DArea());
    }
    //else if (Algorithm == 0 && CoordinateSystem == MProjection::c_Cartesian2D) {
    //  m_BPs.push_back((MBackprojection *) new MBackprojectionCart2DPoint());
    //}
    else {
      mgui<<"Unknown backprojection algorithm / coordinate system combination."<<endl;
      mgui<<"I will use area mode with a spherical coordinate system!"<<error;
      m_BPs.push_back((MBackprojection *) new MBackprojectionSphereArea());   
      m_EMAlgorithm = 2;
    }      
    m_Threads.push_back(0);
    m_ThreadIsInitialized.push_back(false);
    m_ThreadIsFinished.push_back(false);
  }


  m_BPEvents.clear();

  m_UsedBytes = 0;
  m_MaxBytes = numeric_limits<unsigned long>::max();
  m_ComputationAccuracy = 1;

  m_OutOfMemory = false;
}


////////////////////////////////////////////////////////////////////////////////


MBPStorage::~MBPStorage()
{
  // standard destructor

  for (unsigned int i = 0; i < m_BPEvents.size(); ++i) {
    delete m_BPEvents[i];
  }
  m_BPEvents.clear();

  for (unsigned int t = 0; t < m_NThreads; ++t) {
    delete m_BPs[t];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetViewport(double x1Min, double x1Max, int x1NBins, 
                             double x2Min, double x2Max, int x2NBins,
                             double x3Min, double x3Max, int x3NBins,
                             MVector xAxis, MVector zAxis) 
{
  // Set the viewport of this event
  // In spherical coordinates x means theta and y means phi

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    cout<<t<<": "<<m_BPs.size()<<endl;
    m_BPs[t]->SetViewportDimensions(x1Min, x1Max, x1NBins, 
                                    x2Min, x2Max, x2NBins, 
                                    x3Min, x3Max, x3NBins,
                                    xAxis, zAxis);
  }
   
  m_NBins = x1NBins*x2NBins*x3NBins;
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetDeselectedPointSources(TObjArray* DeselectedPS)
{
  // Set the pointsources which are cut out of the picture

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetDeselectedPointSources(DeselectedPS);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetResponseGaussian(double Transversal, double Longitudinal, double Pair)
{
  // Set the Gaussian response parameters

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponseGaussian* Response = new MResponseGaussian(Transversal, Longitudinal, Pair);
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetResponseEnergyLeakage(double Electron, double Gamma)
{
  // Set the energy leakage response parameters

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponseEnergyLeakage* Response = new MResponseEnergyLeakage(Electron, Gamma);
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }
}


////////////////////////////////////////////////////////////////////////////////


bool MBPStorage::SetResponsePRM(const TString& ComptonTrans, 
                                const TString& ComptonLong, 
                                const TString& PairRadial)
{
  // Set the response matrices

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    MResponsePRM* Response = new MResponsePRM();
    if (Response->LoadResponseFiles(ComptonTrans, ComptonLong, PairRadial) == false) {
      mgui<<"Unable to load responsefiles!"<<endl;
      delete Response;
      return false;
    }
    m_BPs[t]->SetResponse(dynamic_cast<MResponse*>(Response));
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetSensitivity(MSensitivity* Sensitivity)
{
  // Set the sensitivity matrix

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetSensitivity(Sensitivity);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetGeometry(MDGeometryQuest* Geometry)
{
  // Set the geometry

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->SetGeometry(Geometry);
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetEventSelector(const MEventSelector& Selector)
{
  // Set all event parameters
  
  m_Selector = Selector;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPStorage::SetFileName(TString FileName)
{
  // Open the event file
  // Make sure to catch a false here, otherwise the class is not correctly initialized!

  if (m_EventFile.IsOpen() == true) {
    m_EventFile.Close();
  }
  if (m_EventFile.Open(FileName) == false) {
    return false;
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPStorage::SetMemoryManagment(int MaxRAM, int MaxSwap, int Exhausted, int Accuracy)
{
  // Set the memory managment features

  m_MaxBytes = (unsigned long) MaxRAM * 1024 * 1024;
  m_ComputationAccuracy = Accuracy;
}


////////////////////////////////////////////////////////////////////////////////


MBPData* MBPStorage::GetResponseSlice(unsigned int i)
{
  // Return the i-th event of the list-mode system-matrix
  
  massert(i < m_BPEvents.size());

  return m_BPEvents[i];
}

////////////////////////////////////////////////////////////////////////////////


void MBPStorage::AddResponseSlice(MBPData* Slice)
{
  // Add a response slice

  m_BPEvents.push_back(Slice);
  m_UsedBytes += Slice->GetUsedBytes();
}


////////////////////////////////////////////////////////////////////////////////


unsigned int MBPStorage::GetNEvents()
{
  // Return the number of events the response is made of

  return m_BPEvents.size();
}
  
 
////////////////////////////////////////////////////////////////////////////////


int MBPStorage::GetNImageBins()
{
  // return the number of bins of the image-space

  return m_NBins;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPStorage::ComputeResponseSlices()
{
  // Computes and stores the system-matrix

  // Test if everything has been initialized:

  for (unsigned int t= 0; t < m_NThreads; ++t) {
    if (m_BPs[t]->GetResponse() == 0) {
      merr<<"The fit-parameters have not been initialized: "
        "Call \"void MBPStorage::SetResponse...(...)\" in advance."<<show;
      return false;
    }
  }

  if (m_EventFile.IsOpen() == false) {
    merr<<"The event file is not open"<<show;
    return false;
  }


  // Available memory:
  MSystem System;

  // Switch to four byte storage if we exceed 2^16 bins:
  if (m_NBins >= 65536) {
    m_ComputationAccuracy = 1;
  }

  // Prepare data-storage:

	// Stop the time ...
  TTime time;
  time = gSystem->Now();


	// Prepare the response calculation
  for (unsigned int t= 0; t < m_NThreads; ++t) {
    m_BPs[t]->PrepareBackprojection();
  }

  m_EventFile.ShowProgress(true);

  if (m_NThreads > 1) {
    m_EventFile.StartThread();

    // Start threads

    for (unsigned int t = 0; t < m_NThreads; ++t) {
      TString Name = "Imaging thread #";
      Name += t;
      TThread* Thread = new TThread(Name, (void(*) (void *)) &MBPStorage_CallThread, (void*) new MBPStorage_ThreadCaller(this, t));
      m_Threads[t] = Thread;
      m_ThreadIsInitialized[t] = false;
      m_ThreadIsFinished[t] = false;

      Thread->Run();
      
      // Wait until thread is initialized:
      while (m_ThreadIsInitialized[t] == false) {
        // Sleep for a while...
        TThread::Sleep(0, 10000000);
      }    

      cout<<Name<<" is running"<<endl;
    }
    
    bool ThreadsAreRunning = true;
    while (ThreadsAreRunning == true) {

      // Sleep for a while...
      TThread::Sleep(0, 10000000);
      
      ThreadsAreRunning = false;
      for (unsigned int t = 0; t < m_NThreads; ++t) {
        if (m_ThreadIsFinished[t] == false) {
          ThreadsAreRunning = true;
          break;
        }
      }
    }

    // None of the threads are running any more --- kill them
    for (unsigned int t = 0; t < m_NThreads; ++t) {
      m_Threads[t]->Kill();
      m_Threads[t] = 0;
    }
  }
  // Non-threaded mode
  else {
    ResponseSliceComputationThread(0);
  }

  if (m_OutOfMemory == true) {
    // Free some space --- remove 5% of the stored BPs
    unsigned int NEventsToDelete = m_BPEvents.size()/20;
    mout<<"Out of memory condition: Erasing "<<NEventsToDelete<<" events to free some memory..."<<endl;
    for (unsigned int i = 0; i < NEventsToDelete; ++i) {
      vector<MBPData*>::iterator Iter = m_BPEvents.begin();
      m_UsedBytes -= (*Iter)->GetUsedBytes();
      delete *Iter;
      m_BPEvents.erase(Iter);
    }
  }

  cout<<"Finished after "<<(gSystem->Now() - time).AsString()<< " milliseconds."<<endl;
  m_EventFile.Close();

  return (m_BPEvents.size() > 0) ? true : false;
}


////////////////////////////////////////////////////////////////////////////////


void* MBPStorage::ResponseSliceComputationThread(unsigned int ThreadID)
{
  // This function is part of the thread
  // Lock all pieces of this class which might be accessed by any other thread!


  double Limit;
  int NAboveLimit;
  bool EnoughMemory = true;
  MPhysicalEvent* Event;
  MBPData* Data = 0;

  double* SingleBackprojection = new double[m_NBins];

  // Create a local copy of the backprojection class 
  
  TThread::Lock();
  m_ThreadIsInitialized[ThreadID] = true;
  TThread::UnLock();


  // The reconstruction loop
  while (true) {
    if (m_OutOfMemory == true) break;

    Event = m_EventFile.GetNextEvent();

    // If we don't get an event a serious error ocurred, or we are finished
    if (Event == 0) break;

    /// IsQualified is NOT reentrant --- but the only thing modified are its counters, which we do not use here...
    if (m_Selector.IsQualifiedEvent(Event) == true) {
      // Reinitialize the array keeping the events backprojection 
      for (int i = 0; i < m_NBins; i++) SingleBackprojection[i] = 0; 

      // Try to backproject the data and store the computed t_ij in SingleBackprojection
      Limit = 0;
      NAboveLimit = 0;
      if (m_BPs[ThreadID]->Backproject(Event, SingleBackprojection, Limit, NAboveLimit) == true) {

        // It might happen that we go out of memory during imaging, catch it!
        // 1-byte-storage:
        if (m_ComputationAccuracy == 0) {
          // Test if we can store it as sparse matrix:
          if (NAboveLimit < 0.33*m_NBins && m_EMAlgorithm != 3) {
            Data = new(nothrow) MBPDataSparseImageOneByte();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(SingleBackprojection, m_NBins, Limit, NAboveLimit);
            } else {
              EnoughMemory = false;
            }
          } else { // no sparse matrix
            Data = new(nothrow) MBPDataImageOneByte();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(SingleBackprojection, m_NBins);
            } else {
              EnoughMemory = false;
            }
          }
        } 
        // 4-byte storage:
        else if (m_ComputationAccuracy == 1) {
          if (NAboveLimit < 0.5*m_NBins && m_EMAlgorithm != 3) {
            Data = new(nothrow) MBPDataSparseImage();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(SingleBackprojection, m_NBins, Limit, NAboveLimit);
            } else {
              EnoughMemory = false;
            }
          }
          else { // no sparse matrix
            Data = new(nothrow) MBPDataImage();
            if (Data != 0) {
              EnoughMemory = Data->Initialize(SingleBackprojection, m_NBins);
            } else {
              EnoughMemory = false;
            }
         }
        } else {
          // "merr" not thread safe --- but we crash anyway ;-)
          merr<<"m_ComputationAccuracy must be 0 (1 byte storage) or 1 (4 byte storage): "<<m_ComputationAccuracy<<fatal;
        }

        if (EnoughMemory == false) {
          cout<<"Thread "<<ThreadID<<": Out of memory --- finishing..."<<endl;
          delete Event;
          break;
        }
          
        TThread::Lock();
        AddResponseSlice(Data);
        if (GetUsedBytes() > m_MaxBytes) {
          cout<<"Thread "<<ThreadID<<": Used RAM exceeds the user set maximum ("<<m_MaxBytes/1024/1024<<" MB)  --- finishing..."<<endl;
          TThread::UnLock();
          break;
        }
        TThread::UnLock();
      }
    }

    //TThread::Lock();
    delete Event;
    //TThread::UnLock();
  }

  //TThread::Lock();
  delete [] SingleBackprojection;
  //TThread::UnLock();
  
  TThread::Lock();
  m_ThreadIsFinished[ThreadID] = true;
  if (EnoughMemory == false) {
    m_OutOfMemory = true;
  }
  TThread::UnLock();

  return 0;
}


// MBPStorage: the end...
////////////////////////////////////////////////////////////////////////////////
