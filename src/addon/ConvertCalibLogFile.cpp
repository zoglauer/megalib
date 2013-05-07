#include <TROOT.h>
#include <sstream>
#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::ofstream;
using std::ifstream;
#include <vector>
using std::vector;
#include <iomanip>
#include <cstdlib>
#include <limits>
using namespace std;

#include "MCsIPixelSingle.h"

int main(int argc, char** argv) 
{
	if (argc != 3) {
		cout << "Wrong number of arguments. " << endl
				 << "Usage: " << argv[0] << " <input> <output>" << endl;
		exit(1);
	}

	ifstream lin;
	// Open input log file
	cout << "Opening log file \"" << argv[1] << "\" for input" << endl; 
	lin.open(argv[1]);
	if (!lin.is_open()) {
		cout << "Cannot open input log file! Exiting ..." << endl;
		exit(1);
	}
	
	// Check, if output file already exists and open output log file
	cout << "Opening output log file \"" << argv[2] << "\"" << endl;

	FILE *File;
	MString text;
  if (NULL != (File = fopen(argv[2], "r"))) {
		fclose(File);
		File = NULL;
		cout << "File \"" << argv[2] << "\" already exists. ";
		for (;text.Data()[0]!='o';) {
			cout << "Overwrite or exit (o/e)?" << endl;
			cin >> text;
			cin.clear();
			cin.ignore(numeric_limits<std::streamsize>::max(), '\n');
			if(text.Data()[0]=='e') {
				cout << "Exiting ..." << endl;
				lin.close();
				exit(1);
			}
		}
	}
  ofstream lout(argv[2]);
	if (!lout) {
		cout << "Cannot open file \"" << argv[2] 
				 << "\" for output! Exiting ..." << endl;
		lin.close();
		exit(1);
	}

	string InputLine;

	for(;!lin.eof();) {
		getline(lin, InputLine);

		if ((InputLine.c_str()[0] == 'N') && (InputLine.c_str()[1] == 'C')) {
			istringstream inputstream(InputLine);
			string dummy;
			int LineNo = -1;
			inputstream >> dummy >> LineNo;
			if (!inputstream.bad() && !inputstream.fail()) {
				lout << "NC " << LineNo << " 0 1000 0 1" << endl;
			} else {
				cout << "Error interpreting NC line" << endl;
			}
		
	
// 		// read background problem
// 		} else if ((InputLine.c_str()[0] == 'B') && (InputLine.c_str()[1] == 'P')) {
// 			istringstream inputstream(InputLine);
// 			string dummy;
// 			int LineNo = -1;
// 			double CalcBk = -1;
// 			double FitBk = -1;
// 			double CalcOverflow = -1;
// 			double FitOverflow = -1;
// 			inputstream >> dummy >> LineNo >> CalcBk >> FitBk >> CalcOverflow 
// 									>> FitOverflow;
// 			if (!inputstream.bad() && !inputstream.fail()) {
// 				lout << "OB " << LineNo << " " << FitBk << " 2 " << FitOverflow
// 						 << " 4 0 1000 0 1" << endl;
// 			} else {
// 				cout << "Error reading BP line from log file" << endl;
// 			}
// 		}

		// read fit of double peak spectrum
		} else if ((InputLine.c_str()[0] == 'F')&&(InputLine.c_str()[1] == 'A')) {
			istringstream inputstream(InputLine);
			string dummy;
			double Par[19];
			for (UInt_t i=0; i<19; ++i) Par[i] = -1;
			double rangelow = -1;
			double rangehigh = -1;
			double chi2 = -1;
			bool fitbad = false;
			inputstream >> dummy; 
			for (UInt_t j=0; j<19; ++j) {
				inputstream >> Par[j];
			} 
			inputstream >> rangelow >> rangehigh >> chi2 >> fitbad;
 					
			if (!inputstream.bad() && !inputstream.fail()) {
				lout << dummy << " 0 ";
				for (UInt_t i=0; i<19; ++i) {
					lout <<  Par[i]  << " ";
				} 
				lout << rangelow << " " << rangehigh << " " << chi2 << " " 
						 << fitbad << endl;
			} else {
				cout << "Error reading FA line from log file" << endl;
			}
		} else {
			lout << InputLine << endl;
		}
	}

	lout.close();
	lin.close();
}

