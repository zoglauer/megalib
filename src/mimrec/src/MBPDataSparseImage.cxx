/*
 * MBPDataSparseImage.cxx
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
// MBPDataSparseImage.cxx
//
//
// The MBPDataSparseImage class stored a backprojected image as a sparse matrix.
// This only reasonable if more than 50% of the entries are empty.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPDataSparseImage.h"

// Standard libs:
#include "iostream"
using namespace std;

// ROOT libs:

// MEGAlib:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBPDataSparseImage)
#endif


////////////////////////////////////////////////////////////////////////////////


MBPDataSparseImage::MBPDataSparseImage() : MBPData()
{
  // Standard constructor

  m_Type = c_SparseImage;

  m_NEntries = 0;
  m_NBins = 0;

  m_Data = 0;
  m_Index = 0;
}



////////////////////////////////////////////////////////////////////////////////


MBPDataSparseImage::~MBPDataSparseImage()
{
  // Destruct an MBPDataSparseImage

  delete [] m_Data;
  delete [] m_Index;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPDataSparseImage::Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum)
{
  // Constructs an event of type MBPDataSparseImage:
  //
  // Image:            the image as a double-array
  // NBins:            its number of bins
  // Limit:            the limit below this pixel can be rejected
  // NBinsAboveLimit:  the number bins which are kept

  m_NEntries = NUsedBins;
  m_NBins = NBins;

  m_Data = new(nothrow) float[m_NEntries];
  m_Index = new(nothrow) int[m_NEntries];

  if (m_Data == 0 || m_Index == 0) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  for (int i = 0; i < NUsedBins; ++i) {
    m_Data[i] = Image[i];
    m_Index[i] = Bins[i];
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImage::Deconvolve(double* Expectation, double* InvYnew, int Event)
{
  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi
      
  for (int bin = 0; bin < m_NEntries; bin++) {
    Expectation[m_Index[bin]] += m_Data[bin] * InvYnew[Event];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImage::Convolve(double* Ynew, int Event, double* Image, int NBins)
{
  //! Perform the list-mode convolution

  double Sum = 0;
  for (int bin = 0; bin < m_NEntries; bin++) {
    Sum += m_Data[bin] * Image[m_Index[bin]];
  }

  Ynew[Event] = Sum;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImage::Sum(double* Image, int NBins)
{
  //! Just sum it up, i.e. add the content to the image 

  for (int bin = 0; bin < m_NEntries; bin++) {
    Image[m_Index[bin]] += m_Data[bin];
  }
}


////////////////////////////////////////////////////////////////////////////////


int MBPDataSparseImage::GetUsedBytes() const
{
  // Return the number of bytes used by this image
  
  int Bytes = 0;

  Bytes += MBPData::GetUsedBytes();
  Bytes += 2*sizeof(void*); // Pointer to m_Data & m_Index
  Bytes += 2*sizeof(int); // m_NEntries & m_NBins
  Bytes += m_NEntries*sizeof(float); // m_Data
  Bytes += m_NEntries*sizeof(int); // m_Index

  return Bytes;
}


// MBPDataSparseImage: the end...
////////////////////////////////////////////////////////////////////////////////
