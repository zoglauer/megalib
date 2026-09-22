/*
 * MResponseImagingCodedMask.h
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


#ifndef __MResponseImagingCodedMask__
#define __MResponseImagingCodedMask__


////////////////////////////////////////////////////////////////////////////////


// Standard libs:

// ROOT libs:

// MEGAlib libs:
#include "MGlobal.h"
#include "MResponseImaging.h"

// Forward declarations:


////////////////////////////////////////////////////////////////////////////////


class MResponseImagingCodedMask : public MResponseImaging
{
  // public interface:
 public:
  MResponseImagingCodedMask();
  virtual ~MResponseImagingCodedMask();

  //! Do all the response creation
  virtual bool CreateResponse();

  // protected methods:
 protected:
  //MResponseImagingCodedMask() {};
  //MResponseImagingCodedMask(const MResponseImagingCodedMask& ResponseImagingCodedMask) {};

  // private methods:
 private:



  // protected members:
 protected:


  // private members:
 private:



#ifdef ___CLING___
 public:
  ClassDef(MResponseImagingCodedMask, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
