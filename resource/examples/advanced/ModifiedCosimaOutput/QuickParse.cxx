/*
 * QuickParse.cxx
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


#include <fstream>
#include <cstdint>
#include <iostream>
using namespace std;

int main(int argc, char** argv) {
  //! File name
  string Name = "Dummy.bsim";
  if (argc == 2) Name = argv[1];
  
  struct Interaction {
    uint64_t ID;
    float X;
    float Y;
    float Z;
    float E;
  } I;
  
  ifstream In;
  In.open(Name.c_str(), ios::binary);
  
  uint64_t LastID = 0;
  while (In.good() == true) {
    In.read(reinterpret_cast<char *>(&I), sizeof(I));
    if (In.good() == true) {
      if (I.ID != LastID) {
        cout<<endl;
      }
      cout<<I.ID<<": ("<<I.X<<", "<<I.Y<<", "<<I.Z<<")";
      if (I.E > 0.0) {
        cout<<" with "<<I.E<<" keV of continuing photon"<<endl;
      } else {
        cout<<" stopped"<<endl; 
      }
      LastID = I.ID;
    }
  }
  
  return 0;
}