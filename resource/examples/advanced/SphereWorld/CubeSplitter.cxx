#include "Base.hxx"

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
  
  for (auto s: FileNames) {
    ifstream in;
    in.open(s);
   
    
    
    in.close();
  }
  
  
  return 0;
}


