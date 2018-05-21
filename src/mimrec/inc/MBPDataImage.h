/*
 * MBPDataImage.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBPDataImage__
#define __MBPDataImage__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBPData.h"


////////////////////////////////////////////////////////////////////////////////


class MBPDataImage : public MBPData
{
  // Public Interface:
 public:
  //! Default constructor
  MBPDataImage();
  //! Default destructor
  ~MBPDataImage();

  //! Initialize the store --- returns false in case we are out of memory
  bool Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum);

  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi
  void Deconvolve(double* Expectation, double* InvYnew, int Event);
  //! Perform the list-mode convolution
  void Convolve(double* Ynew, int Event, double* Image, int NBins);
  //! Just sum it up, i.e. add the content to the image
  void Sum(double* Image, int NBins);

  //! Return the number of bytes used by this image
  virtual int GetUsedBytes() const;

  //! Return the number of used bins
  virtual int GetUsedBins() const { return m_NBins; }

  // private members:
 private:
  // Remember: If you change something you have to add it to the GetUsedBytes-function!

  // number of bins of image-array
  int m_NBins;
  // the image as array
  float* m_Image;



#ifdef ___CLING___
 public:
  ClassDef(MBPDataImage, 0) // a backprojected image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
