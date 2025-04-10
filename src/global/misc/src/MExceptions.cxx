/*
 * MExceptions.cxx
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
ClassImp(MExceptionEmptyArray)
ClassImp(MExceptionValueOutOfBounds)
ClassImp(MExceptionDivisionByZero)
ClassImp(MExceptionNumberNotFinite)
ClassImp(MExceptionObjectDoesNotExist)
ClassImp(MExceptionObjectsNotIdentical)
ClassImp(MExceptionUnknownMode)
ClassImp(MExceptionNeverReachThatLineOfCode)
ClassImp(MExceptionPointerIsInvalid)
ClassImp(MExceptionArbitrary)
#endif

//! Assign static variable
bool MException::m_Abort = false;


// MExceptions.cxx: the end...
////////////////////////////////////////////////////////////////////////////////
