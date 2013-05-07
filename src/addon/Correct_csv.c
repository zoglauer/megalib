#include <stdio.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char** argv)
{
	const int StrLength = 255;
	char FileName[StrLength];
	char Text[StrLength];
	FILE *infile;
	FILE *outfile;
	int i;
	char Token[StrLength];
	int x,y,E;
	double ADC, Sigma;

	if (argc == 0) {
		printf("Wrong number of arguments!\n");
		exit(1);
	}
	if (argc != 2) {
		printf("Usage: \n");
		printf(" %s [FileName]\n", argv[0]);
		exit(1);
	}

	for (i=0; i<StrLength; ++i) FileName[i]=0;
	strcpy(FileName, argv[1]);
	printf("Open file: %s\n", FileName);
	infile = fopen(FileName, "r");
	if (infile == NULL) {
		printf("Error: Cannot open input file \"%s\"! Exiting...\n", FileName);
		exit(1);
	}

	strcat(FileName,".cor");
	printf("Open file: %s\n", FileName);
	outfile = fopen(FileName, "w");
	if (outfile == NULL) {
		printf("Error: Cannot open output file \"%s\"! Exiting...\n", FileName);
		fclose(infile);
		exit(1);
	}

	while(NULL != fgets(Text, StrLength, infile) ) {
		if ((Text[0]=='C') && (Text[1]=='P')) {
			sscanf(Text, "%s %i %i %i %lf %lf", Token, &x, &y, &E, &ADC, &Sigma);
			fprintf(outfile, "%s %i %i %i %.7f %.7f\n", Token, x, y, E, ADC-1, Sigma);
		} else {
			fprintf(outfile, "%s", Text);
		}
	}

	close(outfile);
	close(infile);
	return(0);
}

