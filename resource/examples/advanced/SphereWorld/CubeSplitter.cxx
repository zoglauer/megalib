#include "Base.hxx"

#include "MString.h"
#include "MFile.h"
#include "MTimer.h"

#include <fstream>
#include <cstdint>
#include <iostream>
using namespace std;


int main(int argc, char** argv) {
  
  vector<string> FileNames;
  
  if (argc > 1) {
    FileNames.assign(argv + 1, argv + argc);
  }

  for (auto s: FileNames) {
    cout<<s<<endl; 
  }
  cout<<" --> "<<FileNames.size()<<" files."<<endl;
  
  for (auto FileName: FileNames) {    
    MFile Parser;

    mout<<"Reading "<<FileName<<endl;
    if (Parser.Open(FileName, MFile::c_Read) == false) {
      mout<<"Unable to open file "<<FileName<<endl;
      return false;
    }

    mout<<"Splitting "<<FileName<<endl;
    MTimer Timer;
    unsigned int SubFiles = 0;
    MFile DataCell;
    MString Line;
    while (Parser.ReadLine(Line) == true) {
      if (Line.BeginsWith("CubeID") == true) {
        if (DataCell.IsOpen() == true) {
          DataCell.Close(); 
          DataCell.Reset();
        }
        vector<MString> Tokens = Line.Tokenize(" ");
        
        MString Out = FileName;
        Out.ReplaceAll("DataCube", "DataCell");
        Out.ReplaceAll(".gz", "");
        Out.ReplaceAll(".dat", "");
        Out += "_Height_" + Tokens[1] + "_Distance_" + Tokens[2] + ".dat.gz";
        DataCell.Open(Out, MFile::c_Write);
        ++SubFiles;
      }
      Line += "\n";
      DataCell.Write(Line);
    
    }
    DataCell.Close();
    Parser.Close();
    
    cout<<"Data cube "<<FileName<<" was split into "<<SubFiles<<" data cells in "<<Timer.GetElapsed()<<" seconds"<<endl;
  }
  
  return 0;
}


