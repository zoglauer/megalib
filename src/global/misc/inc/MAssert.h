/******************************************************************************
 *                                                                            *
 * MAssert.h    Version 1.0                                                  *
 *                                                                            *
 * Copyright (C) by Andreas Zoglauer.                               *
 * All rights reserved.                                                       *
 *                                                                            *
 * Please see the file Licence.txt for further copyright information.         *
 *                                                                            *
 ******************************************************************************/

#ifndef ___Assert___
#define ___Assert___

// Dame classes:

// External classes:

/******************************************************************************/


//namespace Zogy {

  // Print an "Assertion failed" message and aborts -
  // but only when NDEBUG is not defined!
  void AssertionFailed(const char* assertion, const char* file,
                       unsigned int line, const char* function);

#ifdef NDEBUG

#define massert(expr) ;

#else // Not N(o)DEBUG, i.e. debugging activated!

#define	__ZDSTRING(x)	#x

#define massert(expr) \
  (static_cast<void> ((expr) ? 0 : \
		       (AssertionFailed (__ZDSTRING(expr), __FILE__, __LINE__, \
				       __ZDASSERT_FUNCTION), 0)))

// Version 2.4 and later of GCC define a magical variable `__PRETTY_FUNCTION__'
// which contains the name of the function currently being defined.
// This is broken in G++ before version 2.6.
// C9x has a similar variable called __func__, but prefer the GCC one since
// it demangles C++ function names.
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __ZDASSERT_FUNCTION	__func__
#  else
#   define __ZDASSERT_FUNCTION	((const char *) 0)
#  endif

#endif // NDEBUG

//}; // Zogy

#endif

/*
 * MAssert end...
 ******************************************************************************/
