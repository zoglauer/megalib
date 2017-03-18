/*
 * MBPData.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MBPData__
#define __MBPData__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


class MBPData
{
  // Public Interface:
 public:
  //! Default constructor
  MBPData();
  //! Default destructor
  virtual ~MBPData();

  //! Initialize the store --- returns false in case we are out of memory
  virtual bool Initialize(double* Image, int* Bins, int NBins, int NUsedBins, double Maximum) { return false; }

  //! Return the data type
  int BPDataType() { return m_Type; }

  //! Perform the list-mode deconvolution - attention the InvYnew is the inverted Yi
  virtual void Deconvolve(double* Expectation, double* InvYnew, int Event) {};
  //! Perform the list-mode convolution
  virtual void Convolve(double* Ynew, int Event, double* Image, int NBins) {};
  //! Just sum it up, i.e. add the content to the image
  virtual void Sum(double* Image, int NBins) {};

  //! Return the number of bytes used by this image
  virtual int GetUsedBytes() const;

  //! Return the number of used bins
  virtual int GetUsedBins() const { return 0; }

  // The data types:
  //! Data type is unknown
  static const int c_Unknown           = 0;
  //! Reponse splice as one complete 2D array of floats
  static const int c_Image             = 1;
  //! Reponse splice as one sparse 2D array of floats
  static const int c_ImageOneBit       = 2;
  //! Reponse splice as one complete 2D array of 8bit values plus maximum
  static const int c_SparseImage       = 3;
  //! Reponse splice as a sparse array of 8bit values plus maximum
  static const int c_SparseImageOneBit = 4;

  // protected members:
 protected:
  // Remember: If you change something you have to add it to the GetUsedBytes-function!

  //! Type of the response slice
  int m_Type;



#ifdef ___CINT___
 public:
  ClassDef(MBPData, 0) // Basic backprojection data type
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
