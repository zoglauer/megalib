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

  m_Data = 0;
  m_Index = 0;

  m_NEntries = 0;
  m_NBins = 0;
}



////////////////////////////////////////////////////////////////////////////////


MBPDataSparseImageOneByte::~MBPDataSparseImageOneByte()
{
  // Destruct an MBPDataSparseImageOneByte

  delete [] m_Data;
  delete [] m_Index;
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


  if (NBins >= 65536) {
    cout<<"Storing images: Not more than 65536 image-pixels allowed!"<<endl
        <<"You want "<<NBins<<" bins!"<<endl
        <<"Please use the 4 byte response matrix!"<<endl
        <<"This event now is incompletely sampled!"<<show;
    NBins = 65535;
  }
  
  m_NBins = NBins;
  m_NEntries = NUsedBins;

  m_Data = new(nothrow) unsigned char[m_NEntries];
  m_Index = new(nothrow) unsigned short[m_NEntries];

  // We are out of memory
  if (m_Data == 0 || m_Index == 0) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  m_Maximum = Maximum;

  if (m_Maximum <= 0 && m_NEntries > 0) {
    cout<<"Storing images: The image is empty!"<<endl;
  }

  // Then save the relative values:
  double InvMaximum = 255.0/m_Maximum;
  for (unsigned short i = 0; i < m_NEntries; ++i) {
    m_Data[i] = (unsigned char) (InvMaximum * Image[i]);
    m_Index[i] = Bins[i];
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Deconvolve(double* Expectation, double* InvYnew, int Event)
{
  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi
      
  double Scaler = m_Maximum / 255.0;
  for (int bin = 0; bin < m_NEntries; ++bin) {
    Expectation[m_Index[bin]] += Scaler * m_Data[bin] * InvYnew[Event];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Convolve(double* Ynew, int Event, double* Image, int NBins)
{
  //! Perform the list-mode convolution

  double Sum = 0;
  double Scaler = m_Maximum / 255.0;
  for (int bin = 0; bin < m_NEntries; ++bin) {
    Sum += Scaler * m_Data[bin] * Image[m_Index[bin]];
  }

  Ynew[Event] = Sum;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataSparseImageOneByte::Sum(double* Image, int NBins)
{
  // Just sum it up, i.e. add the content to the image 
      
  double Scaler = m_Maximum / 255.0;
  for (int bin = 0; bin < m_NEntries; bin++) {
    Image[m_Index[bin]] += Scaler * m_Data[bin];
  }
}


////////////////////////////////////////////////////////////////////////////////


int MBPDataSparseImageOneByte::GetUsedBytes() const
{
  // Return the number of bytes used by this image
  
  int Bytes = 0;

  Bytes += MBPData::GetUsedBytes();
  Bytes += sizeof(float); // m_Maximum
  Bytes += 2*sizeof(void*); // Pointer to m_Data & m_Index
  Bytes += 2*sizeof(int); // m_NEntries & m_NBins
  Bytes += m_NEntries*sizeof(unsigned char); // m_Data
  Bytes += m_NEntries*sizeof(unsigned short); // m_Index

  return Bytes;
}


// MBPDataSparseImageOneByte: the end...
////////////////////////////////////////////////////////////////////////////////
