/*
 * bundler.cxx
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


// Standard
#include <iostream>
#include <fstream>
#include <limits>
#include <cstdlib>
using namespace std;

// Special
#include "mpi.h"


//! An MPI-aware executable that runs system calls provided in the input file.
//! Each instance (procID) of this executable, launched by MPI, will run the corresponding 
//! command on line procID of the input file.
//!
//! Usage : [user@host]$ mpirun -np 100 bundler inputfile.txt
int main(int argc, char** argv) {

  if (argc < 2) {
    cerr<<"Error You must provide an input file with the list of system commands."<<endl;
    return -1;
  }

  // Initialize MPI
  MPI_Init(&argc, &argv);

  // Get process rank
  int procID;
  MPI_Comm_rank(MPI_COMM_WORLD, &procID);

  // Get the total number of processes main + secondaries
  int nproc;
  MPI_Comm_size(MPI_COMM_WORLD, &nproc);

  procID++;

  char cmd_string[1000];
  ifstream infile;
  infile.open(argv[1]);
  if (infile.is_open()) {
    //cout<<"Process "<<procID<<" found the file "<< argv[1] <<endl;
    /*for(int i=0;i<=nproc;i++) {
      infile.getline(cmd_string,1000);
    }*/
    for (int i = 0; i < procID - 1; ++i) {
      infile.ignore(numeric_limits<streamsize>::max(),'\n');
    }
    infile.getline(cmd_string, 1000);
    cout<<"Process "<<procID<<" will execute: "<<cmd_string<<endl;
  } else {
    cerr<<"CHECK YOUR CONFIG... JOBSCRIPT not found"<<endl;
    return -1;
  }
  int Return = system(cmd_string);

  MPI_Finalize();
  
  return Return;
}

