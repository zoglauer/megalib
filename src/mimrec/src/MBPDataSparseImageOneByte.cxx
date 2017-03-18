/*
 * MBPDataSparseImageOneByte.cxx
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
// MBPDataSparseImageOneByte.cxx
//
//
// The MBPDataSparseImageOneByte class stores a backprojected image (single
// event psf) as a sparse matrix.
// This only reasonable if more than 33% of the entries are empty.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPDataSparseImageOneByte.h"

// Standard libs:
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBPDataSparseImageOneByte)
#endif


////////////////////////////////////////////////////////////////////////////////


MBPDataSparseImageOneByte::MBPDataSparseImageOneByte() : MBPData()
{
  // Standard constructor

  m_Type = c_SparseImageOneBit;

  m_Maximum = 0.0;

  m_Data = nullptr;
  m_IndexStart = nullptr;
  m_IndexContinuation = nullptr;

  m_NEntries = 0;
  m_IndexSize = 0;
  m_NBins = 0;
}



////////////////////////////////////////////////////////////////////////////////


MBPDataSparseImageOneByte::~MBPDataSparseImageOneByte()
{
  // Destruct an MBPDataSparseImageOneByte

  delete [] m_Data;
  delete [] m_IndexStart;
  delete [] m_IndexContinuation;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPDataSparseImageOneByte::Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum)
{
  // Constructs an event of type MBPDataSparseImageOneByte:
  //
  // Image:            the image as a double-array
  // Bins:             the bin IDs associated with the image bins
  // NBins:            its number of bins in the image
  // Limit:            the limit below this pixel can be rejected
  // NBinsAboveLimit:  the number bins which are kept


  m_NBins = NBins;
  m_NEntries = NUsedBins;

  m_Data = new(nothrow) uint8_t[m_NEntries];

  // We are out of memory
  if (m_Data == nullptr) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  m_Maximum = Maximum;

  if (m_Maximum <= 0 && m_NEntries > 0) {
    cout<<"Storing images: The image is empty!"<<endl;
  }

  // Then save the relative values:
  double InvMaximum = 255.0/m_Maximum;

  vector<uint32_t> IndexStart;
  vector<uint8_t> IndexContinuation;

  for (int i = 0; i < m_NEntries; ++i) {
    //cout<<i<<": "<<Image[i]<<endl;
    m_Data[i] = (unsigned char) (InvMaximum * Image[i]);
    if (i == 0) {
      IndexStart.push_back(Bins[i]);
      IndexContinuation.push_back(0);
    } else {
      if (Bins[i] == Bins[i-1]+1 && IndexContinuation.back() != 255) {
        IndexContinuation.back()++;
      } else {
        IndexStart.push_back(Bins[i]);
        IndexContinuation.push_back(0);
      }
    }
  }

  // Why do we copy? A vector can reserve much more memory than we really need...
  m_IndexStart = new(nothrow) uint32_t[IndexStart.size()];
  m_IndexContinuation = new(nothrow) uint8_t[IndexContinuation.size()];

  // We are out of memory
  if (m_IndexStart == nullptr || m_IndexContinuation == nullptr) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  std::copy(IndexStart.begin(), IndexStart.end(), m_IndexStart);
  std::copy(IndexContinuation.begin(), IndexContinuation.end(), m_IndexContinuation);

  m_IndexSize = IndexStart.size();

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Deconvolve(double* Expectation, double* InvYnew, int Event)
{
  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi

  double Scaler = m_Maximum / 255.0;

  int Bin = 0;
  for (unsigned int i = 0; i < m_IndexSize; ++i) {
    int IndexMax = m_IndexStart[i] + m_IndexContinuation[i];
    for (unsigned int Index = m_IndexStart[i]; Index <= IndexMax; ++Index) {
      Expectation[Index] += Scaler * m_Data[Bin] * InvYnew[Event];
      ++Bin;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Convolve(double* Ynew, int Event, double* Image, int NBins)
{
  //! Perform the list-mode convolution

  double Sum = 0;
  double Scaler = m_Maximum / 255.0;

  int Bin = 0;
  for (unsigned int i = 0; i < m_IndexSize; ++i) {
    int IndexMax = m_IndexStart[i] + m_IndexContinuation[i];
    for (unsigned int Index = m_IndexStart[i]; Index <= IndexMax; ++Index) {
      Sum += Scaler * m_Data[Bin] * Image[Index];
      ++Bin;
    }
  }

  //for (int bin = 0; bin < m_NEntries; ++bin) {
  //    Sum += Scaler * m_Data[bin] * Image[m_Index[bin]];
  //}

  Ynew[Event] = Sum;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Sum(double* Image, int NBins)
{
  // Just sum it up, i.e. add the content to the image

  double Scaler = m_Maximum / 255.0;

  int Bin = 0;
  for (unsigned int i = 0; i < m_IndexSize; ++i) {
    int IndexMax = m_IndexStart[i] + m_IndexContinuation[i];
    for (unsigned int Index = m_IndexStart[i]; Index <= IndexMax; ++Index) {
      Image[Index] += Scaler * m_Data[Bin];
      ++Bin;
    }
  }

  //for (int bin = 0; bin < m_NEntries; bin++) {
  //  Image[m_Index[bin]] += Scaler * m_Data[bin];
  //}
}


////////////////////////////////////////////////////////////////////////////////


int MBPDataSparseImageOneByte::GetUsedBytes() const
{
  // Return the number of bytes used by this image

  int Bytes = 0;

  Bytes += MBPData::GetUsedBytes();
  Bytes += sizeof(float); // m_Maximum
  Bytes += 3*sizeof(void*); // Pointer to m_Data,  m_IndexStart & m_IndexContinuation
  Bytes += 3*sizeof(int); // m_NEntries, m_NBins, m_IndexSize
  Bytes += m_NEntries*sizeof(uint8_t); // m_Data
  Bytes += m_IndexSize*sizeof(uint32_t); // m_IndexStart
  Bytes += m_IndexSize*sizeof(uint8_t); // m_IndexContinuation

  return Bytes;
}


// MBPDataSparseImageOneByte: the end...
////////////////////////////////////////////////////////////////////////////////
