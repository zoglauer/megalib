/*
 * MBPDataImage.cxx
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
// MBPDataImage.cxx
//
//
// The MBPDataImage class wraps an double-array, which represents a 
// backprojected image.
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MBPDataImage.h"

// Standard libs:
#include <iostream>
using namespace std;

// MEGAlib:
#include "MStreams.h"


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CINT___
ClassImp(MBPDataImage)
#endif


////////////////////////////////////////////////////////////////////////////////


MBPDataImage::MBPDataImage() : MBPData()
{
  // standard constructor

  m_Type = c_Image;
  m_NBins = 0;
  m_Image = 0;
}



////////////////////////////////////////////////////////////////////////////////


MBPDataImage::~MBPDataImage()
{
  // Destruct an MBPDataImage

  delete [] m_Image;
}


////////////////////////////////////////////////////////////////////////////////


bool MBPDataImage::Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum)
{
  // Initalize an backprojected image
  //
  // Image: the image as array
  // NBins: number of bins of the image

  m_NBins = NBins;
  m_Image = new(nothrow) float[m_NBins];

  if (m_Image == 0) {
    cout<<"Storing images: Out of memory"<<endl;
    return false;
  }

  if (NUsedBins != NBins) {
    for (int i = 0; i < m_NBins; ++i) {
      m_Image[i] = 0.0f;
    }    
  }

  for (int i = 0; i < NUsedBins; ++i) {
    m_Image[Bins[i]] = (float) Image[i];
  }

  return true;
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImage::Deconvolve(double* Ej, double* InvYi, int Event)
{
  // Do a deconvolution - attention the InvYnew is the inverted Yi
      
  for (int bin = 0; bin < m_NBins; ++bin) {
    Ej[bin] += m_Image[bin] * InvYi[Event];
  }
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImage::Convolve(double* Yi, int Event, double* Lj, int NBins)
{
  // Do a convolution:

  // ---------> time critical --------->

  double Sum = 0;
  for (int bin = 0; bin < m_NBins; ++bin) {
    Sum += m_Image[bin] * Lj[bin];
  }

  Yi[Event] = Sum;

  // <--------- time critical <---------
}


////////////////////////////////////////////////////////////////////////////////


void MBPDataImage::Sum(double* Image, int NBins)
{
  // Just sum it up, i.e. add the content to the image 
      
  for (int bin = 0; bin < m_NBins; bin++) {
    Image[bin] += m_Image[bin];
  }
}


////////////////////////////////////////////////////////////////////////////////


int MBPDataImage::GetUsedBytes() const
{
  // Return the number of bytes used by this image
  
  int Bytes = 0;

  Bytes += MBPData::GetUsedBytes();
  Bytes += sizeof(void*); // Pointer to m_Image
  Bytes += m_NBins*sizeof(float); // m_Image
  Bytes += sizeof(int); // m_NBins

  return Bytes;
}


// MBPDataImage: the end...
////////////////////////////////////////////////////////////////////////////////
