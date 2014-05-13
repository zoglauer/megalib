/* 
 * Bundler.cxx
 *
 *
 * Copyright (C) by Michael Quinlan, Andreas Zoglauer.
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

  // Get the total number of processes master + slave(s)
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

