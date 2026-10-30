/*
 * MFileDecay.h
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


#ifdef ___CLING___
 public:
  ClassDef(MFileDecay, 0) // no description
#endif

};

#endif


////////////////////////////////////////////////////////////////////////////////
