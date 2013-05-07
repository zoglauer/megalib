/* Compile with: g++ -o TriggerPattern TriggerPattern1_1.cpp    */

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
  for (unsigned int p = 0; p < Pattern.size(); p += 8) {
    fout<<ToHex(Pattern[p+4], Pattern[p+5], Pattern[p+6], Pattern[p+7]);
    fout<<ToHex(Pattern[p],   Pattern[p+1], Pattern[p+2], Pattern[p+3]);
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

  string PatternName = "NoName.pat";

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
  Type[0] = D1;   // odysseus
  Type[1] = D1;   // merkur
  Type[2] = D1;   // hermes
  Type[3] = D1;   // n.c.
  Type[4] = D1;   // poseidon
  Type[5] = D1;   // sisyphus
  Type[6] = D1;   // achilles
  Type[7] = D1;   // ikarus
  Type[8] = D1;   // bachus
  Type[9] = D1;   // apoll
  Type[10] = D1;  // neptun
  Type[11] = D1;  // vulkan
  Type[12] = D2;  // 
  Type[13] = D2;  // 
  Type[14] = D2;  // 
  Type[15] = D2;  // 
  Type[16] = D2;  // 
  Type[17] = D2;  //
  Type[18] = D2;  //
  Type[19] = D2;  //
  Type[20] = ACS; //

	vector<unsigned int> D1Sequence(12, 0);
	D1Sequence[0] = 2; // hermes
	D1Sequence[1] = 8; // bachus
	D1Sequence[2] = 10; // neptun
	D1Sequence[3] = 4; // poseideon
	D1Sequence[4] = 6; // achilles
	D1Sequence[5] = 1; // merkur
	D1Sequence[6] = 0; // odysseus
	D1Sequence[7] = 9; // apoll
	D1Sequence[8] = 7; // ikarus
	D1Sequence[9] = 5; // sisyphus
	D1Sequence[10] = 11; // vulkan
	D1Sequence[11] = 3; // tbd.
  const int MaxD1Position = 11;

	vector<unsigned int> D1Positions(12, 0);
	D1Positions[2] = 0; // hermes
	D1Positions[8] = 1; // bachus
	D1Positions[10] = 2; // neptun
	D1Positions[4] = 3; // poseideon
	D1Positions[6] = 4; // achilles
	D1Positions[1] = 5; // merkur
	D1Positions[0] = 6; // odysseus
	D1Positions[9] = 7; // apoll
	D1Positions[7] = 8; // ikarus
	D1Positions[5] = 9; // sisyphus
	D1Positions[11] = 10; // vulkan
	D1Positions[3] = 11; // tbd.


  cout<<"Generating pattern..."<<endl;

	/*
    PatternName = "Pattern_BachusAphrodite.pat";
  // Simple pattern: D1: exactly one D1 (bachus) and one D2 (?)
  int ND1 = 0; // Number of hits D1
  int ND2 = 0; // Number of hits D2
  int LongestD1Sequence = 0;
  int LongestInterruptedD1Sequence = 0;
  for (unsigned int p = 0; p < PatternMax; p++) {
		if ((p & Powers[0]) == 0 && 
				(p & Powers[1]) == 0 && 
				(p & Powers[2]) == 0 && 
				(p & Powers[3]) == 0 && 
				(p & Powers[4]) == 0 && 
				(p & Powers[5]) == 0 && 
				(p & Powers[6]) == 0 && 
				(p & Powers[7]) == 0 && 
				(p & Powers[8]) != 0 && 
				(p & Powers[9]) == 0 && 
				(p & Powers[10]) == 0 && 
				(p & Powers[11]) == 0 && 
				(p & Powers[12]) == 0 && 
				(p & Powers[13]) == 0 && 
				(p & Powers[14]) == 0 && 
				(p & Powers[15]) == 0 && 
				(p & Powers[16]) == 0 && 
				(p & Powers[17]) == 0 && 
				(p & Powers[18]) != 0 &&
				(p & Powers[19]) == 0 && 
				(p & Powers[20]) == 0 && 
				(p & Powers[21]) == 0) {
			cout<<ToBinaryString(p)<<endl; 
			Pattern[p] = 1;
		} else {
			Pattern[p] = 0;
		}
  }
	*/

 
	// Simple pattern: D1: x+ & D2: y+ (e.g. 1+ 1+ = "MEGA"-Comptel mode)
  PatternName = "Pattern_1p_1p_nohermes.pat";  // <-- Modify here
  const int MinHitsD1 = 1;    // <-- Modify here
  const int MinHitsD2 = 1;    // <-- Modify here
  int ND1 = 0; // Number of hits D1
  int ND2 = 0; // Number of hits D2
  for (unsigned int p = 0; p < PatternMax; p++) {
    // a. Check for enough hits
    ND1 = ND2 = 0;
    bool IsHermes = false;
    for (unsigned int b = 0; b < Max; b++) {
      if ((p & Powers[b]) != 0) {
        if (b == 2) IsHermes = true;
        if (Type[b] == D1) {
          ND1++;
        } else if (Type[b] == D2) {
          ND2++;
        }
      }
    }
    // Condition accepted
    if (ND1 >= MinHitsD1 && ND2 >= MinHitsD2 && IsHermes == false) {
			Pattern[p] = 1;
    } 
    // Condition rejected
    else {
      Pattern[p] = 0;
    }
  }
  

  /*
	// Simple pattern: D1: 2s+ (== Minimum Track!) & D2: 1+ 
	PatternName = "Pattern_2sp_1p.pat";
	int ND1 = 0; // Number of hits D1
	int ND2 = 0; // Number of hits D2
	int LongestD1Sequence = 0;
	int LongestInterruptedD1Sequence = 0;
	for (unsigned int p = 0; p < PatternMax; p++) {
		//cout<<ToBinary(p)<<" -> ";
		// a. Check for enough hits
		ND1 = ND2 = 0;
		for (unsigned int b = 0; b < Max; b++) {
			if ((p & Powers[b]) != 0) {
				if (Type[b] == D1) {
					ND1++;
				} else if (Type[b] == D2) {
					ND2++;
				}
			}
		}
		// Condition accepted
		if (ND1 >= 2 && ND2 >= 1) {
			// Test if we have a track:
			int Position;
			bool HasTrack = false;
			for (unsigned int b = 0; b < Max; b++) {
				if ((p & Powers[b]) != 0) {
					if (Type[b] == D1) {
						// Check if above or below, we have another hit:
						Position = D1Positions[b];
						if (Position > 0) {
							if (p & Powers[D1Sequence[Position-1]]) {
								HasTrack = true;
							}
						}
						if (Position < MaxD1Position) {
							if (p & Powers[D1Sequence[Position+1]]) {
								HasTrack = true;
							}
						}
					}
				}			
			}
      if (HasTrack == true) {
        Pattern[p] = 1;
      } else {
        Pattern[p] = 0;        
      }
		} 
		// Condition rejected
		else {
			Pattern[p] = 0;
		}
	}
  */	


  /*
	// A little more complex: D1: 2s+ (== Minimum Track!) & D2: 1+ OR D1: 1+ & D2: 2+ 
	PatternName = "Pattern_4p_0p_or_1p_1p.pat";
	int ND1 = 0; // Number of hits D1
	int ND2 = 0; // Number of hits D2
	int LongestD1Sequence = 0;
	int LongestInterruptedD1Sequence = 0;
	for (unsigned int p = 0; p < PatternMax; p++) {
		//cout<<ToBinary(p)<<" -> ";
		// a. Check for enough hits
		ND1 = ND2 = 0;
		for (unsigned int b = 0; b < Max; b++) {
			if ((p & Powers[b]) != 0) {
				if (Type[b] == D1) {
					ND1++;
				} else if (Type[b] == D2) {
					ND2++;
				}
			}
		}
		// Condition accepted
		if (ND1 >= 4 && ND2 >= 0) {
      Pattern[p] = 1;
		} else if (ND1 >= 1 && ND2 >= 1) {
      Pattern[p] = 1;
    }
		// Condition rejected
		else {
			Pattern[p] = 0;
		}
	}	
  */

  /*
	// Long tracks: D1: 3outof4+ & D2: 1+  
	PatternName = "Pattern_3outof4p_1p.pat";
  const int MaxInterrupt = 1;
  const int MinLength = 3;
	int ND1 = 0; // Number of hits D1
	int ND2 = 0; // Number of hits D2
	for (unsigned int p = 0; p < PatternMax; p++) {
		//cout<<ToBinary(p)<<" -> ";
		// a. Check for enough hits
		ND1 = ND2 = 0;
		for (unsigned int b = 0; b < Max; b++) {
			if ((p & Powers[b]) != 0) {
				if (Type[b] == D1) {
					ND1++;
				} else if (Type[b] == D2) {
					ND2++;
				}
			}
		}
		// Condition accepted
		if (ND1 >= MinLength && ND2 >= 1) {
			// Test if we have a track:
			int Position;
			bool HasTrack = false;

      // Check tracker for sequence:
      int LongestInterrupt = 0;
      int LongestSequence = 0;
			for (unsigned int pos = 0; pos <= MaxD1Position; ++pos) {
				if ((p & Powers[D1Sequence[pos]]) != 0) {
          LongestSequence++;
          LongestInterrupt = 0;
        } else {
          LongestInterrupt++;
          if (LongestInterrupt > MaxInterrupt) {
            LongestSequence = 0;
          }
        }
      }

      if (LongestSequence >= MinLength) {
        Pattern[p] = 1;
      } else {
        Pattern[p] = 0;        
      }
		} 
		// Condition rejected
		else {
			Pattern[p] = 0;
		}
	}	
  */

  cout<<"Writing pattern..."<<endl;

  // Write it to file:
  WritePattern(PatternName, Pattern);

  cout<<"Finished!"<<endl;

  return;
}

int main()
{
  CreatePattern();

  return 0;
}
