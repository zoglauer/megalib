/*
 * MAssert.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */

#ifndef __MAssert__
#define __MAssert__


////////////////////////////////////////////////////////////////////////////////


// MEGAlib libs:
#include "MGlobal.h"


////////////////////////////////////////////////////////////////////////////////


//! A small assertion helper
class MAssert
{
  // public interface:
 public:
  //! Default constructor
  MAssert();
  //! Default destuctor 
  virtual ~MAssert();

  //! Print an "Assertion failed" message and abort -
  //! but only when NDEBUG is not defined!
  [[noreturn]] static void AssertionFailed(const char* Assertion, const char* File, unsigned int Line, const char* Function);

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:

  // private members:
 private:

#ifdef ___CLING___
 public:
  ClassDef(MAssert, 1)
#endif

};


////////////////////////////////////////////////////////////////////////////////


#ifdef NDEBUG

// Use a true no-op expression so massert(x); stays valid in if/else chains
// without braces and does not introduce a stray semicolon at the call site.
#define massert(expr) ((void)0)

#else // Not N(o)DEBUG, i.e. debugging activated!

#define MASSERT_STRING(x) #x

#define massert(expr) \
  (static_cast<void> ((expr) ? 0 : \
           (MAssert::AssertionFailed (MASSERT_STRING(expr), __FILE__, __LINE__, \
               MASSERT_FUNCTION), nullptr)))

// Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
// which contains the name of the function currently being defined.
// This is broken in G++ before version 2.6.
// C9x has a similar variable called __func__, but prefer the GCC one since
// it demangles C++ function names.
#  if defined __cplusplus
#   define MASSERT_FUNCTION  __PRETTY_FUNCTION__
#  elif defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define MASSERT_FUNCTION  __func__
#  else
#   define MASSERT_FUNCTION  ((const char *) nullptr)
#  endif

#endif // NDEBUG

#endif


// MAssert.h: the end...
////////////////////////////////////////////////////////////////////////////////
