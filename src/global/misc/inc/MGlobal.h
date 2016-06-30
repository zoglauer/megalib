/*
 * MGlobal.h
 *
 * Copyright (C) by Robert Andritschke, Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#define   USEROOT
#define   USEGUI
// #define    HARDWARE  // defined in makefile
#undef    DEBUGGEO
#undef ___THREADS___

// the following defines are only important, if HARDWARE is defined 
//        ----------- Select ONE driver -------------- 
#ifdef HARDWARE

// #undef  VME_INTEL_CPU        // defined in makefile
// #undef  PCI_VME              // defined in makefile
// #define   SBUS_VME_NODRIVER  // defined in makefile
// #undef  SBUS_VME_DRIVER      // not tested!!
// #undef  PAR_HARD             // defined in makefile

#ifdef VME_INTEL_CPU
#undef  ___HTML2___
#undef  SBUS_HARDWARE
#endif

#ifdef PCI_VME
#undef  ___HTML2___
#undef  SBUS_HARDWARE
#endif

#ifdef SBUS_VME_NODRIVER 
// in this case, the VmeBus classes are included in the shared libs
#define  ___HTML2___
#define  SBUS_HARDWARE
#undef  PAR_HARD
#endif

#ifdef SBUS_VME_DRIVER 
// in this case, the VmeBus classes are included in the shared libs
#define  ___HTML2___
#define  SBUS_HARDWARE
#undef  PAR_HARD
#endif

#else

#undef VME_INTEL_CPU
#undef PCI_VME
#undef SBUS_VME_NODRIVER
#undef SBUS_VME_DRIVER
#undef SBUS_HARDWARE
#undef PAR_HARD
#endif

// the ___CINT___ tag  includes classes into the shared libraries
//#ifndef  ___CINT___
//#define ___CINT___
//#endif

#ifdef ___LINUX___
#define ___UNIX___
#endif

#ifdef ___MACOSX___
#define ___UNIX___
#endif



////////////////////////////////////////////////////////////////////////////////


#ifndef __MGlobal__
#define __MGlobal__

#include "TROOT.h"
#include "TSystem.h"
#include "TMutex.h"
#include "TVirtualMutex.h"

#include "MVector.h"
#include "MString.h"
#include "MRotation.h"

// For compatibility with Windows:
#ifdef _MSC_VER
# if _MSC_VER < 1201
#  define for if (0); else for
# endif
#endif


// Some globally defined variables
extern unsigned int g_MajorVersion;
extern unsigned int g_MinorVersion;
extern unsigned int g_Version;
extern MString g_VersionString;

extern const MString g_MEGAlibPath;

extern const MString g_CopyrightYear;
extern const MString g_Homepage;

extern const MString g_StringNotDefined;
extern const MVector g_VectorNotDefined;
extern const MRotation g_RotationNotDefined;
extern const int g_IntNotDefined;
extern const unsigned int g_UnsignedIntNotDefined;
extern const double g_DoubleNotDefined;
extern const float g_FloatNotDefined;

//! The global verbosity
extern int g_Verbosity;
// and its levels
extern const int c_Quiet;
extern const int c_Error;
extern const int c_Warning;
extern const int c_Info;
extern const int c_Chatty;
extern const int c_Extreme;

extern TMutex* g_Mutex;

// And here a lot of other useful constants:
extern const double c_Pi;
extern const double c_TwoPi;
extern const double c_Sqrt2Pi;        // 2.5066283
extern const double c_Rad;            // c_Pi / 180.0
extern const double c_Deg;            // 180.0 / c_Pi
extern const double c_SpeedOfLight;   //  
extern const double c_E0;             // rest mass electron  
extern const double c_FarAway;        // Radius for conversion from spherical to cartesian coordinates   
extern const double c_LargestEnergy;  // 
extern const MVector c_NullVector;

// A dummy class which takes care of initializations
class MGlobal
{
 public:
  MGlobal() {};
  static bool Initialize(MString ProgramName = "", MString ProgramDescription = ""); // Has to be called in main 

 private:
  static void CenterString(MString& String, unsigned int LineLength, bool DoBorders = true); 
  static void ShowIntro(MString ProgramName, MString ProgramDescription = ""); 
};

extern MGlobal g_Global;

#endif
