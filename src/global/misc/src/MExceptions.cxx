/*
 * MExceptions.cxx
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
// MExceptions
//
////////////////////////////////////////////////////////////////////////////////


// Include the header:
#include "MExceptions.h"

// Standard libs:

// ROOT libs:

// MEGAlib libs:


////////////////////////////////////////////////////////////////////////////////


#ifdef ___CLING___
ClassImp(MException)
ClassImp(MExceptionTestFailed)
ClassImp(MExceptionParameterOutOfRange)
ClassImp(MExceptionIndexOutOfBounds)
ClassImp(MExceptionValueNotFound)
ClassImp(MExceptionEmptyArray)
ClassImp(MExceptionValueOutOfBounds)
ClassImp(MExceptionDivisionByZero)
ClassImp(MExceptionNumberNotFinite)
ClassImp(MExceptionObjectDoesNotExist)
ClassImp(MExceptionObjectsNotIdentical)
ClassImp(MExceptionUnknownMode)
ClassImp(MExceptionNeverReachThatLineOfCode)
ClassImp(MExceptionArbitrary)
#endif

//! Assign static variable
bool MException::m_Abort = false;


// MExceptions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
