/* Compile with: g++ -o TriggerPattern TriggerPattern.cpp    */

#include <cmath>
using std::pow;

#include <iostream>
using std::cout;
using std::endl;

#include <vector>
using std::vector;

#include <fstream>
using std::fstream;

#include <string>
using std::string;

#include <limits.h>


char ToHex(const unsigned int Bit0, const unsigned int Bit1, const unsigned int Bit2, const unsigned int Bit3)
{
  int N = 0;

  if (Bit0 != 0) N += 1;
  if (Bit1 != 0) N += 2;
  if (Bit2 != 0) N += 4;
  if (Bit3 != 0) N += 8;

  char Hex = '0';
  switch (N) {
  case  0: Hex = '0'; break;
  case  1: Hex = '1'; break;
  case  2: Hex = '2'; break;
  case  3: Hex = '3'; break;
  case  4: Hex = '4'; break;
  case  5: Hex = '5'; break;
  case  6: Hex = '6'; break;
  case  7: Hex = '7'; break;
  case  8: Hex = '8'; break;
  case  9: Hex = '9'; break;
  case 10: Hex = 'A'; break;
  case 11: Hex = 'B'; break;
  case 12: Hex = 'C'; break;
  case 13: Hex = 'D'; break;
  case 14: Hex = 'E'; break;
  case 15: Hex = 'F'; break;
  default: break;
  }

  return Hex; 
}


void WritePattern(const string Name, const vector<unsigned int>& Pattern)
{
  fstream fout(Name.c_str(), ios::out);
//   for (unsigned int p = 0; p < Pattern.size(); p++) {
//     fout<<Pattern[p];
//   }
  for (unsigned int p = 0; p < Pattern.size(); p += 4) {
    fout<<ToHex(Pattern[p], Pattern[p+1], Pattern[p+2], Pattern[p+3]);
  }
  fout<<endl;
  fout.close();
} 

string ToBinaryString(unsigned int field) 
{
  const unsigned int max_bits = 31;  // Maximum number of bits we can cope with
  unsigned int bits = max_bits;      // Current bit we look at
  unsigned int cutter = (unsigned int) pow(2, max_bits); // highest bit position as int

  string BinString(max_bits+1, '0'); // The string, where the binary "number" is stored

  // As long as we have not reached the last bit
  while (cutter >= 1) {
    // If the number is larger than the bit position we curently look at
    if (field >= cutter) {
     field -= cutter;
     BinString[max_bits-bits] = '1';
    }
    cutter /= 2;
    bits--;
  }

  return BinString;
}


void CreatePattern() 
{
  // Detector description:
  // D1_1 ... D1_12 Boden1 Boden2 Boden3 Boden4 Seit1

  const unsigned int D1 = 1;
  const unsigned int D2 = 2;
  const unsigned int ACS = 3;

  const unsigned int D1Max = 12;
  const unsigned int D2Max = 8;
  const unsigned int ACSMax = 1;
  const unsigned int Max = D1Max + D2Max + ACSMax;

  
  const unsigned int PatternMax = (unsigned int) pow(2, Max);
  const unsigned int InputMax = (unsigned int) pow(2, 18);
  vector<unsigned int> Pattern(PatternMax, 0);

   vector<unsigned int> Powers(Max);
  for (unsigned int d = 0; d < Max; ++d) {
    Powers[d] = (unsigned int) pow(2, d);
  }

  vector<unsigned int> Type(Max, D2);
  Type[0] = D1;   //
  Type[1] = D1;   //
  Type[2] = D1;   //
  Type[3] = D1;   //
  Type[4] = D1;   //
  Type[5] = D1;   //
  Type[6] = D1;   //
  Type[7] = D1;   //
  Type[8] = D1;   //
  Type[9] = D1;   //
  Type[10] = D1;  //
  Type[11] = D1;  //
  Type[12] = D2;  //
  Type[13] = D2;  //
  Type[14] = D2;  //
  Type[15] = D2;  //
  Type[16] = D2;  //
  Type[17] = ACS; //
  Type[18] = D2;  //
  Type[19] = D2;  //
  Type[20] = D2;  //


  cout<<"Generating pattern..."<<endl;

  // Simple pattern: D1: exactly one D1 and one D2
  int ND1 = 0; // Number of hits D1
  int ND2 = 0; // Number of hits D2
  int LongestD1Sequence = 0;
  int LongestInterruptedD1Sequence = 0;
  for (unsigned int p = 0; p < PatternMax; p++) {

  }

	/*
	// Simple pattern: D1: 1+ & D2: 1+ (Comptel mode)
  int ND1 = 0; // Number of hits D1
  int ND2 = 0; // Number of hits D2
  int LongestD1Sequence = 0;
  int LongestInterruptedD1Sequence = 0;
  for (unsigned int p = 0; p < PatternMax; p++) {
    //cout<<ToBinary(p)<<" -> ";
    // a. Check for enough hits
    ND1 = ND2 = 0;
    LongestD1Sequence = LongestInterruptedD1Sequence;
    for (unsigned int b = 0; b < Max; b++) {
      if ((p & Powers[b]) != 0) {
        if (Type[b] == D1) {
          ND1++;
        } else if (Type[b] == D2) {
          ND2++;TriggerPattern.cpp 
        }
      }
    }
    // Condition accepted
    // if (ND1 >= 1 && ND2 >= 1) {
    if (ND1 >= 4 && ND2 >= 1) {
    //if (ND1 >= 3) {
      // At least three hits should be in at least four neighbouring layers
			Pattern[p] = 1;
    } 
    // Condition rejected
    else {
      Pattern[p] = 0;
    }
  }
	*/
  cout<<"Grouping pattern..."<<endl;

  // Re-order the pattern:
  vector<unsigned int> OrderedPattern(PatternMax, 0);
  unsigned int PatternID = 0;
  for (int b = 7; b >= 0; b--) {
    for (unsigned int p = 0; p < InputMax; p++) {
      OrderedPattern[8*p + b] = Pattern[PatternID++];
    }
  }

  cout<<"Writing pattern..."<<endl;

  // Write it to file:
  WritePattern("Pattern_4p_1p.pat", OrderedPattern);

  cout<<"Finished!"<<endl;

  return;
}

int main()
{
  CreatePattern();

  return 0;
}
