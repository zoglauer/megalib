/*
 * MBPDataSparseImage.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBPDataSparseImage__
#define __MBPDataSparseImage__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBPData.h"


////////////////////////////////////////////////////////////////////////////////


class MBPDataSparseImage : public MBPData
{
  // Public Interface:
 public:
  //! Default constructor
  MBPDataSparseImage();
  //! Default destructor
  ~MBPDataSparseImage();

  //! Initialize the store --- returns false in case we are out of memory
  //! This has to be the first function to be called
  //! For performance reasons, there are NO checks later if the arrays are initialized!
  virtual bool Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum);

  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi
  void Deconvolve(double* Expectation, double* InvYnew, int Event);
  //! Perform the list-mode convolution
  void Convolve(double* Ynew, int Event, double* Image, int NBins);
  //! Just sum it up, i.e. add the content to the image 
  void Sum(double* Image, int NBins);

  //! Return the number of bytes used by this image
  virtual int GetUsedBytes() const;

  // private members:
 private:
  // Remember: If you change something you have to add it to the GetUsedBytes-function!

  //! The relative values of the image pixel
  float* m_Data;  
  //! The belonging indices
  int* m_Index; 
  //! The number of stored image pixels
  int m_NEntries; 
  //! The number of image bins
  int m_NBins;


#ifdef ___CINT___
 public:
  ClassDef(MBPDataSparseImage, 0) // a backprojected event stored as sparse matrix
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
