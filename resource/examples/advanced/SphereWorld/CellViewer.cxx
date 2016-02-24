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