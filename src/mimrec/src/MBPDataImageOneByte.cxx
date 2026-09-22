/*
 * MBPDataImageOneByte.cxx
 *
 * Copyright (C) by the MEGAlib contributors.
 *
 * This file is part of MEGAlib.
 *
 * MEGAlib is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * MEGAlib is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
 * License (License.md) for more details.
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */


////////////////////////////////////////////////////////////////////////////////
//
// MBPDataImageOneByte.cxx
//
//
// The MBPDataImageOneByte class wraps an double-array, which represents a 
// backprojected image.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPDataImageOneByte.h"

// Standard libs:
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MBPDataImageOneByte)
#endif


////////////////////////////////////////////////////////////////////////////////


MBPDataImageOneByte::MBPDataImageOneByte() : MBPData()
{
  // standard constructor

  m_Type = c_ImageOneBit;
  m_Image = 0;
}


////////////////////////////////////////////////////////////////////////////////


MBPDataImageOneByte::~MBPDataImageOneByte()
{
  // Destruct an MBPDataImageOneByte

  delete [] m_Image;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPDataImageOneByte::Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum)
{
  // Initalize an backprojected image
  //
  // Image: the image as array
  // NBins: number of bins of the image

  m_NBins = NBins;
  m_Image = new(nothrow) unsigned char[m_NBins];

  if (m_Image == 0) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  m_Maximum = Maximum;

  if (m_Maximum <= 0) {
    cout<<"Storing images: The maximum is zero!"<<endl;
  }
  
  // Initilaize with zero:
  unsigned char Zero = (unsigned char) (0.0);
  for (int i = 0; i < m_NBins; ++i) {
    m_Image[i] = Zero;
  }

  // Then save the sparse values:
  for (int i = 0; i < NUsedBins; ++i) {
    m_Image[Bins[i]] = (unsigned char) ((Image[i]*255)/m_Maximum);
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImageOneByte::Deconvolve(double* Expectation, double* InvYnew, int Event)
{
  // Do a deconvolution - attention the InvYnew is the inverted Yi
      
  double Scaler = m_Maximum / 255;
  for (int bin = 0; bin < m_NBins; bin++) {
    Expectation[bin] += Scaler * m_Image[bin] * InvYnew[Event];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImageOneByte::Convolve(double* Ynew, int Event, double* Image, int NBins)
{
  // Do a convolution:

  double Sum = 0;
  double Scaler = m_Maximum / 255;
  for (int bin = 0; bin < m_NBins; bin++) {
    Sum += Scaler * m_Image[bin] * Image[bin];
  }
  Ynew[Event] = Sum;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImageOneByte::Sum(double* Image, int NBins)
{
  // Just sum it up, i.e. add the content to the image 

  for (int bin = 0; bin < m_NBins; bin++) {
    Image[bin] += m_Maximum / 255 * m_Image[bin];
  }
}


////////////////////////////////////////////////////////////////////////////////


int MBPDataImageOneByte::GetUsedBytes() const
{
  // Return the number of bytes used by this image
  
  int Bytes = 0;

  Bytes += MBPData::GetUsedBytes();
  Bytes += sizeof(float); // m_Maximum
  Bytes += sizeof(void*); // Pointer to m_Image
  Bytes += m_NBins*sizeof(unsigned char); // m_Image
  Bytes += sizeof(int); // m_NBins

  return Bytes;
}


// MBPDataImageOneByte: the end...
////////////////////////////////////////////////////////////////////////////////
