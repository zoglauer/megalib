/*
 * MFileDecay.h
 *
 * Copyright (C) by Andreas Zoglauer.
 * All rights reserved.
 *
 * Please see the source-file for the copyright-notice.
 *
 */


#ifndef __MFileDecay__
#define __MFileDecay__


////////////////////////////////////////////////////////////////////////////////


// ROOT libs:
#include <MString.h>

// MEGAlib libs:
#include "MGlobal.h"
#include "MParser.h"
#include "MTokenizer.h"

// Standard libs:
#include <vector>
#include <sstream>
using namespace std;

// Forward declarations:
class MDecayMatrix;

////////////////////////////////////////////////////////////////////////////////


class MFileDecay : public MParser
{
  // public interface:
 public:
  explicit MFileDecay();
  virtual ~MFileDecay();

  virtual bool Open(MString FileName, unsigned int Way = 1);

  vector<double> GetEnergies() { return m_Energies; }
  vector<double> GetEnergyErrors() const { return m_EnergyErrors; }

  // protected methods:
 protected:

  // private methods:
 private:

  // protected members:
 protected:
  vector<double> m_Energies;
  vector<double> m_EnergyErrors;


#ifdef ___CINT___
 public:
  ClassDef(MFileDecay, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
