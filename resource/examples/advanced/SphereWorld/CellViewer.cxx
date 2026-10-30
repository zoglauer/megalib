/*
 * CellViewer.cxx
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


#include "Base.hxx"

#include <fstream>
#include <cstdint>
#include <iostream>
using namespace std;

// ROOT
#include <TROOT.h>
#include <TEnv.h>
#include <TSystem.h>
#include <TApplication.h>

int main(int argc, char** argv) 
{  
  TApplication StandAloneApp("StandAloneApp", 0, 0);
  
  MString FileName;
  double Energy;
  if (argc == 3) {
    FileName = argv[1];
    Energy = atof(argv[2]);
  } else {
    cout<<"You need to give a file name and an energy in keV"<<endl;
    return 1;
  }

  DataCell Cell;
  Cell.Load(FileName);
  
  Cell.View(Energy);
  
  StandAloneApp.Run();  
  
  return 0;
}