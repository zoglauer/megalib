/*
 * MBPDataImageOneByte.h
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


#ifndef __MBPDataImageOneByte__
#define __MBPDataImageOneByte__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MBPData.h"


////////////////////////////////////////////////////////////////////////////////


class MBPDataImageOneByte : public MBPData
{
  // Public Interface:
 public:
  //! Default constructor
  MBPDataImageOneByte();
  //! Default destructor
  ~MBPDataImageOneByte();

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

  //! Return the number of used bins
  virtual int GetUsedBins() const { return m_NBins; }


  // private members:
 private:
  // Remember: If you change something you have to add it to the GetUsedBytes-function!

  //! The maximum image value
  float m_Maximum;
  //! The relative values of the image pixel
  unsigned char* m_Image;
  //! The number of image bins
  int m_NBins;


#ifdef ___CLING___
 public:
  ClassDef(MBPDataImageOneByte, 0) // a backprojected image
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
